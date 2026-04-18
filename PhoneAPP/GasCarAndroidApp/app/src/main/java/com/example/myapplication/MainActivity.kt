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
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
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
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
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
import com.example.myapplication.spp.EnvCarSppProtocol
import kotlin.math.max
import kotlin.math.min
import com.example.myapplication.ui.theme.AlarmDanger
import com.example.myapplication.ui.theme.AlarmSafe
import com.example.myapplication.ui.theme.MyApplicationTheme
import java.util.Locale

/** 主界面 Activity：申请蓝牙权限、挂载 Compose 仪表盘并预加载已配对设备。 */
class MainActivity : ComponentActivity() {
    private val bluetoothViewModel: BluetoothViewModel by viewModels()

    /** 初始化主题与 [MainScreen]，在 Android 12+ 上请求蓝牙扫描/连接权限。 */
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

/** 顶部标题栏，展示仪表盘标题。 */
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

/** 仪表盘主列表：蓝牙、数据显示、控制与原始报文区块。 */
@Composable
fun MainScreen(
    viewModel: BluetoothViewModel,
    modifier: Modifier = Modifier
) {
    val sts by viewModel.stsUplink.collectAsState()
    val rawLog by viewModel.rawReceivedLog.collectAsState()

    LazyColumn(
        modifier = modifier.fillMaxWidth(),
        contentPadding = PaddingValues(horizontal = 16.dp, vertical = 12.dp),
        verticalArrangement = Arrangement.spacedBy(14.dp)
    ) {
        item {
            BluetoothConnectionPanel(viewModel)
        }
        item {
            DisplayPanel(sts)
        }
        item {
            ControlPanel(viewModel)
        }
        item {
            RawDataPanel(
                receivedData = rawLog,
                onClear = { viewModel.clearRawLog() }
            )
        }
        item { Spacer(modifier = Modifier.height(8.dp)) }
    }
}

/** 蓝牙卡片：刷新/断开、连接状态芯片、已配对设备列表与点击连接。 */
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

/** 只读状态芯片：根据 [status] 文案判断是否已连接并切换图标。 */
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

/**
 * 展示最近一次解析成功的 `$STS:` 上行帧（由 [EnvCarSppProtocol] 解析）。
 */
@Composable
fun DisplayPanel(sts: EnvCarSppProtocol.StsUplink?) {
    val gasData = sts?.let { String.format(Locale.US, "%.2f ppm", it.gasPpm) } ?: "—"
    val obstacleData = sts?.let {
        if (it.obsFlag == EnvCarSppProtocol.ObsFlag.NONE) "无障碍"
        else "${it.obsCm} cm"
    } ?: "—"
    val alarmStatus = sts?.let { EnvCarSppProtocol.alarmLabelZh(it.alarm) } ?: "—"
    val carStatus = sts?.let { EnvCarSppProtocol.carStateLabelZh(it.carState) } ?: "—"
    val runTime = sts?.let { String.format(Locale.US, "%d s", it.runTimeS) } ?: "—"

    val alarmOk = sts == null || sts.alarm == EnvCarSppProtocol.Alarm.NONE

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
            Spacer(modifier = Modifier.height(10.dp))
            MetricTile(
                label = "运行时间",
                value = runTime,
                valueColor = MaterialTheme.colorScheme.onSurface,
                modifier = Modifier.fillMaxWidth()
            )
        }
    }
}

/** 单行指标：左侧标签 + 右侧数值与颜色。 */
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

