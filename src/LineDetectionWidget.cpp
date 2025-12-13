#include "LineDetectionWidget.h"
#include "ui_LineDetectionWidget.h"
#include <QHeaderView>
#include <QDir>
#include <QMessageBox>

const QString LineDetectionWidget::CONFIG_FILENAME = "line_params.ini";

LineDetectionWidget::LineDetectionWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LineDetectionWidget)
    , m_currentAlgorithm(HOUGH_LINES)
{
    ui->setupUi(this);

    setupTable();
    loadAlgorithms();
    updateParameterVisibility();

    // Connect signals
    connect(ui->tableAlgorithms, &QTableWidget::cellClicked,
            this, &LineDetectionWidget::onAlgorithmSelected);
    connect(ui->btnApply, &QPushButton::clicked,
            this, &LineDetectionWidget::onApplyClicked);
    connect(ui->btnReset, &QPushButton::clicked,
            this, &LineDetectionWidget::onResetClicked);

    // Parameter change signals
    connect(ui->doubleSpinRho, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &LineDetectionWidget::onParameterChanged);
    connect(ui->doubleSpinTheta, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &LineDetectionWidget::onParameterChanged);
    connect(ui->sliderThreshold, &QSlider::valueChanged,
            this, &LineDetectionWidget::onParameterChanged);
    connect(ui->doubleSpinMinLineLength, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &LineDetectionWidget::onParameterChanged);
    connect(ui->doubleSpinMaxLineGap, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &LineDetectionWidget::onParameterChanged);
    connect(ui->doubleSpinLsdScale, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &LineDetectionWidget::onParameterChanged);

    // Save/Load buttons
    connect(ui->btnSaveParams, &QPushButton::clicked,
            this, &LineDetectionWidget::onSaveParamsClicked);
    connect(ui->btnLoadParams, &QPushButton::clicked,
            this, &LineDetectionWidget::onLoadParamsClicked);
}

LineDetectionWidget::~LineDetectionWidget()
{
    delete ui;
}

void LineDetectionWidget::setupTable()
{
    ui->tableAlgorithms->setColumnCount(2);
    ui->tableAlgorithms->setHorizontalHeaderLabels({"Algorithm", "Description"});
    ui->tableAlgorithms->horizontalHeader()->setStretchLastSection(true);
    ui->tableAlgorithms->setColumnWidth(0, 150);
    ui->tableAlgorithms->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableAlgorithms->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableAlgorithms->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void LineDetectionWidget::loadAlgorithms()
{
    struct AlgorithmInfo {
        QString name;
        QString description;
        LineAlgorithm type;
    };

    QList<AlgorithmInfo> algorithms = {
        {"Hough Lines", "Standard Hough Line Transform", HOUGH_LINES},
        {"Hough Lines P", "Probabilistic Hough (faster)", HOUGH_LINES_P},
        {"LSD", "Line Segment Detector (fast)", LSD}
    };

    ui->tableAlgorithms->setRowCount(algorithms.size());

    for (int i = 0; i < algorithms.size(); ++i) {
        QTableWidgetItem *nameItem = new QTableWidgetItem(algorithms[i].name);
        QTableWidgetItem *descItem = new QTableWidgetItem(algorithms[i].description);

        nameItem->setData(Qt::UserRole, algorithms[i].type);

        ui->tableAlgorithms->setItem(i, 0, nameItem);
        ui->tableAlgorithms->setItem(i, 1, descItem);
    }

    // Select first row by default
    ui->tableAlgorithms->selectRow(0);
}

void LineDetectionWidget::updateParameterVisibility()
{
    // Hide all parameters first
    ui->labelRho->setVisible(false);
    ui->doubleSpinRho->setVisible(false);
    ui->labelTheta->setVisible(false);
    ui->doubleSpinTheta->setVisible(false);
    ui->labelThreshold->setVisible(false);
    ui->sliderThreshold->setVisible(false);
    ui->spinThreshold->setVisible(false);
    ui->labelMinLineLength->setVisible(false);
    ui->doubleSpinMinLineLength->setVisible(false);
    ui->labelMaxLineGap->setVisible(false);
    ui->doubleSpinMaxLineGap->setVisible(false);
    ui->labelLsdScale->setVisible(false);
    ui->doubleSpinLsdScale->setVisible(false);

    switch (m_currentAlgorithm) {
    case HOUGH_LINES:
        ui->labelRho->setVisible(true);
        ui->doubleSpinRho->setVisible(true);

        ui->labelTheta->setVisible(true);
        ui->doubleSpinTheta->setVisible(true);

        ui->labelThreshold->setVisible(true);
        ui->sliderThreshold->setVisible(true);
        ui->spinThreshold->setVisible(true);
        break;

    case HOUGH_LINES_P:
        ui->labelRho->setVisible(true);
        ui->doubleSpinRho->setVisible(true);

        ui->labelTheta->setVisible(true);
        ui->doubleSpinTheta->setVisible(true);

        ui->labelThreshold->setVisible(true);
        ui->sliderThreshold->setVisible(true);
        ui->spinThreshold->setVisible(true);

        ui->labelMinLineLength->setVisible(true);
        ui->doubleSpinMinLineLength->setVisible(true);

        ui->labelMaxLineGap->setVisible(true);
        ui->doubleSpinMaxLineGap->setVisible(true);
        break;

    case LSD:
        ui->labelLsdScale->setVisible(true);
        ui->doubleSpinLsdScale->setVisible(true);
        break;
    }
}

void LineDetectionWidget::onAlgorithmSelected(int row, int column)
{
    Q_UNUSED(column);

    QTableWidgetItem *item = ui->tableAlgorithms->item(row, 0);
    if (item) {
        LineAlgorithm algorithm = static_cast<LineAlgorithm>(item->data(Qt::UserRole).toInt());
        m_currentAlgorithm = algorithm;
        updateParameterVisibility();
        emit algorithmChanged(algorithm);
    }
}

void LineDetectionWidget::onApplyClicked()
{
    // LSD is disabled due to OpenCV compatibility issues - block before emitting signal
    if (m_currentAlgorithm == LSD) {
        QMessageBox::information(this, "LSD Not Available",
            "LSD (Line Segment Detector) is not available in this OpenCV build.\n\n"
            "Please use 'Hough Lines' or 'Hough Lines P' instead for line detection.");
        return;
    }
    emit applyRequested();
}

void LineDetectionWidget::onResetClicked()
{
    // Reset parameters to default values
    ui->doubleSpinRho->setValue(1.0);
    ui->doubleSpinTheta->setValue(0.017453292519943); // π/180
    ui->sliderThreshold->setValue(100);
    ui->doubleSpinMinLineLength->setValue(50.0);
    ui->doubleSpinMaxLineGap->setValue(10.0);
    ui->doubleSpinLsdScale->setValue(0.8);

    emit resetRequested();
}

void LineDetectionWidget::onParameterChanged()
{
    emit parametersChanged();
}

// Getters
double LineDetectionWidget::getRho() const
{
    return ui->doubleSpinRho->value();
}

double LineDetectionWidget::getTheta() const
{
    return ui->doubleSpinTheta->value();
}

int LineDetectionWidget::getThreshold() const
{
    return ui->spinThreshold->value();
}

double LineDetectionWidget::getMinLineLength() const
{
    return ui->doubleSpinMinLineLength->value();
}

double LineDetectionWidget::getMaxLineGap() const
{
    return ui->doubleSpinMaxLineGap->value();
}

double LineDetectionWidget::getLsdScale() const
{
    return ui->doubleSpinLsdScale->value();
}

QString LineDetectionWidget::getConfigPath() const
{
    // D:\FITO_2026\Prevision\config 폴더에 저장
    QString configDir = "D:/FITO_2026/Prevision/config";
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(configDir);
    }
    return configDir + "/" + CONFIG_FILENAME;
}

