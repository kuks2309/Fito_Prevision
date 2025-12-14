#ifndef IMAGEPROCESSINGPANEL_H
#define IMAGEPROCESSINGPANEL_H

#include <QWidget>
#include <QTabWidget>
#include "EdgeProcessingWidget.h"

// Forward declarations
class LineDetectionWidget;
class AIPrevisionWidget;
class HalconVisionWidget;

/**
 * @brief 이미지 처리 패널 (TabWidget 컨테이너)
 * @details 여러 이미지 처리 탭을 포함하는 메인 패널
 */
class ImageProcessingPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ImageProcessingPanel(QWidget *parent = nullptr);
    ~ImageProcessingPanel();

    // Tab 인덱스
    enum TabIndex {
        TAB_EDGE = 0,
        TAB_LINE = 1,
        TAB_AI_PREVISION = 2,
        TAB_HALCON_VISION = 3
    };

    // 위젯 접근자
    EdgeProcessingWidget* getEdgeWidget() const { return m_pEdgeWidget; }
    LineDetectionWidget* getLineWidget() const { return m_pLineWidget; }
    AIPrevisionWidget* getAIWidget() const { return m_pAIWidget; }
    HalconVisionWidget* getHalconWidget() const { return m_pHalconWidget; }

    // 현재 탭
    int getCurrentTab() const;

signals:
    void tabChanged(int index);
    void processingRequested();

private slots:
    void onTabChanged(int index);

private:
    QTabWidget* m_pTabWidget;

    // Processing Widgets
    EdgeProcessingWidget* m_pEdgeWidget;
    LineDetectionWidget* m_pLineWidget;
    AIPrevisionWidget* m_pAIWidget;
    HalconVisionWidget* m_pHalconWidget;

    void setupTabs();
};

#endif // IMAGEPROCESSINGPANEL_H
