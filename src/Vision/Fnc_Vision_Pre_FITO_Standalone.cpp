// Fnc_Vision_Pre_FITO - 독립 프로젝트용 간소화 버전
// 원본에서 핵심 OpenCV 기능만 추출

#include "Fnc_Vision_Pre_FITO.h"
// 원본 의존성 주석 처리
// #include "../AllInclude.h"
// #include <atlimage.h>
#include <windows.h>
// #include <tlhelp32.h>
// #include "../Crevis/VirtualFG40CL.h"
// #include "../DataDefine/GlobalVariable.h"

// #pragma comment(lib, "VirtualFG40CL.lib")
// IMPLEMENT_RUNTIMECLASS(Fnc_Vision_Pre_FITO)

// using namespace tinyxml2;

// ========================================================================
// GlobalCallback - Frame Grabber 콜백 함수 (주석 처리)
// ========================================================================
/*
void WINAPI GlobalCallbackPre_FITO(PMCSIGNALINFO SigInfo)
{
	if (SigInfo && SigInfo->Context)
	{
		Fnc_Vision_Pre_FITO* pDoc = (Fnc_Vision_Pre_FITO*)SigInfo->Context;
		pDoc->Callback(SigInfo);
	}
}
*/

// ========================================================================
// 생성자
// ========================================================================
Fnc_Vision_Pre_FITO::Fnc_Vision_Pre_FITO()
{
	// m_hDevice = 0;
	// m_Channel = 0;

	pCamImg = nullptr;
	pOriginalImg = nullptr;
	pViewImg = nullptr;
	pGainImg = nullptr;
	pBinImg = nullptr;

	// m_hProssed = nullptr;
	// m_pCurrent = nullptr;

	dResol[0] = 0;
	dResol[1] = 0;

	// 카메라 관련 변수 주석 처리
	// nType = 0;
	// nBoardNo = 0;
	// chCamFile = "";
	// chCannel = "";
	// chCamID = "";
	// chCommChannel = "";
	// chViewShareMemFile = "";
	// nCommPort = 0;
	// m_bCommConnect = false;
	// m_bCameraOpen = false;

	m_nRealSizeX = 0;
	m_nRealSizeY = 0;
	m_nSizeX = 0;
	m_nSizeY = 0;
	m_nViewerSizeX = 0;
	m_nViewerSizeY = 0;

	// m_nFlipX = 0;
	// m_nFlipY = 0;
	// m_nRotate = 0;
	// m_nRotateDegree = 0;

	SizeX = 0;
	SizeY = 0;
	BufferPitch = 0;

	m_nVision = 2;
	// m_bImageMode = false;
	// m_bGrabContinous = false;

	// m_hGuiProcess = NULL;
	// processIdOld = 0;
	// m_nIsInitialize = 0;

	// ✅ OpenCV 캘리브레이션 초기화
	m_bCalibrated = false;
	m_bMapsInitialized = false;
	m_dReprojectionError = 0.0;
	m_bUseCalibration = false;

	// CreateCS();
	// ::InitializeCriticalSection(&cs_Grab);
}

// ========================================================================
// 소멸자
// ========================================================================
Fnc_Vision_Pre_FITO::~Fnc_Vision_Pre_FITO()
{
	// DeleteCS();
	// ::DeleteCriticalSection(&cs_Grab);
	// ::CloseHandle(m_hProssed);
	// ::CloseHandle(m_hGuiProcess);

	// 이미지 버퍼 해제
	if (pCamImg) delete[] pCamImg;
	if (pOriginalImg) delete[] pOriginalImg;
	if (pViewImg) delete[] pViewImg;
	if (pGainImg) delete[] pGainImg;
	if (pBinImg) delete[] pBinImg;
}