void LineDetectionWidget::onSaveParamsClicked()
{
    QString configPath = getConfigPath();
    QSettings settings(configPath, QSettings::IniFormat);

    // 현재 알고리즘 저장
    settings.setValue("Line/Algorithm", static_cast<int>(m_currentAlgorithm));

    // Hough Lines 공통 파라미터
    settings.beginGroup("HoughLines");
    settings.setValue("Rho", ui->doubleSpinRho->value());
    settings.setValue("Theta", ui->doubleSpinTheta->value());
    settings.setValue("Threshold", ui->spinThreshold->value());
    settings.endGroup();

    // Hough Lines P 추가 파라미터
    settings.beginGroup("HoughLinesP");
    settings.setValue("MinLineLength", ui->doubleSpinMinLineLength->value());
    settings.setValue("MaxLineGap", ui->doubleSpinMaxLineGap->value());
    settings.endGroup();

    // LSD 파라미터
    settings.beginGroup("LSD");
    settings.setValue("Scale", ui->doubleSpinLsdScale->value());
    settings.endGroup();

    settings.sync();

    QMessageBox::information(this, "Save Parameters",
        QString("Parameters saved to:\n%1").arg(configPath));
}

void LineDetectionWidget::onLoadParamsClicked()
{
    QString configPath = getConfigPath();

    if (!QFile::exists(configPath)) {
        QMessageBox::warning(this, "Load Parameters",
            QString("Config file not found:\n%1").arg(configPath));
        return;
    }

    QSettings settings(configPath, QSettings::IniFormat);

    // 알고리즘 로드
    int algorithm = settings.value("Line/Algorithm", 0).toInt();
    m_currentAlgorithm = static_cast<LineAlgorithm>(algorithm);
    ui->tableAlgorithms->selectRow(algorithm);

    // Hough Lines 공통 파라미터
    settings.beginGroup("HoughLines");
    ui->doubleSpinRho->setValue(settings.value("Rho", 1.0).toDouble());
    ui->doubleSpinTheta->setValue(settings.value("Theta", 0.017453292519943).toDouble());
    ui->spinThreshold->setValue(settings.value("Threshold", 100).toInt());
    settings.endGroup();

    // Hough Lines P 추가 파라미터
    settings.beginGroup("HoughLinesP");
    ui->doubleSpinMinLineLength->setValue(settings.value("MinLineLength", 50.0).toDouble());
    ui->doubleSpinMaxLineGap->setValue(settings.value("MaxLineGap", 10.0).toDouble());
    settings.endGroup();

    // LSD 파라미터
    settings.beginGroup("LSD");
    ui->doubleSpinLsdScale->setValue(settings.value("Scale", 0.8).toDouble());
    settings.endGroup();

    updateParameterVisibility();

    QMessageBox::information(this, "Load Parameters",
        QString("Parameters loaded from:\n%1").arg(configPath));
}
