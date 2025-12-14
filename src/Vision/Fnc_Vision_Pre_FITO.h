#pragma once
// 독립 프로젝트를 위해 원본 의존성 주석 처리
// #include "../FunctionBaseCustom.h"
// #include "multicam.h"
// #include "../Inc/SCD/Inc/tinyxml/tinyxml2.h"
// #include <memory>
// #include "internal/common/h/SharedMemory.h"
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <string>
#include <vector>

// 원본 프로젝트 타입 정의 (독립 실행을 위해 추가)
typedef unsigned char BYTE;
typedef void* PVOID;
typedef void* HANDLE;
typedef unsigned long DWORD;

// VisionResult 구조체 정의
struct VisionResult {
    bool bSuccess;
    double dProcessTime;
    std::string strErrorMsg;
    std::vector<cv::Point2f> corners;
    std::vector<cv::Vec4f> lines;
};

// TEED 직선 정보 구조체
struct TEEDLineInfo {
    float coef1, coef2;     // 방정식 계수 (수평: y=ax+b, 수직: x=cy+d)
    float length;           // 직선 길이
};

// TEED 직선 클러스터 구조체
struct TEEDLineCluster {
    float coef1, coef2;     // 평균 계수
    float totalLength;      // 총 길이
};

// TEED 교점 결과 구조체
struct TEEDIntersectionResult {
    bool bFound;            // 교점 발견 여부
    float cropX, cropY;     // 축소 이미지 좌표
    float origX, origY;     // 원본 이미지 좌표
};

// 원본 프로젝트 매크로 주석 처리
// #define DECLARE_DYNAMIC(class_name)
// #define IMPLEMENT_RUNTIMECLASS(class_name)

/**
 * @brief OpenCV 기반 카메라 캘리브레이션이 포함된 PreVision 클래스
 * @details Fnc_Vision_Pre를 대체하는 OpenCV 버전 (독립 프로젝트용)
 */
class Fnc_Vision_Pre_FITO /* : public FunctionBaseCustom */ {
    // DECLARE_DYNAMIC(Fnc_Vision_Pre_FITO)

public:
    Fnc_Vision_Pre_FITO();
    ~Fnc_Vision_Pre_FITO();

    // 원본 프로젝트 인터페이스 (주석 처리 - 독립 실행 시 불필요)
    // bool RegistCommand() override;
    // bool InitCode() override;
    // bool LoadIOConfig(std::string& strIOInfo) override;
    // bool PreCode(std::string& strCommandName, std::string& strArgs, bool bRetry, void* pLogPath) override;
    // void PostCode(std::string& strCommandName, std::string& strArgs, enumFunctionResult& eResult) override;
    // void FunctionAbort() override;

public:
    // 카메라 관련 (주석 처리 - 독립 실행 시 불필요)
    // int					InitialBoard();
    // void				InitialCrevisCam();
    // bool				SetCameraGain(int nGain = 1);
    // bool				SetPole(int nMode);
    // bool				GetPole();

public:
    // 파라미터 저장/로드 (주석 처리 - 독립 실행 시 불필요)
    // bool				Saveparameter();
    // bool				LoadParameter();

public:
    // 프레임 그래버 관련 (주석 처리 - 독립 실행 시 불필요)
    // void				Callback(PMCSIGNALINFO SigInfo);
    // bool				Grab();
    // void				Paint(bool GrabContinue = false);

public:
    // 이미지 모드 (주석 처리 - 독립 실행 시 불필요)
    // void				SetImageMode(bool bMode);
    // void				ImageOpen(std::string strFile);
    // void				SenGDIResult(int iStart, int iEnd);

public:
    // ✅ 핵심 Vision 처리 기능 (유지)
    VisionResult		CV_PreVisionFindCorner(int nCorner);
    void				CV_ImgProcess_Prepare(BYTE* img, BYTE* Gainimg, BYTE* Binimg, int nWidth, int nHeight);

