# OpenCV 캘리브레이션 기능 분석 (FITO 버전)

## 개요
Fnc_Vision_Pre_FITO.cpp에 추가된 OpenCV 기반 렌즈 왜곡 보정 기능을 상세 분석합니다.

## 캘리브레이션 변수

### 멤버 변수
```cpp
// 캘리브레이션 상태
bool m_bCalibrated;           // 캘리브레이션 완료 여부
bool m_bMapsInitialized;      // 왜곡 보정 맵 초기화 여부
bool m_bUseCalibration;       // 캘리브레이션 사용 여부
double m_dReprojectionError;  // 재투영 오차 (픽셀)

// OpenCV 캘리브레이션 파라미터
cv::Mat m_cvCameraMatrix;      // 카메라 내부 파라미터 (3x3)
cv::Mat m_cvDistCoeffs;        // 왜곡 계수 (k1, k2, p1, p2, k3, ...)
cv::Mat m_cvNewCameraMatrix;   // 최적화된 카메라 매트릭스
cv::Mat m_cvMapX;              // X 좌표 왜곡 보정 맵
cv::Mat m_cvMapY;              // Y 좌표 왜곡 보정 맵
```

## 캘리브레이션 파일 형식

### XML 파일 경로
```
D:\Execute\SCTC_CUTTER\dat\CutterConfig\OpenCV_Calibration_PreVision.xml
```

### XML 파일 구조 (예시)
```xml
<?xml version="1.0"?>
<opencv_storage>
  <camera_matrix type_id="opencv-matrix">
    <rows>3</rows>
    <cols>3</cols>
    <dt>d</dt>
    <data>
      fx 0 cx
      0 fy cy
      0 0 1
    </data>
  </camera_matrix>

  <distortion_coefficients type_id="opencv-matrix">
    <rows>1</rows>
    <cols>5</cols>
    <dt>d</dt>
    <data>k1 k2 p1 p2 k3</data>
  </distortion_coefficients>

  <image_width>4096</image_width>
  <image_height>3072</image_height>
  <reprojection_error>0.234</reprojection_error>
</opencv_storage>
```

## 주요 함수 분석

### 1. LoadCalibration()
```cpp
bool Fnc_Vision_Pre_FITO::LoadCalibration(const std::string& strFilePath)
```

**기능**
- XML 파일에서 캘리브레이션 파라미터 로드
- 카메라 매트릭스 및 왜곡 계수 검증
- 이미지 크기 확인 후 왜곡 보정 맵 생성

**처리 순서**
1. XML 파일 열기 (`cv::FileStorage`)
2. 카메라 매트릭스 읽기 (3x3)
3. 왜곡 계수 읽기 (최소 4개)
4. 이미지 크기 및 재투영 오차 읽기
5. 유효성 검사
6. 이미지 크기 일치 시 맵 생성

**반환**
- 성공: `true`
- 실패: `false`

### 2. InitUndistortMaps()
```cpp
void Fnc_Vision_Pre_FITO::InitUndistortMaps()
```

**기능**
- 왜곡 보정 맵 생성 (초기화 시 한 번만 실행)
- 반복 사용으로 성능 최적화

**처리 순서**
1. 최적화된 카메라 매트릭스 계산
   ```cpp
   m_cvNewCameraMatrix = cv::getOptimalNewCameraMatrix(
       m_cvCameraMatrix,
       m_cvDistCoeffs,
       imageSize,
       1.0,  // alpha = 1.0: 모든 픽셀 유지
       imageSize
   );
   ```

2. 왜곡 보정 맵 생성
   ```cpp
   cv::initUndistortRectifyMap(
       m_cvCameraMatrix,
       m_cvDistCoeffs,
       cv::Mat(),              // rectification (identity)
       m_cvNewCameraMatrix,
       imageSize,
       CV_16SC2,               // 16-bit signed, 성능 최적화
       m_cvMapX,
       m_cvMapY
   );
   ```

