#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QPainter>
#include <QDir>
#include <QFileInfoList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_pGrabTimer(new QTimer(this))
    , m_bGrabbing(false)
    , m_gain(1.0)
    , m_offset(0)
    , m_threshold(128)
    , m_viewMode(0)
{
    ui->setupUi(this);

    setupViewers();
    setupConnections();
    setupUI();

    updateStatusBar("Ready");
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ========================================================================
// 초기화
// ========================================================================

void MainWindow::setupViewers()
{
    // Halcon 이미지 뷰어 생성
    m_pHalconViewer = new ImageViewer(this);
    m_pHalconViewer->setTitle("Halcon Vision");

    // 기존 labelHalconImage를 ImageViewer로 교체
    QWidget* halconContainer = ui->labelHalconImage->parentWidget();
    QVBoxLayout* halconLayout = qobject_cast<QVBoxLayout*>(halconContainer->layout());
    if (halconLayout)
    {
        halconLayout->replaceWidget(ui->labelHalconImage, m_pHalconViewer);
        ui->labelHalconImage->hide();
    }

    // OpenCV 이미지 뷰어 생성
    m_pOpenCVViewer = new ImageViewer(this);
    m_pOpenCVViewer->setTitle("OpenCV FITO Vision");

    // 기존 labelOpenCVImage를 ImageViewer로 교체
    QWidget* opencvContainer = ui->labelOpenCVImage->parentWidget();
    QVBoxLayout* opencvLayout = qobject_cast<QVBoxLayout*>(opencvContainer->layout());
    if (opencvLayout)
    {
        opencvLayout->replaceWidget(ui->labelOpenCVImage, m_pOpenCVViewer);
        ui->labelOpenCVImage->hide();
    }

    // 이미지 뷰어 이벤트 연결
    connect(m_pHalconViewer, &ImageViewer::imageClicked, this, &MainWindow::onHalconImageClicked);
    connect(m_pOpenCVViewer, &ImageViewer::imageClicked, this, &MainWindow::onOpenCVImageClicked);
}

void MainWindow::setupConnections()
{
    // 파일 메뉴
    connect(ui->actionLoadImage, &QAction::triggered, this, &MainWindow::onLoadImage);
    connect(ui->actionSaveImage, &QAction::triggered, this, &MainWindow::onSaveImage);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::onExit);

    // 보기 메뉴
    connect(ui->actionZoomIn, &QAction::triggered, this, &MainWindow::onZoomIn);
    connect(ui->actionZoomOut, &QAction::triggered, this, &MainWindow::onZoomOut);
    connect(ui->actionZoomFit, &QAction::triggered, this, &MainWindow::onZoomFit);
    connect(ui->actionShowGrid, &QAction::toggled, this, &MainWindow::onShowGrid);

    // 분석 메뉴
    connect(ui->actionCompare, &QAction::triggered, this, &MainWindow::onCompare);
    connect(ui->actionDifferenceMap, &QAction::triggered, this, &MainWindow::onDifferenceMap);
    connect(ui->actionHistogram, &QAction::triggered, this, &MainWindow::onHistogram);

    // 설정 메뉴
    connect(ui->actionCameraSettings, &QAction::triggered, this, &MainWindow::onCameraSettings);
    connect(ui->actionCalibrationSettings, &QAction::triggered, this, &MainWindow::onCalibrationSettings);
    connect(ui->actionPreferences, &QAction::triggered, this, &MainWindow::onPreferences);

    // 도움말 메뉴
    connect(ui->actionUserManual, &QAction::triggered, this, &MainWindow::onUserManual);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onAbout);

    // 제어 패널 버튼
    connect(ui->btnLoadImage, &QPushButton::clicked, this, &MainWindow::onLoadImage);
    connect(ui->btnGrab, &QPushButton::clicked, this, &MainWindow::onGrab);
    connect(ui->btnContinuousGrab, &QPushButton::clicked, this, &MainWindow::onContinuousGrab);
    connect(ui->btnStop, &QPushButton::clicked, this, &MainWindow::onStopGrab);

    // 제어 패널 파라미터
    connect(ui->comboViewMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onViewModeChanged);
    connect(ui->spinGain, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onGainChanged);
    connect(ui->spinOffset, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onOffsetChanged);
    connect(ui->spinThreshold, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onThresholdChanged);

    // 타이머
    connect(m_pGrabTimer, &QTimer::timeout, this, &MainWindow::updateImages);
}

