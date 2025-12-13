#ifndef LINEDETECTIONWIDGET_H
#define LINEDETECTIONWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QSettings>
#include <opencv2/opencv.hpp>

namespace Ui {
class LineDetectionWidget;
}

/**
 * @brief Line Detection 알고리즘 테스트 위젯
 * @details Hough Transform, LSD 등 직선 검출 알고리즘 테스트
 */
class LineDetectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LineDetectionWidget(QWidget *parent = nullptr);
    ~LineDetectionWidget();

    // Line Detection 알고리즘 종류
    enum LineAlgorithm {
        HOUGH_LINES,        // Standard Hough Line Transform
        HOUGH_LINES_P,      // Probabilistic Hough Line Transform
        LSD                 // Line Segment Detector
    };

    // 현재 선택된 알고리즘 가져오기
    LineAlgorithm getCurrentAlgorithm() const { return m_currentAlgorithm; }

    // 파라미터 가져오기
    double getRho() const;
    double getTheta() const;
    int getThreshold() const;
    double getMinLineLength() const;
    double getMaxLineGap() const;
    double getLsdScale() const;

signals:
    void algorithmChanged(LineAlgorithm algorithm);
    void parametersChanged();
    void applyRequested();
    void resetRequested();

private slots:
    void onAlgorithmSelected(int row, int column);
    void onApplyClicked();
    void onResetClicked();
    void onParameterChanged();
    void onSaveParamsClicked();
    void onLoadParamsClicked();

private:
    Ui::LineDetectionWidget *ui;
    LineAlgorithm m_currentAlgorithm;

    void setupTable();
    void updateParameterVisibility();
    void loadAlgorithms();

    // Config path
    QString getConfigPath() const;
    static const QString CONFIG_FILENAME;
};

#endif // LINEDETECTIONWIDGET_H