### 3. UndistortImage()
```cpp
bool Fnc_Vision_Pre_FITO::UndistortImage(
    const unsigned char* pSrc,
    unsigned char* pDst,
    int nWidth,
    int nHeight
)
```

**기능**
- BYTE 배열 이미지의 왜곡 보정
- 사전 생성된 맵 사용으로 빠른 처리

**처리 순서**
1. 캘리브레이션 상태 확인
2. 이미지 크기 변경 확인
   - 크기 변경 시 맵 재생성
3. BYTE 배열 → cv::Mat 래핑 (복사 없음)
   ```cpp
   cv::Mat srcMat(nHeight, nWidth, CV_8UC1, const_cast<unsigned char*>(pSrc));
   cv::Mat dstMat(nHeight, nWidth, CV_8UC1, pDst);
   ```
4. 왜곡 보정 적용
   ```cpp
   cv::remap(srcMat, dstMat, m_cvMapX, m_cvMapY, cv::INTER_LINEAR);
   ```

**성능**
- `cv::remap()` 사용으로 빠른 처리
- 메모리 복사 최소화 (래핑 방식)

### 4. UndistortPoint()
```cpp
bool Fnc_Vision_Pre_FITO::UndistortPoint(
    double x, double y,
    double& outX, double& outY
)
```

**기능**
- 왜곡된 좌표를 보정된 좌표로 변환
- 비전 검사 결과의 좌표 보정에 사용

**처리**
```cpp
cv::undistortPoints(
    srcPoints,
    dstPoints,
    m_cvCameraMatrix,
    m_cvDistCoeffs,
    cv::Mat(),
    m_cvNewCameraMatrix
);
```

## Grab() 함수의 왜곡 보정 파이프라인

### 처리 순서
```cpp
// 1단계: 이미지 획득
McSetParamInt(m_Channel, MC_ForceTrig, MC_ForceTrig_TRIG);
WaitForSingleObject(m_hProssed, 1000);

// 2단계: OpenCV 왜곡 보정
BYTE* pUndistortedImg = nullptr;
if (m_bUseCalibration && m_bCalibrated)
{
    pUndistortedImg = new BYTE[m_nRealSizeX * m_nRealSizeY];
    UndistortImage(pOriginalImg, pUndistortedImg, m_nRealSizeX, m_nRealSizeY);
}
else
{
    pUndistortedImg = pOriginalImg;
}

// 3단계: 회전 처리
if (m_nRotate)
{
    SwapCoordinate(pUndistortedImg, pCamImg, m_nRealSizeX, m_nRealSizeY, (double)m_nRotateDegree);
}

// 4단계: 전처리
CV_ImgProcess_Prepare(pCamImg, pGainImg, pBinImg);

// 5단계: 뷰 모드별 이미지 선택
if (VARI(...) == 0) memcpy(pViewImg, pCamImg, ...);      // Raw
else if (VARI(...) == 1) memcpy(pViewImg, pGainImg, ...); // Gain
else if (VARI(...) == 2) memcpy(pViewImg, pBinImg, ...);  // Binary
```

## 왜곡 보정 수학적 모델

### 카메라 매트릭스 (3x3)
```
[fx  0  cx]
[ 0 fy  cy]
[ 0  0   1]
```
- `fx`, `fy`: 초점 거리 (픽셀 단위)
- `cx`, `cy`: 주점 (Principal Point)

### 왜곡 계수
```
[k1, k2, p1, p2, k3, ...]
```
- `k1, k2, k3`: 방사 왜곡 (Radial Distortion)
- `p1, p2`: 접선 왜곡 (Tangential Distortion)

### 왜곡 보정 공식
```
x_distorted = x * (1 + k1*r^2 + k2*r^4 + k3*r^6) + 2*p1*x*y + p2*(r^2 + 2*x^2)
y_distorted = y * (1 + k1*r^2 + k2*r^4 + k3*r^6) + p1*(r^2 + 2*y^2) + 2*p2*x*y
```
여기서 `r^2 = x^2 + y^2`

