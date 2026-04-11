package com.example.myapplication

import android.Manifest
import android.app.Application
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.viewModels
import androidx.compose.foundation.clickable
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.automirrored.filled.ArrowForward
import androidx.compose.material.icons.filled.ArrowDownward
import androidx.compose.material.icons.filled.ArrowUpward
import androidx.compose.material.icons.filled.Bluetooth
import androidx.compose.material.icons.filled.BluetoothConnected
import androidx.compose.material.icons.filled.BluetoothSearching
import androidx.compose.material.icons.filled.Sensors
import androidx.compose.material.icons.filled.Tune
import androidx.compose.material3.AssistChip
import androidx.compose.material3.AssistChipDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.CenterAlignedTopAppBar
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.FilledIconButton
import androidx.compose.material3.FilledTonalButton
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.ListItem
import androidx.compose.material3.ListItemDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedCard
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Slider
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.example.myapplication.ui.theme.AlarmDanger
import com.example.myapplication.ui.theme.AlarmSafe
import com.example.myapplication.ui.theme.MyApplicationTheme

class MainActivity : ComponentActivity() {
    private val bluetoothViewModel: BluetoothViewModel by viewModels()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

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
                Scaffold(
                    topBar = { DashboardTopBar() }
                ) { innerPadding ->
                    MainScreen(
                        viewModel = bluetoothViewModel,
                        modifier = Modifier.padding(innerPadding)
                    )
                }
            }
        }

        bluetoothViewModel.getPairedDevices()
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun DashboardTopBar() {
    CenterAlignedTopAppBar(
        title = {
            Text(
                text = stringResource(R.string.screen_dashboard_title),
                style = MaterialTheme.typography.titleLarge
            )
        },
        colors = TopAppBarDefaults.centerAlignedTopAppBarColors(
            containerColor = MaterialTheme.colorScheme.surface,
            titleContentColor = MaterialTheme.colorScheme.onSurface
        )
    )
}

@Composable
fun MainScreen(
    viewModel: BluetoothViewModel,
    modifier: Modifier = Modifier
) {
    val receivedData by viewModel.receivedData.collectAsState()

    LazyColumn(
        modifier = modifier.fillMaxWidth(),
        contentPadding = PaddingValues(horizontal = 16.dp, vertical = 12.dp),
        verticalArrangement = Arrangement.spacedBy(14.dp)
    ) {
        item {
            BluetoothConnectionPanel(viewModel)
        }
        item {
            DisplayPanel(receivedData)
        }
        item {
            ControlPanel(viewModel)
        }
        item {
            RawDataPanel(receivedData)
        }
        item { Spacer(modifier = Modifier.height(8.dp)) }
    }
}

