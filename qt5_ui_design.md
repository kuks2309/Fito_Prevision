# Qt5 기반 비전 비교 UI 설계

## 프로젝트 개요
- 목적: Fnc_Vision_Pre.cpp와 Fnc_Vision_Pre_FITO.cpp 비교 분석 UI
- 프레임워크: Qt5
- 언어: C++

## UI 구성

### 메인 윈도우 레이아웃

```
+------------------------------------------------------------------+
|  Vision Pre Comparison Tool                          [_][□][X]  |
+------------------------------------------------------------------+
|  File  View  Analysis  Settings  Help                           |
+------------------------------------------------------------------+
|                                                                  |
|  +---------------------------+  +---------------------------+   |
|  | Fnc_Vision_Pre.cpp        |  | Fnc_Vision_Pre_FITO.cpp   |   |
|  | (Halcon 기반)             |  | (OpenCV 기반)             |   |
|  +---------------------------+  +---------------------------+   |
|  |                           |  |                           |   |
|  |  [이미지 뷰어]            |  |  [이미지 뷰어]            |   |
|  |                           |  |                           |   |
|  |  Width: 4096              |  |  Width: 4096              |   |
|  |  Height: 3072             |  |  Height: 3072             |   |
|  |  Camera: ON               |  |  Camera: ON               |   |
|  |                           |  |  Calibration: Loaded      |   |
|  |                           |  |  Reprojection Err: 0.234  |   |
|  +---------------------------+  +---------------------------+   |
|                                                                  |
|  +------------------------------------------------------------+  |
|  | 제어 패널                                                   |  |
|  +------------------------------------------------------------+  |
|  | [Load Image] [Grab] [Continuous Grab] [Stop]               |  |
|  | View Mode: [Raw] [Gain] [Binary]                           |  |
|  | Gain: [1.0]  Offset: [0]  Binary Threshold: [128]          |  |
|  +------------------------------------------------------------+  |
|                                                                  |
|  +------------------------------------------------------------+  |
|  | 비교 분석 결과                                              |  |
|  +------------------------------------------------------------+  |
|  | 처리 시간 (ms):     Halcon: 15.2    OpenCV: 12.8           |  |
|  | 왜곡 보정:          Halcon: N/A     OpenCV: 적용            |  |
|  | Edge 검출:          Halcon: 234개   OpenCV: 218개           |  |
|  | 템플릿 매칭 점수:   Halcon: 92.3%   OpenCV: N/A            |  |
|  +------------------------------------------------------------+  |
|                                                                  |
|  Status: Ready                                                   |
+------------------------------------------------------------------+
```

## Qt5 프로젝트 구조

### 디렉토리 구조
```
D:\FITO_2026\Prevision\
├── src\
│   ├── main.cpp
│   ├── MainWindow.h
│   ├── MainWindow.cpp
│   ├── VisionComparator.h
│   ├── VisionComparator.cpp
│   ├── ImageViewer.h
│   ├── ImageViewer.cpp
│   ├── ControlPanel.h
│   ├── ControlPanel.cpp
│   └── VisionWrapper\
│       ├── VisionPreWrapper.h
│       ├── VisionPreWrapper.cpp
│       ├── VisionPreFITOWrapper.h
│       └── VisionPreFITOWrapper.cpp
├── ui\
│   ├── MainWindow.ui
│   ├── ControlPanel.ui
│   └── AnalysisPanel.ui
├── resources\
│   ├── icons\
│   └── images\
├── CMakeLists.txt
├── VisionComparison.pro  (qmake)
└── README.md
```

## Qt5 클래스 설계

### 1. MainWindow 클래스
```cpp
// MainWindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include "ImageViewer.h"
#include "ControlPanel.h"
#include "VisionWrapper/VisionPreWrapper.h"
#include "VisionWrapper/VisionPreFITOWrapper.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadImage();
    void onGrab();
    void onContinuousGrab();
    void onStopGrab();
    void onViewModeChanged(int mode);
    void updateImages();
    void updateComparisonResults();

private:
    Ui::MainWindow *ui;

    // Image Viewers
    ImageViewer* m_pHalconViewer;
    ImageViewer* m_pOpenCVViewer;

    // Control Panel
    ControlPanel* m_pControlPanel;

    // Vision Wrappers
    VisionPreWrapper* m_pVisionHalcon;
    VisionPreFITOWrapper* m_pVisionFITO;

    // Timer for continuous grab
    QTimer* m_pGrabTimer;

    // Analysis data
    struct ComparisonData {
        double halconProcessTime;
        double opencvProcessTime;
        int halconEdgeCount;
        int opencvEdgeCount;
        double templateMatchScore;
        bool calibrationApplied;
    } m_comparisonData;
};

#endif // MAINWINDOW_H
```

