#include "HalconVisionWidget.h"
#include "ui_HalconVisionWidget.h"
#include <QDir>
#include <QMessageBox>
#include <QDebug>

const QString HalconVisionWidget::CONFIG_FILENAME = "halcon_vision_params.ini";

HalconVisionWidget::HalconVisionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HalconVisionWidget),
    m_currentAlgorithm(CORNER_DETECTION)
{
    ui->setupUi(this);

    setupTable();
    loadAlgorithms();

    // 시그널 연결
    connect(ui->tableAlgorithms, &QTableWidget::cellClicked,
            this, &HalconVisionWidget::onAlgorithmSelected);
    connect(ui->btnFindCorner, &QPushButton::clicked,
            this, &HalconVisionWidget::onFindCornerClicked);
    connect(ui->btnReset, &QPushButton::clicked,
            this, &HalconVisionWidget::onResetClicked);
    connect(ui->btnSaveParams, &QPushButton::clicked,
            this, &HalconVisionWidget::onSaveParamsClicked);
    connect(ui->btnLoadParams, &QPushButton::clicked,
            this, &HalconVisionWidget::onLoadParamsClicked);

    // 파라미터 변경 시그널 연결
    connect(ui->comboCorner, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &HalconVisionWidget::onParameterChanged);
    connect(ui->spinMatchingRate, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &HalconVisionWidget::onParameterChanged);
    connect(ui->spinROISize, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &HalconVisionWidget::onParameterChanged);
    connect(ui->spinGain, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &HalconVisionWidget::onParameterChanged);
    connect(ui->spinOffset, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &HalconVisionWidget::onParameterChanged);
    connect(ui->spinBinaryThreshold, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &HalconVisionWidget::onParameterChanged);
    connect(ui->spinOpenX, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &HalconVisionWidget::onParameterChanged);
    connect(ui->spinOpenY, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &HalconVisionWidget::onParameterChanged);
    connect(ui->spinCloseX, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &HalconVisionWidget::onParameterChanged);
    connect(ui->spinCloseY, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &HalconVisionWidget::onParameterChanged);

    // Shape Model 초기화
    initializeShapeModels();
}

HalconVisionWidget::~HalconVisionWidget()
{
#ifdef HALCON_FOUND
    // Shape Model 해제
    for (int i = 0; i < 4; ++i) {
        if (m_hv_ModelID[i].Length() > 0) {
            try {
                ClearShapeModel(m_hv_ModelID[i]);
            } catch (...) {}
        }
    }
#endif
    delete ui;
}

void HalconVisionWidget::setupTable()
{
    ui->tableAlgorithms->setColumnCount(2);
    ui->tableAlgorithms->setHorizontalHeaderLabels({"Algorithm", "Description"});
    ui->tableAlgorithms->horizontalHeader()->setStretchLastSection(true);
    ui->tableAlgorithms->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableAlgorithms->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void HalconVisionWidget::loadAlgorithms()
{
    struct AlgorithmInfo {
        HalconAlgorithm algo;
        QString name;
        QString description;
    };

    QVector<AlgorithmInfo> algorithms = {
        {CORNER_DETECTION, "Corner Detection", "Shape Model based L-corner detection (Fnc_Vision_Pre)"}
    };

    ui->tableAlgorithms->setRowCount(algorithms.size());

    for (int i = 0; i < algorithms.size(); ++i) {
        QTableWidgetItem *nameItem = new QTableWidgetItem(algorithms[i].name);
        nameItem->setData(Qt::UserRole, algorithms[i].algo);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);

        QTableWidgetItem *descItem = new QTableWidgetItem(algorithms[i].description);
        descItem->setFlags(descItem->flags() & ~Qt::ItemIsEditable);

        ui->tableAlgorithms->setItem(i, 0, nameItem);
        ui->tableAlgorithms->setItem(i, 1, descItem);
    }

    // 첫 번째 알고리즘 선택
    ui->tableAlgorithms->selectRow(0);
}