// ========================================================================
// ✅ LoadCalibration - XML 파일에서 캘리브레이션 파라미터 로드
// ========================================================================
bool Fnc_Vision_Pre_FITO::LoadCalibration(const std::string& strFilePath)
{
	try
	{
		cv::FileStorage fs(strFilePath, cv::FileStorage::READ);
		if (!fs.isOpened())
		{
			return false;
		}

		fs["camera_matrix"] >> m_cvCameraMatrix;
		fs["distortion_coefficients"] >> m_cvDistCoeffs;

		int width = 0, height = 0;
		fs["image_width"] >> width;
		fs["image_height"] >> height;

		if (fs["reprojection_error"].isReal())
		{
			fs["reprojection_error"] >> m_dReprojectionError;
		}

		fs.release();

		// 유효성 검사
		if (m_cvCameraMatrix.empty() || m_cvDistCoeffs.empty())
		{
			return false;
		}

		if (m_cvCameraMatrix.rows != 3 || m_cvCameraMatrix.cols != 3)
		{
			return false;
		}

		if (m_cvDistCoeffs.total() < 4)
		{
			return false;
		}

		// 이미지 크기 설정 및 맵 생성
		if (width > 0 && height > 0)
		{
			if (width == m_nSizeX && height == m_nSizeY)
			{
				InitUndistortMaps();
			}
		}

		m_bCalibrated = true;
		return true;
	}
	catch (const cv::Exception& e)
	{
		m_bCalibrated = false;
		return false;
	}
}

// ========================================================================
// ✅ SaveCalibration - XML 파일로 캘리브레이션 파라미터 저장
// ========================================================================
bool Fnc_Vision_Pre_FITO::SaveCalibration(const std::string& strFilePath)
{
	if (!m_bCalibrated)
	{
		return false;
	}

	try
	{
		cv::FileStorage fs(strFilePath, cv::FileStorage::WRITE);
		if (!fs.isOpened())
		{
			return false;
		}

		fs << "camera_matrix" << m_cvCameraMatrix;
		fs << "distortion_coefficients" << m_cvDistCoeffs;
		fs << "image_width" << m_nSizeX;
		fs << "image_height" << m_nSizeY;
		fs << "reprojection_error" << m_dReprojectionError;

		fs.release();
		return true;
	}
	catch (const cv::Exception& e)
	{
		return false;
	}
}

// ========================================================================
// ✅ InitUndistortMaps - 왜곡 보정 맵 생성 (초기화 시 한 번만)
// ========================================================================
void Fnc_Vision_Pre_FITO::InitUndistortMaps()
{
	if (!m_bCalibrated || m_nSizeX <= 0 || m_nSizeY <= 0)
	{
		return;
	}

	try
	{
		cv::Size imageSize(m_nSizeX, m_nSizeY);

		// 최적화된 카메라 매트릭스 계산
		// alpha = 1.0: 왜곡 보정 후 모든 픽셀 유지 (검은 영역 최소화)
		m_cvNewCameraMatrix = cv::getOptimalNewCameraMatrix(
			m_cvCameraMatrix,
			m_cvDistCoeffs,
			imageSize,
			1.0,  // alpha
			imageSize
		);

		// 왜곡 보정 맵 생성 (한 번만 계산, 반복 사용으로 성능 향상)
		cv::initUndistortRectifyMap(
			m_cvCameraMatrix,
			m_cvDistCoeffs,
			cv::Mat(),  // rectification transformation (identity)
			m_cvNewCameraMatrix,
			imageSize,
			CV_16SC2,   // 16-bit signed, 성능 최적화
			m_cvMapX,
			m_cvMapY
		);

		m_bMapsInitialized = true;
	}
	catch (const cv::Exception& e)
	{
		m_bMapsInitialized = false;
	}
}

// ========================================================================
// ✅ UndistortImage - BYTE 배열 이미지의 왜곡 보정
// ========================================================================
bool Fnc_Vision_Pre_FITO::UndistortImage(const unsigned char* pSrc, unsigned char* pDst, int nWidth, int nHeight)
{
	if (!m_bCalibrated || pSrc == nullptr || pDst == nullptr)
	{
		// 캘리브레이션 없으면 원본 복사
		if (pSrc != nullptr && pDst != nullptr && nWidth > 0 && nHeight > 0)
		{
			memcpy(pDst, pSrc, nWidth * nHeight);
		}
		return false;
	}

	// 이미지 크기 변경 확인
	if (m_nSizeX != nWidth || m_nSizeY != nHeight)
	{
		m_nSizeX = nWidth;
		m_nSizeY = nHeight;
		m_bMapsInitialized = false;
		InitUndistortMaps();
	}

	if (!m_bMapsInitialized)
	{
		memcpy(pDst, pSrc, nWidth * nHeight);
		return false;
	}

	try
	{
		// BYTE 배열 → cv::Mat (복사 없이 래핑)
		cv::Mat srcMat(nHeight, nWidth, CV_8UC1, const_cast<unsigned char*>(pSrc));
		cv::Mat dstMat(nHeight, nWidth, CV_8UC1, pDst);

		// 왜곡 보정 적용 (빠른 Map 기반 변환)
		cv::remap(srcMat, dstMat, m_cvMapX, m_cvMapY, cv::INTER_LINEAR);

		return true;
	}
	catch (const cv::Exception& e)
	{
		// 오류 시 원본 복사
		memcpy(pDst, pSrc, nWidth * nHeight);
		return false;
	}
}

