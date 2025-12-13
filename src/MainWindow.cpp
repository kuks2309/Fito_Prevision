#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QPainter>
#include <QDir>
#include <QFileInfoList>
#include "Vision/Fnc_Vision_Pre_FITO.h"

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

    // Create Image Processing Panel (with tabs)
    m_pProcessingPanel = new ImageProcessingPanel(this);
    ui->dockProcessing->setWidget(m_pProcessingPanel);

    // Connect Processing Panel signals
    connect(m_pProcessingPanel, &ImageProcessingPanel::tabChanged,
            this, &MainWindow::onProcessingTabChanged);

    // Connect Edge Processing signals
    EdgeProcessingWidget* edgeWidget = m_pProcessingPanel->getEdgeWidget();
    if (edgeWidget) {
        connect(edgeWidget, &EdgeProcessingWidget::algorithmChanged,
                this, &MainWindow::onEdgeAlgorithmChanged);
        connect(edgeWidget, &EdgeProcessingWidget::parametersChanged,
                this, &MainWindow::onEdgeParametersChanged);
        connect(edgeWidget, &EdgeProcessingWidget::applyRequested,
                this, &MainWindow::onEdgeApplyRequested);
        connect(edgeWidget, &EdgeProcessingWidget::resetRequested,
                this, &MainWindow::onEdgeResetRequested);
    }

    // Connect Line Detection signals
    LineDetectionWidget* lineWidget = m_pProcessingPanel->getLineWidget();
    if (lineWidget) {
        connect(lineWidget, &LineDetectionWidget::algorithmChanged,
                this, &MainWindow::onLineAlgorithmChanged);
        // Temporarily disable parametersChanged to prevent infinite loop
        // connect(lineWidget, &LineDetectionWidget::parametersChanged,
        //         this, &MainWindow::onLineParametersChanged);
        connect(lineWidget, &LineDetectionWidget::applyRequested,
                this, &MainWindow::onLineApplyRequested);
        connect(lineWidget, &LineDetectionWidget::resetRequested,
                this, &MainWindow::onLineResetRequested);
    }

    // Connect AI Prevision signals
    AIPrevisionWidget* aiWidget = m_pProcessingPanel->getAIWidget();
    if (aiWidget) {
        connect(aiWidget, &AIPrevisionWidget::algorithmChanged,
                this, &MainWindow::onAIAlgorithmChanged);
        connect(aiWidget, &AIPrevisionWidget::applyRequested,
                this, &MainWindow::onAIApplyRequested);
        connect(aiWidget, &AIPrevisionWidget::houghLinesRequested,
                this, &MainWindow::onAIHoughLinesRequested);
        connect(aiWidget, &AIPrevisionWidget::clusteringRequested,
                this, &MainWindow::onAIClusteringRequested);
        connect(aiWidget, &AIPrevisionWidget::resetRequested,
                this, &MainWindow::onAIResetRequested);
    }

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

// ========================================================================
// Edge Processing
// ========================================================================

void MainWindow::onEdgeAlgorithmChanged(EdgeProcessingWidget::EdgeAlgorithm algorithm)
{
    Q_UNUSED(algorithm);
    updateStatusBar(QString("Edge algorithm changed"));
}

void MainWindow::onEdgeParametersChanged()
{
    // Auto-apply if needed
    updateStatusBar("Parameters changed");
}

void MainWindow::onEdgeApplyRequested()
{
    // Get current image from Halcon viewer
    m_currentInputImage = m_pHalconViewer->getImage();

    if (m_currentInputImage.isNull())
    {
        QMessageBox::warning(this, "Warning", "No image loaded. Please load an image first.");
        return;
    }

    EdgeProcessingWidget* edgeWidget = m_pProcessingPanel->getEdgeWidget();
    if (edgeWidget) {
        applyEdgeDetection(edgeWidget->getCurrentAlgorithm());
    }
}

void MainWindow::onEdgeResetRequested()
{
    // Reset to original image
    if (!m_currentInputImage.isNull())
    {
        m_pOpenCVViewer->setImage(m_currentInputImage);
        updateStatusBar("Reset to original image");
    }
}

