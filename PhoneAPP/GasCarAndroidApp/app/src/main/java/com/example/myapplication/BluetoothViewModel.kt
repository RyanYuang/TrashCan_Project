package com.example.myapplication

import android.app.Application
import android.bluetooth.BluetoothDevice
import androidx.lifecycle.AndroidViewModel
import com.example.myapplication.spp.EnvCarSppProtocol
import kotlinx.coroutines.flow.StateFlow

/**
 * 向界面暴露蓝牙与协议解析状态；所有下行指令经 [EnvCarSppProtocol] 组帧后发送。
 */
class BluetoothViewModel(application: Application) : AndroidViewModel(application) {

    private val bluetoothController = BluetoothController(application)

    val pairedDevices: StateFlow<List<BluetoothDevice>> = bluetoothController.pairedDevices
    val connectionStatus: StateFlow<String> = bluetoothController.connectionState
    val rawReceivedLog: StateFlow<String> = bluetoothController.rawReceivedLog
    val stsUplink: StateFlow<EnvCarSppProtocol.StsUplink?> = bluetoothController.stsUplink
    val thresholdFeedback: StateFlow<String?> = bluetoothController.thresholdFeedback

    fun getPairedDevices() {
        bluetoothController.getPairedDevices()
    }

    fun connectToDevice(deviceAddress: String) {
        bluetoothController.connect(deviceAddress)
    }

    fun disconnect() {
        bluetoothController.disconnect()
    }

    fun clearRawLog() {
        bluetoothController.clearRawLog()
    }

    /** 发送 `@0`～`@10` 控制帧（含 `@9` 手动、`@10` 自动循迹，见规范 v1.3）。 */
    fun sendControl(code: Int) {
        bluetoothController.sendDownlinkPayload(EnvCarSppProtocol.buildControlPayload(code))
    }

    /** 将 0～100 滑条映射为 `@5`～`@8` 四档速度指令。 */
    fun sendSpeedPercent(percent: Int) {
        val code = EnvCarSppProtocol.speedPercentToCommandCode(percent.coerceIn(0, 100))
        sendControl(code)
    }

    /**
     * 发送阈值帧 `#O<trig>,<safe>,G<low>,<high>`。
     * [trigCm] 为避障触发距离；[safeCm] 为恢复距离，须大于触发距离且均 ≤500（与 MCU 校验一致）。
     */
    fun sendThresholdConfig(trigCm: Int, safeCm: Int, lowPpm: Float, highPpm: Float) {
        val payload = EnvCarSppProtocol.buildThresholdPayload(trigCm, safeCm, lowPpm, highPpm)
        bluetoothController.sendDownlinkPayload(payload)
    }
}
