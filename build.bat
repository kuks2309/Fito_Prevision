@echo off
echo ========================================
echo Vision Comparison Tool - Build Script
echo ========================================
echo.

REM Visual Studio 2022 Developer Command Prompt 환경 설정
call "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat"

if errorlevel 1 (
    echo [ERROR] Failed to setup VS2022 Developer Command Prompt
    echo Please check if Visual Studio 2022 is installed at:
    echo "C:\Program Files\Microsoft Visual Studio\2022\Community"
    pause
    exit /b 1
)

echo.
echo [INFO] Building VisionComparison.sln...
echo.

REM 빌드 디렉토리로 이동
cd /d "%~dp0build"

REM MSBuild로 빌드 실행
msbuild VisionComparison.sln /p:Configuration=Debug /m /v:minimal

if errorlevel 1 (
    echo.
    echo [ERROR] Build failed!
    echo.
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Build completed successfully!
echo Executable: build\Debug\VisionComparison.exe
echo.
pause
