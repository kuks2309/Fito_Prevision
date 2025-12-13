#ifndef IMAGEPROCESSINGPANEL_H
#define IMAGEPROCESSINGPANEL_H

#include <QWidget>
#include <QTabWidget>
#include "EdgeProcessingWidget.h"

// Forward declarations
class LineDetectionWidget;
class AIPrevisionWidget;

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
        TAB_CORNER = 3,
        TAB_MORPHOLOGY = 4
    };

    // 위젯 접근자
    EdgeProcessingWidget* getEdgeWidget() const { return m_pEdgeWidget; }
    LineDetectionWidget* getLineWidget() const { return m_pLineWidget; }
    AIPrevisionWidget* getAIWidget() const { return m_pAIWidget; }

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
    // CornerDetectionWidget* m_pCornerWidget;  // 향후 구현
    // MorphologyWidget* m_pMorphologyWidget;    // 향후 구현

    void setupTabs();
};

#endif // IMAGEPROCESSINGPANEL_H