void MainWindow::applyEdgeDetection(EdgeProcessingWidget::EdgeAlgorithm algorithm)
{
    EdgeProcessingWidget* edgeWidget = m_pProcessingPanel->getEdgeWidget();
    if (!edgeWidget) return;

    QImage result;

    switch (algorithm)
    {
    case EdgeProcessingWidget::CANNY:
        result = applyCanny(m_currentInputImage,
                          edgeWidget->getParam1(),
                          edgeWidget->getParam2(),
                          edgeWidget->getParam3(),
                          edgeWidget->getL2Gradient());
        break;

    case EdgeProcessingWidget::SOBEL:
        result = applySobel(m_currentInputImage,
                          edgeWidget->getParam4(),
                          edgeWidget->getParam5(),
                          edgeWidget->getParam6());
        break;

    case EdgeProcessingWidget::SOBEL_PREVISION:
        {
            cv::Mat src = QImageToCvMat(m_currentInputImage);
            cv::Mat edges = sobel_prevision(src,
                                           edgeWidget->getParam4(),    // bilateralD
                                           edgeWidget->getParam5(),    // sigmaColor
                                           edgeWidget->getParam6(),    // sigmaSpace
                                           edgeWidget->getParam1());   // threshold
            result = QImage(edges.data, edges.cols, edges.rows, edges.step,
                           QImage::Format_Grayscale8).copy();
        }
        break;

    case EdgeProcessingWidget::SCHARR:
        result = applyScharr(m_currentInputImage,
                           edgeWidget->getParam5(),
                           edgeWidget->getParam6());
        break;

    case EdgeProcessingWidget::LAPLACIAN:
        result = applyLaplacian(m_currentInputImage,
                              edgeWidget->getParam4(),
                              edgeWidget->getParam5(),
                              edgeWidget->getParam6());
        break;

    case EdgeProcessingWidget::PREWITT:
        result = applyPrewitt(m_currentInputImage);
        break;

    case EdgeProcessingWidget::ROBERTS:
        result = applyRoberts(m_currentInputImage);
        break;

    default:
        updateStatusBar("Algorithm not implemented yet");
        return;
    }

    if (!result.isNull())
    {
        m_currentProcessedImage = result;
        m_currentEdgeImage = result;  // Edge 결과를 별도 보관 (Line Detection용)
        m_pOpenCVViewer->setImage(result);
        m_pOpenCVViewer->setInfo(QString("Edge Detection Applied"));
        updateStatusBar("Edge detection completed");
    }
}

cv::Mat MainWindow::QImageToCvMat(const QImage& image)
{
    cv::Mat mat;

    switch (image.format())
    {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
    {
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        cv::Mat mat3;
        cv::cvtColor(mat, mat3, cv::COLOR_BGRA2BGR);
        return mat3.clone();
    }
    case QImage::Format_RGB888:
    {
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::Mat mat3;
        cv::cvtColor(mat, mat3, cv::COLOR_RGB2BGR);
        return mat3.clone();
    }
    case QImage::Format_Grayscale8:
    {
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        return mat.clone();
    }
    default:
    {
        QImage converted = image.convertToFormat(QImage::Format_RGB888);
        mat = cv::Mat(converted.height(), converted.width(), CV_8UC3, (void*)converted.constBits(), converted.bytesPerLine());
        cv::Mat mat3;
        cv::cvtColor(mat, mat3, cv::COLOR_RGB2BGR);
        return mat3.clone();
    }
    }
}

QImage MainWindow::applyCanny(const QImage& input, int lowThreshold, int highThreshold, int apertureSize, bool l2Gradient)
{
    // Convert QImage to cv::Mat
    cv::Mat src;
    if (input.format() == QImage::Format_Grayscale8)
    {
        src = cv::Mat(input.height(), input.width(), CV_8UC1, (void*)input.constBits(), input.bytesPerLine()).clone();
    }
    else
    {
        QImage gray = input.convertToFormat(QImage::Format_Grayscale8);
        src = cv::Mat(gray.height(), gray.width(), CV_8UC1, (void*)gray.constBits(), gray.bytesPerLine()).clone();
    }

    // Apply Canny edge detection
    cv::Mat edges;
    cv::Canny(src, edges, lowThreshold, highThreshold, apertureSize, l2Gradient);

    // Convert back to QImage
    QImage result(edges.data, edges.cols, edges.rows, edges.step, QImage::Format_Grayscale8);
    return result.copy();
}

QImage MainWindow::applySobel(const QImage& input, int ksize, double scale, double delta)
{
    // Convert to grayscale cv::Mat
    QImage gray = input.convertToFormat(QImage::Format_Grayscale8);
    cv::Mat src(gray.height(), gray.width(), CV_8UC1, (void*)gray.constBits(), gray.bytesPerLine());
    src = src.clone();

    // Apply Sobel operator
    cv::Mat grad_x, grad_y;
    cv::Mat abs_grad_x, abs_grad_y;
    cv::Mat grad;

    cv::Sobel(src, grad_x, CV_16S, 1, 0, ksize, scale, delta, cv::BORDER_DEFAULT);
    cv::Sobel(src, grad_y, CV_16S, 0, 1, ksize, scale, delta, cv::BORDER_DEFAULT);

    cv::convertScaleAbs(grad_x, abs_grad_x);
    cv::convertScaleAbs(grad_y, abs_grad_y);

    cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

    QImage result(grad.data, grad.cols, grad.rows, grad.step, QImage::Format_Grayscale8);
    return result.copy();
}