void HalconVisionWidget::initializeShapeModels()
{
#ifdef HALCON_FOUND
    qDebug() << "[HalconVisionWidget] Checking Halcon License...";

    // 1. 먼저 라이센스 체크
    if (!checkHalconLicense()) {
        qDebug() << "[HalconVisionWidget] Halcon License check failed!";
        m_bLicenseValid = false;
        m_bModelInitialized = false;
        return;
    }

    m_bLicenseValid = true;
    qDebug() << "[HalconVisionWidget] Halcon License valid. Creating L-corner Shape Models...";

    // 2. L 타입 패턴(Shape Model) 생성 - 4개 코너
    try {
        for (int i = 0; i < 4; ++i) {
            HV_PreVisionCreateTemplate(i);
            qDebug() << "[HalconVisionWidget] Created Shape Model for corner" << i;
        }

        m_bModelInitialized = true;
        qDebug() << "[HalconVisionWidget] All Shape Models initialized successfully";
    }
    catch (HException& e) {
        qDebug() << "[HalconVisionWidget] Shape Model creation failed:" << e.ErrorMessage().Text();
        m_bModelInitialized = false;
    }
#else
    qDebug() << "[HalconVisionWidget] HALCON not found, Shape Models not initialized";
#endif
}

HalconVisionWidget::CornerType HalconVisionWidget::getCornerType() const
{
    return static_cast<CornerType>(ui->comboCorner->currentIndex());
}

int HalconVisionWidget::getMatchingRate() const
{
    return ui->spinMatchingRate->value();
}

int HalconVisionWidget::getROISize() const
{
    return ui->spinROISize->value();
}

double HalconVisionWidget::getGain() const
{
    return ui->spinGain->value();
}

int HalconVisionWidget::getOffset() const
{
    return ui->spinOffset->value();
}

int HalconVisionWidget::getBinaryThreshold() const
{
    return ui->spinBinaryThreshold->value();
}

int HalconVisionWidget::getOpenX() const
{
    return ui->spinOpenX->value();
}

int HalconVisionWidget::getOpenY() const
{
    return ui->spinOpenY->value();
}

int HalconVisionWidget::getCloseX() const
{
    return ui->spinCloseX->value();
}

int HalconVisionWidget::getCloseY() const
{
    return ui->spinCloseY->value();
}

bool HalconVisionWidget::getUseTapeInspection() const
{
    return ui->checkUseTapeInspection->isChecked();
}

int HalconVisionWidget::getMaxIntersection() const
{
    return ui->spinMaxIntersection->value();
}

int HalconVisionWidget::getStartGap() const
{
    return ui->spinStartGap->value();
}

int HalconVisionWidget::getStartEndLimit() const
{
    return ui->spinStartEndLimit->value();
}

int HalconVisionWidget::getCropSize() const
{
    return ui->spinCropSize->value();
}

double HalconVisionWidget::getSigma() const
{
    return ui->spinSigma->value();
}

int HalconVisionWidget::getLow() const
{
    return ui->spinLow->value();
}

int HalconVisionWidget::getHigh() const
{
    return ui->spinHigh->value();
}

double HalconVisionWidget::getMoveStep() const
{
    return ui->spinMoveStep->value();
}

void HalconVisionWidget::setResult(double posX, double posY, double score)
{
    ui->labelResult->setText(QString("Position: X=%1, Y=%2, Score=%3%")
                             .arg(posX, 0, 'f', 2)
                             .arg(posY, 0, 'f', 2)
                             .arg(score, 0, 'f', 1));
}

void HalconVisionWidget::setExecutionTime(double ms)
{
    ui->labelExecutionTime->setText(QString("Execution Time: %1 ms").arg(ms, 0, 'f', 2));
}

void HalconVisionWidget::onAlgorithmSelected(int row, int column)
{
    Q_UNUSED(column);

    QTableWidgetItem *item = ui->tableAlgorithms->item(row, 0);
    if (item) {
        HalconAlgorithm algorithm = static_cast<HalconAlgorithm>(item->data(Qt::UserRole).toInt());
        m_currentAlgorithm = algorithm;
        emit algorithmChanged(algorithm);
    }
}

void HalconVisionWidget::onFindCornerClicked()
{
    emit findCornerRequested();
}

void HalconVisionWidget::onResetClicked()
{
    emit resetRequested();
}

void HalconVisionWidget::onParameterChanged()
{
    emit parametersChanged();
}

QString HalconVisionWidget::getConfigPath() const
{
    QString configDir = "D:/FITO_2026/Prevision/config";
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(configDir);
    }
    return configDir + "/" + CONFIG_FILENAME;
}

