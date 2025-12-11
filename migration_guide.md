# Fnc_Vision_Pre → Fnc_Vision_Pre_FITO 마이그레이션 가이드

## 개요
Halcon 기반 비전 시스템에서 OpenCV 왜곡 보정 시스템으로 전환하는 방법을 설명합니다.

## 마이그레이션 체크리스트

### 1. 헤더 파일 변경
```cpp
// 기존
#include "Fnc_Vision_Pre.h"

// 신규
#include "Fnc_Vision_Pre_FITO.h"
```

### 2. 클래스 인스턴스 변경
```cpp
// 기존
Fnc_Vision_Pre* pVision = new Fnc_Vision_Pre();

// 신규
Fnc_Vision_Pre_FITO* pVision = new Fnc_Vision_Pre_FITO();
```

### 3. 캘리브레이션 파일 준비
```
경로: D:\Execute\SCTC_CUTTER\dat\CutterConfig\OpenCV_Calibration_PreVision.xml
```

**파일 생성 방법**
- OpenCV 캘리브레이션 툴 사용
- 체커보드 패턴으로 캘리브레이션
- XML 파일 저장

### 4. 제거된 기능 대응

#### 4.1 명령어 시스템
**제거된 함수**
- `RegistCommand()`
- `Act_Initial()`
- `Act_ModelChange()`
- `Act_Grab()`
- `Act_GrabContinous()`
- `Act_StopGrabContinous()`
- `Act_TestResultClear()`
- `Act_FindPreVision()`

**대응 방안**
- 별도의 명령어 처리 클래스 구현
- FITO 클래스는 순수 이미지 전처리만 담당

#### 4.2 템플릿 매칭
**제거된 함수**
- `HV_PreVisionCreateTeamplate()`
- `HV_PreVisionFindCornerTeamplate()`

**대응 방안**
- Halcon 템플릿 매칭은 별도 클래스로 분리
- OpenCV 템플릿 매칭 구현 (선택사항)

#### 4.3 파라미터 관리
**제거된 함수**
- `Saveparameter()`
- `LoadParameter()`

**대응 방안**
- XML 기반 파라미터 관리로 대체
- 캘리브레이션 파일에 통합

## 코드 변경 가이드

### 1. 초기화 코드

#### 기존 (Fnc_Vision_Pre.cpp)
```cpp
Fnc_Vision_Pre* pVision = new Fnc_Vision_Pre();
pVision->InitialBoard();
pVision->LoadParameter();

// 템플릿 생성
pVision->HV_PreVisionCreateTeamplate(_RU);
pVision->HV_PreVisionCreateTeamplate(_RL);
pVision->HV_PreVisionCreateTeamplate(_LU);
pVision->HV_PreVisionCreateTeamplate(_LL);
```

#### 신규 (Fnc_Vision_Pre_FITO.cpp)
```cpp
Fnc_Vision_Pre_FITO* pVision = new Fnc_Vision_Pre_FITO();
pVision->InitialBoard();  // 캘리브레이션 자동 로드
```

### 2. 이미지 획득 코드

#### 기존
```cpp
// 단일 Grab
pVision->Grab();
pVision->Paint();

// 연속 Grab
pVision->Act_GrabContinous("");
// ...
pVision->Act_StopGrabContinous("");
```

#### 신규
```cpp
// 단일 Grab
pVision->Grab();
pVision->Paint();

// 연속 Grab (직접 구현 필요)
m_bGrabContinous = true;
while (m_bGrabContinous)
{
    pVision->Grab();
    pVision->Paint(true);
    Sleep(30);
}
```

### 3. 비전 검사 코드

#### 기존
```cpp
// 템플릿 매칭
VisionResult result = pVision->HV_PreVisionFindCornerTeamplate(_RU);
if (result.bResult)
{
    double posX = result.dPositionX;
    double posY = result.dPositionY;
}
```

