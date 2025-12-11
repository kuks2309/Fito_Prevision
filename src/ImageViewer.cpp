#include "ImageViewer.h"
#include <QPainter>
#include <QPaintEvent>
#include <QFileInfo>
#include <QDebug>

ImageViewer::ImageViewer(QWidget *parent)
    : QWidget(parent)
    , m_zoomFactor(1.0)
    , m_minZoom(0.1)
    , m_maxZoom(10.0)
{
    // 배경색 설정
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);

    // 최소 크기 설정
    setMinimumSize(400, 300);
}

ImageViewer::~ImageViewer()
{
}

// ========================================================================
// 이미지 설정
// ========================================================================

void ImageViewer::setImage(const QImage& image)
{
    m_originalImage = image;

    if (!m_originalImage.isNull())
    {
        // 화면에 맞게 줌 설정
        zoomFit();
    }
    else
    {
        m_displayImage = QImage();
    }

    update();
}

void ImageViewer::setImage(const unsigned char* data, int width, int height)
{
    if (data == nullptr || width <= 0 || height <= 0)
    {
        clearImage();
        return;
    }

    // BYTE 배열 → QImage (8-bit Grayscale)
    QImage image(width, height, QImage::Format_Grayscale8);
    memcpy(image.bits(), data, width * height);

    setImage(image);
}

void ImageViewer::setImage(const QString& filePath)
{
    QImage image(filePath);

    if (image.isNull())
    {
        qWarning() << "Failed to load image:" << filePath;
        clearImage();
        return;
    }

    setImage(image);
}

void ImageViewer::clearImage()
{
    m_originalImage = QImage();
    m_displayImage = QImage();
    clearGraphics();
    update();
}

QImage ImageViewer::getImage() const
{
    return m_originalImage;
}

bool ImageViewer::hasImage() const
{
    return !m_originalImage.isNull();
}

// ========================================================================
// 제목 및 정보
// ========================================================================

void ImageViewer::setTitle(const QString& title)
{
    m_title = title;
    update();
}

void ImageViewer::setInfo(const QString& info)
{
    m_info = info;
    update();
}

// ========================================================================
// 확대/축소
// ========================================================================

void ImageViewer::zoomIn()
{
    setZoom(m_zoomFactor * 1.2);
}

void ImageViewer::zoomOut()
{
    setZoom(m_zoomFactor / 1.2);
}

void ImageViewer::zoomFit()
{
    if (m_originalImage.isNull()) return;

    double scaleX = (double)width() / m_originalImage.width();
    double scaleY = (double)height() / m_originalImage.height();
    double scale = qMin(scaleX, scaleY);

    setZoom(scale);
}

void ImageViewer::setZoom(double factor)
{
    m_zoomFactor = qBound(m_minZoom, factor, m_maxZoom);
    updateDisplayImage();
    emit zoomChanged(m_zoomFactor);
    update();
}

// ========================================================================
// 그래픽 오버레이
// ========================================================================

void ImageViewer::drawCross(const QPointF& center, int size, const QColor& color, int lineWidth)
{
    GraphicsItem item;
    item.type = GraphicsItem::Cross;
    item.pos1 = center;
    item.lineWidth = lineWidth;
    item.rect = QRectF(center.x() - size/2, center.y() - size/2, size, size);
    item.color = color;
    m_graphics.append(item);
    update();
}

void ImageViewer::drawLine(const QPointF& p1, const QPointF& p2, const QColor& color, int lineWidth)
{
    GraphicsItem item;
    item.type = GraphicsItem::Line;
    item.pos1 = p1;
    item.pos2 = p2;
    item.color = color;
    item.lineWidth = lineWidth;
    m_graphics.append(item);
    update();
}

void ImageViewer::drawRect(const QRectF& rect, const QColor& color, int lineWidth)
{
    GraphicsItem item;
    item.type = GraphicsItem::Rectangle;
    item.rect = rect;
    item.color = color;
    item.lineWidth = lineWidth;
    m_graphics.append(item);
    update();
}

void ImageViewer::drawText(const QPointF& pos, const QString& text, const QColor& color, int fontSize)
{
    GraphicsItem item;
    item.type = GraphicsItem::Text;
    item.pos1 = pos;
    item.text = text;
    item.color = color;
    item.fontSize = fontSize;
    m_graphics.append(item);
    update();
}

void ImageViewer::clearGraphics()
{
    m_graphics.clear();
    update();
}

// ========================================================================
// 이미지 저장
// ========================================================================

bool ImageViewer::saveImage(const QString& filePath)
{
    if (m_originalImage.isNull())
    {
        qWarning() << "No image to save";
        return false;
    }

    return m_originalImage.save(filePath);
}