QImage MainWindow::applyScharr(const QImage& input, double scale, double delta)
{
    // Convert to grayscale cv::Mat
    QImage gray = input.convertToFormat(QImage::Format_Grayscale8);
    cv::Mat src(gray.height(), gray.width(), CV_8UC1, (void*)gray.constBits(), gray.bytesPerLine());
    src = src.clone();

    // Apply Scharr operator
    cv::Mat grad_x, grad_y;
    cv::Mat abs_grad_x, abs_grad_y;
    cv::Mat grad;

    cv::Scharr(src, grad_x, CV_16S, 1, 0, scale, delta, cv::BORDER_DEFAULT);
    cv::Scharr(src, grad_y, CV_16S, 0, 1, scale, delta, cv::BORDER_DEFAULT);

    cv::convertScaleAbs(grad_x, abs_grad_x);
    cv::convertScaleAbs(grad_y, abs_grad_y);

    cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

    QImage result(grad.data, grad.cols, grad.rows, grad.step, QImage::Format_Grayscale8);
    return result.copy();
}

QImage MainWindow::applyLaplacian(const QImage& input, int ksize, double scale, double delta)
{
    // Convert to grayscale cv::Mat
    QImage gray = input.convertToFormat(QImage::Format_Grayscale8);
    cv::Mat src(gray.height(), gray.width(), CV_8UC1, (void*)gray.constBits(), gray.bytesPerLine());
    src = src.clone();

    // Apply Laplacian
    cv::Mat dst;
    cv::Laplacian(src, dst, CV_16S, ksize, scale, delta, cv::BORDER_DEFAULT);

    cv::Mat abs_dst;
    cv::convertScaleAbs(dst, abs_dst);

    QImage result(abs_dst.data, abs_dst.cols, abs_dst.rows, abs_dst.step, QImage::Format_Grayscale8);
    return result.copy();
}

QImage MainWindow::applyPrewitt(const QImage& input)
{
    // Convert to grayscale cv::Mat
    QImage gray = input.convertToFormat(QImage::Format_Grayscale8);
    cv::Mat src(gray.height(), gray.width(), CV_8UC1, (void*)gray.constBits(), gray.bytesPerLine());
    src = src.clone();

    // Prewitt kernels
    cv::Mat kernelX = (cv::Mat_<float>(3, 3) << -1, 0, 1, -1, 0, 1, -1, 0, 1);
    cv::Mat kernelY = (cv::Mat_<float>(3, 3) << -1, -1, -1, 0, 0, 0, 1, 1, 1);

    cv::Mat grad_x, grad_y;
    cv::filter2D(src, grad_x, CV_16S, kernelX);
    cv::filter2D(src, grad_y, CV_16S, kernelY);

    cv::Mat abs_grad_x, abs_grad_y;
    cv::convertScaleAbs(grad_x, abs_grad_x);
    cv::convertScaleAbs(grad_y, abs_grad_y);

    cv::Mat grad;
    cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

    QImage result(grad.data, grad.cols, grad.rows, grad.step, QImage::Format_Grayscale8);
    return result.copy();
}

QImage MainWindow::applyRoberts(const QImage& input)
{
    // Convert to grayscale cv::Mat
    QImage gray = input.convertToFormat(QImage::Format_Grayscale8);
    cv::Mat src(gray.height(), gray.width(), CV_8UC1, (void*)gray.constBits(), gray.bytesPerLine());
    src = src.clone();

    // Roberts Cross kernels
    cv::Mat kernelX = (cv::Mat_<float>(2, 2) << 1, 0, 0, -1);
    cv::Mat kernelY = (cv::Mat_<float>(2, 2) << 0, 1, -1, 0);

    cv::Mat grad_x, grad_y;
    cv::filter2D(src, grad_x, CV_16S, kernelX);
    cv::filter2D(src, grad_y, CV_16S, kernelY);

    cv::Mat abs_grad_x, abs_grad_y;
    cv::convertScaleAbs(grad_x, abs_grad_x);
    cv::convertScaleAbs(grad_y, abs_grad_y);

    cv::Mat grad;
    cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

    QImage result(grad.data, grad.cols, grad.rows, grad.step, QImage::Format_Grayscale8);
    return result.copy();
}

// ========================================================================
// Sobel Prevision (Bilateral Filter + Sobel XY + Threshold)
// ========================================================================

cv::Mat MainWindow::sobel_prevision(const cv::Mat& input, int bilateralD,
                                     double sigmaColor, double sigmaSpace,
                                     int threshold)
{
    cv::Mat gray;

    // 1. Convert to grayscale if needed
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else if (input.channels() == 4) {
        cv::cvtColor(input, gray, cv::COLOR_BGRA2GRAY);
    } else {
        gray = input.clone();
    }

    // 2. Apply Bilateral Filter (edge-preserving noise reduction)
    cv::Mat filtered;
    cv::bilateralFilter(gray, filtered, bilateralD, sigmaColor, sigmaSpace);

    // 3. Apply Sobel X and Y
    cv::Mat sobel_x, sobel_y;
    cv::Sobel(filtered, sobel_x, CV_64F, 1, 0, 3);  // dx=1, dy=0
    cv::Sobel(filtered, sobel_y, CV_64F, 0, 1, 3);  // dx=0, dy=1

    // 4. Combine gradients: sqrt(Sx^2 + Sy^2)
    cv::Mat sobel_combined;
    cv::magnitude(sobel_x, sobel_y, sobel_combined);

    // 5. Normalize to 0-255
    double minVal, maxVal;
    cv::minMaxLoc(sobel_combined, &minVal, &maxVal);
    cv::Mat normalized;
    sobel_combined.convertTo(normalized, CV_8U, 255.0 / maxVal);

    // 6. Apply binary threshold
    cv::Mat edges;
    cv::threshold(normalized, edges, threshold, 255, cv::THRESH_BINARY);

    return edges;
}

