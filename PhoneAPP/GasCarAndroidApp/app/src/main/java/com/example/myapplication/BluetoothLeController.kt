package com.example.myapplication

import android.annotation.SuppressLint
import android.bluetooth.*
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.content.Context
import android.util.Log
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow

@SuppressLint("MissingPermission")
class BluetoothLeController(private val context: Context) {

    private val bluetoothManager: BluetoothManager =
        context.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
    private val bluetoothAdapter: BluetoothAdapter? = bluetoothManager.adapter
    private var bluetoothGatt: BluetoothGatt? = null

    private val _scannedDevices = MutableStateFlow<List<BluetoothDevice>>(emptyList())
    val scannedDevices: StateFlow<List<BluetoothDevice>> = _scannedDevices

    private val _connectionState = MutableStateFlow("Disconnected")
    val connectionState: StateFlow<String> = _connectionState

    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt?, status: Int, newState: Int) {
            when (newState) {
                BluetoothProfile.STATE_CONNECTED -> {
                    _connectionState.value = "Connected"
                    bluetoothGatt = gatt
                    bluetoothGatt?.discoverServices()
                }
                BluetoothProfile.STATE_DISCONNECTED -> {
                    _connectionState.value = "Disconnected"
                    bluetoothGatt?.close()
                    bluetoothGatt = null
                }
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt?, status: Int) {
            Log.d("BluetoothLeController", "Services discovered with status: $status")
        }

        override fun onCharacteristicChanged(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic, value: ByteArray) {
            // TODO: Handle incoming data
        }
    }

    private val leScanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult?) {
            super.onScanResult(callbackType, result)
            result?.device?.let { device ->
                if (device.name != null && !_scannedDevices.value.any { it.address == device.address }) {
                    _scannedDevices.value = _scannedDevices.value + device
                }
            }
        }
    }

    fun startLeScan() {
        _scannedDevices.value = emptyList()
        bluetoothAdapter?.bluetoothLeScanner?.startScan(leScanCallback)
    }

    fun stopLeScan() {
        bluetoothAdapter?.bluetoothLeScanner?.stopScan(leScanCallback)
    }

    fun connect(deviceAddress: String) {
        stopLeScan()
        val device = _scannedDevices.value.find { it.address == deviceAddress }
        if (device != null) {
            _connectionState.value = "Connecting to ${device.name ?: deviceAddress}"
            bluetoothGatt = device.connectGatt(context, false, gattCallback)
        } else {
            _connectionState.value = "Device not found"
        }
    }

    fun disconnect() {
        _connectionState.value = "Disconnecting"
        bluetoothGatt?.disconnect()
    }
}