#ifndef EDGEPROCESSINGWIDGET_H
#define EDGEPROCESSINGWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QSettings>
#include <opencv2/opencv.hpp>

namespace Ui {
class EdgeProcessingWidget;
}

/**
 * @brief Edge 추출 알고리즘 테스트 위젯
 * @details 다양한 Edge Detection 알고리즘을 테스트하고 파라미터를 조정할 수 있는 UI
 */
class EdgeProcessingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EdgeProcessingWidget(QWidget *parent = nullptr);
    ~EdgeProcessingWidget();

    // Edge 알고리즘 종류
    enum EdgeAlgorithm {
        CANNY,              // Canny Edge Detection
        SOBEL,              // Sobel Operator
        SOBEL_PREVISION,    // Sobel + Bilateral Filter + Threshold (MLCC용)
        SCHARR,             // Scharr Operator
        LAPLACIAN,          // Laplacian Operator
        PREWITT,            // Prewitt Operator
        ROBERTS             // Roberts Cross Operator
    };

    // 현재 선택된 알고리즘 가져오기
    EdgeAlgorithm getCurrentAlgorithm() const { return m_currentAlgorithm; }

    // 파라미터 가져오기
    int getParam1() const;
    int getParam2() const;
    int getParam3() const;
    int getParam4() const;
    double getParam5() const;
    double getParam6() const;
    bool getL2Gradient() const;

signals:
    void algorithmChanged(EdgeAlgorithm algorithm);
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
    Ui::EdgeProcessingWidget *ui;
    EdgeAlgorithm m_currentAlgorithm;

    void setupTable();
    void updateParameterVisibility();
    void loadAlgorithms();

    // Config path
    QString getConfigPath() const;
    static const QString CONFIG_FILENAME;
};

#endif // EDGEPROCESSINGWIDGET_H