// ========================================================================
// Line Detection Functions
// ========================================================================

void MainWindow::onProcessingTabChanged(int index)
{
    qDebug() << "Processing tab changed to:" << index;
    updateStatusBar(QString("Tab changed to index: %1").arg(index));
}

void MainWindow::onLineAlgorithmChanged(LineDetectionWidget::LineAlgorithm algorithm)
{
    Q_UNUSED(algorithm);
    updateStatusBar(QString("Line algorithm changed"));
}

void MainWindow::onLineParametersChanged()
{
    // Auto-apply if needed
    updateStatusBar("Line parameters changed");
}

void MainWindow::onLineApplyRequested()
{
    qDebug() << "Line Apply Requested";

    // Get current image from Halcon viewer
    m_currentInputImage = m_pHalconViewer->getImage();

    if (m_currentInputImage.isNull())
    {
        qWarning() << "No image loaded for line detection";
        QMessageBox::warning(this, "Warning", "No image loaded. Please load an image first.");
        return;
    }

    qDebug() << "Input image size:" << m_currentInputImage.width() << "x" << m_currentInputImage.height();

    LineDetectionWidget* lineWidget = m_pProcessingPanel->getLineWidget();
    if (lineWidget) {
        qDebug() << "Applying line detection algorithm:" << lineWidget->getCurrentAlgorithm();
        applyLineDetection(lineWidget->getCurrentAlgorithm());
    } else {
        qWarning() << "Line widget is null!";
    }
}

void MainWindow::onLineResetRequested()
{
    // Reset to original image
    if (!m_currentInputImage.isNull())
    {
        m_pOpenCVViewer->setImage(m_currentInputImage);
        updateStatusBar("Reset to original image");
    }
}

void MainWindow::applyLineDetection(LineDetectionWidget::LineAlgorithm algorithm)
{
    LineDetectionWidget* lineWidget = m_pProcessingPanel->getLineWidget();
    if (!lineWidget) return;

    QImage result;

    switch (algorithm)
    {
    case LineDetectionWidget::HOUGH_LINES:
        result = applyHoughLines(m_currentInputImage,
                                lineWidget->getRho(),
                                lineWidget->getTheta(),
                                lineWidget->getThreshold());
        break;

    case LineDetectionWidget::HOUGH_LINES_P:
        result = applyHoughLinesP(m_currentInputImage,
                                 lineWidget->getRho(),
                                 lineWidget->getTheta(),
                                 lineWidget->getThreshold(),
                                 lineWidget->getMinLineLength(),
                                 lineWidget->getMaxLineGap());
        break;

    case LineDetectionWidget::LSD:
        result = applyLSD(m_currentInputImage, lineWidget->getLsdScale());
        break;

    default:
        updateStatusBar("Line algorithm not implemented yet");
        return;
    }

    if (!result.isNull())
    {
        m_currentProcessedImage = result;
        m_pOpenCVViewer->setImage(result);
        m_pOpenCVViewer->setInfo(QString("Line Detection Applied"));
        updateStatusBar("Line detection completed");
    }
}

QImage MainWindow::applyHoughLines(const QImage& input, double rho, double theta, int threshold)
{
    cv::Mat src = QImageToCvMat(input);
    if (src.empty()) {
        qWarning() << "applyHoughLines: Input image is empty";
        return QImage();
    }

    // Convert to grayscale if needed
    cv::Mat gray;
    if (src.channels() == 3) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = src.clone();
    }

    // Use edge image if available, otherwise apply Canny
    cv::Mat edges;
    if (!m_currentEdgeImage.isNull() && m_currentEdgeImage.format() == QImage::Format_Grayscale8) {
        // Use edge result from Edge Processing tab (e.g., sobel_prevision)
        edges = cv::Mat(m_currentEdgeImage.height(), m_currentEdgeImage.width(),
                       CV_8UC1, (void*)m_currentEdgeImage.constBits(),
                       m_currentEdgeImage.bytesPerLine()).clone();
        qDebug() << "Using pre-processed edge image for Hough Lines";
    } else {
        // Fallback to Canny
        cv::Canny(gray, edges, 50, 150);
        qDebug() << "Using Canny edge detection for Hough Lines";
    }

    // Detect lines using Hough Transform
    std::vector<cv::Vec2f> lines;
    cv::HoughLines(edges, lines, rho, theta * CV_PI / 180.0, threshold);

    qDebug() << "Hough Lines detected:" << lines.size() << "lines with threshold:" << threshold;

    // Draw lines on color image
    cv::Mat result;
    if (src.channels() == 1) {
        cv::cvtColor(src, result, cv::COLOR_GRAY2BGR);
    } else {
        result = src.clone();
    }

    for (size_t i = 0; i < lines.size(); i++) {
        float rho_val = lines[i][0];
        float theta_val = lines[i][1];

        double a = cos(theta_val);
        double b = sin(theta_val);
        double x0 = a * rho_val;
        double y0 = b * rho_val;

        cv::Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * (a)));
        cv::Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * (a)));

        cv::line(result, pt1, pt2, cv::Scalar(0, 255, 0), 2);
    }

    // Convert cv::Mat to QImage safely
    QImage resultImage = QImage((const uchar*)result.data, result.cols, result.rows,
                                result.step, QImage::Format_RGB888).rgbSwapped();
    return resultImage.copy();
}