    // 공유 메모리 관련 (주석 처리 - 독립 실행 시 불필요)
    // BYTE*				CreateShareMemory(HANDLE* Handle, int Width, int Height, string sFileName, BOOL* bDone);

    VisionResult		GetVisionResult();

    // 좌표 변환 (주석 처리 - 독립 실행 시 불필요)
    // void				SwapCoordinate(BYTE* ORG, BYTE* DEST, int Width, int Height, double dAngle);

    // 폴더/이미지 저장 (주석 처리 - 독립 실행 시 불필요)
    // void				CreateFolder(CString strPath);
    // void				SavePreImage(BYTE* Image, int Width, int Height, int nResult, int nSaveChannel);

    // ✅ OpenCV 캘리브레이션 기능 (유지)
    bool				LoadCalibration(const std::string& strFilePath);
    bool				SaveCalibration(const std::string& strFilePath);
    void				SetCalibrationParams(const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs);
    bool				UndistortImage(const unsigned char* pSrc, unsigned char* pDst, int nWidth, int nHeight);
    bool				UndistortPoint(double x, double y, double& outX, double& outY);
    bool				IsCalibrated() const { return m_bCalibrated; }

    // ✅ Edge 추출 기능 (유지)
    bool				CV_ExtractEdgesCanny(const unsigned char* pSrc, unsigned char* pDst,
                                        int nWidth, int nHeight,
                                        double dLowThreshold = 50.0, double dHighThreshold = 150.0,
                                        int nApertureSize = 3, bool bL2Gradient = false);

    bool				CV_ExtractLinesLSD(const unsigned char* pSrc, std::vector<cv::Vec4f>& lines,
                                       int nWidth, int nHeight,
                                       double dScale = 0.8, double dSigmaScale = 0.6,
                                       double dMinLength = 10.0);

    bool				CV_FitLineRobust(const std::vector<cv::Point2f>& points,
                                     cv::Vec4f& line,
                                     int nDistType = cv::DIST_HUBER,
                                     double dParam = 0.0, double dRadiusEps = 0.01, double dAngleEps = 0.01);

    // ✅ Hough 변환 직선 검출 (유지)
    bool				CV_HoughLinesStandard(const unsigned char* pSrc, std::vector<cv::Vec2f>& lines,
                                             int nWidth, int nHeight,
                                             double dRho = 1.0, double dTheta = CV_PI / 180.0,
                                             int nThreshold = 100, double dMinTheta = 0.0, double dMaxTheta = CV_PI);

    bool				CV_HoughLinesP(const unsigned char* pSrc, std::vector<cv::Vec4f>& lines,
                                   int nWidth, int nHeight,
                                   double dRho = 1.0, double dTheta = CV_PI / 180.0,
                                   int nThreshold = 50, double dMinLineLength = 50.0, double dMaxLineGap = 10.0);

    bool				CV_ConvertHoughLinesToCartesian(const std::vector<cv::Vec2f>& polarLines,
                                                    std::vector<cv::Vec4f>& cartesianLines,
                                                    int nWidth, int nHeight);

    // ✅ TEED 공유 메모리 인터페이스
    bool				TEED_Connect(int height = 512, int width = 608);
    void				TEED_Disconnect();
    bool				TEED_IsConnected() const { return m_bTeedConnected; }

    /**
     * @brief TEED 추론 실행 (BGR 이미지 입력)
     * @param inputBGR 입력 BGR 이미지 (원본 크기)
     * @param outputEdge 출력 Edge map (축소된 크기, CV_8UC1)
     * @param timeoutMs 타임아웃 (밀리초, 기본 5000ms)
     * @return 성공 여부
     */
    bool				TEED_Inference(const cv::Mat& inputBGR, cv::Mat& outputEdge, int timeoutMs = 5000);

    /**
     * @brief TEED 추론 실행 (BYTE 버퍼 입력, Grayscale)
     * @param pSrc 입력 Grayscale 버퍼
     * @param nWidth 입력 이미지 너비
     * @param nHeight 입력 이미지 높이
     * @param outputEdge 출력 Edge map
     * @param timeoutMs 타임아웃 (밀리초)
     * @return 성공 여부
     */
    bool				TEED_InferenceFromGray(const BYTE* pSrc, int nWidth, int nHeight,
                                               cv::Mat& outputEdge, int timeoutMs = 5000);

