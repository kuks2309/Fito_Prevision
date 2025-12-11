# 함수별 상세 비교표

## 생성자 비교

| 항목 | Fnc_Vision_Pre.cpp | Fnc_Vision_Pre_FITO.cpp | 차이점 |
|------|-------------------|------------------------|--------|
| pGainImg | 선언 안 됨 | `pGainImg = nullptr` | FITO에서 추가 |
| pBinImg | 선언 안 됨 | `pBinImg = nullptr` | FITO에서 추가 |
| OpenCV 캘리브레이션 변수 | 없음 | `m_bCalibrated = false`<br>`m_bMapsInitialized = false`<br>`m_dReprojectionError = 0.0`<br>`m_bUseCalibration = false` | FITO 전용 |

## InitialBoard() 비교

### 공통 부분
- MultiCam 초기화
- XML 파라미터 로드
- 공유 메모리 생성
- 이미지 버퍼 할당

### 차이점

| 기능 | Fnc_Vision_Pre.cpp | Fnc_Vision_Pre_FITO.cpp |
|------|-------------------|------------------------|
| 콜백 등록 | `McRegisterCallback(m_Channel, GlobalCallbackPre, this)` | `McRegisterCallback(m_Channel, GlobalCallbackPre_FITO, this)` |
| 템플릿 생성 | `HV_PreVisionCreateTeamplate(_RU)`<br>`HV_PreVisionCreateTeamplate(_RL)`<br>`HV_PreVisionCreateTeamplate(_LU)`<br>`HV_PreVisionCreateTeamplate(_LL)` | 없음 |
| 캘리브레이션 로드 | 없음 | `LoadCalibration(strCalibPath)` |

## Grab() 함수 비교

### Fnc_Vision_Pre.cpp 처리 순서
1. Frame Grabber 트리거
2. 이미지 수신 대기
3. **회전 처리**
4. Halcon 전처리 (`HV_ImgProcess_Prepare2`)
5. 뷰 모드별 이미지 선택

### Fnc_Vision_Pre_FITO.cpp 처리 순서
1. Frame Grabber 트리거
2. 이미지 수신 대기
3. **OpenCV 왜곡 보정** (새로 추가)
4. **회전 처리** (보정된 이미지 사용)
5. OpenCV 전처리 (`CV_ImgProcess_Prepare`)
6. 뷰 모드별 이미지 선택

### 코드 비교

#### Fnc_Vision_Pre.cpp
```cpp
if (m_nRotate)
{
    SwapCoordinate(pOriginalImg, pCamImg, m_nRealSizeX, m_nRealSizeY, (double)m_nRotateDegree);
}
else
{
    memcpy(pCamImg, pOriginalImg, m_nSizeX * m_nSizeY);
}

RECT fullRect;
fullRect.left = 0;
fullRect.top = 0;
fullRect.right = m_nSizeX - 1;
fullRect.bottom = m_nSizeY - 1;

HV_ImgProcess_Prepare2(pCamImg, pGainImg, pBinImg);
```

#### Fnc_Vision_Pre_FITO.cpp
```cpp
// 1단계: OpenCV 왜곡 보정
BYTE* pUndistortedImg = nullptr;

if (m_bUseCalibration && m_bCalibrated)
{
    pUndistortedImg = new BYTE[m_nRealSizeX * m_nRealSizeY];
    bool bSuccess = UndistortImage(pOriginalImg, pUndistortedImg, m_nRealSizeX, m_nRealSizeY);

    if (!bSuccess)
    {
        memcpy(pUndistortedImg, pOriginalImg, m_nRealSizeX * m_nRealSizeY);
    }
}
else
{
    pUndistortedImg = pOriginalImg;
}

// 2단계: 회전 처리
if (m_nRotate)
{
    SwapCoordinate(pUndistortedImg, pCamImg, m_nRealSizeX, m_nRealSizeY, (double)m_nRotateDegree);
}
else
{
    memcpy(pCamImg, pUndistortedImg, m_nSizeX * m_nSizeY);
}

// 임시 버퍼 정리
if (m_bUseCalibration && m_bCalibrated && pUndistortedImg != nullptr)
{
    delete[] pUndistortedImg;
}

// 전처리
CV_ImgProcess_Prepare(pCamImg, pGainImg, pBinImg);
```

## ImageOpen() 비교

