package com.example.myapplication

import android.app.Application
import android.bluetooth.BluetoothDevice
import androidx.lifecycle.AndroidViewModel
import kotlinx.coroutines.flow.StateFlow

class BluetoothViewModel(application: Application) : AndroidViewModel(application) {

    private val bluetoothController = BluetoothController(application)

    val pairedDevices: StateFlow<List<BluetoothDevice>> = bluetoothController.pairedDevices
    val connectionStatus: StateFlow<String> = bluetoothController.connectionState
    val receivedData: StateFlow<String> = bluetoothController.receivedData

    fun getPairedDevices() {
        bluetoothController.getPairedDevices()
    }

    fun connectToDevice(deviceAddress: String) {
        bluetoothController.connect(deviceAddress)
    }

    fun disconnect() {
        bluetoothController.disconnect()
    }

    fun sendData(data: String) {
        bluetoothController.sendData(data)
    }
}
