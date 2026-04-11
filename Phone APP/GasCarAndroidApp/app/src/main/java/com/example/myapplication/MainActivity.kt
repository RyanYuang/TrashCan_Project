package com.example.myapplication

import android.Manifest
import android.app.Application
import android.bluetooth.BluetoothDevice
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.viewModels
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.example.myapplication.ui.theme.MyApplicationTheme

class MainActivity : ComponentActivity() {
    private val bluetoothViewModel: BluetoothViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val enableBluetoothLauncher = registerForActivityResult(
            ActivityResultContracts.RequestMultiplePermissions()
        ) { /* Handle permissions */ }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            enableBluetoothLauncher.launch(
                arrayOf(
                    Manifest.permission.BLUETOOTH_SCAN,
                    Manifest.permission.BLUETOOTH_CONNECT,
                )
            )
        }

        setContent {
            MyApplicationTheme {
                MainScreen(bluetoothViewModel)
            }
        }

        // Get paired devices when the app starts
        bluetoothViewModel.getPairedDevices()
    }
}

@Composable
fun MainScreen(viewModel: BluetoothViewModel) {
    val receivedData by viewModel.receivedData.collectAsState()
    val scrollState = rememberScrollState()

    Column(
        modifier = Modifier
            .padding(16.dp)
            .verticalScroll(scrollState)
    ) {
        BluetoothConnectionPanel(viewModel)
        Spacer(modifier = Modifier.height(16.dp))
        DisplayPanel(receivedData)
        Spacer(modifier = Modifier.height(16.dp))
        ControlPanel(viewModel)
        Spacer(modifier = Modifier.height(16.dp))
        RawDataPanel(receivedData)
    }
}

@Composable
fun BluetoothConnectionPanel(viewModel: BluetoothViewModel) {
    val pairedDevices by viewModel.pairedDevices.collectAsState()
    val connectionStatus by viewModel.connectionStatus.collectAsState()

    Card(modifier = Modifier.fillMaxWidth()) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text(text = "蓝牙连接", style = MaterialTheme.typography.headlineSmall)
            Spacer(modifier = Modifier.height(8.dp))
            Row {
                Button(onClick = { viewModel.getPairedDevices() }) {
                    Text("获取已配对设备")
                }
                Spacer(modifier = Modifier.width(8.dp))
                Button(onClick = { viewModel.disconnect() }) {
                    Text("断开连接")
                }
            }
            Spacer(modifier = Modifier.height(8.dp))
            Text(text = "状态: $connectionStatus")
            Spacer(modifier = Modifier.height(8.dp))
            LazyColumn(modifier = Modifier.height(100.dp)) {
                items(pairedDevices) { device ->
                    Text(text = "${device.name ?: "Unknown"} (${device.address})", modifier = Modifier.clickable { viewModel.connectToDevice(device.address) })
                }
            }
        }
    }
}

@Composable
fun DisplayPanel(receivedData: String) {
    var gasData by remember { mutableStateOf("...") }
    var obstacleData by remember { mutableStateOf("...") }
    var alarmStatus by remember { mutableStateOf("...") }
    var carStatus by remember { mutableStateOf("...") }

    // Parse the received data when it changes
    LaunchedEffect(receivedData) {
        if (receivedData.startsWith("@") && receivedData.contains(",")) {
            // Remove prefix and any trailing whitespace/newlines, then split
            val parts = receivedData.removePrefix("@").trim().split(",")
            if (parts.size == 4) {
                gasData = parts[0]
                obstacleData = parts[1]
                alarmStatus = if (parts[2] == "1") "警报" else "正常"
                carStatus = if (parts[3] == "1") "巡检状态" else "停止巡检状态"
            }
        }
    }

    Card(modifier = Modifier.fillMaxWidth()) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text(text = "显示面板", style = MaterialTheme.typography.headlineSmall)
            Spacer(modifier = Modifier.height(8.dp))
            Text(text = "气体检测数据: $gasData")
            Text(text = "障碍物距离信息: $obstacleData")
            Text(text = "警报: $alarmStatus")
            Text(text = "小车状态: $carStatus")
        }
    }
}

@Composable
fun ControlPanel(viewModel: BluetoothViewModel) {
    var carSpeed by remember { mutableStateOf(50f) }
    var obstacleDistance by remember { mutableStateOf(100f) }
    var gasThreshold by remember { mutableStateOf(500f) }
    var infraredStatus by remember { mutableStateOf(false) }

    Card(modifier = Modifier.fillMaxWidth()) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text(text = "控制面板", style = MaterialTheme.typography.headlineSmall)
            Spacer(modifier = Modifier.height(8.dp))
            // Car movement buttons
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.Center,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Button(onClick = { viewModel.sendData("forward") }) { Text("↑") }
            }
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceEvenly,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Button(onClick = { viewModel.sendData("left") }) { Text("←") }
                Button(onClick = { viewModel.sendData("right") }) { Text("→") }
            }
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.Center,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Button(onClick = { viewModel.sendData("backward") }) { Text("↓") }
            }
            Spacer(modifier = Modifier.height(16.dp))

            // Sliders and Switch
            Text(text = "小车速度: ${carSpeed.toInt()}")
            Slider(value = carSpeed, onValueChange = { carSpeed = it; viewModel.sendData("speed:${it.toInt()}") }, valueRange = 0f..100f)

            Text(text = "障碍物检测距离: ${obstacleDistance.toInt()} cm")
            Slider(value = obstacleDistance, onValueChange = { obstacleDistance = it; viewModel.sendData("obstacle:${it.toInt()}") }, valueRange = 0f..200f)

            Text(text = "气体浓度阈值: ${gasThreshold.toInt()} ppm")
            Slider(value = gasThreshold, onValueChange = { gasThreshold = it; viewModel.sendData("gas:${it.toInt()}") }, valueRange = 0f..1000f)

//            Row(verticalAlignment = Alignment.CenterVertically) {
//                Text(text = "红外循迹的状态: ")
//                Switch(checked = infraredStatus, onCheckedChange = { infraredStatus = it; viewModel.sendData("infrared:${if (it) "on" else "off"}") })
//                Text(text = if (infraredStatus) "开启" else "关闭")
//            }
        }
    }
}

@Composable
fun RawDataPanel(receivedData: String) {
    Card(modifier = Modifier.fillMaxWidth()) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text(text = "原始数据显示面板", style = MaterialTheme.typography.headlineSmall)
            Spacer(modifier = Modifier.height(8.dp))
            Text(text = receivedData)
        }
    }
}

@Preview(showBackground = true)
@Composable
fun DefaultPreview() {
    MyApplicationTheme {
        val context = LocalContext.current
        MainScreen(BluetoothViewModel(context.applicationContext as Application))
    }
}
