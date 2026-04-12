# 解决 Windows 上 processDebugResources 无法删除 R.jar（文件被占用）的常见做法：
# 1) 先结束 Gradle 守护进程；2) 多次尝试删除 app\build 与 .android-app-build；3) assembleDebug --no-daemon
# 使用前请先关闭 Android Studio 对本项目的构建/同步（或整个 IDE），否则仍可能删不掉。

$ErrorActionPreference = 'Continue'
$Root = $PSScriptRoot
Set-Location -LiteralPath $Root

Write-Host '==> gradlew --stop' -ForegroundColor Cyan
& (Join-Path $Root 'gradlew.bat') --stop 2>&1 | Out-Host
Start-Sleep -Seconds 3

$Dirs = @(
    (Join-Path $Root 'app\build'),
    (Join-Path $Root '.android-app-build')
)
foreach ($BuildDir in $Dirs) {
    if (Test-Path -LiteralPath $BuildDir) {
        Write-Host "==> 删除: $BuildDir" -ForegroundColor Cyan
        $ok = $false
        for ($i = 1; $i -le 8; $i++) {
            try {
                Remove-Item -LiteralPath $BuildDir -Recurse -Force -ErrorAction Stop
                $ok = $true
                Write-Host "    已删除 (第 $i 次尝试)" -ForegroundColor Green
                break
            } catch {
                Write-Warning "    第 $i 次删除失败: $($_.Exception.Message)"
                Start-Sleep -Seconds 2
            }
        }
        if (-not $ok) {
            Write-Host @'

仍无法删除上述目录。请手动：
  - 关闭 Android Studio / Cursor 中正在运行的 Gradle 同步与运行配置；
  - 任务管理器结束「Java(TM) Platform SE binary」里占用该工程路径的进程；
  - 将本工程目录加入 Windows Defender「排除项」；
  - 再重新运行本脚本，或手动删除 app\build、.android-app-build 后执行: .\gradlew.bat assembleDebug --no-daemon
'@ -ForegroundColor Yellow
            exit 1
        }
    }
}

Write-Host '==> assembleDebug (--no-daemon)' -ForegroundColor Cyan
& (Join-Path $Root 'gradlew.bat') assembleDebug --no-daemon
exit $LASTEXITCODE
