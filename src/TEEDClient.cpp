#include "TEEDClient.h"
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

TEEDClient::TEEDClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_host("127.0.0.1")
    , m_port(9999)
{
    connect(m_socket, &QTcpSocket::connected, this, [this]() {
        emit connectionStateChanged(true);
    });

    connect(m_socket, &QTcpSocket::disconnected, this, [this]() {
        emit connectionStateChanged(false);
    });

    connect(m_socket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error) {
        Q_UNUSED(error);
        emit errorOccurred(m_socket->errorString());
    });
}

TEEDClient::~TEEDClient()
{
    disconnect();
}

bool TEEDClient::connectToServer(const QString& host, quint16 port)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }

    m_host = host;
    m_port = port;

    m_socket->connectToHost(host, port);

    if (!m_socket->waitForConnected(5000)) {
        qWarning() << "[TEEDClient] Connection failed:" << m_socket->errorString();
        return false;
    }

    qDebug() << "[TEEDClient] Connected to" << host << ":" << port;
    return true;
}

void TEEDClient::disconnect()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->waitForDisconnected(1000);
        }
    }
}

bool TEEDClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

bool TEEDClient::ping()
{
    QJsonObject request;
    request["command"] = "ping";

    QJsonObject response = sendRequest(request);
    return response["status"].toString() == "ok";
}

TEEDClient::InferenceResult TEEDClient::inference(const QString& imagePath, const QString& outputPath)
{
    InferenceResult result;
    result.success = false;

    QJsonObject request;
    request["command"] = "inference";
    request["path"] = imagePath;
    request["output"] = outputPath;

    QJsonObject response = sendRequest(request);

    if (response.isEmpty()) {
        result.errorMessage = "No response from server";
        return result;
    }

    if (response["status"].toString() == "ok") {
        result.success = true;
        result.inferenceTimeMs = response["inference_time_ms"].toDouble();
        result.outputPath = response["output_path"].toString();

        QJsonArray sizeArray = response["size"].toArray();
        if (sizeArray.size() >= 2) {
            result.width = sizeArray[0].toInt();
            result.height = sizeArray[1].toInt();
        }
    } else {
        result.errorMessage = response["message"].toString();
    }

    return result;
}

TEEDClient::InferenceResult TEEDClient::detectLines(const QString& imagePath, int threshold, int minLength)
{
    InferenceResult result;
    result.success = false;

    QJsonObject request;
    request["command"] = "lines";
    request["path"] = imagePath;
    request["threshold"] = threshold;
    request["min_length"] = minLength;

    QJsonObject response = sendRequest(request);

    if (response.isEmpty()) {
        result.errorMessage = "No response from server";
        return result;
    }

    if (response["status"].toString() == "ok") {
        result.success = true;
        result.inferenceTimeMs = response["inference_time_ms"].toDouble();

        QJsonArray linesArray = response["lines"].toArray();
        result.lines = parseLines(linesArray);
    } else {
        result.errorMessage = response["message"].toString();
    }

    return result;
}

bool TEEDClient::quitServer()
{
    QJsonObject request;
    request["command"] = "quit";

    QJsonObject response = sendRequest(request);
    return response["status"].toString() == "ok";
}

QJsonObject TEEDClient::sendRequest(const QJsonObject& request)
{
    if (!isConnected()) {
        if (!connectToServer(m_host, m_port)) {
            qWarning() << "[TEEDClient] Not connected";
            return QJsonObject();
        }
    }

    // JSON 직렬화
    QJsonDocument doc(request);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    // 길이 (4바이트, big-endian) + 데이터 전송
    quint32 length = static_cast<quint32>(data.size());
    QByteArray lengthBytes;
    QDataStream lengthStream(&lengthBytes, QIODevice::WriteOnly);
    lengthStream.setByteOrder(QDataStream::BigEndian);
    lengthStream << length;

    m_socket->write(lengthBytes);
    m_socket->write(data);
    m_socket->flush();

    // 응답 수신 (타임아웃 30초)
    if (!m_socket->waitForReadyRead(30000)) {
        qWarning() << "[TEEDClient] Timeout waiting for response";
        return QJsonObject();
    }

    // 길이 수신
    while (m_socket->bytesAvailable() < 4) {
        if (!m_socket->waitForReadyRead(5000)) {
            qWarning() << "[TEEDClient] Timeout reading length";
            return QJsonObject();
        }
    }

    QByteArray respLengthBytes = m_socket->read(4);
    QDataStream respLengthStream(respLengthBytes);
    respLengthStream.setByteOrder(QDataStream::BigEndian);
    quint32 respLength;
    respLengthStream >> respLength;

    // 데이터 수신
    QByteArray respData;
    while (static_cast<quint32>(respData.size()) < respLength) {
        if (!m_socket->waitForReadyRead(5000)) {
            qWarning() << "[TEEDClient] Timeout reading data";
            return QJsonObject();
        }
        respData.append(m_socket->read(respLength - respData.size()));
    }

    // JSON 파싱
    QJsonDocument respDoc = QJsonDocument::fromJson(respData);
    if (respDoc.isNull()) {
        qWarning() << "[TEEDClient] Invalid JSON response";
        return QJsonObject();
    }

    return respDoc.object();
}

std::vector<TEEDClient::LineInfo> TEEDClient::parseLines(const QJsonArray& linesArray)
{
    std::vector<LineInfo> lines;
    lines.reserve(linesArray.size());

    for (const QJsonValue& value : linesArray) {
        QJsonObject obj = value.toObject();
        LineInfo line;
        line.x1 = obj["x1"].toInt();
        line.y1 = obj["y1"].toInt();
        line.x2 = obj["x2"].toInt();
        line.y2 = obj["y2"].toInt();
        line.length = obj["length"].toDouble();
        line.angle = obj["angle"].toDouble();
        lines.push_back(line);
    }

    return lines;
}