// ========================================================================
// 이벤트 핸들러
// ========================================================================

void ImageViewer::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    // 배경 그리기
    painter.fillRect(rect(), Qt::black);

    if (m_displayImage.isNull())
    {
        // 이미지가 없을 때
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "이미지 없음");
        return;
    }

    // 이미지 중앙 정렬
    int x = (width() - m_displayImage.width()) / 2;
    int y = (height() - m_displayImage.height()) / 2;

    // 이미지 그리기
    painter.drawImage(x, y, m_displayImage);

    // 그래픽 오버레이 그리기
    for (const auto& item : m_graphics)
    {
        painter.setPen(QPen(item.color, item.lineWidth));

        switch (item.type)
        {
        case GraphicsItem::Cross:
        {
            QPointF center = imageToScreen(item.pos1);
            int halfSize = item.rect.width() * m_zoomFactor / 2;

            painter.drawLine(center.x() - halfSize, center.y(),
                           center.x() + halfSize, center.y());
            painter.drawLine(center.x(), center.y() - halfSize,
                           center.x(), center.y() + halfSize);
            break;
        }

        case GraphicsItem::Line:
        {
            QPointF p1 = imageToScreen(item.pos1);
            QPointF p2 = imageToScreen(item.pos2);
            painter.drawLine(p1, p2);
            break;
        }

        case GraphicsItem::Rectangle:
        {
            QPointF topLeft = imageToScreen(item.rect.topLeft());
            QPointF bottomRight = imageToScreen(item.rect.bottomRight());
            painter.drawRect(QRectF(topLeft, bottomRight));
            break;
        }

        case GraphicsItem::Text:
        {
            QPointF pos = imageToScreen(item.pos1);
            QFont font = painter.font();
            font.setPointSize(item.fontSize);
            painter.setFont(font);
            painter.drawText(pos, item.text);
            break;
        }
        }
    }

    // 제목 표시
    if (!m_title.isEmpty())
    {
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPointSize(12);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(10, 20, m_title);
    }

    // 정보 표시
    if (!m_info.isEmpty())
    {
        painter.setPen(Qt::lightGray);
        QFont font = painter.font();
        font.setPointSize(10);
        painter.setFont(font);
        painter.drawText(10, height() - 10, m_info);
    }
}

void ImageViewer::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        QPointF imagePos = screenToImage(event->pos());
        emit imageClicked(imagePos);
    }

    QWidget::mousePressEvent(event);
}

void ImageViewer::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        QPointF imagePos = screenToImage(event->pos());
        emit imageDoubleClicked(imagePos);
    }

    QWidget::mouseDoubleClickEvent(event);
}

void ImageViewer::wheelEvent(QWheelEvent* event)
{
    // Ctrl + 휠: 확대/축소
    if (event->modifiers() & Qt::ControlModifier)
    {
        if (event->angleDelta().y() > 0)
        {
            zoomIn();
        }
        else
        {
            zoomOut();
        }

        event->accept();
    }
    else
    {
        QWidget::wheelEvent(event);
    }
}

void ImageViewer::resizeEvent(QResizeEvent* event)
{
    // 창 크기 변경 시 화면에 맞게 줌 조정 (선택사항)
    // zoomFit();

    QWidget::resizeEvent(event);
}

// ========================================================================
// Helper 함수
// ========================================================================

void ImageViewer::updateDisplayImage()
{
    if (m_originalImage.isNull())
    {
        m_displayImage = QImage();
        return;
    }

    int newWidth = m_originalImage.width() * m_zoomFactor;
    int newHeight = m_originalImage.height() * m_zoomFactor;

    m_displayImage = m_originalImage.scaled(newWidth, newHeight,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);
}

QPointF ImageViewer::screenToImage(const QPointF& screenPos) const
{
    if (m_displayImage.isNull()) return QPointF();

    int x = (width() - m_displayImage.width()) / 2;
    int y = (height() - m_displayImage.height()) / 2;

    double imageX = (screenPos.x() - x) / m_zoomFactor;
    double imageY = (screenPos.y() - y) / m_zoomFactor;

    return QPointF(imageX, imageY);
}

QPointF ImageViewer::imageToScreen(const QPointF& imagePos) const
{
    if (m_displayImage.isNull()) return QPointF();

    int x = (width() - m_displayImage.width()) / 2;
    int y = (height() - m_displayImage.height()) / 2;

    double screenX = imagePos.x() * m_zoomFactor + x;
    double screenY = imagePos.y() * m_zoomFactor + y;

    return QPointF(screenX, screenY);
}
