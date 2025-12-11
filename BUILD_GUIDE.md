# Qt5 빌드 가이드

## 사전 요구사항

### 1. Qt5 설치
- **다운로드**: https://www.qt.io/download-qt-installer
- **권장 버전**: Qt 5.15.2 (LTS)
- **설치 구성요소**:
  - Qt 5.15.2 → MSVC 2019 64-bit
  - Qt Creator (IDE)
  - Qt Charts, Qt Data Visualization (선택사항)

### 2. Visual Studio 2019/2022
- **다운로드**: https://visualstudio.microsoft.com/downloads/
- **필수 워크로드**:
  - C++를 사용한 데스크톱 개발
  - CMake Tools

### 3. OpenCV
- **다운로드**: https://opencv.org/releases/
- **권장 버전**: OpenCV 4.5.5
- **설치 경로**: `C:\opencv`
- **환경 변수**: `OpenCV_DIR=C:\opencv\build`

### 4. Halcon (선택사항)
- MVTec HALCON 20.11 이상
- 라이선스 필요

### 5. MultiCam (선택사항)
- Euresys MultiCam SDK
- Frame Grabber 하드웨어 필요

## 빌드 환경 설정

### 1. 환경 변수 설정
```batch
# Qt5 경로
set Qt5_DIR=C:\Qt\5.15.2\msvc2019_64

# OpenCV 경로
set OpenCV_DIR=C:\opencv\build

# PATH에 추가
set PATH=%Qt5_DIR%\bin;%OpenCV_DIR%\x64\vc15\bin;%PATH%
```

### 2. CMake 설정 확인
```batch
cmake --version
# CMake 3.16 이상 필요
```

## 빌드 방법

### 방법 1: CMake + Visual Studio

#### 1) CMake 프로젝트 생성
```batch
cd D:\FITO_2026\Prevision
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
```

#### 2) Visual Studio에서 빌드
```batch
# Release 빌드
cmake --build . --config Release

# Debug 빌드
cmake --build . --config Debug
```

#### 3) 실행
```batch
cd Release
VisionComparison.exe
```

### 방법 2: Qt Creator

#### 1) Qt Creator 실행
- Qt Creator 실행
- `File` → `Open File or Project...`
- `D:\FITO_2026\Prevision\CMakeLists.txt` 선택

#### 2) 프로젝트 설정
- Kit 선택: Desktop Qt 5.15.2 MSVC2019 64bit
- Build Directory 설정

#### 3) 빌드 및 실행
- `Ctrl+B`: 빌드
- `Ctrl+R`: 실행

### 방법 3: qmake (.pro 파일)

#### 1) .pro 파일 생성 (선택사항)
```batch
# VisionComparison.pro 파일 생성
```

#### 2) qmake 실행
```batch
cd D:\FITO_2026\Prevision
qmake VisionComparison.pro

# Visual Studio nmake 사용
nmake

# 또는 mingw32-make 사용
mingw32-make
```

## 경로 설정 수정

### CMakeLists.txt 수정

```cmake
# Halcon 경로 수정
set(HALCON_DIR "C:/Program Files/MVTec/HALCON-20.11")

# MultiCam 경로 수정
set(MULTICAM_DIR "C:/Program Files/Euresys/MultiCam")

# Original vision sources 경로 수정
D:/Execute/SCTC_CUTTER/SCTCApplication/Src/ControlLogic/Cutter/PM_Function/Functional_Layer/Fnc_Vision_Pre.cpp
D:/Execute/SCTC_CUTTER/SCTCApplication/Src/ControlLogic/Cutter/PM_Function/Functional_Layer/Fnc_Vision_Pre_FITO.cpp
```

## 빌드 문제 해결

### 1. Qt5 not found
```
CMake Error: Could not find a package configuration file provided by "Qt5"
```

**해결 방법**:
```batch
set CMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2019_64
cmake ..
```

### 2. OpenCV not found
```
CMake Error: Could not find a package configuration file provided by "OpenCV"
```

**해결 방법**:
```batch
set OpenCV_DIR=C:\opencv\build
cmake ..
```

### 3. LNK2019 unresolved external symbol
```
error LNK2019: unresolved external symbol "class std::..."
```

**해결 방법**:
- Visual Studio 버전 확인 (Qt5와 동일한 컴파일러 사용)
- Qt5 MSVC2019 64bit 버전 사용
- CMakeLists.txt에서 런타임 라이브러리 설정 확인

### 4. Qt5Widgets.dll not found (실행 시)
```
The code execution cannot proceed because Qt5Widgets.dll was not found.
```

**해결 방법**:
```batch
# Qt5 bin 경로를 PATH에 추가
set PATH=C:\Qt\5.15.2\msvc2019_64\bin;%PATH%

# 또는 windeployqt 사용
cd build\Release
C:\Qt\5.15.2\msvc2019_64\bin\windeployqt.exe VisionComparison.exe
```

### 5. OpenCV DLL not found (실행 시)
```
opencv_world455.dll not found
```

**해결 방법**:
```batch
# OpenCV bin 경로를 PATH에 추가
set PATH=C:\opencv\build\x64\vc15\bin;%PATH%

# 또는 DLL을 실행 파일 디렉토리에 복사
copy C:\opencv\build\x64\vc15\bin\opencv_world455.dll build\Release\
```

## 배포 (Deployment)

### 1. windeployqt 사용
```batch
cd build\Release
C:\Qt\5.15.2\msvc2019_64\bin\windeployqt.exe VisionComparison.exe
```

### 2. 필요한 DLL 수동 복사
```
VisionComparison.exe
Qt5Core.dll
Qt5Gui.dll
Qt5Widgets.dll
opencv_world455.dll
halcon.dll (선택)
halconcpp.dll (선택)
MultiCam.dll (선택)
VirtualFG40CL.dll (선택)
```

### 3. MSVC 런타임 라이브러리
```
vcruntime140.dll
msvcp140.dll
```

### 4. Qt Plugins
```
platforms/
  qwindows.dll
styles/
  qwindowsvistastyle.dll
imageformats/
  qjpeg.dll
  qbmp.dll
  qpng.dll
```

## 디렉토리 구조 확인

빌드 전 다음 구조가 올바른지 확인:
```
D:\FITO_2026\Prevision\
├── CMakeLists.txt
├── src\
│   └── main.cpp
├── ui\
│   └── (UI 파일들)
├── resources\
│   └── (리소스 파일들)
└── build\
    └── (빌드 결과)
```

## 개발 환경 권장 설정

### Qt Creator 설정
1. `Tools` → `Options` → `Kits`
2. Desktop Qt 5.15.2 MSVC2019 64bit 선택
3. Compiler: Microsoft Visual C++ Compiler 16.0 (amd64)
4. Debugger: Auto-detected CDB

### CMake 설정
1. `Tools` → `Options` → `Kits` → `CMake`
2. CMake Tool 자동 감지 확인

## 추가 리소스

- Qt 공식 문서: https://doc.qt.io/qt-5/
- CMake 문서: https://cmake.org/documentation/
- OpenCV 문서: https://docs.opencv.org/
- HALCON 문서: https://www.mvtec.com/products/halcon/documentation

## 빌드 체크리스트

- [ ] Qt5 설치 완료
- [ ] Visual Studio 2019/2022 설치 완료
- [ ] OpenCV 설치 및 환경 변수 설정
- [ ] CMakeLists.txt 경로 수정
- [ ] 환경 변수 PATH 설정
- [ ] CMake 빌드 성공
- [ ] 실행 파일 생성 확인
- [ ] 필요한 DLL 복사
- [ ] 프로그램 정상 실행 확인
