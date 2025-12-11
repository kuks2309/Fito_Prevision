# 다음 단계 가이드

## 현재 상태

### 완료된 작업
- ✅ 프로젝트 구조 생성
- ✅ Qt5 UI 파일 생성 (MainWindow.ui, CameraSettingsDialog.ui, CalibrationSettingsDialog.ui)
- ✅ ImageViewer 커스텀 위젯 구현 (이미지 표시, 확대/축소, 그래픽 오버레이)
- ✅ MainWindow 클래스 구현 (메뉴, 제어 패널, 이벤트 핸들러)
- ✅ CMakeLists.txt 생성
- ✅ 문서화 (README.md, BUILD_GUIDE.md, comparison_table.md 등)

### 현재 구조
```
D:\FITO_2026\Prevision\
├── src\
│   ├── main.cpp              ✅ 완료
│   ├── MainWindow.h          ✅ 완료
│   ├── MainWindow.cpp        ✅ 완료
│   ├── ImageViewer.h         ✅ 완료
│   └── ImageViewer.cpp       ✅ 완료
├── ui\
│   ├── MainWindow.ui         ✅ 완료
│   ├── CameraSettingsDialog.ui   ✅ 완료
│   └── CalibrationSettingsDialog.ui  ✅ 완료
├── CMakeLists.txt            ✅ 완료
└── README.md                 ✅ 완료
```

## 다음 구현 단계

### 1단계: 기본 프로젝트 빌드 및 테스트
**목표**: 현재 코드를 빌드하고 UI가 정상 작동하는지 확인

#### 작업 항목
1. **CMake 설정 수정**
   ```cmake
   # CMakeLists.txt에서 주석 해제
   set(SOURCES
       src/main.cpp
       src/MainWindow.cpp
       src/ImageViewer.cpp
   )

   set(HEADERS
       src/MainWindow.h
       src/ImageViewer.h
   )

   set(UI_FILES
       ui/MainWindow.ui
       ui/CameraSettingsDialog.ui
       ui/CalibrationSettingsDialog.ui
   )
   ```

2. **빌드 및 테스트**
   ```bash
   cd D:\FITO_2026\Prevision
   mkdir build
   cd build
   cmake .. -G "Visual Studio 16 2019" -A x64
   cmake --build . --config Release

   # 실행
   cd Release
   VisionComparison.exe
   ```

3. **확인 사항**
   - [x] UI가 정상적으로 표시되는가?
   - [x] 이미지 로드 기능이 작동하는가?
   - [x] 이미지 저장 기능이 작동하는가?
   - [x] 확대/축소 (Ctrl+휠)가 작동하는가?
   - [x] 메뉴가 정상적으로 표시되는가?

### 2단계: Vision Wrapper 클래스 구현
**목표**: Fnc_Vision_Pre 및 Fnc_Vision_Pre_FITO 클래스를 Qt 환경에 통합

#### 작업 파일
- [ ] `src/VisionWrapper/VisionPreWrapper.h`
- [ ] `src/VisionWrapper/VisionPreWrapper.cpp`
- [ ] `src/VisionWrapper/VisionPreFITOWrapper.h`
- [ ] `src/VisionWrapper/VisionPreFITOWrapper.cpp`

#### 구현 내용
```cpp
// VisionPreWrapper.cpp
bool VisionPreWrapper::initialize()
{
    m_pVision = std::make_unique<Fnc_Vision_Pre>();
    return m_pVision->InitialBoard() == PASS;
}

bool VisionPreWrapper::grab()
{
    if (!m_pVision) return false;

    QElapsedTimer timer;
    timer.start();

    bool result = m_pVision->Grab();

    m_processTime = timer.elapsed();

    return result;
}

QImage VisionPreWrapper::getImage()
{
    // BYTE 배열 → QImage 변환
    QImage image(m_pVision->m_nSizeX, m_pVision->m_nSizeY, QImage::Format_Grayscale8);
    memcpy(image.bits(), m_pVision->pViewImg, m_pVision->m_nSizeX * m_pVision->m_nSizeY);
    return image;
}
```

