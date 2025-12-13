#ifndef TEEDCLIENT_H
#define TEEDCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QImage>
#include <vector>

/**
 * @brief TEED 서버와 통신하는 클라이언트
 * @details Python TEED 서버에 TCP로 연결하여 엣지 검출 수행
 */
class TEEDClient : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 검출된 직선 정보
     */
    struct LineInfo {
        int x1, y1, x2, y2;
        double length;
        double angle;
    };

    /**
     * @brief 추론 결과
     */
    struct InferenceResult {
        bool success;
        QString errorMessage;
        double inferenceTimeMs;
        QString outputPath;
        int width;
        int height;
        std::vector<LineInfo> lines;
    };

    explicit TEEDClient(QObject *parent = nullptr);
    ~TEEDClient();

    /**
     * @brief 서버 연결
     * @param host 호스트 주소 (기본: 127.0.0.1)
     * @param port 포트 번호 (기본: 9999)
     * @return 연결 성공 여부
     */
    bool connectToServer(const QString& host = "127.0.0.1", quint16 port = 9999);

    /**
     * @brief 서버 연결 해제
     */
    void disconnect();

    /**
     * @brief 연결 상태 확인
     */
    bool isConnected() const;

    /**
     * @brief 서버 상태 확인 (ping)
     */
    bool ping();

    /**
     * @brief 엣지맵 추론
     * @param imagePath 입력 이미지 경로
     * @param outputPath 출력 이미지 경로 (빈 문자열이면 저장 안함)
     * @return 추론 결과
     */
    InferenceResult inference(const QString& imagePath, const QString& outputPath = "");

    /**
     * @brief 직선 검출
     * @param imagePath 입력 이미지 경로
     * @param threshold 이진화 임계값 (기본: 50)
     * @param minLength 최소 직선 길이 (기본: 30)
     * @return 추론 결과 (lines 포함)
     */
    InferenceResult detectLines(const QString& imagePath, int threshold = 50, int minLength = 30);

    /**
     * @brief 서버 종료 요청
     */
    bool quitServer();

signals:
    /**
     * @brief 연결 상태 변경 시그널
     */
    void connectionStateChanged(bool connected);

    /**
     * @brief 에러 발생 시그널
     */
    void errorOccurred(const QString& error);

private:
    QTcpSocket* m_socket;
    QString m_host;
    quint16 m_port;

    /**
     * @brief 요청 전송 및 응답 수신
     */
    QJsonObject sendRequest(const QJsonObject& request);

    /**
     * @brief 응답에서 LineInfo 파싱
     */
    std::vector<LineInfo> parseLines(const QJsonArray& linesArray);
};

#endif // TEEDCLIENT_H