QImage MainWindow::applyHoughLinesP(const QImage& input, double rho, double theta,
                                    int threshold, double minLineLength, double maxLineGap)
{
    cv::Mat src = QImageToCvMat(input);
    if (src.empty()) {
        return QImage();
    }

    // Convert to grayscale if needed
    cv::Mat gray;
    if (src.channels() == 3) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = src.clone();
    }

    // Use edge image if available, otherwise apply Canny
    cv::Mat edges;
    if (!m_currentEdgeImage.isNull() && m_currentEdgeImage.format() == QImage::Format_Grayscale8) {
        // Use edge result from Edge Processing tab (e.g., sobel_prevision)
        edges = cv::Mat(m_currentEdgeImage.height(), m_currentEdgeImage.width(),
                       CV_8UC1, (void*)m_currentEdgeImage.constBits(),
                       m_currentEdgeImage.bytesPerLine()).clone();
        qDebug() << "Using pre-processed edge image for Hough Lines P";
    } else {
        // Fallback to Canny
        cv::Canny(gray, edges, 50, 150);
        qDebug() << "Using Canny edge detection for Hough Lines P";
    }

    // Detect lines using Probabilistic Hough Transform
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, rho, theta * CV_PI / 180.0, threshold,
                    minLineLength, maxLineGap);

    // Draw lines on color image
    cv::Mat result;
    if (src.channels() == 1) {
        cv::cvtColor(src, result, cv::COLOR_GRAY2BGR);
    } else {
        result = src.clone();
    }

    for (size_t i = 0; i < lines.size(); i++) {
        cv::line(result,
                cv::Point(lines[i][0], lines[i][1]),
                cv::Point(lines[i][2], lines[i][3]),
                cv::Scalar(0, 255, 0), 2);
    }

    // Convert cv::Mat to QImage safely
    QImage resultImage = QImage((const uchar*)result.data, result.cols, result.rows,
                                result.step, QImage::Format_RGB888).rgbSwapped();
    return resultImage.copy();
}

QImage MainWindow::applyLSD(const QImage& input, double scale)
{
    Q_UNUSED(scale);

    // LSD is disabled due to OpenCV compatibility issues
    // Using Canny + HoughLinesP as alternative
    QMessageBox::information(nullptr, "LSD Not Available",
        "LSD (Line Segment Detector) is not available in this OpenCV build.\n\n"
        "Please use 'Hough Lines P' instead for line detection.");

    // Return empty image - no crash
    return QImage();
}

// ========================================================================
// AI Prevision
// ========================================================================

void MainWindow::onAIAlgorithmChanged(AIPrevisionWidget::AIAlgorithm algorithm)
{
    Q_UNUSED(algorithm);
    updateStatusBar(QString("AI Algorithm changed"));
}

void MainWindow::onAIParametersChanged()
{
    updateStatusBar("AI Parameters changed");
}

void MainWindow::onAIApplyRequested()
{
    qDebug() << "[MainWindow] onAIApplyRequested called";
    AIPrevisionWidget* aiWidget = m_pProcessingPanel->getAIWidget();
    if (!aiWidget) {
        qDebug() << "[MainWindow] aiWidget is null";
        return;
    }

    qDebug() << "[MainWindow] Current algorithm:" << aiWidget->getCurrentAlgorithm();
    applyAIPrevision(aiWidget->getCurrentAlgorithm());
}

void MainWindow::onAIResetRequested()
{
    updateStatusBar("AI Parameters reset");
}