#### 신규
```cpp
// FITO 버전은 템플릿 매칭 미포함
// 별도의 비전 검사 클래스 사용 또는
// OpenCV 기반 템플릿 매칭 직접 구현

// 예: OpenCV 템플릿 매칭
cv::Mat img(height, width, CV_8UC1, pCamImg);
cv::Mat templ = cv::imread("template.bmp", cv::IMREAD_GRAYSCALE);
cv::Mat result;
cv::matchTemplate(img, templ, result, cv::TM_CCOEFF_NORMED);

double minVal, maxVal;
cv::Point minLoc, maxLoc;
cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
```

### 4. 왜곡 보정 사용

#### 신규 기능
```cpp
// 왜곡 보정은 Grab() 함수에서 자동 적용
pVision->Grab();  // 내부에서 UndistortImage() 호출

// 좌표 왜곡 보정 (검사 결과 좌표 보정용)
double correctedX, correctedY;
pVision->UndistortPoint(distortedX, distortedY, correctedX, correctedY);
```

### 5. Edge/Line 검출

#### 신규 기능
```cpp
// Canny Edge 검출
BYTE* pEdgeImg = new BYTE[width * height];
pVision->CV_ExtractEdgesCanny(pCamImg, pEdgeImg, width, height, 50, 150);

// LSD 직선 검출
std::vector<cv::Vec4f> lines;
pVision->CV_ExtractLinesLSD(pCamImg, lines, width, height);

// Hough 변환
std::vector<cv::Vec4f> houghLines;
pVision->CV_HoughLinesP(pEdgeImg, houghLines, width, height);
```

## 성능 최적화

### 1. 왜곡 보정 맵 재사용
```cpp
// 초기화 시 한 번만 실행
pVision->InitialBoard();  // InitUndistortMaps() 자동 호출

// 이후 Grab()에서 빠른 remap 사용
pVision->Grab();
```

### 2. 메모리 관리
```cpp
// 임시 버퍼 자동 정리
pVision->Grab();  // 내부에서 pUndistortedImg 자동 delete
```

### 3. 이미지 크기 변경
```cpp
// 이미지 크기 변경 시 맵 자동 재생성
pVision->UndistortImage(pSrc, pDst, newWidth, newHeight);
```

## 에러 처리

### 1. 캘리브레이션 파일 없음
```cpp
// InitialBoard() 후 확인
if (!pVision->m_bUseCalibration)
{
    // 캘리브레이션 없음 경고
    MessageBox("캘리브레이션 파일이 없습니다. Raw 이미지 모드로 동작합니다.");
}
```

### 2. 왜곡 보정 실패
```cpp
// Grab() 반환값 확인
if (!pVision->Grab())
{
    // 이미지 획득 실패
    MessageBox("이미지 획득 실패");
}
```

## 호환성 이슈

### 1. Halcon 함수 제거
| 제거된 함수 | 대체 방안 |
|-----------|----------|
| `ReadImage()` | `cv::imread()` |
| `ZoomImageSize()` | `cv::resize()` |
| `LinesGauss()` | `CV_ExtractLinesLSD()` |
| `FitLineContourXld()` | `CV_FitLineRobust()` |
| `FindShapeModel()` | `cv::matchTemplate()` |

### 2. 데이터 타입 변환
```cpp
// Halcon HObject → OpenCV Mat
HObject ho_Image;
HTuple hv_ptr, hv_typ, hv_w, hv_h;
GetImagePointer1(ho_Image, &hv_ptr, &hv_typ, &hv_w, &hv_h);
cv::Mat cvImg(hv_h.L(), hv_w.L(), CV_8UC1, (BYTE*)hv_ptr.L());

// OpenCV Mat → BYTE 배열
memcpy(pDst, cvImg.data, width * height);
```

## 테스트 시나리오

### 1. 기본 동작 테스트
```cpp
// 1. 초기화
Fnc_Vision_Pre_FITO* pVision = new Fnc_Vision_Pre_FITO();
ASSERT(pVision->InitialBoard() == PASS);

// 2. 캘리브레이션 확인
ASSERT(pVision->m_bUseCalibration == true);
ASSERT(pVision->m_bCalibrated == true);

// 3. 이미지 획득
ASSERT(pVision->Grab() == true);

// 4. 왜곡 보정 확인
// (시각적 검사 필요)
```