void MainWindow::setupUI()
{
    // 윈도우 타이틀
    setWindowTitle("Vision Pre Comparison Tool - FITO 2026");

    // 초기 상태바 메시지
    updateStatusBar("Ready");

    // D:\Image\20240229_pre 폴더의 이미지 자동 로드
    QString imageFolder = "D:/Image/20240229_pre";
    QDir dir(imageFolder);

    if (dir.exists())
    {
        // 이미지 필터 설정
        QStringList filters;
        filters << "*.bmp" << "*.png" << "*.jpg" << "*.jpeg" << "*.tiff";
        dir.setNameFilters(filters);

        QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);

        if (!fileList.isEmpty())
        {
            // 첫 번째 이미지 로드
            QString firstImagePath = fileList.first().absoluteFilePath();
            QImage image(firstImagePath);

            if (!image.isNull())
            {
                m_pHalconViewer->setImage(image);
                m_pHalconViewer->setInfo(QString("Loaded: %1 | Size: %2x%3")
                    .arg(fileList.first().fileName())
                    .arg(image.width())
                    .arg(image.height()));

                m_pOpenCVViewer->setImage(image);
                m_pOpenCVViewer->setInfo(QString("Loaded: %1 | Size: %2x%3 | Calibration: Not Loaded")
                    .arg(fileList.first().fileName())
                    .arg(image.width())
                    .arg(image.height()));

                updateStatusBar(QString("Loaded: %1 (Total: %2 images in folder)")
                    .arg(firstImagePath)
                    .arg(fileList.count()));
            }
            else
            {
                updateStatusBar("Failed to load image from: " + imageFolder);
                loadDefaultTestImages();
            }
        }
        else
        {
            updateStatusBar("No images found in: " + imageFolder);
            loadDefaultTestImages();
        }
    }
    else
    {
        updateStatusBar("Image folder not found: " + imageFolder);
        loadDefaultTestImages();
    }
}

void MainWindow::loadDefaultTestImages()
{
    // 테스트 이미지 로드 (폴더 없을 때 기본)
    QImage testImage1 = createTestImage(800, 600, Qt::blue);
    QImage testImage2 = createTestImage(800, 600, Qt::darkGreen);

    m_pHalconViewer->setImage(testImage1);
    m_pHalconViewer->setInfo("Camera: OFF | Size: 800x600");

    m_pOpenCVViewer->setImage(testImage2);
    m_pOpenCVViewer->setInfo("Camera: OFF | Size: 800x600 | Calibration: Not Loaded");
}

// ========================================================================
// 파일 메뉴
// ========================================================================

void MainWindow::onLoadImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "이미지 열기",
        "D:/Image/20240229_pre",
        "Images (*.bmp *.png *.jpg *.jpeg *.tiff);;All Files (*.*)"
    );

    if (fileName.isEmpty())
        return;

    QImage image(fileName);

    if (image.isNull())
    {
        QMessageBox::warning(this, "오류", "이미지를 불러올 수 없습니다.");
        return;
    }

    // 좌우 이미지에 동일한 이미지 로드
    m_pHalconViewer->setImage(image);
    m_pOpenCVViewer->setImage(image);

    updateStatusBar(QString("이미지 로드: %1 (%2x%3)")
                    .arg(QFileInfo(fileName).fileName())
                    .arg(image.width())
                    .arg(image.height()));
}

