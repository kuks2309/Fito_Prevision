#include "EdgeProcessingWidget.h"
#include "ui_EdgeProcessingWidget.h"
#include <QHeaderView>
#include <QDir>
#include <QMessageBox>
#include <QCoreApplication>

const QString EdgeProcessingWidget::CONFIG_FILENAME = "edge_params.ini";

EdgeProcessingWidget::EdgeProcessingWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EdgeProcessingWidget)
    , m_currentAlgorithm(CANNY)
{
    ui->setupUi(this);

    setupTable();
    loadAlgorithms();
    updateParameterVisibility();

    // Connect signals
    connect(ui->tableAlgorithms, &QTableWidget::cellClicked,
            this, &EdgeProcessingWidget::onAlgorithmSelected);
    connect(ui->btnApply, &QPushButton::clicked,
            this, &EdgeProcessingWidget::onApplyClicked);
    connect(ui->btnReset, &QPushButton::clicked,
            this, &EdgeProcessingWidget::onResetClicked);

    // Parameter change signals
    connect(ui->sliderParam1, &QSlider::valueChanged,
            this, &EdgeProcessingWidget::onParameterChanged);
    connect(ui->sliderParam2, &QSlider::valueChanged,
            this, &EdgeProcessingWidget::onParameterChanged);
    connect(ui->comboParam3, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EdgeProcessingWidget::onParameterChanged);
    connect(ui->spinParam4, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &EdgeProcessingWidget::onParameterChanged);
    connect(ui->doubleSpinParam5, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EdgeProcessingWidget::onParameterChanged);
    connect(ui->doubleSpinParam6, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EdgeProcessingWidget::onParameterChanged);
    connect(ui->checkL2Gradient, &QCheckBox::stateChanged,
            this, &EdgeProcessingWidget::onParameterChanged);

    // Save/Load buttons
    connect(ui->btnSaveParams, &QPushButton::clicked,
            this, &EdgeProcessingWidget::onSaveParamsClicked);
    connect(ui->btnLoadParams, &QPushButton::clicked,
            this, &EdgeProcessingWidget::onLoadParamsClicked);
}

EdgeProcessingWidget::~EdgeProcessingWidget()
{
    delete ui;
}

