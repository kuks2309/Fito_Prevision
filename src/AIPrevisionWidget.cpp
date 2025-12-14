#include "AIPrevisionWidget.h"
#include "ui_AIPrevisionWidget.h"
#include <QDir>
#include <QMessageBox>

const QString AIPrevisionWidget::CONFIG_FILENAME = "ai_prevision_params.ini";

AIPrevisionWidget::AIPrevisionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AIPrevisionWidget),
    m_currentAlgorithm(TEED_EDGE_DETECTION)
{
    ui->setupUi(this);

    setupTable();
    loadAlgorithms();
    updateParameterVisibility();

    // 시그널 연결
    connect(ui->tableAlgorithms, &QTableWidget::cellClicked,
            this, &AIPrevisionWidget::onAlgorithmSelected);
    connect(ui->btnApply, &QPushButton::clicked,
            this, &AIPrevisionWidget::onApplyClicked);
    connect(ui->btnHoughLines, &QPushButton::clicked,
            this, &AIPrevisionWidget::onHoughLinesClicked);
    connect(ui->btnClustering, &QPushButton::clicked,
            this, &AIPrevisionWidget::onClusteringClicked);
    connect(ui->btnFindIntersection, &QPushButton::clicked,
            this, &AIPrevisionWidget::onFindIntersectionClicked);
    connect(ui->btnRunAll, &QPushButton::clicked,
            this, &AIPrevisionWidget::onRunAllClicked);
    connect(ui->btnShowOriginalSize, &QPushButton::clicked,
            this, &AIPrevisionWidget::onShowOriginalSizeClicked);
    connect(ui->btnReset, &QPushButton::clicked,
            this, &AIPrevisionWidget::onResetClicked);
    connect(ui->btnSaveParams, &QPushButton::clicked,
            this, &AIPrevisionWidget::onSaveParamsClicked);
    connect(ui->btnLoadParams, &QPushButton::clicked,
            this, &AIPrevisionWidget::onLoadParamsClicked);
}

AIPrevisionWidget::~AIPrevisionWidget()
{
    delete ui;
}

