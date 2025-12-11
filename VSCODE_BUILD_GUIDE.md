# VSCode에서 빌드하기

## 사전 요구사항

### 1. VSCode 설치
- **다운로드**: https://code.visualstudio.com/

### 2. 필수 확장 프로그램 설치
VSCode에서 다음 확장 프로그램을 설치하세요:

1. **C/C++** (Microsoft)
   - C++ IntelliSense, 디버깅, 코드 탐색 지원

2. **CMake Tools** (Microsoft)
   - CMake 프로젝트 빌드 및 디버깅 지원

3. **CMake** (twxs)
   - CMake 언어 지원 (syntax highlighting)

4. **XML** (Red Hat)
   - Qt .ui 파일 편집 지원

#### 확장 프로그램 설치 방법
```
1. VSCode 실행
2. Ctrl+Shift+X (확장 프로그램 뷰)
3. 검색창에 "C++" 입력 → "C/C++" 설치
4. 검색창에 "CMake Tools" 입력 → "CMake Tools" 설치
5. 검색창에 "CMake" 입력 → "CMake" 설치
6. 검색창에 "XML" 입력 → "XML" 설치
```

### 3. CMake 설치
- **다운로드**: https://cmake.org/download/
- **설치 시 주의**: "Add CMake to system PATH" 옵션 선택
- **확인**: 터미널에서 `cmake --version` 실행

### 4. Visual Studio Build Tools
- Visual Studio 2019 또는 Build Tools for Visual Studio 2019 필요
- C++ 빌드 도구 포함

### 5. Qt5 및 OpenCV
- Qt5 설치: `C:\Qt\5.15.2\msvc2019_64`
- OpenCV 설치: `C:\opencv\build`

## VSCode 프로젝트 설정

### 1. 프로젝트 폴더 열기
```
1. VSCode 실행
2. File → Open Folder...
3. D:\FITO_2026\Prevision 선택
```

### 2. .vscode 폴더 확인
프로젝트 루트에 `.vscode` 폴더와 다음 파일들이 있는지 확인:
- `settings.json` - VSCode 설정
- `launch.json` - 디버그 설정
- `tasks.json` - 빌드 작업 설정
- `c_cpp_properties.json` - IntelliSense 설정
- `extensions.json` - 권장 확장 프로그램

이미 생성되어 있습니다!

### 3. 경로 설정 수정 (필요시)
`.vscode/settings.json` 파일을 열고 경로 확인:

```json
{
    "cmake.configureSettings": {
        "CMAKE_PREFIX_PATH": "C:/Qt/5.15.2/msvc2019_64",  // Qt 경로
        "OpenCV_DIR": "C:/opencv/build"                    // OpenCV 경로
    }
}
```

실제 설치 경로와 다르면 수정하세요.

## 빌드 방법

### 방법 1: CMake Tools 사용 (권장)

#### 1) CMake 설정
```
1. Ctrl+Shift+P (명령 팔레트)
2. "CMake: Configure" 입력 → 선택
3. 빌드 도구 선택:
   - "Visual Studio Community 2019 Release - amd64" 선택
```

또는 VSCode 하단 상태바에서:
- **Kit 선택**: 🔧 아이콘 클릭 → "Visual Studio Community 2019 Release - amd64" 선택

#### 2) 빌드
```
1. Ctrl+Shift+P
2. "CMake: Build" 입력 → 선택
```

또는 VSCode 하단 상태바에서:
- **Build**: ⚙️ 아이콘 클릭 (또는 F7)

#### 3) 실행
```
1. Ctrl+Shift+P
2. "CMake: Run Without Debugging" 입력 → 선택
```

또는 VSCode 하단 상태바에서:
- **Run**: ▶️ 아이콘 클릭

### 방법 2: Tasks 사용

#### 1) 빌드
```
1. Ctrl+Shift+B (기본 빌드 작업 실행)
```

또는
```
1. Terminal → Run Build Task...
2. "CMake: build" 선택
```

#### 2) 실행
```
1. F5 (디버그 모드로 실행)
```

또는
```
1. Ctrl+F5 (디버그 없이 실행)
```

### 방법 3: 터미널 사용

#### 1) VSCode 내장 터미널 열기
```
Ctrl+` (백틱)
```

#### 2) CMake 설정
```bash
cmake -B build -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/5.15.2/msvc2019_64 -DOpenCV_DIR=C:/opencv/build
```

#### 3) 빌드
```bash
cmake --build build --config Release
```

#### 4) 실행
```bash
cd build\Release
VisionComparison.exe
```

## 상태바 사용법

VSCode 하단 상태바에는 CMake Tools 관련 버튼들이 표시됩니다:

```
[Kit: Visual Studio 2019] [Debug▼] [⚙️Build] [▶️Run] [🐛Debug]
```

- **Kit**: 빌드 도구 선택
- **Debug/Release**: 빌드 구성 선택
- **⚙️ Build**: 빌드 실행 (F7)
- **▶️ Run**: 실행 (Shift+F5)
- **🐛 Debug**: 디버그 모드 실행 (F5)

## 디버깅

### 1. 중단점 설정
```
1. 코드 줄 번호 왼쪽 클릭 → 빨간 점 표시
```

### 2. 디버그 시작
```
F5 (또는 Run → Start Debugging)
```

### 3. 디버그 제어
- **F5**: 계속 실행
- **F10**: 다음 줄로 이동 (Step Over)
- **F11**: 함수 내부로 이동 (Step Into)
- **Shift+F11**: 함수 밖으로 이동 (Step Out)
- **Shift+F5**: 디버그 중지

## IntelliSense 설정

### 자동 완성이 작동하지 않을 때

1. **설정 확인**
   ```
   Ctrl+Shift+P → "C/C++: Edit Configurations (UI)"
   ```

2. **Include Path 추가**
   - Configuration provider: "ms-vscode.cmake-tools" 선택
   - IntelliSense mode: "windows-msvc-x64" 선택

3. **IntelliSense 재시작**
   ```
   Ctrl+Shift+P → "C/C++: Restart IntelliSense for Active Document"
   ```

## 문제 해결

### 1. CMake 설정 실패
**증상**: "CMake: Configure" 실행 시 오류

**해결 방법**:
```bash
# build 폴더 삭제 후 재시도
rm -rf build
# 또는
rmdir /s /q build

