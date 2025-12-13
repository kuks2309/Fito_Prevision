#ifndef AIPREVISIONWIDGET_H
#define AIPREVISIONWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QSettings>
#include <opencv2/opencv.hpp>

namespace Ui {
class AIPrevisionWidget;
}

/**
 * @brief AI Prevision 위젯
 * @details AI 기반 영상 처리 알고리즘 테스트 및 파라미터 조정 UI
 */
class AIPrevisionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AIPrevisionWidget(QWidget *parent = nullptr);
    ~AIPrevisionWidget();

    // AI 알고리즘 종류
    enum AIAlgorithm {
        TEED_EDGE_DETECTION = 0  // TEED Edge Detection (Tiny and Efficient Edge Detector)
    };

    // 현재 선택된 알고리즘 가져오기
    AIAlgorithm getCurrentAlgorithm() const { return m_currentAlgorithm; }

    // Hough 파라미터 가져오기
    int getThreshold() const;
    int getMinLength() const;
    int getMaxGap() const;
    float getAngleTolerance() const;
    float getClusterDistance() const;

signals:
    void algorithmChanged(AIAlgorithm algorithm);
    void parametersChanged();
    void applyRequested();
    void houghLinesRequested();
    void clusteringRequested();
    void resetRequested();

private slots:
    void onAlgorithmSelected(int row, int column);
    void onApplyClicked();
    void onHoughLinesClicked();
    void onClusteringClicked();
    void onResetClicked();
    void onParameterChanged();
    void onSaveParamsClicked();
    void onLoadParamsClicked();

private:
    Ui::AIPrevisionWidget *ui;
    AIAlgorithm m_currentAlgorithm;

    void setupTable();
    void updateParameterVisibility();
    void loadAlgorithms();

    // Config path
    QString getConfigPath() const;
    static const QString CONFIG_FILENAME;
};

#endif // AIPREVISIONWIDGET_H