void MainWindow::applyAIPrevision(AIPrevisionWidget::AIAlgorithm algorithm)
{
    qDebug() << "[MainWindow] applyAIPrevision called with algorithm:" << algorithm;

    // 현재 Halcon 뷰어의 이미지 가져오기
    m_currentInputImage = m_pHalconViewer->getImage();

    if (m_currentInputImage.isNull()) {
        qDebug() << "[MainWindow] m_currentInputImage is null!";
        updateStatusBar("No input image loaded");
        return;
    }

    qDebug() << "[MainWindow] Input image size:" << m_currentInputImage.width() << "x" << m_currentInputImage.height();

    QImage result;

    switch (algorithm) {
    case AIPrevisionWidget::TEED_EDGE_DETECTION:
        qDebug() << "[MainWindow] Applying TEED Edge Detection";
        result = applyTEEDEdgeDetection(m_currentInputImage);
        break;

    default:
        qDebug() << "[MainWindow] Unknown algorithm";
        updateStatusBar("AI algorithm not implemented yet");
        return;
    }

    qDebug() << "[MainWindow] Result image null:" << result.isNull();
    if (!result.isNull()) {
        m_currentProcessedImage = result;
        m_pOpenCVViewer->setImage(result);
        m_pOpenCVViewer->setInfo(QString("AI Prevision Applied"));
        updateStatusBar("AI Prevision completed");
    }
}

QImage MainWindow::applyCNNMLCCPanelEdge(const QImage& input)
{
    Q_UNUSED(input);

    // CNN MLCC Panel Edge - Not implemented yet
    QMessageBox::information(nullptr, "CNN MLCC Panel Edge",
        "CNN MLCC Panel Edge Detection is not implemented yet.\n\n"
        "This algorithm will be implemented in future updates.");

    return QImage();
}

QImage MainWindow::applyTEEDEdgeDetection(const QImage& input)
{
    if (input.isNull()) {
        QMessageBox::warning(this, "TEED Error", "Input image is null.");
        return QImage();
    }

    updateStatusBar("TEED Edge Detection: Connecting to server...");
    QApplication::processEvents();

    // QImage -> cv::Mat 변환 (BGR)
    cv::Mat inputMat = QImageToCvMat(input);
    if (inputMat.empty()) {
        QMessageBox::warning(this, "TEED Error", "Failed to convert image to cv::Mat.");
        return QImage();
    }

    // Grayscale이면 BGR로 변환
    cv::Mat bgrMat;
    if (inputMat.channels() == 1) {
        cv::cvtColor(inputMat, bgrMat, cv::COLOR_GRAY2BGR);
    } else if (inputMat.channels() == 4) {
        cv::cvtColor(inputMat, bgrMat, cv::COLOR_BGRA2BGR);
    } else {
        bgrMat = inputMat;
    }

    // TEED 추론
    static Fnc_Vision_Pre_FITO visionProcessor;
    cv::Mat edgeMap;

    // 연결 상태 확인
    if (!visionProcessor.TEED_IsConnected()) {
        updateStatusBar("TEED Edge Detection: Starting Python server...");
        QApplication::processEvents();
    }

    updateStatusBar("TEED Edge Detection: Processing...");
    QApplication::processEvents();

    if (!visionProcessor.TEED_Inference(bgrMat, edgeMap, 15000)) {
        QMessageBox::warning(this, "TEED Error",
            QString("TEED inference failed.\n\n"
            "Please check:\n"
            "1. Python server: D:\\FITO_2026\\TEED\\teed_shared_memory.py\n"
            "2. TEED model file exists\n"
            "3. Image size: %1x%2\n\n"
            "Try running Python server manually:\n"
            "python D:\\FITO_2026\\TEED\\teed_shared_memory.py")
            .arg(bgrMat.cols).arg(bgrMat.rows));
        return QImage();
    }

    // Edge map -> QImage 변환
    if (edgeMap.empty()) {
        QMessageBox::warning(this, "TEED Error", "Edge map is empty after inference.");
        return QImage();
    }

    // 원본 크기 및 crop 정보 계산 (TEED_Preprocess와 동일한 로직)
    int origHeight = bgrMat.rows;
    int origWidth = bgrMat.cols;
    int h_crop = (origHeight / 32) * 32;
    int w_crop = (origWidth / 32) * 32;
    int x_offset = (origWidth - w_crop) / 2;
    int y_offset = (origHeight - h_crop) / 2;

    qDebug() << "[TEED] Original:" << origWidth << "x" << origHeight
             << "Crop:" << w_crop << "x" << h_crop
             << "Offset:" << x_offset << "," << y_offset;

    // 1. edge map을 crop 영역 크기로 4배 확대
    cv::Mat resizedCrop;
    cv::resize(edgeMap, resizedCrop, cv::Size(w_crop, h_crop), 0, 0, cv::INTER_LINEAR);

    // 2. 원본 크기의 검은 배경 이미지 생성
    cv::Mat fullEdge = cv::Mat::zeros(origHeight, origWidth, CV_8UC1);

    // 3. crop된 결과를 원래 위치에 복사
    resizedCrop.copyTo(fullEdge(cv::Rect(x_offset, y_offset, w_crop, h_crop)));

    qDebug() << "[TEED] Edge map:" << edgeMap.cols << "x" << edgeMap.rows
             << "-> Full:" << fullEdge.cols << "x" << fullEdge.rows;

    // CV_8UC1 -> QImage (Grayscale)
    QImage result(fullEdge.cols, fullEdge.rows, QImage::Format_Grayscale8);
    for (int y = 0; y < fullEdge.rows; y++) {
        memcpy(result.scanLine(y), fullEdge.ptr(y), fullEdge.cols);
    }

    // Edge 이미지 저장 (Line Detection용)
    m_currentEdgeImage = result;

    updateStatusBar(QString("TEED Edge Detection completed (%1x%2 -> %3x%4, offset: %5,%6)")
                    .arg(edgeMap.cols).arg(edgeMap.rows)
                    .arg(fullEdge.cols).arg(fullEdge.rows)
                    .arg(x_offset).arg(y_offset));

    return result;
}