### 2. ImageViewer 클래스
```cpp
// ImageViewer.h
#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>

class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);
    ~ImageViewer();

    void setImage(const QImage& image);
    void setImage(const unsigned char* data, int width, int height);
    void clearImage();

    void setTitle(const QString& title);
    void setInfo(const QString& info);

    void drawCross(const QPointF& center, int size, const QColor& color);
    void drawLine(const QPointF& p1, const QPointF& p2, const QColor& color);
    void drawRect(const QRectF& rect, const QColor& color);

signals:
    void imageClicked(const QPointF& pos);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QImage m_image;
    QString m_title;
    QString m_info;
    double m_zoomFactor;

    // Overlay graphics
    struct GraphicsItem {
        enum Type { Cross, Line, Rectangle } type;
        QPointF pos1;
        QPointF pos2;
        QRectF rect;
        QColor color;
    };
    QList<GraphicsItem> m_graphics;
};

#endif // IMAGEVIEWER_H
```

### 3. VisionPreWrapper 클래스
```cpp
// VisionPreWrapper.h
#ifndef VISIONPREWRAPPER_H
#define VISIONPREWRAPPER_H

#include <QObject>
#include <QImage>
#include <memory>

// Forward declaration
class Fnc_Vision_Pre;

class VisionPreWrapper : public QObject
{
    Q_OBJECT

public:
    explicit VisionPreWrapper(QObject *parent = nullptr);
    ~VisionPreWrapper();

    bool initialize();
    bool grab();
    QImage getImage();

    void setViewMode(int mode);  // 0: Raw, 1: Gain, 2: Binary
    void setGain(double gain);
    void setOffset(int offset);
    void setBinaryThreshold(int threshold);

    double getProcessTime() const { return m_processTime; }
    int getEdgeCount() const { return m_edgeCount; }
    double getTemplateMatchScore() const { return m_matchScore; }

signals:
    void imageUpdated();
    void errorOccurred(const QString& message);

private:
    std::unique_ptr<Fnc_Vision_Pre> m_pVision;
    double m_processTime;
    int m_edgeCount;
    double m_matchScore;
};

#endif // VISIONPREWRAPPER_H
```

### 4. VisionPreFITOWrapper 클래스
```cpp
// VisionPreFITOWrapper.h
#ifndef VISIONPREFITOWRAPPER_H
#define VISIONPREFITOWRAPPER_H

#include <QObject>
#include <QImage>
#include <memory>

// Forward declaration
class Fnc_Vision_Pre_FITO;

class VisionPreFITOWrapper : public QObject
{
    Q_OBJECT

public:
    explicit VisionPreFITOWrapper(QObject *parent = nullptr);
    ~VisionPreFITOWrapper();

    bool initialize();
    bool grab();
    QImage getImage();

    void setViewMode(int mode);
    void setGain(double gain);
    void setOffset(int offset);
    void setBinaryThreshold(int threshold);

    bool isCalibrationLoaded() const { return m_calibrationLoaded; }
    double getReprojectionError() const { return m_reprojectionError; }
    double getProcessTime() const { return m_processTime; }
    int getEdgeCount() const { return m_edgeCount; }

signals:
    void imageUpdated();
    void calibrationLoaded(double reprojectionError);
    void errorOccurred(const QString& message);

private:
    std::unique_ptr<Fnc_Vision_Pre_FITO> m_pVision;
    bool m_calibrationLoaded;
    double m_reprojectionError;
    double m_processTime;
    int m_edgeCount;
};

#endif // VISIONPREFITOWRAPPER_H
```

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(VisionComparison VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt5 설정
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)

# OpenCV 설정
find_package(OpenCV REQUIRED)

# Halcon 설정 (경로 수정 필요)
set(HALCON_DIR "C:/Program Files/MVTec/HALCON-20.11")
include_directories(${HALCON_DIR}/include)
link_directories(${HALCON_DIR}/lib/x64-win64)