### 3단계: 카메라 설정 다이얼로그 구현
**목표**: 카메라 파라미터 설정 UI 구현

#### 작업 파일
- [ ] `src/CameraSettingsDialog.h`
- [ ] `src/CameraSettingsDialog.cpp`

#### 구현 내용
- [ ] XML 파일에서 카메라 설정 로드
- [ ] UI에 파라미터 표시
- [ ] 사용자 입력 받아 XML 파일 저장
- [ ] 카메라 재초기화 기능

### 4단계: 캘리브레이션 설정 다이얼로그 구현
**목표**: OpenCV 캘리브레이션 파라미터 관리 UI 구현

#### 작업 파일
- [ ] `src/CalibrationSettingsDialog.h`
- [ ] `src/CalibrationSettingsDialog.cpp`

#### 구현 내용
- [ ] 캘리브레이션 XML 파일 로드
- [ ] 카메라 매트릭스 표시
- [ ] 왜곡 계수 표시
- [ ] 재투영 오차 표시
- [ ] 왜곡 보정 활성화/비활성화

### 5단계: 실시간 Grab 기능 구현
**목표**: 카메라에서 실시간 이미지 획득 및 표시

#### 작업 내용
- [ ] VisionWrapper에서 실제 Grab 호출
- [ ] QTimer를 사용한 연속 Grab 구현
- [ ] 이미지 업데이트 성능 최적화
- [ ] FPS 표시 기능 추가

### 6단계: 비교 분석 기능 구현
**목표**: Halcon vs OpenCV 성능 및 결과 비교

#### 작업 내용
- [ ] 처리 시간 측정 및 표시
- [ ] Edge 검출 개수 비교
- [ ] 템플릿 매칭 점수 비교 (Halcon만)
- [ ] 왜곡 보정 상태 표시
- [ ] 재투영 오차 표시

### 7단계: 추가 기능 구현
**목표**: 사용성 향상 기능 추가

#### 작업 내용
- [ ] 차이 맵 (Difference Map) 표시
- [ ] 히스토그램 비교
- [ ] ROI 설정 기능
- [ ] 배치 처리 기능
- [ ] 결과 CSV 출력

## 빌드 순서 (단계별)

### 1단계 빌드
```bash
# 현재 파일만으로 빌드
cd D:\FITO_2026\Prevision\build
cmake ..
cmake --build . --config Release
```

### 2단계 빌드 (Vision Wrapper 추가 후)
```cmake
# CMakeLists.txt에 추가
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/ImageViewer.cpp
    src/VisionWrapper/VisionPreWrapper.cpp
    src/VisionWrapper/VisionPreFITOWrapper.cpp
    D:/Execute/SCTC_CUTTER/.../Fnc_Vision_Pre.cpp
    D:/Execute/SCTC_CUTTER/.../Fnc_Vision_Pre_FITO.cpp
)
```

### 3단계 빌드 (다이얼로그 추가 후)
```cmake
set(SOURCES
    ...
    src/CameraSettingsDialog.cpp
    src/CalibrationSettingsDialog.cpp
)
```

## 테스트 계획

### 단계별 테스트
1. **UI 테스트**
   - 윈도우 크기 조절
   - 메뉴 동작 확인
   - 버튼 클릭 확인

2. **이미지 로드/저장 테스트**
   - BMP, PNG, JPG 파일 로드
   - 다양한 해상도 이미지 테스트
   - 저장 기능 확인

3. **Vision 통합 테스트**
   - 카메라 초기화
   - 이미지 Grab
   - 연속 Grab
   - 성능 측정

4. **비교 분석 테스트**
   - 동일 이미지로 Halcon vs OpenCV 비교
   - 왜곡 보정 전/후 비교
   - 처리 시간 비교

## 예상 일정 (참고용)