// ========================================================================
// ✅ UndistortPoint - 좌표 왜곡 보정
// ========================================================================
bool Fnc_Vision_Pre_FITO::UndistortPoint(double x, double y, double& outX, double& outY)
{
	if (!m_bCalibrated)
	{
		outX = x;
		outY = y;
		return false;
	}

	try
	{
		std::vector<cv::Point2f> srcPoints;
		std::vector<cv::Point2f> dstPoints;

		srcPoints.push_back(cv::Point2f(static_cast<float>(x), static_cast<float>(y)));

		// 왜곡 보정된 좌표 계산
		cv::undistortPoints(srcPoints, dstPoints, m_cvCameraMatrix, m_cvDistCoeffs, cv::Mat(), m_cvNewCameraMatrix);

		if (!dstPoints.empty())
		{
			outX = dstPoints[0].x;
			outY = dstPoints[0].y;
			return true;
		}

		outX = x;
		outY = y;
		return false;
	}
	catch (const cv::Exception& e)
	{
		outX = x;
		outY = y;
		return false;
	}
}

// ========================================================================
// ✅ SetCalibrationParams - 캘리브레이션 파라미터 직접 설정
// ========================================================================
void Fnc_Vision_Pre_FITO::SetCalibrationParams(const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs)
{
	m_cvCameraMatrix = cameraMatrix.clone();
	m_cvDistCoeffs = distCoeffs.clone();

	m_bCalibrated = (!m_cvCameraMatrix.empty() && !m_cvDistCoeffs.empty());
	m_bMapsInitialized = false;

	if (m_bCalibrated && m_nSizeX > 0 && m_nSizeY > 0)
	{
		InitUndistortMaps();
	}
}

// ========================================================================
// ✅ CV_ImgProcess_Prepare - 이미지 전처리 (Gain, Binary)
// ========================================================================
void Fnc_Vision_Pre_FITO::CV_ImgProcess_Prepare(BYTE* img, BYTE* Gainimg, BYTE* Binimg, int nWidth, int nHeight)
{
	if (img == nullptr || nWidth <= 0 || nHeight <= 0)
		return;

	int size = nWidth * nHeight;

	// Gain 이미지 생성
	if (Gainimg != nullptr)
	{
		for (int i = 0; i < size; i++)
		{
			int val = img[i] * 2; // 간단한 Gain 적용 (2배)
			Gainimg[i] = (val > 255) ? 255 : (BYTE)val;
		}
	}

	// Binary 이미지 생성
	if (Binimg != nullptr)
	{
		int threshold = 128; // 고정 threshold
		for (int i = 0; i < size; i++)
		{
			Binimg[i] = (img[i] > threshold) ? 255 : 0;
		}
	}
}

// ========================================================================
// ✅ GetVisionResult - Vision 처리 결과 반환
// ========================================================================
VisionResult Fnc_Vision_Pre_FITO::GetVisionResult()
{
	return stVisionResult;
}

// ========================================================================
// ✅ CV_PreVisionFindCorner - 코너 검출 (간소화 버전)
// ========================================================================
VisionResult Fnc_Vision_Pre_FITO::CV_PreVisionFindCorner(int nCorner)
{
	VisionResult result;
	result.bSuccess = false;
	result.dProcessTime = 0.0;
	result.strErrorMsg = "Not implemented in standalone version";

	// 실제 구현은 필요 시 추가
	return result;
}