void MainWindow::onAIHoughLinesRequested()
{
    qDebug() << "[MainWindow] onAIHoughLinesRequested called";

    if (m_currentEdgeImage.isNull()) {
        QMessageBox::warning(this, "Hough Lines Error",
            "Edge image not available.\n\n"
            "Please apply TEED Edge Detection first.");
        return;
    }

    QImage result = applyTEEDHoughLines(m_currentEdgeImage);

    if (!result.isNull()) {
        m_currentProcessedImage = result;
        m_pOpenCVViewer->setImage(result);
        m_pOpenCVViewer->setInfo(QString("TEED Hough Lines Applied"));
        updateStatusBar("TEED Hough Lines completed");
    }
}

void MainWindow::onAIClusteringRequested()
{
    qDebug() << "[MainWindow] onAIClusteringRequested called";

    if (m_currentEdgeImage.isNull()) {
        QMessageBox::warning(this, "Clustering Error",
            "Edge image not available.\n\n"
            "Please apply TEED Edge Detection first.");
        return;
    }

    QImage result = applyTEEDClustering(m_currentEdgeImage);

    if (!result.isNull()) {
        m_currentProcessedImage = result;
        m_pOpenCVViewer->setImage(result);
        m_pOpenCVViewer->setInfo(QString("TEED Clustering Applied"));
        updateStatusBar("TEED Clustering completed");
    }
}

QImage MainWindow::applyTEEDHoughLines(const QImage& edgeImage)
{
    if (edgeImage.isNull()) {
        return QImage();
    }

    // QImage -> cv::Mat 변환
    cv::Mat edgeMat;
    if (edgeImage.format() == QImage::Format_Grayscale8) {
        edgeMat = cv::Mat(edgeImage.height(), edgeImage.width(), CV_8UC1,
                         (void*)edgeImage.constBits(), edgeImage.bytesPerLine()).clone();
    } else {
        QImage gray = edgeImage.convertToFormat(QImage::Format_Grayscale8);
        edgeMat = cv::Mat(gray.height(), gray.width(), CV_8UC1,
                         (void*)gray.constBits(), gray.bytesPerLine()).clone();
    }

    // Fnc_Vision_Pre_FITO를 사용하여 직선 추출
    static Fnc_Vision_Pre_FITO visionProcessor;

    std::vector<TEEDLineInfo> hLines, vLines;

    // UI에서 파라미터 가져오기
    int threshold = m_pProcessingPanel->getAIWidget()->getThreshold();
    int minLength = m_pProcessingPanel->getAIWidget()->getMinLength();
    int maxGap = m_pProcessingPanel->getAIWidget()->getMaxGap();
    float angleTolerance = m_pProcessingPanel->getAIWidget()->getAngleTolerance();

    // TEED_ExtractLines 호출
    if (!visionProcessor.TEED_ExtractLines(edgeMat, hLines, vLines, threshold, minLength, maxGap, angleTolerance)) {
        QMessageBox::warning(this, "Hough Lines Error", "Failed to extract lines from edge image.");
        return QImage();
    }

    qDebug() << "[TEED Hough] Horizontal lines:" << hLines.size()
             << "Vertical lines:" << vLines.size();

    // 원본 이미지에 직선 그리기
    cv::Mat result;
    cv::Mat inputMat = QImageToCvMat(m_pHalconViewer->getImage());
    if (inputMat.channels() == 1) {
        cv::cvtColor(inputMat, result, cv::COLOR_GRAY2BGR);
    } else {
        result = inputMat.clone();
    }

    int width = result.cols;
    int height = result.rows;

    // 수평선 그리기 (녹색)
    for (const auto& line : hLines) {
        // y = coef1 * x + coef2
        float a = line.coef1;
        float b = line.coef2;

        cv::Point pt1(0, static_cast<int>(b));
        cv::Point pt2(width - 1, static_cast<int>(a * (width - 1) + b));
        cv::line(result, pt1, pt2, cv::Scalar(0, 255, 0), 2);
    }

    // 수직선 그리기 (파란색)
    for (const auto& line : vLines) {
        // x = coef1 * y + coef2
        float c = line.coef1;
        float d = line.coef2;

        cv::Point pt1(static_cast<int>(d), 0);
        cv::Point pt2(static_cast<int>(c * (height - 1) + d), height - 1);
        cv::line(result, pt1, pt2, cv::Scalar(255, 0, 0), 2);
    }

    // cv::Mat -> QImage 변환
    QImage resultImage = QImage((const uchar*)result.data, result.cols, result.rows,
                                result.step, QImage::Format_RGB888).rgbSwapped();
    return resultImage.copy();
}