# MultiCam 설정 (경로 수정 필요)
set(MULTICAM_DIR "C:/Program Files/Euresys/MultiCam")
include_directories(${MULTICAM_DIR}/include)
link_directories(${MULTICAM_DIR}/lib)

# 소스 파일
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/ImageViewer.cpp
    src/ControlPanel.cpp
    src/VisionWrapper/VisionPreWrapper.cpp
    src/VisionWrapper/VisionPreFITOWrapper.cpp

    # Original vision sources
    D:/Execute/SCTC_CUTTER/SCTCApplication/Src/ControlLogic/Cutter/PM_Function/Functional_Layer/Fnc_Vision_Pre.cpp
    D:/Execute/SCTC_CUTTER/SCTCApplication/Src/ControlLogic/Cutter/PM_Function/Functional_Layer/Fnc_Vision_Pre_FITO.cpp
)

set(HEADERS
    src/MainWindow.h
    src/ImageViewer.h
    src/ControlPanel.h
    src/VisionWrapper/VisionPreWrapper.h
    src/VisionWrapper/VisionPreFITOWrapper.h
)

set(UI_FILES
    ui/MainWindow.ui
    ui/ControlPanel.ui
    ui/AnalysisPanel.ui
)

# 실행 파일 생성
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
)

# 링크 라이브러리
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    ${OpenCV_LIBS}
    halcon.lib
    halconcpp.lib
    MultiCam.lib
    VirtualFG40CL.lib
)

# Include 디렉토리
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
    D:/Execute/SCTC_CUTTER/SCTCApplication/Src/ControlLogic/Cutter/PM_Function/Functional_Layer
)
```

## .pro 파일 (qmake 사용 시)

```qmake
# VisionComparison.pro
QT += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VisionComparison
TEMPLATE = app

CONFIG += c++17

# 소스 파일
SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/ImageViewer.cpp \
    src/ControlPanel.cpp \
    src/VisionWrapper/VisionPreWrapper.cpp \
    src/VisionWrapper/VisionPreFITOWrapper.cpp \
    D:/Execute/SCTC_CUTTER/SCTCApplication/Src/ControlLogic/Cutter/PM_Function/Functional_Layer/Fnc_Vision_Pre.cpp \
    D:/Execute/SCTC_CUTTER/SCTCApplication/Src/ControlLogic/Cutter/PM_Function/Functional_Layer/Fnc_Vision_Pre_FITO.cpp

HEADERS += \
    src/MainWindow.h \
    src/ImageViewer.h \
    src/ControlPanel.h \
    src/VisionWrapper/VisionPreWrapper.h \
    src/VisionWrapper/VisionPreFITOWrapper.h

FORMS += \
    ui/MainWindow.ui \
    ui/ControlPanel.ui \
    ui/AnalysisPanel.ui

# OpenCV
INCLUDEPATH += C:/opencv/build/include
LIBS += -LC:/opencv/build/x64/vc15/lib \
    -lopencv_world455

# Halcon
INCLUDEPATH += "C:/Program Files/MVTec/HALCON-20.11/include"
LIBS += -L"C:/Program Files/MVTec/HALCON-20.11/lib/x64-win64" \
    -lhalcon \
    -lhalconcpp

# MultiCam
INCLUDEPATH += "C:/Program Files/Euresys/MultiCam/include"
LIBS += -L"C:/Program Files/Euresys/MultiCam/lib" \
    -lMultiCam \
    -lVirtualFG40CL

# 추가 Include 경로
INCLUDEPATH += \
    D:/Execute/SCTC_CUTTER/SCTCApplication/Src/ControlLogic/Cutter/PM_Function/Functional_Layer