void HalconVisionWidget::onSaveParamsClicked()
{
    QString configPath = getConfigPath();
    QSettings settings(configPath, QSettings::IniFormat);

    settings.beginGroup("HalconVision");
    settings.setValue("algorithm", static_cast<int>(m_currentAlgorithm));
    settings.setValue("corner", ui->comboCorner->currentIndex());
    settings.endGroup();

    settings.beginGroup("ShapeModel");
    settings.setValue("matchingRate", ui->spinMatchingRate->value());
    settings.setValue("roiSize", ui->spinROISize->value());
    settings.endGroup();

    settings.beginGroup("ImageProcessing");
    settings.setValue("gain", ui->spinGain->value());
    settings.setValue("offset", ui->spinOffset->value());
    settings.setValue("binaryThreshold", ui->spinBinaryThreshold->value());
    settings.setValue("openX", ui->spinOpenX->value());
    settings.setValue("openY", ui->spinOpenY->value());
    settings.setValue("closeX", ui->spinCloseX->value());
    settings.setValue("closeY", ui->spinCloseY->value());
    settings.endGroup();

    settings.beginGroup("TapeInspection");
    settings.setValue("useTapeInspection", ui->checkUseTapeInspection->isChecked());
    settings.setValue("maxIntersection", ui->spinMaxIntersection->value());
    settings.setValue("startGap", ui->spinStartGap->value());
    settings.setValue("startEndLimit", ui->spinStartEndLimit->value());
    settings.setValue("cropSize", ui->spinCropSize->value());
    settings.setValue("sigma", ui->spinSigma->value());
    settings.setValue("low", ui->spinLow->value());
    settings.setValue("high", ui->spinHigh->value());
    settings.setValue("moveStep", ui->spinMoveStep->value());
    settings.endGroup();

    settings.sync();

    QMessageBox::information(this, "Save Parameters",
        QString("Parameters saved to:\n%1").arg(configPath));
}

void HalconVisionWidget::onLoadParamsClicked()
{
    QString configPath = getConfigPath();

    if (!QFile::exists(configPath)) {
        QMessageBox::warning(this, "Load Parameters",
            QString("Config file not found:\n%1").arg(configPath));
        return;
    }

    QSettings settings(configPath, QSettings::IniFormat);

    settings.beginGroup("HalconVision");
    int algo = settings.value("algorithm", 0).toInt();
    m_currentAlgorithm = static_cast<HalconAlgorithm>(algo);
    ui->tableAlgorithms->selectRow(algo);
    ui->comboCorner->setCurrentIndex(settings.value("corner", 0).toInt());
    settings.endGroup();

    settings.beginGroup("ShapeModel");
    ui->spinMatchingRate->setValue(settings.value("matchingRate", 50).toInt());
    ui->spinROISize->setValue(settings.value("roiSize", 300).toInt());
    settings.endGroup();

    settings.beginGroup("ImageProcessing");
    ui->spinGain->setValue(settings.value("gain", 1.0).toDouble());
    ui->spinOffset->setValue(settings.value("offset", 0).toInt());
    ui->spinBinaryThreshold->setValue(settings.value("binaryThreshold", 128).toInt());
    ui->spinOpenX->setValue(settings.value("openX", 1).toInt());
    ui->spinOpenY->setValue(settings.value("openY", 1).toInt());
    ui->spinCloseX->setValue(settings.value("closeX", 1).toInt());
    ui->spinCloseY->setValue(settings.value("closeY", 1).toInt());
    settings.endGroup();

    settings.beginGroup("TapeInspection");
    ui->checkUseTapeInspection->setChecked(settings.value("useTapeInspection", false).toBool());
    ui->spinMaxIntersection->setValue(settings.value("maxIntersection", 30).toInt());
    ui->spinStartGap->setValue(settings.value("startGap", 10).toInt());
    ui->spinStartEndLimit->setValue(settings.value("startEndLimit", 200).toInt());
    ui->spinCropSize->setValue(settings.value("cropSize", 200).toInt());
    ui->spinSigma->setValue(settings.value("sigma", 1.2).toDouble());
    ui->spinLow->setValue(settings.value("low", 1).toInt());
    ui->spinHigh->setValue(settings.value("high", 3).toInt());
    ui->spinMoveStep->setValue(settings.value("moveStep", 0.5).toDouble());
    settings.endGroup();

    QMessageBox::information(this, "Load Parameters",
        QString("Parameters loaded from:\n%1").arg(configPath));
}

// ========================================================================
// Halcon Vision 핵심 함수 구현 (Fnc_Vision_Pre.cpp와 100% 동일)
// ========================================================================

#ifdef HALCON_FOUND