    /**
     * @brief TEED Edge map에서 수평/수직 직선 추출
     * @param edgeMap 입력 Edge map (CV_8UC1)
     * @param hLines 출력 수평선 리스트
     * @param vLines 출력 수직선 리스트
     * @param threshold 이진화 threshold (기본 50)
     * @param minLength 최소 선 길이 (기본 30)
     * @param maxGap 최대 gap (기본 10)
     * @param angleTolerance 수평/수직 허용 각도 (기본 30도)
     * @return 성공 여부
     */
    bool				TEED_ExtractLines(const cv::Mat& edgeMap,
                                          std::vector<TEEDLineInfo>& hLines,
                                          std::vector<TEEDLineInfo>& vLines,
                                          int threshold = 50, int minLength = 30,
                                          int maxGap = 10, float angleTolerance = 30.0f);

    /**
     * @brief 직선들을 절편 기준으로 클러스터링
     * @param lines 입력 직선 리스트
     * @param clusters 출력 클러스터 리스트
     * @param distThreshold 클러스터링 거리 threshold (기본 10픽셀)
     */
    void				TEED_ClusterLines(const std::vector<TEEDLineInfo>& lines,
                                          std::vector<TEEDLineCluster>& clusters,
                                          float distThreshold = 10.0f);

    /**
     * @brief 수평선과 수직선의 교점 계산 및 원본 좌표 변환
     * @param hLine 수평선 클러스터 (y = ax + b)
     * @param vLine 수직선 클러스터 (x = cy + d)
     * @param cropWidth 축소 이미지 너비
     * @param cropHeight 축소 이미지 높이
     * @param origWidth 원본 이미지 너비
     * @param origHeight 원본 이미지 높이
     * @return TEEDIntersectionResult 교점 좌표 (축소/원본)
     */
    TEEDIntersectionResult TEED_FindIntersection(const TEEDLineCluster& hLine,
                                                  const TEEDLineCluster& vLine,
                                                  int cropWidth, int cropHeight,
                                                  int origWidth, int origHeight);

private:
    // 원본 프로젝트 커맨드 함수들 (주석 처리 - 독립 실행 시 불필요)
    // enumFunctionResult Act_Initial(std::string strArg, structFunctionResult* pstFunctionResult);
    // enumFunctionResult Act_ModelChange(std::string strArg, structFunctionResult* pstFunctionResult);
    // enumFunctionResult Act_Grab(std::string strArg, structFunctionResult* pstFunctionResult);
    // enumFunctionResult Act_GrabContinous(std::string strArg, structFunctionResult* pstFunctionResult);
    // enumFunctionResult Act_StopGrabContinous(std::string strArg, structFunctionResult* pstFunctionResult);
    // enumFunctionResult Act_FindPreVision(std::string strArg, structFunctionResult* pstFunctionResult);
    // enumFunctionResult Act_TestResultClear(std::string strArg, structFunctionResult* pstFunctionResult);

    // ✅ OpenCV 캘리브레이션 내부 함수 (유지)
    void				InitUndistortMaps();

    // ✅ TEED 전처리 헬퍼
    void				TEED_Preprocess(const cv::Mat& inputBGR, float* outputTensor,
                                        int& outHeight, int& outWidth);

private:
    // 로깅 (주석 처리 - 독립 실행 시 불필요)
    // MARSLog m_MarsLog;

public:
    // 프레임 그래버 관련 (주석 처리 - 독립 실행 시 불필요)
    // MCHANDLE			m_Channel;
    // PVOID				m_pCurrent;
    // int					m_hDevice;
    // HANDLE				m_hProssed;

public:
    // ✅ 이미지 버퍼 (유지)
    BYTE* pOriginalImg;
    BYTE* pCamImg;
    BYTE* pGainImg;
    BYTE* pBinImg;
    BYTE* pViewImg;

public:
    // ✅ Vision 결과 (유지)
    VisionResult			stVisionResult;

public:
    // ✅ Vision 파라미터 (유지)
    int m_nVision;
    double dResol[2];