/** 方向、停止 `@0`；`@9`/`@10` 运行模式；`@5`～`@8` 速度；`#O...,G...` 阈值及 MCU 应答提示。 */
@Composable
fun ControlPanel(viewModel: BluetoothViewModel) {
    val thresholdFb by viewModel.thresholdFeedback.collectAsState()

    var carSpeed by remember { mutableStateOf(50f) }
    var obstacleDistance by remember { mutableStateOf(20f) }
    var safeDistance by remember { mutableStateOf(30f) }
    var gasHigh by remember { mutableStateOf(500f) }
    var useGasLowAlarm by remember { mutableStateOf(false) }
    var gasLow by remember { mutableStateOf(10f) }

    val speedCode = EnvCarSppProtocol.speedPercentToCommandCode(carSpeed.toInt())
    val speedLabel = EnvCarSppProtocol.speedCommandCodeToPercentLabel(speedCode)

    LaunchedEffect(obstacleDistance) {
        val minSafe = obstacleDistance + 1f
        if (safeDistance < minSafe) {
            safeDistance = (obstacleDistance + 10f).coerceIn(minSafe, 50f)
        }
    }
    LaunchedEffect(gasHigh, useGasLowAlarm) {
        if (!useGasLowAlarm) return@LaunchedEffect
        val maxLow = (gasHigh - 0.02f).coerceAtLeast(0.02f)
        if (gasLow > maxLow) gasLow = max(0.01f, maxLow)
    }

    val pushThreshold: () -> Unit = {
        val trig = obstacleDistance.toInt().coerceIn(0, 49)
        val safe = safeDistance.toInt().coerceIn(trig + 1, 50)
        val high = gasHigh.coerceAtLeast(1f)
        val low = if (useGasLowAlarm) {
            min(max(gasLow, 0.01f), high - 0.01f)
        } else {
            0f
        }
        viewModel.sendThresholdConfig(trig, safe, low, high)
    }

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
                    onClick = { viewModel.sendControl(EnvCarSppProtocol.Cmd.FORWARD) },
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
                        onClick = { viewModel.sendControl(EnvCarSppProtocol.Cmd.TURN_LEFT) },
                        modifier = Modifier.size(56.dp)
                    ) {
                        Icon(
                            imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                            contentDescription = stringResource(R.string.content_desc_left)
                        )
                    }
                    FilledIconButton(
                        onClick = { viewModel.sendControl(EnvCarSppProtocol.Cmd.TURN_RIGHT) },
                        modifier = Modifier.size(56.dp)
                    ) {
                        Icon(
                            imageVector = Icons.AutoMirrored.Filled.ArrowForward,
                            contentDescription = stringResource(R.string.content_desc_right)
                        )
                    }
                }
                FilledIconButton(
                    onClick = { viewModel.sendControl(EnvCarSppProtocol.Cmd.BACKWARD) },
                    modifier = Modifier.size(56.dp)
                ) {
                    Icon(
                        imageVector = Icons.Default.ArrowDownward,
                        contentDescription = stringResource(R.string.content_desc_back)
                    )
                }
            }
            Spacer(modifier = Modifier.height(12.dp))
            FilledTonalButton(
                onClick = { viewModel.sendControl(EnvCarSppProtocol.Cmd.STOP) },
                modifier = Modifier.fillMaxWidth()
            ) {
                Text("停止（@0）")
            }
            Spacer(modifier = Modifier.height(14.dp))
            Text(
                text = stringResource(R.string.section_run_mode),
                style = MaterialTheme.typography.labelLarge,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Spacer(modifier = Modifier.height(6.dp))
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                FilledTonalButton(
                    onClick = { viewModel.sendControl(EnvCarSppProtocol.Cmd.MODE_MANUAL) },
                    modifier = Modifier.weight(1f)
                ) {
                    Text(stringResource(R.string.action_mode_manual), maxLines = 2)
                }
                FilledTonalButton(
                    onClick = { viewModel.sendControl(EnvCarSppProtocol.Cmd.MODE_AUTO_TRACK) },
                    modifier = Modifier.weight(1f)
                ) {
                    Text(stringResource(R.string.action_mode_auto), maxLines = 2)
                }
            }
            Spacer(modifier = Modifier.height(16.dp))
            HorizontalDivider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.25f))
            Spacer(modifier = Modifier.height(12.dp))
            Text(
                text = "小车速度：${carSpeed.toInt()}% → MCU 档位 $speedLabel（@${speedCode}）",
                style = MaterialTheme.typography.bodyLarge
            )
            Slider(
                value = carSpeed,
                onValueChange = { carSpeed = it },
                onValueChangeFinished = { viewModel.sendSpeedPercent(carSpeed.toInt()) },
                valueRange = 0f..100f
            )
            Text(
                text = "${stringResource(R.string.label_obstacle_trig)}：${obstacleDistance.toInt()}",
                style = MaterialTheme.typography.bodyLarge
            )
            Slider(
                value = obstacleDistance,
                onValueChange = { obstacleDistance = min(it, 49f) },
                onValueChangeFinished = pushThreshold,
                valueRange = 0f..50f
            )
            Text(
                text = "${stringResource(R.string.label_obstacle_safe)}：${safeDistance.toInt()}",
                style = MaterialTheme.typography.bodyLarge
            )
            Slider(
                value = safeDistance,
                onValueChange = { safeDistance = it },
                onValueChangeFinished = pushThreshold,
                valueRange = (obstacleDistance + 1f)..50f
            )
            HorizontalDivider(
                modifier = Modifier.padding(vertical = 8.dp),
                color = MaterialTheme.colorScheme.outline.copy(alpha = 0.2f)
            )
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = stringResource(R.string.label_gas_low_enable),
                    style = MaterialTheme.typography.bodyLarge,
                    modifier = Modifier.padding(end = 8.dp)
                )
                Switch(
                    checked = useGasLowAlarm,
                    onCheckedChange = { useGasLowAlarm = it }
                )
            }
            if (useGasLowAlarm) {
                Text(
                    text = "${stringResource(R.string.label_gas_low_ppm)}：${
                        String.format(Locale.US, "%.2f", gasLow)
                    }",
                    style = MaterialTheme.typography.bodyLarge
                )
                Slider(
                    value = gasLow,
                    onValueChange = { gasLow = it },
                    onValueChangeFinished = pushThreshold,
                    valueRange = 0.01f..min(gasHigh - 0.02f, 499f).coerceAtLeast(0.02f)
                )
            }
            Text(
                text = "${stringResource(R.string.label_gas_high_ppm)}：${gasHigh.toInt()}",
                style = MaterialTheme.typography.bodyLarge
            )
            Slider(
                value = gasHigh,
                onValueChange = { gasHigh = it },
                onValueChangeFinished = pushThreshold,
                valueRange = 1f..1000f
            )
            thresholdFb?.let { msg ->
                Spacer(modifier = Modifier.height(10.dp))
                Text(
                    text = "${stringResource(R.string.label_threshold_feedback)}：$msg",
                    style = MaterialTheme.typography.bodySmall,
                    color = if (msg.contains("已写入") || msg.contains("#OK")) {
                        AlarmSafe
                    } else {
                        AlarmDanger
                    }
                )
            }
        }
    }
}