| 라이브러리 | Fnc_Vision_Pre.cpp | Fnc_Vision_Pre_FITO.cpp |
|-----------|-------------------|------------------------|
| 사용 라이브러리 | Halcon | OpenCV |
| 이미지 로드 | `ReadImage(&ho_Image, strFile.c_str())` | `cv::imread(strFile, cv::IMREAD_GRAYSCALE)` |
| 크기 조정 | `ZoomImageSize(ho_Image, &ho_Image, m_nSizeX, m_nSizeY, "constant")` | `cv::resize(image, resized, cv::Size(m_nSizeX, m_nSizeY))` |
| 버퍼 복사 | `memcpy(pCamImg, (BYTE*)hv_ptr.L(), m_nSizeX * m_nSizeY)` | `memcpy(pCamImg, resized.data, m_nSizeX * m_nSizeY)` |
| 예외 처리 | 없음 | `try-catch (cv::Exception)` |

## 함수 존재 여부 비교

| 함수명 | Fnc_Vision_Pre.cpp | Fnc_Vision_Pre_FITO.cpp | 비고 |
|--------|-------------------|------------------------|------|
| `RegistCommand()` | O | X | 명령어 등록 |
| `InitCode()` | O | X | 초기화 |
| `LoadIOConfig()` | O | X | IO 설정 |
| `PreCode()` / `PostCode()` | O | X | 전/후처리 |
| `FunctionAbort()` | O | X | 중단 |
| `Callback()` | O | O | 동일 |
| `Grab()` | O | O | 로직 다름 |
| `Paint()` | O | O | 동일 |
| `SetImageMode()` | O | O | 동일 |
| `ImageOpen()` | O | O | 라이브러리 다름 |
| `InitialBoard()` | O | O | 로직 다름 |
| `InitialCrevisCam()` | O | O | FITO는 주석 처리 |
| `HV_PreVisionCreateTeamplate()` | O | X | Halcon 템플릿 생성 |
| `HV_PreVisionFindCornerTeamplate()` | O | X | 템플릿 매칭 |
| `SavePreImage()` | O | X | 이미지 저장 |
| `Saveparameter()` | O | X | 파라미터 저장 |
| `LoadParameter()` | O | X | 파라미터 로드 |
| `HV_ImgProcess_Prepare()` | O | X | Halcon 전처리 |
| `HV_ImgProcess_Prepare2()` | O | X | Halcon 전처리 |
| `SwapCoordinate()` | O | O | 동일 |
| `Act_Initial()` | O | X | Action |
| `Act_ModelChange()` | O | X | Action |
| `Act_Grab()` | O | X | Action |
| `Act_GrabContinous()` | O | X | Action |
| `Act_StopGrabContinous()` | O | X | Action |
| `Act_TestResultClear()` | O | X | Action |
| `Act_FindPreVision()` | O | X | Action |
| `GetVisionResult()` | O | X | 결과 반환 |
| `CreateFolder()` | O | X | 폴더 생성 |
| `LoadCalibration()` | X | O | OpenCV 캘리브레이션 로드 |
| `SaveCalibration()` | X | O | OpenCV 캘리브레이션 저장 |
| `InitUndistortMaps()` | X | O | 왜곡 보정 맵 생성 |
| `UndistortImage()` | X | O | 이미지 왜곡 보정 |
| `UndistortPoint()` | X | O | 좌표 왜곡 보정 |
| `SetCalibrationParams()` | X | O | 캘리브레이션 파라미터 설정 |
| `CV_ExtractEdgesCanny()` | X | O | Canny Edge 검출 |
| `CV_ExtractLinesLSD()` | X | O | LSD 직선 검출 |
| `CV_FitLineRobust()` | X | O | Robust Line Fitting |
| `CV_HoughLinesStandard()` | X | O | 표준 Hough 변환 |
| `CV_HoughLinesP()` | X | O | 확률적 Hough 변환 |
| `CV_ConvertHoughLinesToCartesian()` | X | O | 좌표계 변환 |
| `CV_ImgProcess_Prepare()` | X | O | OpenCV 전처리 |

## 총 함수 개수

| 파일 | 총 함수 개수 | 고유 함수 | 공통 함수 |
|------|-------------|----------|----------|
| Fnc_Vision_Pre.cpp | 약 27개 | 18개 | 9개 |
| Fnc_Vision_Pre_FITO.cpp | 약 22개 | 13개 | 9개 |

## 코드 라인 수

| 파일 | 라인 수 |
|------|---------|
| Fnc_Vision_Pre.cpp | 1,706 라인 |
| Fnc_Vision_Pre_FITO.cpp | 1,143 라인 |

## 핵심 차이 요약

1. **Fnc_Vision_Pre.cpp**
   - Halcon 중심 비전 시스템
   - 템플릿 매칭 기반 코너 검출
   - 명령어 처리 시스템 포함
   - Tape Inspection 기능

2. **Fnc_Vision_Pre_FITO.cpp**
   - OpenCV 캘리브레이션 왜곡 보정 전용
   - 이미지 전처리에 집중
   - 명령어 처리 시스템 제거
   - Edge/Line 검출 알고리즘 추가