bool HalconVisionWidget::checkHalconLicense()
{
    try {
        // Halcon 라이센스 체크 - 간단한 연산으로 확인
        HTuple hv_SystemInfo;
        GetSystem("halcon_info", &hv_SystemInfo);

        qDebug() << "[HalconVisionWidget] Halcon Info:" << hv_SystemInfo.S().Text();

        // 추가로 간단한 이미지 생성 테스트
        HObject ho_TestImage;
        GenImageConst(&ho_TestImage, "byte", 10, 10);
        ho_TestImage.Clear();

        qDebug() << "[HalconVisionWidget] Halcon License check passed";
        return true;
    }
    catch (HException& e) {
        qDebug() << "[HalconVisionWidget] Halcon License check failed:" << e.ErrorMessage().Text();
        QMessageBox::critical(nullptr, "Halcon License Error",
            QString("Halcon License check failed!\n\nError: %1\n\n"
                    "Please check your Halcon license.")
            .arg(e.ErrorMessage().Text()));
        return false;
    }
}

void HalconVisionWidget::HV_PreVisionCreateTemplate(int nCorner)
{
    // Fnc_Vision_Pre.cpp의 HV_PreVisionCreateTeamplate 함수와 동일
    HObject ho_RegionLinesH, ho_RegionLinesV, ho_RegionBase;
    HObject ho_BinImage, ho_Lines;
    HTuple hv_nSize = 300;

    GenImageConst(&ho_BinImage, "byte", 512, 512);

    if (nCorner == _RU)
    {
        GenRectangle1(&ho_RegionLinesH, 0, 0, 0, hv_nSize);
        GenRectangle1(&ho_RegionLinesV, 0, hv_nSize, hv_nSize, hv_nSize);
    }
    else if (nCorner == _RL)
    {
        GenRectangle1(&ho_RegionLinesH, hv_nSize, 0, hv_nSize, hv_nSize);
        GenRectangle1(&ho_RegionLinesV, 0, hv_nSize, hv_nSize, hv_nSize);
    }
    else if (nCorner == _LU)
    {
        GenRectangle1(&ho_RegionLinesH, 0, 0, 0, hv_nSize);
        GenRectangle1(&ho_RegionLinesV, 0, 0, hv_nSize, 0);
    }
    else if (nCorner == _LL)
    {
        GenRectangle1(&ho_RegionLinesH, hv_nSize, 0, hv_nSize, hv_nSize);
        GenRectangle1(&ho_RegionLinesV, 0, 0, hv_nSize, 0);
    }

    Union2(ho_RegionLinesH, ho_RegionLinesV, &ho_RegionBase);
    RegionToBin(ho_RegionBase, &ho_BinImage, 255, 0, 512, 512);
    LinesGauss(ho_BinImage, &ho_Lines, 1.5, 3, 8, "light", "true", "bar-shaped", "true");
    CreateShapeModelXld(ho_Lines, "auto", -0.39, 0.79, "auto", "auto", "ignore_local_polarity", 20, &m_hv_ModelID[nCorner]);

    ho_RegionLinesH.Clear();
    ho_RegionLinesV.Clear();
    ho_RegionBase.Clear();
    ho_BinImage.Clear();
    ho_Lines.Clear();

    qDebug() << "[HalconVisionWidget] Created Shape Model for corner:" << nCorner;
}

