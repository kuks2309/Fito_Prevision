#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QString>
#include <opencv2/opencv.hpp>
#include "ImageViewer.h"
#include "ImageProcessingPanel.h"
#include "EdgeProcessingWidget.h"
#include "LineDetectionWidget.h"
#include "AIPrevisionWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief 메인 윈도우 클래스
 * @details Vision Pre 비교 툴의 메인 UI
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 파일 메뉴
    void onLoadImage();
    void onSaveImage();
    void onExit();

    // 보기 메뉴
    void onZoomIn();
    void onZoomOut();
    void onZoomFit();
    void onShowGrid(bool checked);

    // 분석 메뉴
    void onCompare();
    void onDifferenceMap();
    void onHistogram();

    // 설정 메뉴
    void onCameraSettings();
    void onCalibrationSettings();
    void onPreferences();

    // 도움말 메뉴
    void onUserManual();
    void onAbout();

    // 제어 패널
    void onGrab();
    void onContinuousGrab();
    void onStopGrab();
    void onViewModeChanged(int index);
    void onGainChanged(double value);
    void onOffsetChanged(int value);
    void onThresholdChanged(int value);

    // 연속 Grab 타이머
    void updateImages();

    // 이미지 뷰어 이벤트
    void onHalconImageClicked(const QPointF& pos);
    void onOpenCVImageClicked(const QPointF& pos);

    // Image Processing Panel
    void onProcessingTabChanged(int index);

    // Edge Processing
    void onEdgeAlgorithmChanged(EdgeProcessingWidget::EdgeAlgorithm algorithm);
    void onEdgeParametersChanged();
    void onEdgeApplyRequested();
    void onEdgeResetRequested();

    // Line Detection
    void onLineAlgorithmChanged(LineDetectionWidget::LineAlgorithm algorithm);
    void onLineParametersChanged();
    void onLineApplyRequested();
    void onLineResetRequested();

    // AI Prevision
    void onAIAlgorithmChanged(AIPrevisionWidget::AIAlgorithm algorithm);
    void onAIParametersChanged();
    void onAIApplyRequested();
    void onAIHoughLinesRequested();
    void onAIClusteringRequested();
    void onAIResetRequested();

private:
    Ui::MainWindow *ui;

    // Image Viewers
    ImageViewer* m_pHalconViewer;
    ImageViewer* m_pOpenCVViewer;

    // Processing Panel
    ImageProcessingPanel* m_pProcessingPanel;

    // Vision Wrappers (향후 구현)
    // VisionPreWrapper* m_pVisionHalcon;
    // VisionPreFITOWrapper* m_pVisionFITO;

    // Timer for continuous grab
    QTimer* m_pGrabTimer;
    bool m_bGrabbing;

    // Settings
    double m_gain;
    int m_offset;
    int m_threshold;
    int m_viewMode;  // 0: Raw, 1: Gain, 2: Binary

    // Helper functions
    void setupUI();
    void setupConnections();
    void setupViewers();
    void updateComparisonResults();
    void updateStatusBar(const QString& message);
    void loadDefaultTestImages();

    // 임시 테스트 이미지 생성 (개발용)
    QImage createTestImage(int width, int height, const QColor& color);

    // Edge Processing Helper
    void applyEdgeDetection(EdgeProcessingWidget::EdgeAlgorithm algorithm);
    QImage applyCanny(const QImage& input, int lowThreshold, int highThreshold, int apertureSize, bool l2Gradient);
    QImage applySobel(const QImage& input, int ksize, double scale, double delta);
    QImage applyScharr(const QImage& input, double scale, double delta);
    QImage applyLaplacian(const QImage& input, int ksize, double scale, double delta);
    QImage applyPrewitt(const QImage& input);
    QImage applyRoberts(const QImage& input);

    // Sobel Prevision (Bilateral Filter + Sobel XY + Threshold)
    cv::Mat sobel_prevision(const cv::Mat& input, int bilateralD = 9,
                            double sigmaColor = 75.0, double sigmaSpace = 75.0,
                            int threshold = 30);

    // Line Detection Helper
    void applyLineDetection(LineDetectionWidget::LineAlgorithm algorithm);
    QImage applyHoughLines(const QImage& input, double rho, double theta, int threshold);
    QImage applyHoughLinesP(const QImage& input, double rho, double theta, int threshold, double minLineLength, double maxLineGap);
    QImage applyLSD(const QImage& input, double scale);

    // AI Prevision Helper
    void applyAIPrevision(AIPrevisionWidget::AIAlgorithm algorithm);
    QImage applyCNNMLCCPanelEdge(const QImage& input);  // CNN MLCC Panel Edge (Not implemented)
    QImage applyTEEDEdgeDetection(const QImage& input); // TEED Edge Detection
    QImage applyTEEDHoughLines(const QImage& edgeImage); // TEED Hough Lines
    QImage applyTEEDClustering(const QImage& edgeImage); // TEED Clustering

    // Image conversion helper
    cv::Mat QImageToCvMat(const QImage& image);

    // Current images for processing
    QImage m_currentInputImage;
    QImage m_currentProcessedImage;
    QImage m_currentEdgeImage;  // Edge 처리 결과 (Line Detection용)
};

#endif // MAINWINDOW_H
