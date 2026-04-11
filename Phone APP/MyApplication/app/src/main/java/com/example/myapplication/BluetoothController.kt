package com.example.myapplication

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothSocket
import android.content.Context
import android.util.Log
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

@SuppressLint("MissingPermission")
class BluetoothController(private val context: Context) {

    private val bluetoothManager: BluetoothManager =
        context.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
    private val bluetoothAdapter: BluetoothAdapter? = bluetoothManager.adapter

    private val _pairedDevices = MutableStateFlow<List<BluetoothDevice>>(emptyList())
    val pairedDevices: StateFlow<List<BluetoothDevice>> = _pairedDevices

    private val _connectionState = MutableStateFlow("Disconnected")
    val connectionState: StateFlow<String> = _connectionState

    private val _receivedData = MutableStateFlow("")
    val receivedData: StateFlow<String> = _receivedData

    private var bluetoothSocket: BluetoothSocket? = null
    private var inputStream: InputStream? = null
    private var outputStream: OutputStream? = null

    // Standard SPP UUID
    private val sppUuid: UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")

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
                    bluetoothSocket?.connect() // This is a blocking call
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

    private fun startListening() {
        CoroutineScope(Dispatchers.IO).launch {
            val buffer = ByteArray(1024)
            var bytes: Int
            while (true) {
                try {
                    bytes = inputStream?.read(buffer) ?: -1
                    if (bytes == -1) break
                    val receivedMessage = String(buffer, 0, bytes)
                    _receivedData.value = receivedMessage
                } catch (e: IOException) {
                    Log.e("BluetoothController", "Input stream was disconnected", e)
                    break
                }
            }
        }
    }

    fun sendData(data: String) {
        CoroutineScope(Dispatchers.IO).launch {
            try {
                outputStream?.write(data.toByteArray())
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
            _connectionState.value = "Disconnected"
        } catch (e: IOException) {
            Log.e("BluetoothController", "Failed to disconnect", e)
            _connectionState.value = "Failed to disconnect: ${e.message}"
        }
    }
}