/** 等宽展示累积原始接收文本（含 `$STS:`、`ECHO:` 等），支持上下滚动、回底与清空。 */
@Composable
fun RawDataPanel(
    receivedData: String,
    onClear: () -> Unit
) {
    val hScroll = rememberScrollState()
    val vScroll = rememberScrollState()
    var autoStickToBottom by remember { mutableStateOf(true) }
    val isNearBottom = vScroll.maxValue - vScroll.value <= 6

    LaunchedEffect(vScroll.value, vScroll.maxValue) {
        if (!isNearBottom) {
            autoStickToBottom = false
        }
    }
    LaunchedEffect(receivedData, autoStickToBottom) {
        if (autoStickToBottom) {
            vScroll.scrollTo(vScroll.maxValue)
        }
    }

    Card(
        modifier = Modifier.fillMaxWidth(),
        elevation = CardDefaults.cardElevation(defaultElevation = 0.dp),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.4f)
        )
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = stringResource(R.string.section_raw),
                    style = MaterialTheme.typography.titleMedium
                )
                Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    if (!isNearBottom) {
                        FilledTonalButton(
                            onClick = { autoStickToBottom = true },
                        ) {
                            Text(stringResource(R.string.action_scroll_to_bottom))
                        }
                    }
                    FilledTonalButton(onClick = onClear) {
                        Text(stringResource(R.string.action_clear_raw))
                    }
                }
            }
            Spacer(modifier = Modifier.height(8.dp))
            Surface(
                shape = MaterialTheme.shapes.small,
                color = MaterialTheme.colorScheme.surface,
                tonalElevation = 1.dp
            ) {
                Text(
                    text = if (receivedData.isBlank()) "（无）" else receivedData,
                    modifier = Modifier
                        .heightIn(min = 120.dp, max = 260.dp)
                        .padding(12.dp)
                        .verticalScroll(vScroll)
                        .horizontalScroll(hScroll),
                    style = MaterialTheme.typography.bodySmall,
                    fontFamily = FontFamily.Monospace,
                    color = MaterialTheme.colorScheme.onSurface
                )
            }
        }
    }
}

/** Compose 预览用：在主题内展示 [MainScreen]。 */
@Preview(showBackground = true)
@Composable
fun DefaultPreview() {
    MyApplicationTheme {
        MainScreen(
            viewModel = BluetoothViewModel(LocalContext.current.applicationContext as Application)
        )
    }
}