# VSCode 재시작
```

### 2. Kit을 찾을 수 없음
**증상**: "No kit selected"

**해결 방법**:
```
1. Ctrl+Shift+P → "CMake: Scan for Kits"
2. Ctrl+Shift+P → "CMake: Select a Kit"
3. "Visual Studio Community 2019 Release - amd64" 선택
```

### 3. Qt5 not found
**증상**: CMake 오류 "Could not find a package configuration file provided by Qt5"

**해결 방법**:
```json
// .vscode/settings.json 수정
{
    "cmake.configureSettings": {
        "CMAKE_PREFIX_PATH": "C:/Qt/5.15.2/msvc2019_64"  // 실제 경로로 수정
    }
}
```

### 4. OpenCV not found
**증상**: CMake 오류 "Could not find OpenCV"

**해결 방법**:
```json
// .vscode/settings.json 수정
{
    "cmake.configureSettings": {
        "OpenCV_DIR": "C:/opencv/build"  // 실제 경로로 수정
    }
}
```

### 5. 실행 시 DLL not found
**증상**: "Qt5Core.dll was not found"

**해결 방법**:
```json
// .vscode/launch.json 수정
{
    "environment": [
        {
            "name": "PATH",
            "value": "C:/Qt/5.15.2/msvc2019_64/bin;C:/opencv/build/x64/vc15/bin;${env:PATH}"
        }
    ]
}
```

또는 windeployqt 사용:
```bash
cd build\Release
C:\Qt\5.15.2\msvc2019_64\bin\windeployqt.exe VisionComparison.exe
```

## 단축키 모음

### 빌드 관련
- **F7**: 빌드
- **Ctrl+Shift+B**: 빌드 작업 실행
- **Shift+F7**: 다시 빌드

### 실행 관련
- **F5**: 디버그 모드로 실행
- **Ctrl+F5**: 디버그 없이 실행
- **Shift+F5**: 디버그 중지

### 편집 관련
- **Ctrl+Space**: IntelliSense 트리거
- **F12**: 정의로 이동
- **Alt+F12**: 정의 미리보기
- **Shift+F12**: 참조 찾기

### 터미널
- **Ctrl+`**: 터미널 열기/닫기

### 명령 팔레트
- **Ctrl+Shift+P**: 명령 팔레트 열기

## CMake Output 확인

### 빌드 로그 보기
```
1. View → Output (Ctrl+Shift+U)
2. 드롭다운에서 "CMake/Build" 선택
```

### CMake 설정 로그 보기
```
1. View → Output
2. 드롭다운에서 "CMake" 선택
```

## UI 파일 편집

### Qt Designer에서 열기
```
1. .ui 파일 우클릭
2. "Reveal in File Explorer" 선택
3. Qt Designer로 파일 열기
```

### VSCode에서 XML 편집
```
1. .ui 파일 클릭 (XML 형식으로 표시됨)
2. 직접 XML 편집 가능
```

## 권장 워크플로우

### 1. 프로젝트 열기
```
File → Open Folder → D:\FITO_2026\Prevision
```

### 2. Kit 선택 (최초 1회)
```
상태바에서 🔧 아이콘 클릭 → Visual Studio 2019 선택
```

### 3. 빌드 구성 선택
```
상태바에서 [Debug▼] 클릭 → Release 선택
```

### 4. 빌드
```
F7 또는 Ctrl+Shift+B
```

### 5. 실행
```
Ctrl+F5 (디버그 없이)
또는
F5 (디버그 모드)
```

### 6. 코드 수정 후
```
저장 (Ctrl+S) → 빌드 (F7) → 실행 (Ctrl+F5)
```

## 추천 VSCode 설정

### settings.json에 추가하면 좋은 설정
```json
{
    "files.autoSave": "afterDelay",
    "files.autoSaveDelay": 1000,
    "editor.formatOnSave": true,
    "C_Cpp.formatting": "vcFormat",
    "cmake.buildBeforeRun": true
}
```

## 참고 자료

- VSCode CMake Tools: https://github.com/microsoft/vscode-cmake-tools
- VSCode C/C++: https://code.visualstudio.com/docs/languages/cpp
- CMake 공식 문서: https://cmake.org/documentation/

## 빠른 시작 체크리스트

- [ ] VSCode 설치
- [ ] 확장 프로그램 설치 (C/C++, CMake Tools)
- [ ] CMake 설치 및 PATH 설정
- [ ] 프로젝트 폴더 열기 (`D:\FITO_2026\Prevision`)
- [ ] `.vscode/settings.json` 경로 확인
- [ ] Kit 선택 (Visual Studio 2019)
- [ ] CMake Configure (Ctrl+Shift+P → CMake: Configure)
- [ ] Build (F7)
- [ ] Run (Ctrl+F5)

완료! 🎉