@Composable
fun BluetoothConnectionPanel(viewModel: BluetoothViewModel) {
    val pairedDevices by viewModel.pairedDevices.collectAsState()
    val connectionStatus by viewModel.connectionStatus.collectAsState()

    Card(
        modifier = Modifier.fillMaxWidth(),
        elevation = CardDefaults.cardElevation(defaultElevation = 1.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surface)
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Row(
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Icon(
                    imageVector = Icons.Default.BluetoothSearching,
                    contentDescription = null,
                    tint = MaterialTheme.colorScheme.primary
                )
                Text(
                    text = stringResource(R.string.section_bluetooth),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.onSurface
                )
            }
            Spacer(modifier = Modifier.height(12.dp))
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                FilledTonalButton(onClick = { viewModel.getPairedDevices() }) {
                    Text(stringResource(R.string.action_refresh_paired))
                }
                FilledTonalButton(onClick = { viewModel.disconnect() }) {
                    Text(stringResource(R.string.action_disconnect))
                }
            }
            Spacer(modifier = Modifier.height(10.dp))
            ConnectionStatusChip(connectionStatus)
            Spacer(modifier = Modifier.height(8.dp))
            HorizontalDivider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.25f))
            Spacer(modifier = Modifier.height(8.dp))
            if (pairedDevices.isEmpty()) {
                Text(
                    text = "暂无已配对设备，请先在系统蓝牙中配对。",
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            } else {
                Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                    pairedDevices.forEach { device ->
                        ListItem(
                            headlineContent = {
                                Text(
                                    text = device.name ?: stringResource(R.string.device_unknown),
                                    maxLines = 1,
                                    overflow = TextOverflow.Ellipsis
                                )
                            },
                            supportingContent = {
                                Text(
                                    text = device.address,
                                    style = MaterialTheme.typography.bodySmall,
                                    maxLines = 1,
                                    overflow = TextOverflow.Ellipsis
                                )
                            },
                            leadingContent = {
                                Icon(
                                    imageVector = Icons.Default.Bluetooth,
                                    contentDescription = null,
                                    tint = MaterialTheme.colorScheme.primary
                                )
                            },
                            modifier = Modifier
                                .clickable { viewModel.connectToDevice(device.address) },
                            colors = ListItemDefaults.colors(
                                containerColor = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.35f)
                            )
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun ConnectionStatusChip(status: String) {
    val connected = status.contains("已连接", ignoreCase = true) ||
        status.contains("connected", ignoreCase = true)
    AssistChip(
        onClick = {},
        enabled = false,
        label = {
            Text(
                text = "${stringResource(R.string.status_prefix)}：$status",
                maxLines = 2,
                overflow = TextOverflow.Ellipsis
            )
        },
        leadingIcon = {
            Icon(
                imageVector = if (connected) Icons.Default.BluetoothConnected else Icons.Default.Bluetooth,
                contentDescription = null,
                modifier = Modifier.size(18.dp)
            )
        },
        colors = AssistChipDefaults.assistChipColors(
            disabledContainerColor = MaterialTheme.colorScheme.secondaryContainer.copy(alpha = 0.5f),
            disabledLabelColor = MaterialTheme.colorScheme.onSecondaryContainer
        )
    )
}

@Composable
fun DisplayPanel(receivedData: String) {
    var gasData by remember { mutableStateOf("—") }
    var obstacleData by remember { mutableStateOf("—") }
    var alarmStatus by remember { mutableStateOf("—") }
    var carStatus by remember { mutableStateOf("—") }

    LaunchedEffect(receivedData) {
        if (receivedData.startsWith("@") && receivedData.contains(",")) {
            val parts = receivedData.removePrefix("@").trim().split(",")
            if (parts.size == 4) {
                gasData = parts[0]
                obstacleData = parts[1]
                alarmStatus = if (parts[2] == "1") "警报" else "正常"
                carStatus = if (parts[3] == "1") "巡检中" else "已停止"
            }
        }
    }

    val alarmOk = alarmStatus == "正常" || alarmStatus == "—"

    Card(
        modifier = Modifier.fillMaxWidth(),
        elevation = CardDefaults.cardElevation(defaultElevation = 1.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surface)
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Row(
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Icon(
                    imageVector = Icons.Default.Sensors,
                    contentDescription = null,
                    tint = MaterialTheme.colorScheme.primary
                )
                Text(
                    text = stringResource(R.string.section_display),
                    style = MaterialTheme.typography.titleMedium
                )
            }
            Spacer(modifier = Modifier.height(12.dp))
            Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(10.dp)) {
                MetricTile(
                    label = "气体",
                    value = gasData,
                    valueColor = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.weight(1f)
                )
                MetricTile(
                    label = "障碍物",
                    value = obstacleData,
                    valueColor = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.weight(1f)
                )
            }
            Spacer(modifier = Modifier.height(10.dp))
            Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(10.dp)) {
                MetricTile(
                    label = "警报",
                    value = alarmStatus,
                    valueColor = if (alarmOk) AlarmSafe else AlarmDanger,
                    modifier = Modifier.weight(1f)
                )
                MetricTile(
                    label = "小车",
                    value = carStatus,
                    valueColor = MaterialTheme.colorScheme.onSurface,
                    modifier = Modifier.weight(1f)
                )
            }
        }
    }
}

@Composable
private fun MetricTile(
    label: String,
    value: String,
    valueColor: Color,
    modifier: Modifier = Modifier
) {
    OutlinedCard(
        modifier = modifier,
        colors = CardDefaults.outlinedCardColors(
            containerColor = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.25f)
        )
    ) {
        Column(modifier = Modifier.padding(horizontal = 12.dp, vertical = 10.dp)) {
            Text(
                text = label,
                style = MaterialTheme.typography.labelLarge,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Spacer(modifier = Modifier.height(4.dp))
            Text(
                text = value,
                style = MaterialTheme.typography.titleMedium,
                color = valueColor,
                maxLines = 2,
                overflow = TextOverflow.Ellipsis
            )
        }
    }
}