```

## main.cpp

```cpp
// main.cpp
#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 애플리케이션 정보 설정
    app.setApplicationName("Vision Pre Comparison Tool");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("FITO");

    // 메인 윈도우 생성
    MainWindow mainWindow;
    mainWindow.setWindowTitle("Vision Pre Comparison Tool");
    mainWindow.resize(1600, 900);
    mainWindow.show();

    return app.exec();
}
```

## MainWindow.cpp 구현 예제

```cpp
// MainWindow.cpp
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_pGrabTimer(new QTimer(this))
{
    ui->setupUi(this);

    // Image Viewers 생성
    m_pHalconViewer = new ImageViewer(this);
    m_pHalconViewer->setTitle("Halcon Vision");
    ui->halconViewerLayout->addWidget(m_pHalconViewer);

    m_pOpenCVViewer = new ImageViewer(this);
    m_pOpenCVViewer->setTitle("OpenCV FITO Vision");
    ui->opencvViewerLayout->addWidget(m_pOpenCVViewer);

    // Vision Wrappers 생성
    m_pVisionHalcon = new VisionPreWrapper(this);
    m_pVisionFITO = new VisionPreFITOWrapper(this);

    // 시그널-슬롯 연결
    connect(ui->btnLoadImage, &QPushButton::clicked, this, &MainWindow::onLoadImage);
    connect(ui->btnGrab, &QPushButton::clicked, this, &MainWindow::onGrab);
    connect(ui->btnContinuousGrab, &QPushButton::clicked, this, &MainWindow::onContinuousGrab);
    connect(ui->btnStop, &QPushButton::clicked, this, &MainWindow::onStopGrab);
    connect(ui->comboViewMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onViewModeChanged);

    connect(m_pGrabTimer, &QTimer::timeout, this, &MainWindow::updateImages);

    connect(m_pVisionFITO, &VisionPreFITOWrapper::calibrationLoaded,
            [this](double error) {
                ui->lblCalibrationStatus->setText(
                    QString("Calibration Loaded (Error: %1 px)").arg(error, 0, 'f', 3)
                );
            });

    // 초기화
    initializeVision();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeVision()
{
    // Halcon Vision 초기화
    if (!m_pVisionHalcon->initialize())
    {
        QMessageBox::warning(this, "Warning", "Halcon Vision 초기화 실패");
    }

    // OpenCV FITO Vision 초기화
    if (!m_pVisionFITO->initialize())
    {
        QMessageBox::warning(this, "Warning", "OpenCV FITO Vision 초기화 실패");
    }
}

void MainWindow::onLoadImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "이미지 열기",
        "",
        "Images (*.bmp *.png *.jpg)"
    );

    if (!fileName.isEmpty())
    {
        // 이미지 로드 및 표시
        QImage image(fileName);
        m_pHalconViewer->setImage(image);
        m_pOpenCVViewer->setImage(image);
    }
}

void MainWindow::onGrab()
{
    QElapsedTimer timer;

    // Halcon Grab
    timer.start();
    if (m_pVisionHalcon->grab())
    {
        m_comparisonData.halconProcessTime = timer.elapsed();
        m_pHalconViewer->setImage(m_pVisionHalcon->getImage());
    }

    // OpenCV FITO Grab
    timer.restart();
    if (m_pVisionFITO->grab())
    {
        m_comparisonData.opencvProcessTime = timer.elapsed();
        m_pOpenCVViewer->setImage(m_pVisionFITO->getImage());
    }

    updateComparisonResults();
}

void MainWindow::onContinuousGrab()
{
    m_pGrabTimer->start(30);  // 30ms 간격
    ui->btnContinuousGrab->setEnabled(false);
    ui->btnStop->setEnabled(true);
}

void MainWindow::onStopGrab()
{
    m_pGrabTimer->stop();
    ui->btnContinuousGrab->setEnabled(true);
    ui->btnStop->setEnabled(false);
}

void MainWindow::updateImages()
{
    onGrab();
}

void MainWindow::updateComparisonResults()
{
    // 처리 시간 업데이트
    ui->lblHalconTime->setText(QString("%1 ms").arg(m_comparisonData.halconProcessTime, 0, 'f', 1));
    ui->lblOpenCVTime->setText(QString("%1 ms").arg(m_comparisonData.opencvProcessTime, 0, 'f', 1));

    // Edge 개수 업데이트
    ui->lblHalconEdges->setText(QString("%1").arg(m_pVisionHalcon->getEdgeCount()));
    ui->lblOpenCVEdges->setText(QString("%1").arg(m_pVisionFITO->getEdgeCount()));

    // 템플릿 매칭 점수
    ui->lblTemplateMatch->setText(QString("%1 %").arg(m_pVisionHalcon->getTemplateMatchScore(), 0, 'f', 1));

    // 캘리브레이션 상태
    if (m_pVisionFITO->isCalibrationLoaded())
    {
        ui->lblCalibration->setText("적용");
        ui->lblReprojectionError->setText(
            QString("%1 px").arg(m_pVisionFITO->getReprojectionError(), 0, 'f', 3)
        );
    }
    else
    {
        ui->lblCalibration->setText("미적용");
        ui->lblReprojectionError->setText("N/A");
    }
}