HalconVisionResult HalconVisionWidget::HV_PreVisionFindCornerTemplate(int nCorner, const QImage& inputImage)
{
    // Fnc_Vision_Pre.cpp의 HV_PreVisionFindCornerTeamplate 함수와 100% 동일
    HalconVisionResult stResult;
    stResult.Reset();

    if (inputImage.isNull()) {
        qDebug() << "[HalconVisionWidget] Input image is null";
        return stResult;
    }

    int m_nSizeX = inputImage.width();
    int m_nSizeY = inputImage.height();

    // QImage를 Halcon 이미지로 변환
    QImage grayImage = inputImage.convertToFormat(QImage::Format_Grayscale8);

    HObject Ho_OrgImage, Ho_Processimage;
    GenImage1Extern(&Ho_OrgImage, "byte", m_nSizeX, m_nSizeY,
                    (Hlong)grayImage.constBits(), (Hlong)NULL);

    // 파라미터 가져오기
    double dGain = getGain();
    int nOffset = getOffset();
    int nBinaryThreshold = getBinaryThreshold();
    int nopenx = getOpenX();
    int nopeny = getOpenY();
    int nclosex = getCloseX();
    int nclosey = getCloseY();
    double MatchingRate = getMatchingRate() / 100.0;
    int nROISize = getROISize();

    // Tape Inspection 파라미터
    bool nTapeInspection = getUseTapeInspection();
    int hv_maxIntersection = getMaxIntersection();
    int iStartGap = getStartGap();
    int iSrartEndLimite = getStartEndLimit();
    int iCropSize = getCropSize();
    double dSigma = getSigma();
    int iLow = getLow();
    int iHigh = getHigh();
    double dMoveStep = getMoveStep();

    HObject ho_RectangleRow, ho_RectangleCol, ho_RegionBase;
    HObject ho_ModelContours, ho_ContoursAffineTrans;
    HObject ho_Cross1, ho_RegionCross, ho_RegionIntersection1;
    HObject ho_RegionLinesRow, ho_RegionIntersection2, ho_RegionLinesCol;
    HObject ho_Lines1, ho_Region4, ho_RegionUnion2, ho_RegionMoved;
    HObject ho_RegionIntersection, ho_RegionMovedRow, ho_RegionMovedCol;
    HObject ho_Contours, ho_RegionLinesTapeRow, ho_RegionLinesTapeCol;
    HObject ho_Image, ho_InspectionROI, ho_DomainImage;

    HTuple hv_Width, hv_Height;
    HTuple hv_nSize = 300;
    HTuple hv_AX, hv_AY, hv_StartX, hv_EndX, hv_StepX;
    HTuple hv_StartY, hv_EndY, hv_StepY;
    HTuple hv_Row, hv_Column, hv_Angle, hv_Score;
    HTuple hv_HomMat2DIdentity, hv_HomMat2DTranslate, hv_HomMat2DRotate;
    HTuple hv_Result_Row, hv_Result_Col;
    HTuple hv_Area, hv_Row1, hv_Column1;
    HTuple hv_MaxVal, hv_MaxVal2, hv_maxIndex, hv_maxIndex2;
    HTuple hv_IndexX, hv_IndexY;
    HTuple hv_RowBegin, hv_ColBegin, hv_RowEnd, hv_ColEnd;
    HTuple hv_Nr, hv_Nc, hv_Dist;
    HTuple hv_dInter, hv_dSlope;
    HTuple hv_dRowStartR, hv_dRowEndR, hv_dColumnStartR, hv_dColumnEndR;
    HTuple hv_dRowStartC, hv_dRowEndC, hv_dColumnStartC, hv_dColumnEndC;
    HTuple hv_IsOverlapping, hv_error;
    HTuple hv_Area1, hv_Row_Row, hv_Column_Row;
    HTuple hv_Area2, hv_Row_Col, hv_Column_Col;

    CopyImage(Ho_OrgImage, &Ho_Processimage);

    // GrayOpening/Closing (전처리)
    GrayOpeningRect(Ho_Processimage, &Ho_Processimage, 11, 11);
    GrayClosingRect(Ho_Processimage, &Ho_Processimage, 11, 11);

    GetImageSize(Ho_Processimage, &hv_Width, &hv_Height);

    hv_AX = hv_nSize / 2;
    hv_AY = hv_nSize / 2;

    // Inspection ROI 생성
    GenRectangle1(&ho_InspectionROI, (m_nSizeY / 2) - nROISize, (m_nSizeX / 2) - nROISize,
                  (m_nSizeY / 2) + nROISize, (m_nSizeX / 2) + nROISize);
    ReduceDomain(Ho_Processimage, ho_InspectionROI, &ho_DomainImage);

    // 코너별 파라미터 설정
    if (nCorner == _RU)
    {
        GenRectangle1(&ho_RectangleRow, hv_Height - 1, 0, hv_Height - 1, hv_Width);
        GenRectangle1(&ho_RectangleCol, 0, 0, hv_Height, 0);

        hv_AX = hv_nSize / 2;
        hv_AY = (-hv_nSize) / 2;

        hv_StartX = iStartGap;
        hv_EndX = iSrartEndLimite;
        hv_StepX = dMoveStep;

        hv_StartY = -iStartGap;
        hv_EndY = -iSrartEndLimite;
        hv_StepY = -dMoveStep;
    }
    else if (nCorner == _RL)
    {
        GenRectangle1(&ho_RectangleRow, 0, 0, 0, hv_Width - 1);
        GenRectangle1(&ho_RectangleCol, 0, 0, hv_Height - 1, 0);

        hv_AX = hv_nSize / 2;
        hv_AY = hv_nSize / 2;

        hv_StartX = iStartGap;
        hv_EndX = iSrartEndLimite;
        hv_StepX = dMoveStep;

        hv_StartY = iStartGap;
        hv_EndY = iSrartEndLimite;
        hv_StepY = dMoveStep;
    }
    else if (nCorner == _LU)
    {
        GenRectangle1(&ho_RectangleRow, hv_Height - 1, 0, hv_Height - 1, hv_Width);
        GenRectangle1(&ho_RectangleCol, 0, hv_Width - 1, hv_Height, hv_Width - 1);

        hv_AX = (-hv_nSize) / 2;
        hv_AY = (-hv_nSize) / 2;

        hv_StartX = -iStartGap;
        hv_EndX = -iSrartEndLimite;
        hv_StepX = -dMoveStep;

        hv_StartY = -iStartGap;
        hv_EndY = -iSrartEndLimite;
        hv_StepY = -dMoveStep;
    }
    else if (nCorner == _LL)
    {
        GenRectangle1(&ho_RectangleRow, 0, 0, 0, hv_Width);
        GenRectangle1(&ho_RectangleCol, 0, hv_Width - 1, hv_Height, hv_Width - 1);

        hv_AX = (-hv_nSize) / 2;
        hv_AY = hv_nSize / 2;

        hv_StartX = -iStartGap;
        hv_EndX = -iSrartEndLimite;
        hv_StepX = -dMoveStep;

        hv_StartY = iStartGap;
        hv_EndY = iSrartEndLimite;
        hv_StepY = dMoveStep;
    }

    try
    {
        // Shape Model 매칭
        FindShapeModel(ho_DomainImage, m_hv_ModelID[nCorner], -0.39, 0.79, 0.1, 1, 0.5,
                       "least_squares", 0, 0.9, &hv_Row, &hv_Column, &hv_Angle, &hv_Score);

        if (hv_Score.Length() == 0 || hv_Score < MatchingRate)
        {
            qDebug() << "[HalconVisionWidget] Score too low or no match found:"
                     << (hv_Score.Length() > 0 ? hv_Score.D() * 100.0 : 0);
            Ho_OrgImage.Clear();
            Ho_Processimage.Clear();
            ho_InspectionROI.Clear();
            ho_DomainImage.Clear();
            return stResult;
        }

        qDebug() << "[HalconVisionWidget] Shape Model matched. Score:" << hv_Score.D() * 100.0;

        // 교점 계산
        GetShapeModelContours(&ho_ModelContours, m_hv_ModelID[nCorner], 1);

        HomMat2dIdentity(&hv_HomMat2DIdentity);
        HomMat2dTranslate(hv_HomMat2DIdentity, hv_AY, hv_AX, &hv_HomMat2DTranslate);
        HomMat2dRotate(hv_HomMat2DTranslate, hv_Angle, hv_Row, hv_Column, &hv_HomMat2DRotate);
        AffineTransPoint2d(hv_HomMat2DRotate, hv_Row, hv_Column, &hv_Result_Row, &hv_Result_Col);
        GenCrossContourXld(&ho_Cross1, hv_Result_Row, hv_Result_Col, 8000, hv_Angle);

        // ROI 내 체크
        HTuple hv_bInside;
        TestRegionPoint(ho_InspectionROI, hv_Result_Row, hv_Result_Col, &hv_bInside);
        if (hv_bInside.L() == 0)
        {
            qDebug() << "[HalconVisionWidget] Result point outside ROI";
            Ho_OrgImage.Clear();
            Ho_Processimage.Clear();
            ho_InspectionROI.Clear();
            ho_DomainImage.Clear();
            return stResult;
        }

        if (nTapeInspection)
        {
            // Tape Inspection 모드 (Fnc_Vision_Pre.cpp와 동일)
            GenRegionContourXld(ho_Cross1, &ho_RegionCross, "margin");

            Intersection(ho_RectangleRow, ho_RegionCross, &ho_RegionIntersection1);
            AreaCenter(ho_RegionIntersection1, &hv_Area1, &hv_Row_Row, &hv_Column_Row);
            GenRegionLine(&ho_RegionLinesRow, hv_Row_Row, hv_Column_Row, hv_Result_Row, hv_Result_Col);

            Intersection(ho_RectangleCol, ho_RegionCross, &ho_RegionIntersection2);
            AreaCenter(ho_RegionIntersection2, &hv_Area2, &hv_Row_Col, &hv_Column_Col);
            GenRegionLine(&ho_RegionLinesCol, hv_Row_Col, hv_Column_Col, hv_Result_Row, hv_Result_Col);

            HObject rrect;
            GenRectangle1(&rrect, hv_Result_Row - iCropSize, hv_Result_Col - iCropSize,
                          hv_Result_Row + iCropSize, hv_Result_Col + iCropSize);
            ReduceDomain(Ho_OrgImage, rrect, &ho_DomainImage);

            LinesGauss(ho_DomainImage, &ho_Lines1, dSigma, iLow, iHigh, "light", "false", "bar-shaped", "false");
            GenRegionContourXld(ho_Lines1, &ho_Region4, "margin");
            Union1(ho_Region4, &ho_RegionUnion2);

            hv_MaxVal = 0;
            hv_maxIndex = 0;

            HTuple LastIndex = -1;
            HTuple end_val147 = hv_EndX;
            HTuple step_val147 = hv_StepX;

            for (hv_IndexX = hv_StartX; hv_IndexX.Continue(end_val147, step_val147); hv_IndexX += step_val147)
            {
                MoveRegion(ho_RegionLinesRow, &ho_RegionMoved, 0, hv_IndexX);
                Intersection(ho_RegionMoved, ho_RegionUnion2, &ho_RegionIntersection);
                AreaCenter(ho_RegionIntersection, &hv_Area, &hv_Row1, &hv_Column1);

                if (0 != (hv_Area > hv_maxIntersection))
                {
                    LastIndex = hv_IndexX;
                }

                if (0 != (hv_Area > hv_MaxVal))
                {
                    hv_maxIndex = hv_IndexX;
                    hv_MaxVal = hv_Area;
                }
            }

            if (LastIndex < 0)
            {
                LastIndex = hv_maxIndex;
            }

            MoveRegion(ho_RegionLinesRow, &ho_RegionMovedRow, 0, LastIndex);

            hv_MaxVal2 = 0;
            hv_maxIndex2 = 0;
            LastIndex = -1;

            HTuple end_val162 = hv_EndY;
            HTuple step_val162 = hv_StepY;
            for (hv_IndexY = hv_StartY; hv_IndexY.Continue(end_val162, step_val162); hv_IndexY += step_val162)
            {
                MoveRegion(ho_RegionLinesCol, &ho_RegionMoved, hv_IndexY, 0);
                Intersection(ho_RegionMoved, ho_RegionUnion2, &ho_RegionIntersection);
                AreaCenter(ho_RegionIntersection, &hv_Area, &hv_Row1, &hv_Column1);

                if (0 != (hv_Area > hv_maxIntersection))
                {
                    LastIndex = hv_IndexY;
                }
                if (0 != (hv_Area > hv_MaxVal2))
                {
                    hv_maxIndex2 = hv_IndexY;
                    hv_MaxVal2 = hv_Area;
                }
            }
            if (LastIndex > 0)
            {
                LastIndex = hv_maxIndex2;
            }

            MoveRegion(ho_RegionLinesCol, &ho_RegionMovedCol, LastIndex, 0);

            GenContourRegionXld(ho_RegionMovedRow, &ho_Contours, "center");
            FitLineContourXld(ho_Contours, "tukey", -1, 0, 5, 2, &hv_RowBegin, &hv_ColBegin,
                              &hv_RowEnd, &hv_ColEnd, &hv_Nr, &hv_Nc, &hv_Dist);
            if (hv_Nr == 0.0)
            {
                hv_dInter = hv_Dist;
                hv_dSlope = 0;
                hv_dRowStartR = 0;
                hv_dRowEndR = hv_Height;
                hv_dColumnStartR = hv_ColBegin;
                hv_dColumnEndR = hv_ColBegin;
            }
            else
            {
                hv_dInter = hv_Dist / hv_Nr;
                hv_dSlope = (-hv_Nc) / hv_Nr;
                hv_dRowStartR = 0;
                hv_dRowEndR = hv_Height;
                hv_dColumnStartR = (-hv_dInter) / hv_dSlope;
                hv_dColumnEndR = (hv_dRowEndR - hv_dInter) / hv_dSlope;
            }
            GenRegionLine(&ho_RegionLinesTapeRow, hv_dRowStartR, hv_dColumnStartR, hv_dRowEndR, hv_dColumnEndR);

            GenContourRegionXld(ho_RegionMovedCol, &ho_Contours, "center");
            FitLineContourXld(ho_Contours, "tukey", -1, 0, 5, 2, &hv_RowBegin, &hv_ColBegin,
                              &hv_RowEnd, &hv_ColEnd, &hv_Nr, &hv_Nc, &hv_Dist);
            hv_dInter = hv_Dist / hv_Nr;
            hv_dSlope = (-hv_Nc) / hv_Nr;
            hv_dRowStartC = hv_dInter;
            hv_dRowEndC = (hv_dSlope * hv_Width) + hv_dInter;
            hv_dColumnStartC = 0;
            hv_dColumnEndC = hv_Width;
            GenRegionLine(&ho_RegionLinesTapeCol, hv_dRowStartC, hv_dColumnStartC, hv_dRowEndC, hv_dColumnEndC);

            IntersectionLines(hv_dRowStartR, hv_dColumnStartR, hv_dRowEndR, hv_dColumnEndR,
                              hv_dRowStartC, hv_dColumnStartC, hv_dRowEndC, hv_dColumnEndC,
                              &hv_Result_Row, &hv_Result_Col, &hv_IsOverlapping);

            qDebug() << "[HalconVisionWidget] Tape Inspection Result: Row=" << hv_Result_Row.D()
                     << "Col=" << hv_Result_Col.D();
        }

        // 결과 검증
        HTuple hv_len;
        TupleLength(hv_Result_Row, &hv_len);
        if (hv_len < 1)
        {
            qDebug() << "[HalconVisionWidget] No result row found";
            Ho_OrgImage.Clear();
            Ho_Processimage.Clear();
            return stResult;
        }

        // 결과 저장 (절대 좌표 - 화면 표시용)
        double dPointX = hv_Result_Col.D();
        double dPointY = hv_Result_Row.D();

        stResult.bExist = true;
        stResult.bResult = true;
        stResult.dPositionX = dPointX;  // 절대 좌표 X (Column)
        stResult.dPositionY = dPointY;  // 절대 좌표 Y (Row)
        stResult.dScore = hv_Score.D() * 100.0;

        qDebug() << "[HalconVisionWidget] Final Result: X=" << stResult.dPositionX
                 << "Y=" << stResult.dPositionY << "Score=" << stResult.dScore;

        m_lastResult = stResult;

        // 리소스 해제
        Ho_OrgImage.Clear();
        Ho_Processimage.Clear();
        ho_InspectionROI.Clear();
        ho_DomainImage.Clear();
        ho_RectangleRow.Clear();
        ho_RectangleCol.Clear();
        ho_ModelContours.Clear();
        ho_Cross1.Clear();
        ho_RegionCross.Clear();
        ho_RegionIntersection1.Clear();
        ho_RegionLinesRow.Clear();
        ho_RegionIntersection2.Clear();
        ho_RegionLinesCol.Clear();
        ho_Lines1.Clear();
        ho_Region4.Clear();
        ho_RegionUnion2.Clear();
        ho_RegionMoved.Clear();
        ho_RegionIntersection.Clear();
        ho_RegionMovedRow.Clear();
        ho_RegionMovedCol.Clear();
        ho_Contours.Clear();
        ho_RegionLinesTapeCol.Clear();
        ho_RegionLinesTapeRow.Clear();
    }
    catch (HException& e)
    {
        qDebug() << "[HalconVisionWidget] Halcon Exception:" << e.ErrorMessage().Text();
        Ho_OrgImage.Clear();
        Ho_Processimage.Clear();
        return stResult;
    }

    return stResult;
}

