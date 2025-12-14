#ifndef HALCONVISIONWIDGET_H
#define HALCONVISIONWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QSettings>

#ifdef HALCON_FOUND
#include <HalconCpp.h>
using namespace HalconCpp;
#endif

namespace Ui {
class HalconVisionWidget;
}

// 코너 타입 정의 (Fnc_Vision_Pre.cpp와 동일)
#define _RU 0  // Right-Up
#define _RL 1  // Right-Low
#define _LU 2  // Left-Up
#define _LL 3  // Left-Low

// Vision 결과 구조체
struct HalconVisionResult {
    bool bExist = false;
    bool bResult = false;
    double dPositionX = 0.0;
    double dPositionY = 0.0;
    double dScore = 0.0;

    void Reset() {
        bExist = false;
        bResult = false;
        dPositionX = 0.0;
        dPositionY = 0.0;
        dScore = 0.0;
    }
};

/**
 * @brief Halcon Vision Widget
 * @details Halcon 기반 코너 검출 및 Shape Model 매칭 UI
 *          Fnc_Vision_Pre.cpp의 기능을 100% 동일하게 구현
 */
class HalconVisionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HalconVisionWidget(QWidget *parent = nullptr);
    ~HalconVisionWidget();

    // 코너 종류
    enum CornerType {
        CORNER_RU = 0,  // Right-Up
        CORNER_RL = 1,  // Right-Low
        CORNER_LU = 2,  // Left-Up
        CORNER_LL = 3   // Left-Low
    };

    // 알고리즘 종류
    enum HalconAlgorithm {
        CORNER_DETECTION = 0  // Shape Model 기반 코너 검출
    };

    // 현재 선택된 알고리즘 가져오기
    HalconAlgorithm getCurrentAlgorithm() const { return m_currentAlgorithm; }

    // 코너 타입 가져오기
    CornerType getCornerType() const;

    // Shape Model 파라미터 가져오기
    int getMatchingRate() const;
    int getROISize() const;

    // Image Processing 파라미터 가져오기
    double getGain() const;
    int getOffset() const;
    int getBinaryThreshold() const;
    int getOpenX() const;
    int getOpenY() const;
    int getCloseX() const;
    int getCloseY() const;

    // Tape Inspection 파라미터 가져오기
    bool getUseTapeInspection() const;
    int getMaxIntersection() const;
    int getStartGap() const;
    int getStartEndLimit() const;
    int getCropSize() const;
    double getSigma() const;
    int getLow() const;
    int getHigh() const;
    double getMoveStep() const;

    // 결과 표시
    void setResult(double posX, double posY, double score);
    void setExecutionTime(double ms);

    // ========================================================================
    // Halcon Vision 핵심 함수 (Fnc_Vision_Pre.cpp와 100% 동일)
    // ========================================================================
#ifdef HALCON_FOUND
    // Halcon 라이센스 체크
    bool checkHalconLicense();

    // Shape Model 생성 (4개 코너)
    void HV_PreVisionCreateTemplate(int nCorner);

    // 코너 검출 (Shape Model 매칭 + 교점 계산)
    HalconVisionResult HV_PreVisionFindCornerTemplate(int nCorner, const QImage& inputImage);

    // 이미지 전처리 (Gain, Offset, Binary, Open, Close)
    void HV_ImgProcess_Prepare2(HObject& Ho_Processimage, HObject& Ho_GainImage, HObject& Ho_BinImage);

    // Shape Model ID (4개 코너)
    HTuple m_hv_ModelID[4];

    // 모델 초기화 여부
    bool m_bModelInitialized = false;

    // 라이센스 유효 여부
    bool m_bLicenseValid = false;
#endif

    // 결과 가져오기
    HalconVisionResult getLastResult() const { return m_lastResult; }

signals:
    void algorithmChanged(HalconAlgorithm algorithm);
    void parametersChanged();
    void findCornerRequested();
    void resetRequested();

private slots:
    void onAlgorithmSelected(int row, int column);
    void onFindCornerClicked();
    void onResetClicked();
    void onParameterChanged();
    void onSaveParamsClicked();
    void onLoadParamsClicked();

private:
    Ui::HalconVisionWidget *ui;
    HalconAlgorithm m_currentAlgorithm;
    HalconVisionResult m_lastResult;

    void setupTable();
    void loadAlgorithms();
    void initializeShapeModels();

    // Config path
    QString getConfigPath() const;
    static const QString CONFIG_FILENAME;
};

#endif // HALCONVISIONWIDGET_H
