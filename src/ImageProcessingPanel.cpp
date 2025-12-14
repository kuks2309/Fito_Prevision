#include "ImageProcessingPanel.h"
#include "LineDetectionWidget.h"
#include "AIPrevisionWidget.h"
#include "HalconVisionWidget.h"
#include <QVBoxLayout>

ImageProcessingPanel::ImageProcessingPanel(QWidget *parent)
    : QWidget(parent)
    , m_pTabWidget(new QTabWidget(this))
    , m_pEdgeWidget(nullptr)
    , m_pLineWidget(nullptr)
    , m_pAIWidget(nullptr)
    , m_pHalconWidget(nullptr)
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
    // Tab 0: Halcon Vision
    m_pHalconWidget = new HalconVisionWidget(this);
    m_pTabWidget->addTab(m_pHalconWidget, "Halcon Vision");

    // Tab 1: AI Prevision
    m_pAIWidget = new AIPrevisionWidget(this);
    m_pTabWidget->addTab(m_pAIWidget, "AI Prevision");

    // Tab 2: Edge Detection
    m_pEdgeWidget = new EdgeProcessingWidget(this);
    m_pTabWidget->addTab(m_pEdgeWidget, "Edge Detection");

    // Tab 3: Line Detection (Hough, LSD)
    m_pLineWidget = new LineDetectionWidget(this);
    m_pTabWidget->addTab(m_pLineWidget, "Line Detection");
}

int ImageProcessingPanel::getCurrentTab() const
{
    return m_pTabWidget->currentIndex();
}

void ImageProcessingPanel::onTabChanged(int index)
{
    emit tabChanged(index);
}