QImage MainWindow::applyTEEDClustering(const QImage& edgeImage)
{
    if (edgeImage.isNull()) {
        return QImage();
    }

    // QImage -> cv::Mat 변환
    cv::Mat edgeMat;
    if (edgeImage.format() == QImage::Format_Grayscale8) {
        edgeMat = cv::Mat(edgeImage.height(), edgeImage.width(), CV_8UC1,
                         (void*)edgeImage.constBits(), edgeImage.bytesPerLine()).clone();
    } else {
        QImage gray = edgeImage.convertToFormat(QImage::Format_Grayscale8);
        edgeMat = cv::Mat(gray.height(), gray.width(), CV_8UC1,
                         (void*)gray.constBits(), gray.bytesPerLine()).clone();
    }

    // Fnc_Vision_Pre_FITO를 사용하여 직선 추출 및 클러스터링
    static Fnc_Vision_Pre_FITO visionProcessor;

    std::vector<TEEDLineInfo> hLines, vLines;

    // UI에서 파라미터 가져오기
    int threshold = m_pProcessingPanel->getAIWidget()->getThreshold();
    int minLength = m_pProcessingPanel->getAIWidget()->getMinLength();
    int maxGap = m_pProcessingPanel->getAIWidget()->getMaxGap();
    float angleTolerance = m_pProcessingPanel->getAIWidget()->getAngleTolerance();
    float clusterDist = m_pProcessingPanel->getAIWidget()->getClusterDistance();

    // TEED_ExtractLines 호출
    if (!visionProcessor.TEED_ExtractLines(edgeMat, hLines, vLines, threshold, minLength, maxGap, angleTolerance)) {
        QMessageBox::warning(this, "Clustering Error", "Failed to extract lines from edge image.");
        return QImage();
    }

    // 클러스터링 수행
    std::vector<TEEDLineCluster> hClusters, vClusters;
    visionProcessor.TEED_ClusterLines(hLines, hClusters, clusterDist);
    visionProcessor.TEED_ClusterLines(vLines, vClusters, clusterDist);

    qDebug() << "[TEED Clustering] H clusters:" << hClusters.size()
             << "V clusters:" << vClusters.size();

    // 원본 이미지에 최적 직선 그리기
    cv::Mat result;
    cv::Mat inputMat = QImageToCvMat(m_pHalconViewer->getImage());
    if (inputMat.channels() == 1) {
        cv::cvtColor(inputMat, result, cv::COLOR_GRAY2BGR);
    } else {
        result = inputMat.clone();
    }

    int width = result.cols;
    int height = result.rows;

    // 최적 수평선 1개 그리기 (녹색) - 총 길이가 가장 긴 클러스터
    if (!hClusters.empty()) {
        const auto& cluster = hClusters[0];  // 첫 번째 = 가장 긴 클러스터
        float a = cluster.coef1;
        float b = cluster.coef2;

        cv::Point pt1(0, static_cast<int>(b));
        cv::Point pt2(width - 1, static_cast<int>(a * (width - 1) + b));
        cv::line(result, pt1, pt2, cv::Scalar(0, 255, 0), 3);

        // 텍스트로 직선 정보 표시
        QString info = QString("Best H: y=%.3fx+%.1f (L:%.0f)")
                       .arg(a).arg(b).arg(cluster.totalLength);
        cv::putText(result, info.toStdString(),
                   cv::Point(10, static_cast<int>(b) - 5),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
    }

    // 최적 수직선 1개 그리기 (파란색) - 총 길이가 가장 긴 클러스터
    if (!vClusters.empty()) {
        const auto& cluster = vClusters[0];  // 첫 번째 = 가장 긴 클러스터
        float c = cluster.coef1;
        float d = cluster.coef2;

        cv::Point pt1(static_cast<int>(d), 0);
        cv::Point pt2(static_cast<int>(c * (height - 1) + d), height - 1);
        cv::line(result, pt1, pt2, cv::Scalar(255, 0, 0), 3);

        // 텍스트로 직선 정보 표시
        QString info = QString("Best V: x=%.3fy+%.1f (L:%.0f)")
                       .arg(c).arg(d).arg(cluster.totalLength);
        cv::putText(result, info.toStdString(),
                   cv::Point(static_cast<int>(d) + 5, 20),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);
    }

    // 요약 정보 표시
    QString summary = QString("Best Lines (from H:%1, V:%2 clusters)")
                      .arg(hClusters.size()).arg(vClusters.size());
    cv::putText(result, summary.toStdString(),
               cv::Point(10, height - 10),
               cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 2);

    // cv::Mat -> QImage 변환
    QImage resultImage = QImage((const uchar*)result.data, result.cols, result.rows,
                                result.step, QImage::Format_RGB888).rgbSwapped();
    return resultImage.copy();
}
