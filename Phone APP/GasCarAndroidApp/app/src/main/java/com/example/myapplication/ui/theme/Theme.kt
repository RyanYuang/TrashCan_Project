package com.example.myapplication.ui.theme

import android.os.Build
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.dynamicDarkColorScheme
import androidx.compose.material3.dynamicLightColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext

private val DarkColorScheme = darkColorScheme(
    primary = TealSecondary,
    onPrimary = ColorBlackish,
    primaryContainer = TealPrimaryDark,
    onPrimaryContainer = Color(0xFFB2DFDB),
    secondary = TealSecondary,
    onSecondary = ColorBlackish,
    tertiary = Pink80,
    background = Color(0xFF121416),
    surface = Color(0xFF1A1D21),
    surfaceVariant = Color(0xFF2A3036),
    onBackground = Color(0xFFE8ECEE),
    onSurface = Color(0xFFE8ECEE),
    onSurfaceVariant = Color(0xFFB0B8C0),
)

private val LightColorScheme = lightColorScheme(
    primary = TealPrimary,
    onPrimary = Color(0xFFFFFFFF),
    primaryContainer = Color(0xFFB2DFDB),
    onPrimaryContainer = TealPrimaryDark,
    secondary = TealSecondary,
    onSecondary = Color(0xFFFFFFFF),
    tertiary = TealPrimaryDark,
    background = SurfaceLight,
    surface = Color(0xFFFFFFFF),
    surfaceVariant = SurfaceVariantLight,
    onBackground = Color(0xFF1A1D21),
    onSurface = Color(0xFF1A1D21),
    onSurfaceVariant = OnSurfaceMuted,
    outline = Color(0xFF90A4AE),
)

@Composable
fun MyApplicationTheme(
    darkTheme: Boolean = isSystemInDarkTheme(),
    dynamicColor: Boolean = false,
    content: @Composable () -> Unit
) {
    val colorScheme = when {
        dynamicColor && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S -> {
            val context = LocalContext.current
            if (darkTheme) dynamicDarkColorScheme(context) else dynamicLightColorScheme(context)
        }

        darkTheme -> DarkColorScheme
        else -> LightColorScheme
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content
    )
}