| 단계 | 작업 내용 | 예상 시간 |
|-----|---------|----------|
| 1단계 | 기본 빌드 및 UI 테스트 | 2~3시간 |
| 2단계 | Vision Wrapper 구현 | 1일 |
| 3단계 | 카메라 설정 다이얼로그 | 0.5일 |
| 4단계 | 캘리브레이션 설정 다이얼로그 | 0.5일 |
| 5단계 | 실시간 Grab 기능 | 1일 |
| 6단계 | 비교 분석 기능 | 1일 |
| 7단계 | 추가 기능 | 2~3일 |

**총 예상 기간**: 약 1~2주

## 현재 바로 할 수 있는 작업

### 즉시 실행 가능한 작업
1. **CMakeLists.txt 수정**
   - 주석 처리된 소스 파일 주석 해제
   - Qt5, OpenCV 경로 확인

2. **빌드 테스트**
   ```bash
   cd D:\FITO_2026\Prevision
   mkdir build
   cd build
   cmake .. -G "Visual Studio 16 2019" -A x64
   cmake --build . --config Release
   ```

3. **UI 테스트**
   - 프로그램 실행
   - 이미지 로드 테스트
   - 메뉴 동작 확인

4. **문서 검토**
   - README.md 읽기
   - BUILD_GUIDE.md 확인
   - comparison_table.md 검토

## 필요한 추가 파일

### 즉시 필요한 파일
- [ ] `src/VisionWrapper/VisionPreWrapper.h`
- [ ] `src/VisionWrapper/VisionPreWrapper.cpp`
- [ ] `src/VisionWrapper/VisionPreFITOWrapper.h`
- [ ] `src/VisionWrapper/VisionPreFITOWrapper.cpp`

### 향후 필요한 파일
- [ ] `src/CameraSettingsDialog.h/.cpp`
- [ ] `src/CalibrationSettingsDialog.h/.cpp`
- [ ] `src/DifferenceMapDialog.h/.cpp`
- [ ] `src/HistogramDialog.h/.cpp`

## 주의 사항

### 빌드 시 주의사항
1. Qt5 경로 확인 (`C:\Qt\5.15.2\msvc2019_64`)
2. OpenCV 경로 확인 (`C:\opencv\build`)
3. Halcon 경로 확인 (`C:\Program Files\MVTec\HALCON-20.11`)
4. MultiCam 경로 확인 (`C:\Program Files\Euresys\MultiCam`)

### 실행 시 주의사항
1. DLL 경로 확인 (Qt5, OpenCV, Halcon, MultiCam)
2. 캘리브레이션 XML 파일 경로 확인
3. 카메라 하드웨어 연결 확인

## 도움말

### 문제 발생 시 확인 사항
1. BUILD_GUIDE.md의 "빌드 문제 해결" 섹션 참고
2. CMakeLists.txt의 경로 설정 확인
3. 환경 변수 PATH 확인
4. Qt Creator 콘솔 출력 확인

### 참고 자료
- Qt5 문서: https://doc.qt.io/qt-5/
- OpenCV 문서: https://docs.opencv.org/
- CMake 문서: https://cmake.org/documentation/

## 진행 상황 체크리스트

### 1단계: 기본 빌드
- [ ] CMakeLists.txt 수정
- [ ] 빌드 성공
- [ ] 프로그램 실행
- [ ] UI 정상 표시
- [ ] 이미지 로드 테스트

### 2단계: Vision 통합
- [ ] VisionWrapper 클래스 작성
- [ ] Fnc_Vision_Pre 통합
- [ ] Fnc_Vision_Pre_FITO 통합
- [ ] Grab 기능 테스트

### 3단계: 설정 다이얼로그
- [ ] CameraSettingsDialog 구현
- [ ] CalibrationSettingsDialog 구현
- [ ] XML 로드/저장 기능

### 4단계: 고급 기능
- [ ] 비교 분석 기능
- [ ] 차이 맵 표시
- [ ] 히스토그램 비교

## 연락처 및 지원

프로젝트 관련 문의:
- GitHub Issues: (리포지토리 URL)
- 이메일: (담당자 이메일)

## 라이선스

- Qt5: LGPL / Commercial
- OpenCV: Apache 2.0
- Halcon: Commercial (라이선스 필요)