void MainWindow::onSaveImage()
{
    // 어느 이미지를 저장할지 선택
    QMessageBox msgBox;
    msgBox.setWindowTitle("이미지 저장");
    msgBox.setText("어느 이미지를 저장하시겠습니까?");
    QPushButton* btnHalcon = msgBox.addButton("Halcon", QMessageBox::YesRole);
    QPushButton* btnOpenCV = msgBox.addButton("OpenCV", QMessageBox::NoRole);
    msgBox.addButton(QMessageBox::Cancel);
    msgBox.exec();

    ImageViewer* viewer = nullptr;
    if (msgBox.clickedButton() == btnHalcon)
        viewer = m_pHalconViewer;
    else if (msgBox.clickedButton() == btnOpenCV)
        viewer = m_pOpenCVViewer;
    else
        return;

    if (!viewer->hasImage())
    {
        QMessageBox::warning(this, "오류", "저장할 이미지가 없습니다.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "이미지 저장",
        "",
        "BMP (*.bmp);;PNG (*.png);;JPEG (*.jpg);;All Files (*.*)"
    );

    if (fileName.isEmpty())
        return;

    if (viewer->saveImage(fileName))
    {
        updateStatusBar(QString("이미지 저장: %1").arg(QFileInfo(fileName).fileName()));
    }
    else
    {
        QMessageBox::warning(this, "오류", "이미지를 저장할 수 없습니다.");
    }
}

void MainWindow::onExit()
{
    close();
}

// ========================================================================
// 보기 메뉴
// ========================================================================

void MainWindow::onZoomIn()
{
    m_pHalconViewer->zoomIn();
    m_pOpenCVViewer->zoomIn();
}

void MainWindow::onZoomOut()
{
    m_pHalconViewer->zoomOut();
    m_pOpenCVViewer->zoomOut();
}

void MainWindow::onZoomFit()
{
    m_pHalconViewer->zoomFit();
    m_pOpenCVViewer->zoomFit();
}

void MainWindow::onShowGrid(bool checked)
{
    // TODO: 그리드 표시 기능 구현
    updateStatusBar(checked ? "그리드 표시 ON" : "그리드 표시 OFF");
}

// ========================================================================
// 분석 메뉴
// ========================================================================

void MainWindow::onCompare()
{
    // TODO: 비교 분석 기능 구현
    updateComparisonResults();
    updateStatusBar("비교 분석 완료");
}

void MainWindow::onDifferenceMap()
{
    // TODO: 차이 맵 표시 기능 구현
    QMessageBox::information(this, "차이 맵", "차이 맵 기능은 준비 중입니다.");
}

void MainWindow::onHistogram()
{
    // TODO: 히스토그램 표시 기능 구현
    QMessageBox::information(this, "히스토그램", "히스토그램 기능은 준비 중입니다.");
}

// ========================================================================
// 설정 메뉴
// ========================================================================

void MainWindow::onCameraSettings()
{
    // TODO: 카메라 설정 다이얼로그 표시
    QMessageBox::information(this, "카메라 설정", "카메라 설정 기능은 준비 중입니다.");
}

void MainWindow::onCalibrationSettings()
{
    // TODO: 캘리브레이션 설정 다이얼로그 표시
    QMessageBox::information(this, "캘리브레이션 설정", "캘리브레이션 설정 기능은 준비 중입니다.");
}

void MainWindow::onPreferences()
{
    // TODO: 환경 설정 다이얼로그 표시
    QMessageBox::information(this, "환경 설정", "환경 설정 기능은 준비 중입니다.");
}

// ========================================================================
// 도움말 메뉴
// ========================================================================

void MainWindow::onUserManual()
{
    QMessageBox::information(this, "사용자 매뉴얼",
        "Vision Pre Comparison Tool v1.0\n\n"
        "주요 기능:\n"
        "- 이미지 로드 및 저장\n"
        "- 좌우 비교 뷰\n"
        "- 확대/축소 (Ctrl+휠)\n"
        "- 비교 분석\n\n"
        "자세한 내용은 README.md를 참고하세요.");
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "정보",
        "Vision Pre Comparison Tool\n"
        "Version 1.0.0\n\n"
        "Fnc_Vision_Pre.cpp vs Fnc_Vision_Pre_FITO.cpp\n"
        "Halcon vs OpenCV 비교 분석 도구\n\n"
        "Copyright (C) 2025 FITO");
}

// ========================================================================
// 제어 패널
// ========================================================================

void MainWindow::onGrab()
{
    // TODO: 실제 카메라에서 Grab 구현
    updateStatusBar("Grab 완료");

    // 테스트: 그래픽 오버레이 추가
    m_pHalconViewer->clearGraphics();
    m_pHalconViewer->drawCross(QPointF(400, 300), 50, Qt::green);
    m_pHalconViewer->drawText(QPointF(10, 30), "Halcon Grab", Qt::yellow);

    m_pOpenCVViewer->clearGraphics();
    m_pOpenCVViewer->drawCross(QPointF(400, 300), 50, Qt::red);
    m_pOpenCVViewer->drawText(QPointF(10, 30), "OpenCV Grab", Qt::cyan);
}

