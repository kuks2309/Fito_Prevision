@echo off
setlocal

echo ========================================
echo Rebuilding VisionComparison...
echo ========================================

REM Try to find MSBuild
set MSBUILD_2022_PRO="C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"

if exist %MSBUILD_2022_PRO% (
    echo Found MSBuild 2022 Professional Edition
    echo.
    echo Cleaning...
    %MSBUILD_2022_PRO% build\VisionComparison.sln /t:Clean /p:Configuration=Debug /nologo
    echo.
    echo Rebuilding...
    %MSBUILD_2022_PRO% build\VisionComparison.sln /t:Rebuild /p:Configuration=Debug /m /v:minimal /nologo
    goto :end
)

echo ERROR: MSBuild not found!
pause
exit /b 1

:end
echo.
if %ERRORLEVEL% EQU 0 (
    echo ========================================
    echo Rebuild completed successfully!
    echo Executable: build\Debug\VisionComparison.exe
    echo ========================================
) else (
    echo ========================================
    echo Rebuild failed with error code %ERRORLEVEL%
    echo ========================================
)
pause
