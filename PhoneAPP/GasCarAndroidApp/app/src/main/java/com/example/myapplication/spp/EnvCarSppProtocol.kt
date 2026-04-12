package com.example.myapplication.spp

import java.util.Locale
import kotlin.math.roundToInt

/**
 * 与 MCU `USART2_通信协议规范` / `uart_protocol` / `protocol_parser` 对齐的 SPP 文本协议。
 * `$STS:` 含运行时间字段（规范 **v1.4** 起第 6 字段）；仍兼容仅 5 字段的旧固件。
 * 控制码 **0～10**（含 v1.3：`@9` 手动、`@10` 自动循迹）；终端「原始数据」区以外的逻辑应通过本文件组帧、拆帧与解析。
 */
object EnvCarSppProtocol {

    const val LINE_SUFFIX: String = "\r\n"

    /** 上行状态帧前缀（避免在字符串里写 `$STS:` 被 Kotlin 当成插值）。 */
    private const val STS_PREFIX: String = "$" + "STS:"

    /** 下行控制指令码 0～10，与 `ControlCommand_t` / 规范 v1.3 一致。 */
    object Cmd {
        const val STOP: Int = 0
        const val FORWARD: Int = 1
        const val BACKWARD: Int = 2
        const val TURN_LEFT: Int = 3
        const val TURN_RIGHT: Int = 4
        const val SPEED_25: Int = 5
        const val SPEED_50: Int = 6
        const val SPEED_75: Int = 7
        const val SPEED_100: Int = 8
        /** 手动模式：`ENVCAR_MODE_MANUAL`（规范 3.1 `@9`）。 */
        const val MODE_MANUAL: Int = 9
        /** 自动循迹：`ENVCAR_MODE_AUTO`（规范 3.1 `@10`）。 */
        const val MODE_AUTO_TRACK: Int = 10
    }

    /** 上行状态帧 `alarm` 字段。 */
    object Alarm {
        const val NONE: Int = 0
        const val OBST: Int = 1
        const val GAS: Int = 2
        const val BOTH: Int = 3
    }

    /** 上行状态帧 `obs_flag` 字段。 */
    object ObsFlag {
        const val NONE: Int = 0
        const val NEAR: Int = 1
    }

    /**
     * 周期上行 `$STS:...` 解析结果（与 MCU `UART_StatusUplink_t` 对应）。
     */
    data class StsUplink(
        val gasPpm: Float,
        val obsFlag: Int,
        val obsCm: Int,
        val alarm: Int,
        val carState: Int,
        /** 系统运行时间（秒），与 MCU `run_time_s` 一致；规范 v1.4 起为第 6 字段。 */
        val runTimeS: Long,
    )

    /** 完整一行的语义（不含行尾 CRLF）。 */
    sealed class InboundLine {
        data class Sts(val data: StsUplink) : InboundLine()
        data class Echo(val payload: String) : InboundLine()
        data object ConfigOk : InboundLine()
        data object ConfigErrParse : InboundLine()
        data object ConfigErrParams : InboundLine()
        data class Unknown(val raw: String) : InboundLine()
    }

    fun isValidControlCode(code: Int): Boolean = code in Cmd.STOP..Cmd.MODE_AUTO_TRACK

    /** 下行控制帧 `@<0..10>`（`@10` 为双字符），不含行尾。 */
    fun buildControlPayload(code: Int): String {
        require(isValidControlCode(code)) { "control code must be 0..10" }
        return "@$code"
    }

    /**
     * 下行阈值帧 `#O<trig>,<safe>,G<low>,<high>`（与 MCU sscanf 一致，浮点用小数点）。
     */
    fun buildThresholdPayload(
        trigCm: Int,
        safeCm: Int,
        lowPpm: Float,
        highPpm: Float,
    ): String {
        val lowStr = formatPpm(lowPpm)
        val highStr = formatPpm(highPpm)
        return "#O$trigCm,$safeCm,G$lowStr,$highStr"
    }

    /** 经 SPP 发送的完整字节序列（UTF-8 + CRLF）。 */
    fun encodeDownlink(payload: String): ByteArray =
        (payload + LINE_SUFFIX).toByteArray(Charsets.UTF_8)

    /**
     * 将 0～100 的 UI 速度映射为 `@5`～`@8` 四档（MCU 无连续速度）。
     */
    fun speedPercentToCommandCode(percent: Int): Int = when (percent.coerceIn(0, 100)) {
        in 0..24 -> Cmd.SPEED_25
        in 25..49 -> Cmd.SPEED_50
        in 50..74 -> Cmd.SPEED_75
        else -> Cmd.SPEED_100
    }

    fun speedCommandCodeToPercentLabel(code: Int): String = when (code) {
        Cmd.SPEED_25 -> "25%"
        Cmd.SPEED_50 -> "50%"
        Cmd.SPEED_75 -> "75%"
        Cmd.SPEED_100 -> "100%"
        else -> "—"
    }

    /**
     * 解析 MCU 上行一行（已去掉 `\n`，可含末尾 `\r`）。
     * 返回 `null` 表示空行应忽略。
     */
    fun parseInboundLine(line: String): InboundLine? {
        val t = line.trimEnd('\r').trim()
        if (t.isEmpty()) return null

        if (t.startsWith(STS_PREFIX)) {
            parseSts(t)?.let { return InboundLine.Sts(it) }
            return InboundLine.Unknown(t)
        }
        if (t.startsWith("ECHO:")) {
            return InboundLine.Echo(t.removePrefix("ECHO:").trimStart())
        }
        if (t == "#OK:Config synced") return InboundLine.ConfigOk
        if (t == "#ERR:Parse failed") return InboundLine.ConfigErrParse
        if (t == "#ERR:Invalid params") return InboundLine.ConfigErrParams

        return InboundLine.Unknown(t)
    }

    private fun parseSts(line: String): StsUplink? {
        val body = line.removePrefix(STS_PREFIX).trim()
        val parts = body.split(',')
        if (parts.size !in 5..6) return null
        return try {
            val runTimeS = if (parts.size >= 6) parts[5].toLong() else 0L
            StsUplink(
                gasPpm = parts[0].toFloat(),
                obsFlag = parts[1].toInt(),
                obsCm = parts[2].toInt(),
                alarm = parts[3].toInt(),
                carState = parts[4].toInt(),
                runTimeS = runTimeS,
            )
        } catch (_: NumberFormatException) {
            null
        }
    }

    private fun formatPpm(v: Float): String {
        val rounded = (v * 100f).roundToInt() / 100f
        return if (rounded == rounded.toLong().toFloat()) {
            String.format(Locale.US, "%.1f", rounded)
        } else {
            String.format(Locale.US, "%.2f", rounded)
        }
    }

    /** 与 `USART2_通信协议规范` 4.3 节 `alarm` 一致，用于界面展示。 */
    fun alarmLabelZh(alarm: Int): String = when (alarm) {
        Alarm.NONE -> "无"
        Alarm.OBST -> "障碍"
        Alarm.GAS -> "气体"
        Alarm.BOTH -> "障碍+气体"
        else -> "未知($alarm)"
    }

    /** 与 `EnvCar_State_Enum` 数值一致，用于界面展示。 */
    fun carStateLabelZh(state: Int): String = when (state) {
        0 -> "空闲"
        1 -> "循迹中"
        2 -> "障碍报警"
        3 -> "气体报警"
        4 -> "手动控制"
        5 -> "故障"
        else -> "状态$state"
    }
}