## OpenCV Edge/Line 검출 함수

### 1. CV_ExtractEdgesCanny()
- Canny Edge 검출
- Gaussian Blur 전처리 포함
- 권장 파라미터:
  - 일반: `dLowThreshold=50, dHighThreshold=150`
  - 노이즈 많음: `dLowThreshold=100, dHighThreshold=200`

### 2. CV_ExtractLinesLSD()
- LSD (Line Segment Detector)
- Halcon의 `LinesGauss`와 유사
- 왜곡된 직선도 어느 정도 검출

### 3. CV_FitLineRobust()
- RANSAC 기반 Robust Line Fitting
- Halcon의 `FitLineContourXld("tukey")`와 유사
- `DIST_HUBER` 권장

### 4. CV_HoughLinesStandard()
- 표준 Hough 변환 (극좌표)
- Edge 이미지 입력 필요

### 5. CV_HoughLinesP()
- 확률적 Hough 변환 (직교좌표)
- 표준 Hough보다 빠름
- 직선 세그먼트 직접 반환

## 성능 최적화 기법

### 1. 맵 기반 왜곡 보정
- `InitUndistortMaps()`: 한 번만 실행
- `cv::remap()`: 빠른 변환

### 2. 메모리 복사 최소화
```cpp
// 복사 없이 BYTE 배열을 cv::Mat으로 래핑
cv::Mat srcMat(nHeight, nWidth, CV_8UC1, const_cast<unsigned char*>(pSrc));
```

### 3. 16-bit 맵 사용
```cpp
cv::initUndistortRectifyMap(..., CV_16SC2, ...);
```
- 32-bit float보다 빠름
- 정확도 충분

## 에러 처리

### 1. 캘리브레이션 파일 없음
```cpp
if (!fs.isOpened())
{
    return false;
}
```
- Raw 이미지 모드로 동작

### 2. 왜곡 보정 실패
```cpp
if (!bSuccess)
{
    memcpy(pUndistortedImg, pOriginalImg, ...);
}
```
- 원본 이미지 사용

### 3. 로그 출력
```cpp
WriteFunctionLogI(StringFormat("OpenCV Calibration Loaded: Error=%.3f pixels", m_dReprojectionError));
WriteFunctionLogW("OpenCV Calibration Not Found - Raw Image Mode");
```

## 사용 예제

### 캘리브레이션 로드
```cpp
std::string strCalibPath = "D:\\dat\\CutterConfig\\OpenCV_Calibration_PreVision.xml";
if (LoadCalibration(strCalibPath))
{
    m_bUseCalibration = true;
}
```

### 이미지 왜곡 보정
```cpp
BYTE* pUndistorted = new BYTE[width * height];
UndistortImage(pOriginal, pUndistorted, width, height);
```

### 좌표 왜곡 보정
```cpp
double correctedX, correctedY;
UndistortPoint(distortedX, distortedY, correctedX, correctedY);
```

## Halcon vs OpenCV 비교

| 기능 | Halcon (Fnc_Vision_Pre.cpp) | OpenCV (Fnc_Vision_Pre_FITO.cpp) |
|------|----------------------------|----------------------------------|
| 왜곡 보정 | 미지원 | 지원 (캘리브레이션 XML) |
| Edge 검출 | LinesGauss | Canny, LSD |
| Line Fitting | FitLineContourXld | FitLineRobust, Hough |
| 이미지 로드 | ReadImage | cv::imread |
| 템플릿 매칭 | FindShapeModel | 미지원 |
| 라이선스 | 상용 | 오픈소스 |

## 결론

**FITO 버전의 핵심 가치**
1. 렌즈 왜곡 보정으로 정확도 향상
2. OpenCV 기반 오픈소스 솔루션
3. 빠른 왜곡 보정 성능 (맵 기반)
4. 다양한 Edge/Line 검출 알고리즘

**주의 사항**
- 캘리브레이션 파일 필수
- 이미지 크기 변경 시 맵 재생성
- OpenCV 라이브러리 링크 필요
