#include "ImageProcessingPanel.h"
#include "LineDetectionWidget.h"
#include "AIPrevisionWidget.h"
#include <QVBoxLayout>

ImageProcessingPanel::ImageProcessingPanel(QWidget *parent)
    : QWidget(parent)
    , m_pTabWidget(new QTabWidget(this))
    , m_pEdgeWidget(nullptr)
    , m_pLineWidget(nullptr)
    , m_pAIWidget(nullptr)
{
    setupTabs();

    // Main layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_pTabWidget);

    // Connect tab change signal
    connect(m_pTabWidget, &QTabWidget::currentChanged,
            this, &ImageProcessingPanel::onTabChanged);
}

ImageProcessingPanel::~ImageProcessingPanel()
{
}

void ImageProcessingPanel::setupTabs()
{
    // Tab 1: Edge Detection
    m_pEdgeWidget = new EdgeProcessingWidget(this);
    m_pTabWidget->addTab(m_pEdgeWidget, "Edge Detection");

    // Tab 2: Line Detection (Hough, LSD)
    m_pLineWidget = new LineDetectionWidget(this);
    m_pTabWidget->addTab(m_pLineWidget, "Line Detection");

    // Tab 3: AI Prevision
    m_pAIWidget = new AIPrevisionWidget(this);
    m_pTabWidget->addTab(m_pAIWidget, "AI Prevision");

    // Tab 4: Corner Detection (향후 구현)
    QWidget* cornerPlaceholder = new QWidget(this);
    m_pTabWidget->addTab(cornerPlaceholder, "Corner Detection");
    m_pTabWidget->setTabEnabled(TAB_CORNER, false);

    // Tab 5: Morphology (향후 구현)
    QWidget* morphologyPlaceholder = new QWidget(this);
    m_pTabWidget->addTab(morphologyPlaceholder, "Morphology");
    m_pTabWidget->setTabEnabled(TAB_MORPHOLOGY, false);
}

int ImageProcessingPanel::getCurrentTab() const
{
    return m_pTabWidget->currentIndex();
}

void ImageProcessingPanel::onTabChanged(int index)
{
    emit tabChanged(index);
}
