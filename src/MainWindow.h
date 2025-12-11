#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QString>
#include "ImageViewer.h"

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

private:
    Ui::MainWindow *ui;

    // Image Viewers
    ImageViewer* m_pHalconViewer;
    ImageViewer* m_pOpenCVViewer;

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
};

#endif // MAINWINDOW_H