### 2. 왜곡 보정 검증
```cpp
// 체커보드 이미지로 테스트
pVision->ImageOpen("checkerboard.bmp");
pVision->Grab();

// 보정 전/후 이미지 비교
// - 직선이 직선으로 보정되는지 확인
// - 모서리 왜곡이 제거되는지 확인
```

### 3. 성능 테스트
```cpp
// 1000번 반복 Grab
DWORD startTime = GetTickCount();
for (int i = 0; i < 1000; i++)
{
    pVision->Grab();
}
DWORD elapsedTime = GetTickCount() - startTime;

// 평균 처리 시간 확인
// 목표: < 20ms/frame
```

## 롤백 계획

### 1. FITO 버전 문제 발생 시
```cpp
// Fnc_Vision_Pre.cpp로 복원
#ifdef USE_FITO_VERSION
    #include "Fnc_Vision_Pre_FITO.h"
    typedef Fnc_Vision_Pre_FITO VisionClass;
#else
    #include "Fnc_Vision_Pre.h"
    typedef Fnc_Vision_Pre VisionClass;
#endif

VisionClass* pVision = new VisionClass();
```

### 2. 하이브리드 모드
```cpp
// 두 클래스 동시 사용
Fnc_Vision_Pre* pVisionHalcon = new Fnc_Vision_Pre();
Fnc_Vision_Pre_FITO* pVisionFITO = new Fnc_Vision_Pre_FITO();

// 이미지 획득 + 왜곡 보정: FITO
pVisionFITO->Grab();

// 템플릿 매칭: Halcon
// pCamImg를 Halcon 클래스로 전달
VisionResult result = pVisionHalcon->HV_PreVisionFindCornerTeamplate(_RU);
```

## 참고 자료

### 1. OpenCV 캘리브레이션
- [OpenCV Camera Calibration Tutorial](https://docs.opencv.org/4.x/dc/dbb/tutorial_py_calibration.html)
- 체커보드 패턴 다운로드: [OpenCV Chessboard Pattern](https://github.com/opencv/opencv/blob/master/doc/pattern.png)

### 2. 왜곡 모델
- 방사 왜곡: k1, k2, k3
- 접선 왜곡: p1, p2

### 3. 권장 캘리브레이션 설정
- 체커보드 크기: 9x6 또는 11x8
- 이미지 수: 최소 20장 이상
- 재투영 오차: < 0.5 픽셀

## FAQ

### Q1. 캘리브레이션 파일이 없으면?
**A:** Raw 이미지 모드로 동작합니다. 왜곡 보정은 건너뜁니다.

### Q2. 이미지 크기가 변경되면?
**A:** `UndistortImage()`에서 자동으로 맵을 재생성합니다.

### Q3. 성능이 느려지면?
**A:**
- 맵 기반 방식 사용 (최초 1회만 생성)
- CV_16SC2 타입 사용
- 메모리 복사 최소화

### Q4. Halcon 템플릿 매칭이 필요하면?
**A:**
- 별도 클래스로 분리
- 또는 OpenCV `matchTemplate()` 사용

### Q5. 기존 코드와 호환성?
**A:**
- 명령어 시스템 제거됨
- 템플릿 매칭 제거됨
- 순수 이미지 전처리만 제공

## 결론

**마이그레이션 권장 사항**
1. 왜곡 보정이 필요한 경우: FITO 버전 사용
2. 템플릿 매칭이 필요한 경우: 하이브리드 모드
3. 기존 시스템 유지: Fnc_Vision_Pre.cpp 사용

**FITO 버전의 장점**
- 렌즈 왜곡 보정으로 정확도 향상
- OpenCV 오픈소스 라이선스
- 다양한 Edge/Line 검출 알고리즘

**FITO 버전의 단점**
- 템플릿 매칭 미포함
- 명령어 시스템 제거
- 캘리브레이션 파일 필수
