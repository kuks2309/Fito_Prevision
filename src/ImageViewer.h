#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPoint>
#include <QVector>

/**
 * @brief 이미지 뷰어 위젯
 * @details 이미지 표시, 확대/축소, 그래픽 오버레이 기능 제공
 */
class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);
    ~ImageViewer();

    // 이미지 설정
    void setImage(const QImage& image);
    void setImage(const unsigned char* data, int width, int height);
    void setImage(const QString& filePath);
    void clearImage();

    // 이미지 가져오기
    QImage getImage() const;
    bool hasImage() const;

    // 제목 및 정보
    void setTitle(const QString& title);
    void setInfo(const QString& info);

    // 확대/축소
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void setZoom(double factor);
    double getZoom() const { return m_zoomFactor; }

    // 그래픽 오버레이
    void drawCross(const QPointF& center, int size, const QColor& color = Qt::green, int lineWidth = 2);
    void drawLine(const QPointF& p1, const QPointF& p2, const QColor& color = Qt::green, int lineWidth = 2);
    void drawRect(const QRectF& rect, const QColor& color = Qt::green, int lineWidth = 2);
    void drawText(const QPointF& pos, const QString& text, const QColor& color = Qt::green, int fontSize = 12);
    void clearGraphics();

    // 이미지 저장
    bool saveImage(const QString& filePath);

signals:
    void imageClicked(const QPointF& pos);
    void imageDoubleClicked(const QPointF& pos);
    void zoomChanged(double factor);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    // 이미지 데이터
    QImage m_originalImage;   // 원본 이미지
    QImage m_displayImage;    // 표시용 이미지 (확대/축소 적용)

    // 확대/축소
    double m_zoomFactor;
    double m_minZoom;
    double m_maxZoom;

    // 제목 및 정보
    QString m_title;
    QString m_info;

    // 그래픽 오버레이
    struct GraphicsItem {
        enum Type { Cross, Line, Rectangle, Text } type;
        QPointF pos1;
        QPointF pos2;
        QRectF rect;
        QString text;
        QColor color;
        int lineWidth;
        int fontSize;
    };
    QVector<GraphicsItem> m_graphics;

    // Helper 함수
    void updateDisplayImage();
    QPointF screenToImage(const QPointF& screenPos) const;
    QPointF imageToScreen(const QPointF& imagePos) const;
};

#endif // IMAGEVIEWER_H