void MainWindow::onContinuousGrab()
{
    m_bGrabbing = true;
    m_pGrabTimer->start(30);  // 30ms 간격 (약 33fps)

    ui->btnContinuousGrab->setEnabled(false);
    ui->btnStop->setEnabled(true);

    updateStatusBar("연속 Grab 중...");
}

void MainWindow::onStopGrab()
{
    m_bGrabbing = false;
    m_pGrabTimer->stop();

    ui->btnContinuousGrab->setEnabled(true);
    ui->btnStop->setEnabled(false);

    updateStatusBar("연속 Grab 정지");
}

void MainWindow::onViewModeChanged(int index)
{
    m_viewMode = index;

    QString mode;
    switch (index)
    {
    case 0: mode = "Raw Image"; break;
    case 1: mode = "Gain Image"; break;
    case 2: mode = "Binary Image"; break;
    default: mode = "Unknown"; break;
    }

    updateStatusBar(QString("표시 모드: %1").arg(mode));
}

void MainWindow::onGainChanged(double value)
{
    m_gain = value;
    updateStatusBar(QString("Gain: %1").arg(value));
}

void MainWindow::onOffsetChanged(int value)
{
    m_offset = value;
    updateStatusBar(QString("Offset: %1").arg(value));
}

void MainWindow::onThresholdChanged(int value)
{
    m_threshold = value;
    updateStatusBar(QString("Binary Threshold: %1").arg(value));
}

void MainWindow::updateImages()
{
    // TODO: 실제 카메라에서 이미지 업데이트
    // 테스트: 랜덤 그래픽 추가
    static int frameCount = 0;
    frameCount++;

    m_pHalconViewer->clearGraphics();
    m_pHalconViewer->drawText(QPointF(10, 30),
        QString("Halcon Frame: %1").arg(frameCount), Qt::yellow);

    m_pOpenCVViewer->clearGraphics();
    m_pOpenCVViewer->drawText(QPointF(10, 30),
        QString("OpenCV Frame: %1").arg(frameCount), Qt::cyan);
}

// ========================================================================
// 이미지 뷰어 이벤트
// ========================================================================

void MainWindow::onHalconImageClicked(const QPointF& pos)
{
    updateStatusBar(QString("Halcon 이미지 클릭: (%1, %2)")
                    .arg(pos.x(), 0, 'f', 1)
                    .arg(pos.y(), 0, 'f', 1));
}

void MainWindow::onOpenCVImageClicked(const QPointF& pos)
{
    updateStatusBar(QString("OpenCV 이미지 클릭: (%1, %2)")
                    .arg(pos.x(), 0, 'f', 1)
                    .arg(pos.y(), 0, 'f', 1));
}

// ========================================================================
// Helper 함수
// ========================================================================

void MainWindow::updateComparisonResults()
{
    // TODO: 실제 분석 결과 업데이트
    ui->labelHalconTime->setText("15.2 ms");
    ui->labelOpenCVTime->setText("12.8 ms");

    ui->labelHalconEdges->setText("234");
    ui->labelOpenCVEdges->setText("218");

    ui->labelHalconMatch->setText("92.3 %");
    ui->labelOpenCVMatch->setText("N/A");

    ui->labelHalconCalib->setText("N/A");
    ui->labelOpenCVCalib->setText("적용");

    ui->labelHalconReproj->setText("N/A");
    ui->labelOpenCVReproj->setText("0.234 px");
}

void MainWindow::updateStatusBar(const QString& message)
{
    statusBar()->showMessage(message);
}

QImage MainWindow::createTestImage(int width, int height, const QColor& color)
{
    QImage image(width, height, QImage::Format_RGB32);
    image.fill(color);

    // 중앙에 원 그리기
    QPainter painter(&image);
    painter.setPen(Qt::white);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(width/2 - 100, height/2 - 100, 200, 200);

    // 십자선 그리기
    painter.drawLine(width/2, 0, width/2, height);
    painter.drawLine(0, height/2, width, height/2);

    return image;
}