void EdgeProcessingWidget::setupTable()
{
    ui->tableAlgorithms->setColumnCount(2);
    ui->tableAlgorithms->setHorizontalHeaderLabels({"Algorithm", "Description"});
    ui->tableAlgorithms->horizontalHeader()->setStretchLastSection(true);
    ui->tableAlgorithms->setColumnWidth(0, 150);
    ui->tableAlgorithms->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableAlgorithms->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableAlgorithms->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void EdgeProcessingWidget::loadAlgorithms()
{
    struct AlgorithmInfo {
        QString name;
        QString description;
        EdgeAlgorithm type;
    };

    QList<AlgorithmInfo> algorithms = {
        {"Canny", "Optimal edge detector with hysteresis", CANNY},
        {"Sobel", "First derivative gradient operator", SOBEL},
        {"Sobel Prevision", "Bilateral + Sobel XY + Threshold (MLCC)", SOBEL_PREVISION},
        {"Scharr", "More accurate than Sobel for small kernels", SCHARR},
        {"Laplacian", "Second derivative operator", LAPLACIAN},
        {"Prewitt", "Simple gradient operator", PREWITT},
        {"Roberts", "2x2 cross operator for diagonal edges", ROBERTS}
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

void EdgeProcessingWidget::updateParameterVisibility()
{
    // Hide all parameters first
    ui->labelParam1->setVisible(false);
    ui->sliderParam1->setVisible(false);
    ui->spinParam1->setVisible(false);
    ui->labelParam2->setVisible(false);
    ui->sliderParam2->setVisible(false);
    ui->spinParam2->setVisible(false);
    ui->labelParam3->setVisible(false);
    ui->comboParam3->setVisible(false);
    ui->labelParam4->setVisible(false);
    ui->spinParam4->setVisible(false);
    ui->labelParam5->setVisible(false);
    ui->doubleSpinParam5->setVisible(false);
    ui->labelParam6->setVisible(false);
    ui->doubleSpinParam6->setVisible(false);
    ui->checkL2Gradient->setVisible(false);

    switch (m_currentAlgorithm) {
    case CANNY:
        ui->labelParam1->setText("Low Threshold:");
        ui->labelParam1->setVisible(true);
        ui->sliderParam1->setVisible(true);
        ui->spinParam1->setVisible(true);
        ui->sliderParam1->setMaximum(255);
        ui->spinParam1->setMaximum(255);

        ui->labelParam2->setText("High Threshold:");
        ui->labelParam2->setVisible(true);
        ui->sliderParam2->setVisible(true);
        ui->spinParam2->setVisible(true);
        ui->sliderParam2->setMaximum(255);
        ui->spinParam2->setMaximum(255);

        ui->labelParam3->setText("Aperture Size:");
        ui->labelParam3->setVisible(true);
        ui->comboParam3->setVisible(true);

        ui->checkL2Gradient->setVisible(true);
        break;

    case SOBEL:
    case SCHARR:
        ui->labelParam4->setText("Kernel Size:");
        ui->labelParam4->setVisible(true);
        ui->spinParam4->setVisible(true);

        ui->labelParam5->setText("Scale:");
        ui->labelParam5->setVisible(true);
        ui->doubleSpinParam5->setVisible(true);

        ui->labelParam6->setText("Delta:");
        ui->labelParam6->setVisible(true);
        ui->doubleSpinParam6->setVisible(true);
        break;

    case SOBEL_PREVISION:
        // Bilateral Filter D
        ui->labelParam4->setText("Bilateral D:");
        ui->labelParam4->setVisible(true);
        ui->spinParam4->setVisible(true);
        ui->spinParam4->setMinimum(1);
        ui->spinParam4->setMaximum(15);
        ui->spinParam4->setValue(9);

        // Sigma Color
        ui->labelParam5->setText("Sigma Color:");
        ui->labelParam5->setVisible(true);
        ui->doubleSpinParam5->setVisible(true);
        ui->doubleSpinParam5->setMinimum(1.0);
        ui->doubleSpinParam5->setMaximum(200.0);
        ui->doubleSpinParam5->setValue(75.0);

        // Sigma Space
        ui->labelParam6->setText("Sigma Space:");
        ui->labelParam6->setVisible(true);
        ui->doubleSpinParam6->setVisible(true);
        ui->doubleSpinParam6->setMinimum(1.0);
        ui->doubleSpinParam6->setMaximum(200.0);
        ui->doubleSpinParam6->setValue(75.0);

        // Threshold (reuse sliderParam1)
        ui->labelParam1->setText("Threshold:");
        ui->labelParam1->setVisible(true);
        ui->sliderParam1->setVisible(true);
        ui->spinParam1->setVisible(true);
        ui->sliderParam1->setMinimum(0);
        ui->sliderParam1->setMaximum(255);
        ui->sliderParam1->setValue(30);
        ui->spinParam1->setMinimum(0);
        ui->spinParam1->setMaximum(255);
        ui->spinParam1->setValue(30);
        break;

    case LAPLACIAN:
        ui->labelParam4->setText("Kernel Size:");
        ui->labelParam4->setVisible(true);
        ui->spinParam4->setVisible(true);

        ui->labelParam5->setText("Scale:");
        ui->labelParam5->setVisible(true);
        ui->doubleSpinParam5->setVisible(true);

        ui->labelParam6->setText("Delta:");
        ui->labelParam6->setVisible(true);
        ui->doubleSpinParam6->setVisible(true);
        break;

    case PREWITT:
    case ROBERTS:
        // No adjustable parameters for these
        break;
    }
}

void EdgeProcessingWidget::onAlgorithmSelected(int row, int column)
{
    Q_UNUSED(column);

    QTableWidgetItem *item = ui->tableAlgorithms->item(row, 0);
    if (item) {
        EdgeAlgorithm algorithm = static_cast<EdgeAlgorithm>(item->data(Qt::UserRole).toInt());
        m_currentAlgorithm = algorithm;
        updateParameterVisibility();
        emit algorithmChanged(algorithm);
    }
}

void EdgeProcessingWidget::onApplyClicked()
{
    emit applyRequested();
}

void EdgeProcessingWidget::onResetClicked()
{
    // Reset parameters to default values
    ui->sliderParam1->setValue(50);
    ui->sliderParam2->setValue(150);
    ui->comboParam3->setCurrentIndex(0);
    ui->spinParam4->setValue(3);
    ui->doubleSpinParam5->setValue(1.0);
    ui->doubleSpinParam6->setValue(0.0);
    ui->checkL2Gradient->setChecked(false);

    emit resetRequested();
}

void EdgeProcessingWidget::onParameterChanged()
{
    emit parametersChanged();
}

// Getters
int EdgeProcessingWidget::getParam1() const
{
    return ui->spinParam1->value();
}

int EdgeProcessingWidget::getParam2() const
{
    return ui->spinParam2->value();
}

int EdgeProcessingWidget::getParam3() const
{
    return ui->comboParam3->currentText().toInt();
}

int EdgeProcessingWidget::getParam4() const
{
    return ui->spinParam4->value();
}

double EdgeProcessingWidget::getParam5() const
{
    return ui->doubleSpinParam5->value();
}

double EdgeProcessingWidget::getParam6() const
{
    return ui->doubleSpinParam6->value();
}

bool EdgeProcessingWidget::getL2Gradient() const
{
    return ui->checkL2Gradient->isChecked();
}

QString EdgeProcessingWidget::getConfigPath() const
{
    // D:\FITO_2026\Prevision\config 폴더에 저장
    QString configDir = "D:/FITO_2026/Prevision/config";
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(configDir);
    }
    return configDir + "/" + CONFIG_FILENAME;
}

void EdgeProcessingWidget::onSaveParamsClicked()
{
    QString configPath = getConfigPath();
    QSettings settings(configPath, QSettings::IniFormat);

    // 현재 알고리즘 저장
    settings.setValue("Edge/Algorithm", static_cast<int>(m_currentAlgorithm));

    // Canny 파라미터
    settings.beginGroup("Canny");
    settings.setValue("LowThreshold", ui->spinParam1->value());
    settings.setValue("HighThreshold", ui->spinParam2->value());
    settings.setValue("ApertureSize", ui->comboParam3->currentIndex());
    settings.setValue("L2Gradient", ui->checkL2Gradient->isChecked());
    settings.endGroup();

    // Sobel/Scharr/Laplacian 파라미터
    settings.beginGroup("Sobel");
    settings.setValue("KernelSize", ui->spinParam4->value());
    settings.setValue("Scale", ui->doubleSpinParam5->value());
    settings.setValue("Delta", ui->doubleSpinParam6->value());
    settings.endGroup();

    // Sobel Prevision 파라미터 (Sobel 그룹에 추가)
    settings.beginGroup("SobelPrevision");
    settings.setValue("BilateralD", ui->spinParam4->value());
    settings.setValue("SigmaColor", ui->doubleSpinParam5->value());
    settings.setValue("SigmaSpace", ui->doubleSpinParam6->value());
    settings.setValue("Threshold", ui->spinParam1->value());
    settings.endGroup();

    settings.sync();

    QMessageBox::information(this, "Save Parameters",
        QString("Parameters saved to:\n%1").arg(configPath));
}

void EdgeProcessingWidget::onLoadParamsClicked()
{
    QString configPath = getConfigPath();

    if (!QFile::exists(configPath)) {
        QMessageBox::warning(this, "Load Parameters",
            QString("Config file not found:\n%1").arg(configPath));
        return;
    }

    QSettings settings(configPath, QSettings::IniFormat);

    // 알고리즘 로드
    int algorithm = settings.value("Edge/Algorithm", 0).toInt();
    m_currentAlgorithm = static_cast<EdgeAlgorithm>(algorithm);
    ui->tableAlgorithms->selectRow(algorithm);

    // Canny 파라미터
    settings.beginGroup("Canny");
    ui->spinParam1->setValue(settings.value("LowThreshold", 50).toInt());
    ui->spinParam2->setValue(settings.value("HighThreshold", 150).toInt());
    ui->comboParam3->setCurrentIndex(settings.value("ApertureSize", 0).toInt());
    ui->checkL2Gradient->setChecked(settings.value("L2Gradient", false).toBool());
    settings.endGroup();

    // Sobel/Scharr/Laplacian 파라미터
    settings.beginGroup("Sobel");
    ui->spinParam4->setValue(settings.value("KernelSize", 3).toInt());
    ui->doubleSpinParam5->setValue(settings.value("Scale", 1.0).toDouble());
    ui->doubleSpinParam6->setValue(settings.value("Delta", 0.0).toDouble());
    settings.endGroup();

    // Sobel Prevision 파라미터
    if (m_currentAlgorithm == SOBEL_PREVISION) {
        settings.beginGroup("SobelPrevision");
        ui->spinParam4->setValue(settings.value("BilateralD", 9).toInt());
        ui->doubleSpinParam5->setValue(settings.value("SigmaColor", 75.0).toDouble());
        ui->doubleSpinParam6->setValue(settings.value("SigmaSpace", 75.0).toDouble());
        ui->spinParam1->setValue(settings.value("Threshold", 30).toInt());
        settings.endGroup();
    }

    updateParameterVisibility();

    QMessageBox::information(this, "Load Parameters",
        QString("Parameters loaded from:\n%1").arg(configPath));
}