void AIPrevisionWidget::setupTable()
{
    ui->tableAlgorithms->setColumnCount(2);
    ui->tableAlgorithms->setHorizontalHeaderLabels({"Algorithm", "Description"});
    ui->tableAlgorithms->horizontalHeader()->setStretchLastSection(true);
    ui->tableAlgorithms->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableAlgorithms->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void AIPrevisionWidget::loadAlgorithms()
{
    struct AlgorithmInfo {
        AIAlgorithm algo;
        QString name;
        QString description;
    };

    QVector<AlgorithmInfo> algorithms = {
        {TEED_EDGE_DETECTION, "TEED Edge Detection", "Tiny and Efficient Edge Detector (58K params)"}
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

void AIPrevisionWidget::updateParameterVisibility()
{
    // Hough 파라미터는 항상 표시
}

int AIPrevisionWidget::getThreshold() const
{
    return ui->spinThreshold->value();
}

int AIPrevisionWidget::getMinLength() const
{
    return ui->spinMinLength->value();
}

int AIPrevisionWidget::getMaxGap() const
{
    return ui->spinMaxGap->value();
}

float AIPrevisionWidget::getAngleTolerance() const
{
    return static_cast<float>(ui->spinAngleTolerance->value());
}

float AIPrevisionWidget::getClusterDistance() const
{
    return static_cast<float>(ui->spinClusterDist->value());
}

void AIPrevisionWidget::setResult(double posX, double posY)
{
    ui->labelResult->setText(QString("Intersection: X=%1, Y=%2")
                             .arg(posX, 0, 'f', 2)
                             .arg(posY, 0, 'f', 2));
}

void AIPrevisionWidget::setOriginalCoord(double posX, double posY)
{
    ui->labelOriginalCoord->setText(QString("Original: X=%1, Y=%2")
                                    .arg(posX, 0, 'f', 2)
                                    .arg(posY, 0, 'f', 2));
}

void AIPrevisionWidget::setExecutionTime(double ms)
{
    ui->labelExecutionTime->setText(QString("Execution Time: %1 ms").arg(ms, 0, 'f', 2));
}

void AIPrevisionWidget::onAlgorithmSelected(int row, int column)
{
    Q_UNUSED(column);

    QTableWidgetItem *item = ui->tableAlgorithms->item(row, 0);
    if (item) {
        AIAlgorithm algorithm = static_cast<AIAlgorithm>(item->data(Qt::UserRole).toInt());
        m_currentAlgorithm = algorithm;
        updateParameterVisibility();
        emit algorithmChanged(algorithm);
    }
}

void AIPrevisionWidget::onApplyClicked()
{
    emit applyRequested();
}

void AIPrevisionWidget::onHoughLinesClicked()
{
    emit houghLinesRequested();
}

void AIPrevisionWidget::onClusteringClicked()
{
    emit clusteringRequested();
}

void AIPrevisionWidget::onFindIntersectionClicked()
{
    emit findIntersectionRequested();
}

void AIPrevisionWidget::onRunAllClicked()
{
    emit runAllRequested();
}

void AIPrevisionWidget::onShowOriginalSizeClicked()
{
    emit showOriginalSizeRequested();
}

void AIPrevisionWidget::onResetClicked()
{
    emit resetRequested();
}

void AIPrevisionWidget::onParameterChanged()
{
    emit parametersChanged();
}

QString AIPrevisionWidget::getConfigPath() const
{
    QString configDir = "D:/FITO_2026/Prevision/config";
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(configDir);
    }
    return configDir + "/" + CONFIG_FILENAME;
}

void AIPrevisionWidget::onSaveParamsClicked()
{
    QString configPath = getConfigPath();
    QSettings settings(configPath, QSettings::IniFormat);

    settings.beginGroup("AIPrevision");
    settings.setValue("algorithm", static_cast<int>(m_currentAlgorithm));
    settings.endGroup();

    settings.beginGroup("HoughParameters");
    settings.setValue("threshold", ui->spinThreshold->value());
    settings.setValue("minLength", ui->spinMinLength->value());
    settings.setValue("maxGap", ui->spinMaxGap->value());
    settings.setValue("angleTolerance", ui->spinAngleTolerance->value());
    settings.setValue("clusterDistance", ui->spinClusterDist->value());
    settings.endGroup();

    settings.sync();

    QMessageBox::information(this, "Save Parameters",
        QString("Parameters saved to:\n%1").arg(configPath));
}

void AIPrevisionWidget::onLoadParamsClicked()
{
    QString configPath = getConfigPath();

    if (!QFile::exists(configPath)) {
        QMessageBox::warning(this, "Load Parameters",
            QString("Config file not found:\n%1").arg(configPath));
        return;
    }

    QSettings settings(configPath, QSettings::IniFormat);

    settings.beginGroup("AIPrevision");
    int algo = settings.value("algorithm", 0).toInt();
    m_currentAlgorithm = static_cast<AIAlgorithm>(algo);
    ui->tableAlgorithms->selectRow(algo);
    settings.endGroup();

    settings.beginGroup("HoughParameters");
    ui->spinThreshold->setValue(settings.value("threshold", 50).toInt());
    ui->spinMinLength->setValue(settings.value("minLength", 30).toInt());
    ui->spinMaxGap->setValue(settings.value("maxGap", 10).toInt());
    ui->spinAngleTolerance->setValue(settings.value("angleTolerance", 30.0).toDouble());
    ui->spinClusterDist->setValue(settings.value("clusterDistance", 10.0).toDouble());
    settings.endGroup();

    updateParameterVisibility();

    QMessageBox::information(this, "Load Parameters",
        QString("Parameters loaded from:\n%1").arg(configPath));
}