@Composable
fun ControlPanel(viewModel: BluetoothViewModel) {
    var carSpeed by remember { mutableFloatStateOf(50f) }
    var obstacleDistance by remember { mutableFloatStateOf(100f) }
    var gasThreshold by remember { mutableFloatStateOf(500f) }

    Card(
        modifier = Modifier.fillMaxWidth(),
        elevation = CardDefaults.cardElevation(defaultElevation = 1.dp),
        colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surface)
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Row(
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Icon(
                    imageVector = Icons.Default.Tune,
                    contentDescription = null,
                    tint = MaterialTheme.colorScheme.primary
                )
                Text(
                    text = stringResource(R.string.section_control),
                    style = MaterialTheme.typography.titleMedium
                )
            }
            Spacer(modifier = Modifier.height(12.dp))
            Text(
                text = "方向",
                style = MaterialTheme.typography.labelLarge,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Spacer(modifier = Modifier.height(6.dp))
            Column(
                modifier = Modifier.fillMaxWidth(),
                horizontalAlignment = Alignment.CenterHorizontally,
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                FilledIconButton(
                    onClick = { viewModel.sendData("forward") },
                    modifier = Modifier.size(56.dp)
                ) {
                    Icon(
                        imageVector = Icons.Default.ArrowUpward,
                        contentDescription = stringResource(R.string.content_desc_forward)
                    )
                }
                Row(
                    horizontalArrangement = Arrangement.spacedBy(16.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    FilledIconButton(
                        onClick = { viewModel.sendData("left") },
                        modifier = Modifier.size(56.dp)
                    ) {
                        Icon(
                            imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                            contentDescription = stringResource(R.string.content_desc_left)
                        )
                    }
                    FilledIconButton(
                        onClick = { viewModel.sendData("right") },
                        modifier = Modifier.size(56.dp)
                    ) {
                        Icon(
                            imageVector = Icons.AutoMirrored.Filled.ArrowForward,
                            contentDescription = stringResource(R.string.content_desc_right)
                        )
                    }
                }
                FilledIconButton(
                    onClick = { viewModel.sendData("backward") },
                    modifier = Modifier.size(56.dp)
                ) {
                    Icon(
                        imageVector = Icons.Default.ArrowDownward,
                        contentDescription = stringResource(R.string.content_desc_back)
                    )
                }
            }
            Spacer(modifier = Modifier.height(16.dp))
            HorizontalDivider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.25f))
            Spacer(modifier = Modifier.height(12.dp))
            Text(
                text = "小车速度：${carSpeed.toInt()}",
                style = MaterialTheme.typography.bodyLarge
            )
            Slider(
                value = carSpeed,
                onValueChange = {
                    carSpeed = it
                    viewModel.sendData("speed:${it.toInt()}")
                },
                valueRange = 0f..100f
            )
            Text(
                text = "障碍物检测距离：${obstacleDistance.toInt()} cm",
                style = MaterialTheme.typography.bodyLarge
            )
            Slider(
                value = obstacleDistance,
                onValueChange = {
                    obstacleDistance = it
                    viewModel.sendData("obstacle:${it.toInt()}")
                },
                valueRange = 0f..200f
            )
            Text(
                text = "气体浓度阈值：${gasThreshold.toInt()} ppm",
                style = MaterialTheme.typography.bodyLarge
            )
            Slider(
                value = gasThreshold,
                onValueChange = {
                    gasThreshold = it
                    viewModel.sendData("gas:${it.toInt()}")
                },
                valueRange = 0f..1000f
            )
        }
    }
}

@Composable
fun RawDataPanel(receivedData: String) {
    val hScroll = rememberScrollState()
    Card(
        modifier = Modifier.fillMaxWidth(),
        elevation = CardDefaults.cardElevation(defaultElevation = 0.dp),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.4f)
        )
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text(
                text = stringResource(R.string.section_raw),
                style = MaterialTheme.typography.titleMedium
            )
            Spacer(modifier = Modifier.height(8.dp))
            Surface(
                shape = MaterialTheme.shapes.small,
                color = MaterialTheme.colorScheme.surface,
                tonalElevation = 1.dp
            ) {
                Text(
                    text = if (receivedData.isBlank()) "（无）" else receivedData,
                    modifier = Modifier
                        .padding(12.dp)
                        .horizontalScroll(hScroll),
                    style = MaterialTheme.typography.bodySmall,
                    fontFamily = FontFamily.Monospace,
                    color = MaterialTheme.colorScheme.onSurface
                )
            }
        }
    }
}

@Preview(showBackground = true)
@Composable
fun DefaultPreview() {
    MyApplicationTheme {
        MainScreen(
            viewModel = BluetoothViewModel(LocalContext.current.applicationContext as Application)
        )
    }
}