    // 카메라 설정 (주석 처리 - 독립 실행 시 불필요)
    // int nType;
    // int nBoardNo;
    // const char* chCamFile;
    // const char* chCannel;
    // const char* chCamID;
    // const char* chCommChannel;
    // const char* chViewShareMemFile;
    // int nCommPort;
    // bool m_bCommConnect;
    // bool m_bCameraOpen;

    // ✅ 이미지 크기 (유지)
    int	m_nRealSizeX;
    int	m_nRealSizeY;
    int m_nSizeX;
    int m_nSizeY;
    int m_nViewerSizeX;
    int m_nViewerSizeY;

    // 이미지 변환 (주석 처리 - 독립 실행 시 불필요)
    // int m_nFlipX;
    // int m_nFlipY;
    // int m_nRotate;
    // int m_nRotateDegree;

    // 모드 설정 (주석 처리 - 독립 실행 시 불필요)
    // bool m_bImageMode;
    // bool m_bGrabContinous;
    // int m_nUseCameraInitial;

    int SizeX;
    int SizeY;
    int BufferPitch;

    // Grab 위치/오프셋 (주석 처리 - 독립 실행 시 불필요)
    // int	m_nGrabPosition;
    // int	m_nOffsetMode;

private:
    // Critical Section (주석 처리 - 독립 실행 시 불필요)
    // CRITICAL_SECTION cs_Grab;

    // 공유 메모리 (주석 처리 - 독립 실행 시 불필요)
    // SharedMemory smViewImage;
    // SharedMemory smImageStructure;
    // stVisionData* stImage;

    // 프로세스 관련 (주석 처리 - 독립 실행 시 불필요)
    // DWORD processIdOld;
    // HANDLE m_hGuiProcess;
    // int m_nIsInitialize;

    // ✅ OpenCV 캘리브레이션 변수 (유지)
    cv::Mat m_cvCameraMatrix;        // 카메라 내부 파라미터 (3x3)
    cv::Mat m_cvDistCoeffs;          // 왜곡 계수 [k1, k2, p1, p2, k3]
    cv::Mat m_cvNewCameraMatrix;     // 최적화된 카메라 매트릭스
    cv::Mat m_cvMapX;                // X 좌표 매핑
    cv::Mat m_cvMapY;                // Y 좌표 매핑
    bool m_bCalibrated;              // 캘리브레이션 완료 여부
    bool m_bMapsInitialized;         // 맵 초기화 완료 여부
    double m_dReprojectionError;     // RMS 재투영 오차
    bool m_bUseCalibration;          // 캘리브레이션 사용 여부 플래그

    // ✅ TEED 공유 메모리 변수
    HANDLE m_hTeedInputMapping;      // 입력 공유 메모리 핸들
    HANDLE m_hTeedOutputMapping;     // 출력 공유 메모리 핸들
    BYTE* m_pTeedInputBuffer;        // 입력 버퍼 포인터
    BYTE* m_pTeedOutputBuffer;       // 출력 버퍼 포인터
    bool m_bTeedConnected;           // TEED 연결 상태
    int m_nTeedHeight;               // TEED 입력 높이
    int m_nTeedWidth;                // TEED 입력 너비
    HANDLE m_hTeedProcess;           // Python 서버 프로세스 핸들

    // TEED 공유 메모리 상수
    static constexpr int TEED_HEADER_SIZE = 8;  // height(4) + width(4)
    static constexpr const char* TEED_INPUT_NAME = "teed_input";
    static constexpr const char* TEED_OUTPUT_NAME = "teed_output";

    // TEED Python 서버 경로 (필요시 수정)
    std::string m_strTeedServerPath;

    // Python 서버 실행 헬퍼
    bool TEED_LaunchServer(int height, int width);
};
