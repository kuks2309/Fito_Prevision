# Vision Pre 파일 비교 프로젝트

## 프로젝트 개요
- 목적: Fnc_Vision_Pre.cpp와 Fnc_Vision_Pre_FITO.cpp 비교 분석
- 작성일: 2025-12-11

## 비교 대상 파일

### 1. Fnc_Vision_Pre.cpp (기존)
- 경로: `D:\Execute\SCTC_CUTTER\SCTCApplication\Src\ControlLogic\Cutter\PM_Function\Functional_Layer\Fnc_Vision_Pre.cpp`
- 라이브러리: Halcon
- 주요 기능: 비전 검사, 템플릿 매칭

### 2. Fnc_Vision_Pre_FITO.cpp (신규)
- 경로: `D:\Execute\SCTC_CUTTER\SCTCApplication\Src\ControlLogic\Cutter\PM_Function\Functional_Layer\Fnc_Vision_Pre_FITO.cpp`
- 라이브러리: OpenCV + Halcon 혼용
- 주요 기능: 비전 검사 + OpenCV 캘리브레이션 왜곡 보정

## 주요 차이점 요약

### 1. 초기화 변수 차이
| 항목 | Fnc_Vision_Pre.cpp | Fnc_Vision_Pre_FITO.cpp |
|------|-------------------|------------------------|
| pGainImg | 선언 안 됨 | 선언됨 |
| pBinImg | 선언 안 됨 | 선언됨 |
| OpenCV 캘리브레이션 변수 | 없음 | 추가됨 (m_bCalibrated, m_cvCameraMatrix 등) |

### 2. 콜백 함수 이름
- **Fnc_Vision_Pre.cpp**: `GlobalCallbackPre()`
- **Fnc_Vision_Pre_FITO.cpp**: `GlobalCallbackPre_FITO()`

### 3. InitialBoard() 함수
- **Fnc_Vision_Pre.cpp**:
  - 템플릿 모델 생성 (`HV_PreVisionCreateTeamplate`)
- **Fnc_Vision_Pre_FITO.cpp**:
  - OpenCV 캘리브레이션 XML 로드
  - 왜곡 보정 맵 초기화

### 4. Grab() 함수 - 핵심 차이
#### Fnc_Vision_Pre.cpp
```cpp
// 회전만 수행
if (m_nRotate)
{
    SwapCoordinate(pOriginalImg, pCamImg, m_nRealSizeX, m_nRealSizeY, (double)m_nRotateDegree);
}
else
{
    memcpy(pCamImg, pOriginalImg, m_nSizeX * m_nSizeY);
}

HV_ImgProcess_Prepare2(pCamImg, pGainImg, pBinImg);
```

#### Fnc_Vision_Pre_FITO.cpp
```cpp
// 1단계: OpenCV 왜곡 보정 먼저 수행
if (m_bUseCalibration && m_bCalibrated)
{
    pUndistortedImg = new BYTE[m_nRealSizeX * m_nRealSizeY];
    UndistortImage(pOriginalImg, pUndistortedImg, m_nRealSizeX, m_nRealSizeY);
}

// 2단계: 보정된 이미지를 회전
if (m_nRotate)
{
    SwapCoordinate(pUndistortedImg, pCamImg, m_nRealSizeX, m_nRealSizeY, (double)m_nRotateDegree);
}

CV_ImgProcess_Prepare(pCamImg, pGainImg, pBinImg);
```

### 5. 이미지 전처리 함수
- **Fnc_Vision_Pre.cpp**: `HV_ImgProcess_Prepare2()` - Halcon 전용
- **Fnc_Vision_Pre_FITO.cpp**: `CV_ImgProcess_Prepare()` - OpenCV 전용

### 6. ImageOpen() 함수
#### Fnc_Vision_Pre.cpp (Halcon)
```cpp
HObject ho_Image;
ReadImage(&ho_Image, strFile.c_str());
ZoomImageSize(ho_Image, &ho_Image, m_nSizeX, m_nSizeY, "constant");
```

#### Fnc_Vision_Pre_FITO.cpp (OpenCV)
```cpp
cv::Mat image = cv::imread(strFile, cv::IMREAD_GRAYSCALE);
cv::Mat resized;
cv::resize(image, resized, cv::Size(m_nSizeX, m_nSizeY), 0, 0, cv::INTER_LINEAR);
```

### 7. 추가된 OpenCV 전용 기능 (FITO 버전)
- `LoadCalibration()` - 캘리브레이션 파라미터 로드
- `SaveCalibration()` - 캘리브레이션 파라미터 저장
- `InitUndistortMaps()` - 왜곡 보정 맵 생성
- `UndistortImage()` - 이미지 왜곡 보정
- `UndistortPoint()` - 좌표 왜곡 보정
- `CV_ExtractEdgesCanny()` - Canny Edge 검출
- `CV_ExtractLinesLSD()` - LSD 직선 검출
- `CV_FitLineRobust()` - Robust Line Fitting
- `CV_HoughLinesStandard()` - 표준 Hough 변환
- `CV_HoughLinesP()` - 확률적 Hough 변환
- `CV_ConvertHoughLinesToCartesian()` - 좌표계 변환

### 8. 비전 검사 로직
- **Fnc_Vision_Pre.cpp**:
  - `HV_PreVisionFindCornerTeamplate()` - Halcon 템플릿 매칭
  - 4개 코너(_RU, _RL, _LU, _LL) 템플릿 검사
  - Tape Inspection 기능 포함

- **Fnc_Vision_Pre_FITO.cpp**:
  - 템플릿 매칭 함수 없음 (주석 처리)
  - OpenCV 왜곡 보정에 집중

### 9. 삭제된 기능 (FITO 버전)
- `RegistCommand()` - 명령어 등록
- `InitCode()` - 초기화 코드
- `LoadIOConfig()` - IO 설정 로드
- `PreCode()` / `PostCode()` - 전/후처리
- `FunctionAbort()` - 중단 처리
- `HV_PreVisionCreateTeamplate()` - 템플릿 생성
- `HV_PreVisionFindCornerTeamplate()` - 코너 검사
- `SavePreImage()` - 이미지 저장
- `Saveparameter()` / `LoadParameter()` - 파라미터 관리
- 모든 `Act_*()` 함수들 (Initial, ModelChange, Grab 등)

## 아키텍처 차이

### Fnc_Vision_Pre.cpp (기존)
```
[Frame Grabber] → [Callback] → [회전] → [Halcon 전처리] → [템플릿 매칭] → [결과]
```

### Fnc_Vision_Pre_FITO.cpp (신규)
```
[Frame Grabber] → [Callback] → [OpenCV 왜곡 보정] → [회전] → [OpenCV 전처리] → [결과]
```

## 캘리브레이션 파일 경로
```
D:\Execute\SCTC_CUTTER\dat\CutterConfig\OpenCV_Calibration_PreVision.xml
```

## 결론

**Fnc_Vision_Pre_FITO.cpp**는 OpenCV 기반 렌즈 왜곡 보정 기능을 추가한 버전입니다.
- 기존 Halcon 템플릿 매칭 기능 제거
- OpenCV 캘리브레이션 왜곡 보정 추가
- 이미지 획득 파이프라인 개선 (왜곡 보정 → 회전)
- 명령어 처리 및 비전 검사 로직 제거 (순수 이미지 전처리에 집중)

## 참고 사항
- FITO 버전은 왜곡 보정 전용 클래스로 설계됨
- 비전 검사 로직은 별도 클래스로 분리된 것으로 추정
- OpenCV와 Halcon을 동시에 링크해야 함
