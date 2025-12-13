@echo off
setlocal

REM Try to find MSBuild
set MSBUILD_2022_PRO="C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
set MSBUILD_2022_COM="C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
set MSBUILD_2022_ENT="C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"

if exist %MSBUILD_2022_PRO% (
    echo Found MSBuild 2022 Professional Edition
    %MSBUILD_2022_PRO% build\VisionComparison.sln /p:Configuration=Debug /m /v:minimal /nologo
    goto :end
)

if exist %MSBUILD_2022_COM% (
    echo Found MSBuild 2022 Community Edition
    %MSBUILD_2022_COM% build\VisionComparison.sln /p:Configuration=Debug /m /v:minimal /nologo
    goto :end
)

if exist %MSBUILD_2022_ENT% (
    echo Found MSBuild 2022 Enterprise Edition
    %MSBUILD_2022_ENT% build\VisionComparison.sln /p:Configuration=Debug /m /v:minimal /nologo
    goto :end
)

echo ERROR: MSBuild not found!
echo Please open build\VisionComparison.sln in Visual Studio and build manually.
pause
exit /b 1

:end
echo.
if %ERRORLEVEL% EQU 0 (
    echo Build completed successfully!
    echo Executable: build\Debug\VisionComparison.exe
) else (
    echo Build failed with error code %ERRORLEVEL%
)
pause