void HalconVisionWidget::HV_ImgProcess_Prepare2(HObject& Ho_Processimage, HObject& Ho_GainImage, HObject& Ho_BinImage)
{
    // Fnc_Vision_Pre.cpp의 HV_ImgProcess_Prepare2 함수와 동일
    HObject Ho_thregion;

    double dGain = getGain();
    int nOffset = getOffset();
    int nThreshold = getBinaryThreshold();
    int nOpenX = getOpenX();
    int nOpenY = getOpenY();
    int nCloseX = getCloseX();
    int nCloseY = getCloseY();

    HTuple hv_typ, hv_w, hv_h, hv_ptr;

    // Gain/Offset 적용
    ScaleImage(Ho_Processimage, &Ho_Processimage, dGain, nOffset);

    GetImagePointer1(Ho_Processimage, &hv_ptr, &hv_typ, &hv_w, &hv_h);
    CopyImage(Ho_Processimage, &Ho_GainImage);

    // Binary 처리
    Threshold(Ho_Processimage, &Ho_thregion, nThreshold, 256);

    // Opening
    if (nOpenX > 1 || nOpenY > 1)
    {
        OpeningRectangle1(Ho_thregion, &Ho_thregion, nOpenX, nOpenY);
    }

    // Closing
    if (nCloseX > 1 || nCloseY > 1)
    {
        ClosingRectangle1(Ho_thregion, &Ho_thregion, nCloseX, nCloseY);
    }

    HTuple hv_Width, hv_Height;
    GetImageSize(Ho_Processimage, &hv_Width, &hv_Height);
    RegionToBin(Ho_thregion, &Ho_BinImage, 255, 0, hv_Width, hv_Height);

    Ho_thregion.Clear();
}

#endif // HALCON_FOUND
