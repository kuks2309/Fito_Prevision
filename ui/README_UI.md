# UI 파일 설명

## 개요
이 폴더는 Qt Designer로 작성된 UI 파일(.ui)을 포함합니다.

## UI 파일 목록

### 1. MainWindow.ui
**메인 윈도우 UI**

#### 주요 구성 요소
- **이미지 뷰어 영역** (좌우 분할)
  - `labelHalconImage`: Halcon 이미지 표시
  - `labelOpenCVImage`: OpenCV FITO 이미지 표시

- **제어 패널**
  - `btnLoadImage`: 이미지 파일 열기
  - `btnGrab`: 단일 Grab
  - `btnContinuousGrab`: 연속 Grab 시작
  - `btnStop`: 연속 Grab 정지
  - `comboViewMode`: 표시 모드 선택 (Raw/Gain/Binary)
  - `spinGain`: Gain 값 조절 (0.1 ~ 10.0)
  - `spinOffset`: Offset 값 조절 (-255 ~ 255)
  - `spinThreshold`: Binary Threshold 조절 (0 ~ 255)

- **분석 결과 패널**
  - 처리 시간 비교
  - Edge 검출 개수 비교
  - 템플릿 매칭 점수
  - 왜곡 보정 상태
  - 재투영 오차

#### 메뉴바
- **파일**: 이미지 열기, 저장, 종료
- **보기**: 확대, 축소, 화면 맞춤, 그리드 표시
- **분석**: 비교 분석, 차이 맵, 히스토그램
- **설정**: 카메라 설정, 캘리브레이션 설정, 환경 설정
- **도움말**: 사용자 매뉴얼, 정보

### 2. CameraSettingsDialog.ui
**카메라 설정 다이얼로그**

#### 주요 구성 요소
- **카메라 파라미터**
  - `spinBoardNo`: 보드 번호 (0~10)
  - `editChannel`: 채널 (예: "M")
  - `editCamFile`: CAM 파일 경로
  - `spinExposure`: 노출 시간 (μs)

- **이미지 크기**
  - `spinWidth`: 이미지 너비 (최대 8192)
  - `spinHeight`: 이미지 높이 (최대 8192)

- **이미지 변환**
  - `checkFlipX`: X축 반전
  - `checkFlipY`: Y축 반전
  - `checkRotate`: 회전 활성화
  - `spinRotateDegree`: 회전 각도 (-180° ~ 180°)

### 3. CalibrationSettingsDialog.ui
**캘리브레이션 설정 다이얼로그**

#### 주요 구성 요소
- **캘리브레이션 파일**
  - `editCalibrationFile`: 캘리브레이션 XML 파일 경로
  - `btnLoadCalibration`: 캘리브레이션 로드
  - `btnSaveCalibration`: 캘리브레이션 저장

- **캘리브레이션 상태**
  - `labelLoadedStatus`: 로드 상태 (로드됨/미로드)
  - `labelImageSize`: 이미지 크기
  - `labelReprojError`: 재투영 오차 (픽셀)

- **카메라 매트릭스**
  - `textCameraMatrix`: 3x3 카메라 내부 파라미터 표시 (읽기 전용)

- **왜곡 계수**
  - `textDistCoeffs`: 왜곡 계수 [k1, k2, p1, p2, k3] 표시 (읽기 전용)

- **설정**
  - `checkUseCalibration`: 왜곡 보정 사용 여부

## UI 파일 수정 방법

### Qt Designer 사용

1. **Qt Designer 실행**
   ```batch
   C:\Qt\5.15.2\msvc2019_64\bin\designer.exe
   ```

2. **UI 파일 열기**
   - `File` → `Open...`
   - 해당 .ui 파일 선택

3. **위젯 편집**
   - 드래그 앤 드롭으로 위젯 추가/삭제
   - 속성 편집기에서 속성 수정
   - 시그널-슬롯 연결

4. **저장**
   - `File` → `Save`

### Qt Creator 사용

1. **프로젝트 열기**
   - Qt Creator에서 `CMakeLists.txt` 또는 `.pro` 파일 열기

2. **UI 파일 편집**
   - 프로젝트 트리에서 .ui 파일 더블클릭
   - 통합 디자이너에서 편집

## UI 파일 사용 (C++ 코드)

### 헤더 파일
```cpp
// MainWindow.h
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;  // UI 포인터
};
```

### 구현 파일
```cpp
// MainWindow.cpp
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);  // UI 설정

    // 위젯 접근
    connect(ui->btnGrab, &QPushButton::clicked, this, &MainWindow::onGrab);
}

MainWindow::~MainWindow()
{
    delete ui;
}
```

## 위젯 명명 규칙

### 접두사
- `btn`: QPushButton (예: `btnLoadImage`)
- `label`: QLabel (예: `labelHalconTime`)
- `edit`: QLineEdit (예: `editCamFile`)
- `spin`: QSpinBox/QDoubleSpinBox (예: `spinGain`)
- `combo`: QComboBox (예: `comboViewMode`)
- `check`: QCheckBox (예: `checkFlipX`)
- `text`: QTextEdit (예: `textCameraMatrix`)

### 이름 규칙
- 명확하고 설명적인 이름 사용
- 카멜 케이스 사용
- 용도를 나타내는 접미사 추가 (예: `Dialog`, `Panel`, `Widget`)

## 스타일시트 (선택사항)

### CSS 스타일 적용
```cpp
// C++ 코드에서 적용
ui->labelLoadedStatus->setStyleSheet("color: green; font-weight: bold;");

// 또는 .ui 파일의 속성 편집기에서 styleSheet 속성 설정
```

### 테마 적용
```cpp
// main.cpp
app.setStyle("Fusion");
```

## 다국어 지원 (선택사항)

### .ts 파일 생성
```xml
<!-- CMakeLists.txt 또는 .pro 파일에 추가 -->
<lupdate>
  <sources>
    <file>src/MainWindow.cpp</file>
  </sources>
  <translations>
    <file>translations/app_ko.ts</file>
    <file>translations/app_en.ts</file>
  </translations>
</lupdate>
```

### Qt Linguist 사용
```batch
C:\Qt\5.15.2\msvc2019_64\bin\linguist.exe
```

## 리소스 파일 (선택사항)

### .qrc 파일 생성
```xml
<!-- resources.qrc -->
<RCC>
  <qresource prefix="/">
    <file>icons/camera.png</file>
    <file>icons/load.png</file>
  </qresource>
</RCC>
```

### UI에서 사용
```xml
<!-- .ui 파일에서 -->
<property name="icon">
  <iconset resource="resources.qrc">
    <normaloff>:/icons/camera.png</normaloff>
  </iconset>
</property>
```

## UI 미리보기

### Qt Designer에서
- `Form` → `Preview` (Ctrl+R)

### Qt Creator에서
- UI 파일 열기 → `Tools` → `Form Editor` → `Preview`

## 권장 레이아웃

### 반응형 디자인
- `QVBoxLayout`, `QHBoxLayout`, `QGridLayout` 사용
- Spacer 활용하여 공간 관리
- `minimumSize`, `maximumSize` 속성 설정

### 크기 정책 (Size Policy)
- `Expanding`: 가능한 공간 확장
- `Preferred`: 기본 크기 유지
- `Fixed`: 고정 크기

## 참고 자료

- Qt Designer 매뉴얼: https://doc.qt.io/qt-5/qtdesigner-manual.html
- Qt Widgets: https://doc.qt.io/qt-5/qtwidgets-index.html
- Qt Style Sheets: https://doc.qt.io/qt-5/stylesheet.html
