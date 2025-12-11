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
};