void MainWindow::onViewModeChanged(int mode)
{
    m_pVisionHalcon->setViewMode(mode);
    m_pVisionFITO->setViewMode(mode);
    onGrab();
}
```

## UI 파일 (.ui) 예제

### MainWindow.ui 주요 위젯
```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>MainWindow</class>
 <widget class="QMainWindow" name="MainWindow">
  <property name="geometry">
   <rect>
    <x>0</x>
    <y>0</y>
    <width>1600</width>
    <height>900</height>
   </rect>
  </property>
  <widget class="QWidget" name="centralwidget">
   <layout class="QVBoxLayout" name="verticalLayout">
    <item>
     <layout class="QHBoxLayout" name="viewersLayout">
      <item>
       <layout class="QVBoxLayout" name="halconViewerLayout">
        <item>
         <widget class="QLabel" name="lblHalconTitle">
          <property name="text">
           <string>Fnc_Vision_Pre.cpp (Halcon)</string>
          </property>
         </widget>
        </item>
       </layout>
      </item>
      <item>
       <layout class="QVBoxLayout" name="opencvViewerLayout">
        <item>
         <widget class="QLabel" name="lblOpenCVTitle">
          <property name="text">
           <string>Fnc_Vision_Pre_FITO.cpp (OpenCV)</string>
          </property>
         </widget>
        </item>
       </layout>
      </item>
     </layout>
    </item>

    <item>
     <widget class="QGroupBox" name="controlPanel">
      <property name="title">
       <string>제어 패널</string>
      </property>
      <layout class="QHBoxLayout" name="controlLayout">
       <item>
        <widget class="QPushButton" name="btnLoadImage">
         <property name="text">
          <string>Load Image</string>
         </property>
        </widget>
       </item>
       <item>
        <widget class="QPushButton" name="btnGrab">
         <property name="text">
          <string>Grab</string>
         </property>
        </widget>
       </item>
       <item>
        <widget class="QPushButton" name="btnContinuousGrab">
         <property name="text">
          <string>Continuous Grab</string>
         </property>
        </widget>
       </item>
       <item>
        <widget class="QPushButton" name="btnStop">
         <property name="text">
          <string>Stop</string>
         </property>
         <property name="enabled">
          <bool>false</bool>
         </property>
        </widget>
       </item>
      </layout>
     </widget>
    </item>

    <item>
     <widget class="QGroupBox" name="analysisPanel">
      <property name="title">
       <string>비교 분석 결과</string>
      </property>
      <!-- 분석 결과 테이블 -->
     </widget>
    </item>
   </layout>
  </widget>

  <widget class="QMenuBar" name="menubar">
   <!-- 메뉴바 정의 -->
  </widget>

  <widget class="QStatusBar" name="statusbar"/>
 </widget>
</ui>
```

## 빌드 방법

### CMake 사용
```bash
cd D:\FITO_2026\Prevision
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### qmake 사용
```bash
cd D:\FITO_2026\Prevision
qmake VisionComparison.pro
nmake
```

## 실행 방법
```bash
cd build\Release
VisionComparison.exe
```

## 주요 기능

### 1. 이미지 비교
- 좌우 분할 뷰어
- 동기화된 확대/축소
- 오버레이 그래픽 (Cross, Line, Rectangle)

### 2. 실시간 Grab
- 단일 Grab
- 연속 Grab (30fps)
- 처리 시간 측정

### 3. 성능 비교
- 처리 시간
- Edge 검출 개수
- 템플릿 매칭 점수
- 왜곡 보정 상태

### 4. 이미지 전처리 제어
- View Mode (Raw/Gain/Binary)
- Gain/Offset 조절
- Binary Threshold 조절

## 확장 기능 (향후)

1. **차이 맵 표시**
   - 두 이미지의 픽셀 차이 시각화

2. **히스토그램 비교**
   - 이미지 품질 분석

3. **ROI 설정**
   - 관심 영역 지정 비교

4. **결과 저장**
   - 이미지 저장
   - 분석 결과 CSV 출력

5. **배치 처리**
   - 여러 이미지 자동 비교
