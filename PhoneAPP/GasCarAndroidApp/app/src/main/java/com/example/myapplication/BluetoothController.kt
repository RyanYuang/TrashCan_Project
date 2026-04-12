package com.example.myapplication

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothSocket
import android.content.Context
import android.util.Log
import com.example.myapplication.spp.EnvCarSppProtocol
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.util.UUID

/**
 * 经典蓝牙 SPP：配对、连接、按行缓冲下行字节流，经 [EnvCarSppProtocol] 解析 `$STS:`；
 * 上行通过 [sendDownlinkPayload] 组帧（CRLF）。原始文本日志供界面「原始数据」区展示。
 */
@SuppressLint("MissingPermission")
class BluetoothController(private val context: Context) {

    private val bluetoothManager: BluetoothManager =
        context.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
    private val bluetoothAdapter: BluetoothAdapter? = bluetoothManager.adapter

    private val _pairedDevices = MutableStateFlow<List<BluetoothDevice>>(emptyList())
    val pairedDevices: StateFlow<List<BluetoothDevice>> = _pairedDevices

    private val _connectionState = MutableStateFlow("Disconnected")
    val connectionState: StateFlow<String> = _connectionState

    /** 累积的原始接收文本（按行追加），仅用于调试展示，不经业务解析。 */
    private val _rawReceivedLog = MutableStateFlow("")
    val rawReceivedLog: StateFlow<String> = _rawReceivedLog

    /** 最近一次成功解析的 `$STS:` 状态。 */
    private val _stsUplink = MutableStateFlow<EnvCarSppProtocol.StsUplink?>(null)
    val stsUplink: StateFlow<EnvCarSppProtocol.StsUplink?> = _stsUplink

    /** MCU 对阈值帧 `#O...,G...` 的应答摘要（`#OK:` / `#ERR:`），供界面提示。 */
    private val _thresholdFeedback = MutableStateFlow<String?>(null)
    val thresholdFeedback: StateFlow<String?> = _thresholdFeedback

    private var bluetoothSocket: BluetoothSocket? = null
    private var inputStream: InputStream? = null
    private var outputStream: OutputStream? = null

    private val sppUuid: UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")

    private val rxLineBuffer = StringBuilder(512)
    private val rawLog = StringBuilder(8192)
    private val rxLock = Any()

    companion object {
        private const val RAW_LOG_MAX = 24_000
    }

    fun getPairedDevices() {
        _pairedDevices.value = bluetoothAdapter?.bondedDevices?.toList() ?: emptyList()
    }

    fun connect(deviceAddress: String) {
        CoroutineScope(Dispatchers.IO).launch {
            val device = bluetoothAdapter?.getRemoteDevice(deviceAddress)
            if (device != null) {
                withContext(Dispatchers.Main) {
                    _connectionState.value = "Connecting to ${device.name ?: deviceAddress}"
                }
                try {
                    bluetoothSocket = device.createRfcommSocketToServiceRecord(sppUuid)
                    bluetoothSocket?.connect()
                    resetSessionBuffers()
                    withContext(Dispatchers.Main) {
                        _connectionState.value = "Connected to ${device.name ?: deviceAddress}"
                    }
                    inputStream = bluetoothSocket?.inputStream
                    outputStream = bluetoothSocket?.outputStream
                    startListening()
                } catch (e: IOException) {
                    withContext(Dispatchers.Main) {
                        _connectionState.value = "Connection failed: ${e.message}"
                    }
                    disconnect()
                }
            } else {
                withContext(Dispatchers.Main) {
                    _connectionState.value = "Device not found"
                }
            }
        }
    }

    private fun resetSessionBuffers() {
        synchronized(rxLock) {
            rxLineBuffer.clear()
        }
        rawLog.setLength(0)
        _rawReceivedLog.value = ""
        _stsUplink.value = null
        _thresholdFeedback.value = null
    }

    private fun startListening() {
        CoroutineScope(Dispatchers.IO).launch {
            val buffer = ByteArray(1024)
            while (true) {
                try {
                    val bytes = inputStream?.read(buffer) ?: -1
                    if (bytes <= 0) break
                    val chunk = String(buffer, 0, bytes, Charsets.UTF_8)
                    processIncomingChunk(chunk)
                } catch (e: IOException) {
                    Log.e("BluetoothController", "Input stream was disconnected", e)
                    break
                }
            }
            withContext(Dispatchers.Main) {
                if (_connectionState.value.startsWith("Connected")) {
                    _connectionState.value = "Disconnected"
                }
            }
        }
    }

    private fun processIncomingChunk(chunk: String) {
        val linesToParse = mutableListOf<String>()
        synchronized(rxLock) {
            rxLineBuffer.append(chunk)
            while (true) {
                val s = rxLineBuffer.toString()
                val nl = s.indexOf('\n')
                if (nl < 0) break
                val line = s.substring(0, nl)
                rxLineBuffer.delete(0, nl + 1)
                linesToParse.add(line)
            }
        }
        for (line in linesToParse) {
            appendRawLine(line)
            when (val msg = EnvCarSppProtocol.parseInboundLine(line)) {
                is EnvCarSppProtocol.InboundLine.Sts -> _stsUplink.value = msg.data
                EnvCarSppProtocol.InboundLine.ConfigOk ->
                    _thresholdFeedback.value = "阈值已写入 MCU（#OK:Config synced）"
                EnvCarSppProtocol.InboundLine.ConfigErrParse ->
                    _thresholdFeedback.value = "阈值格式错误（#ERR:Parse failed）"
                EnvCarSppProtocol.InboundLine.ConfigErrParams ->
                    _thresholdFeedback.value = "阈值未通过校验（#ERR:Invalid params）"
                else -> Unit
            }
        }
    }

    private fun appendRawLine(line: String) {
        rawLog.append(line).append('\n')
        while (rawLog.length > RAW_LOG_MAX) {
            val drop = rawLog.length - RAW_LOG_MAX + 4000
            rawLog.delete(0, drop.coerceAtLeast(1))
        }
        _rawReceivedLog.value = rawLog.toString()
    }

    /** 发送一行协议载荷（自动追加 CRLF，UTF-8）。 */
    fun sendDownlinkPayload(payload: String) {
        CoroutineScope(Dispatchers.IO).launch {
            try {
                val os = outputStream ?: return@launch
                os.write(EnvCarSppProtocol.encodeDownlink(payload))
                os.flush()
            } catch (e: IOException) {
                Log.e("BluetoothController", "Error sending data", e)
                disconnect()
            }
        }
    }

    fun disconnect() {
        try {
            inputStream?.close()
            outputStream?.close()
            bluetoothSocket?.close()
            _thresholdFeedback.value = null
            _connectionState.value = "Disconnected"
        } catch (e: IOException) {
            Log.e("BluetoothController", "Failed to disconnect", e)
            _connectionState.value = "Failed to disconnect: ${e.message}"
        }
    }
}
