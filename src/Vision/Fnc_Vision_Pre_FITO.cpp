#include "Fnc_Vision_Pre_FITO.h"
#include "../AllInclude.h"
#include <atlimage.h>
#include <windows.h>
#include <tlhelp32.h>
#include "../Crevis/VirtualFG40CL.h"
#include "../DataDefine/GlobalVariable.h"

#pragma comment(lib, "VirtualFG40CL.lib")
IMPLEMENT_RUNTIMECLASS(Fnc_Vision_Pre_FITO)

using namespace tinyxml2;

// ========================================================================
// GlobalCallback - Frame Grabber 콜백 함수
// ========================================================================
void WINAPI GlobalCallbackPre_FITO(PMCSIGNALINFO SigInfo)
{
	if (SigInfo && SigInfo->Context)
	{
		Fnc_Vision_Pre_FITO* pDoc = (Fnc_Vision_Pre_FITO*)SigInfo->Context;
		pDoc->Callback(SigInfo);
	}
}

// ========================================================================
// 생성자
// ========================================================================
Fnc_Vision_Pre_FITO::Fnc_Vision_Pre_FITO()
{
	m_hDevice = 0;
	m_Channel = 0;

	pCamImg = nullptr;
	pOriginalImg = nullptr;
	pViewImg = nullptr;
	pGainImg = nullptr;
	pBinImg = nullptr;

	m_hProssed = nullptr;
	m_pCurrent = nullptr;

	dResol[0] = 0;
	dResol[1] = 0;

	nType = 0;
	nBoardNo = 0;
	chCamFile = "";
	chCannel = "";
	chCamID = "";
	chCommChannel = "";
	chViewShareMemFile = "";

	nCommPort = 0;
	m_bCommConnect = false;
	m_bCameraOpen = false;

	m_nRealSizeX = 0;
	m_nRealSizeY = 0;
	m_nSizeX = 0;
	m_nSizeY = 0;
	m_nViewerSizeX = 0;
	m_nViewerSizeY = 0;

	m_nFlipX = 0;
	m_nFlipY = 0;
	m_nRotate = 0;
	m_nRotateDegree = 0;

	SizeX = 0;
	SizeY = 0;
	BufferPitch = 0;

	m_nVision = 2;
	m_bImageMode = false;
	m_bGrabContinous = false;

	m_hGuiProcess = NULL;
	processIdOld = 0;
	m_nIsInitialize = 0;

	// ✅ OpenCV 캘리브레이션 초기화
	m_bCalibrated = false;
	m_bMapsInitialized = false;
	m_dReprojectionError = 0.0;
	m_bUseCalibration = false;

	CreateCS();
	::InitializeCriticalSection(&cs_Grab);
}

// ========================================================================
// 소멸자
// ========================================================================
Fnc_Vision_Pre_FITO::~Fnc_Vision_Pre_FITO()
{
	DeleteCS();
	::DeleteCriticalSection(&cs_Grab);
	::CloseHandle(m_hProssed);
	::CloseHandle(m_hGuiProcess);
}

// ========================================================================
// Callback - Frame Grabber에서 이미지 수신
// ========================================================================
void Fnc_Vision_Pre_FITO::Callback(PMCSIGNALINFO SigInfo)
{
	MCHANDLE hSurface = NULL;
	switch (SigInfo->Signal)
	{
	case MC_SIG_SURFACE_FILLED:
	case MC_SIG_SURFACE_PROCESSING:
		// Frame Grabber 메모리에서 이미지 포인터 획득
		McGetParamPtr(SigInfo->SignalInfo, MC_SurfaceAddr, &m_pCurrent);

		hSurface = (MCHANDLE)SigInfo->SignalInfo;

		// 원본 버퍼에 복사
		memcpy(pOriginalImg, (BYTE*)m_pCurrent, m_nSizeX * m_nSizeY);

		SetEvent(m_hProssed);
		break;

	case MC_SIG_ACQUISITION_FAILURE:
		RaiseError((int)enAlarm::VISION_OPEN_FAIL);
		break;

	case MC_SIG_END_CHANNEL_ACTIVITY:
		break;

	default:
		break;
	}
}

// ========================================================================
// InitialBoard - MultiCam 초기화 + OpenCV 캘리브레이션 로드
// ========================================================================
int Fnc_Vision_Pre_FITO::InitialBoard()
{
	char* p_path;
	int length;
	m_hProssed = CreateEventA(NULL, TRUE, FALSE, NULL);
	ResetEvent(m_hProssed);
	std::string strPath;
	std::string strConfigPath;
	std::string strRecipePath;
	length = GetCurrentDirectoryA(0, NULL);
	p_path = (char*)malloc(length + 1);
	if (GetCurrentDirectoryA(length, p_path) > 0)
	{
		strPath = p_path;
	}

	strConfigPath = strPath + "\\Data\\VisionParameter.xml";
	free(p_path);

	char cBuffer[1024];
	tinyxml2::XMLDocument xml;

	if (XML_SUCCESS != xml.LoadFile(strConfigPath.c_str()))
	{
		WriteFunctionLogE("Fail to Load Recipe XML");
		return FAIL;
	}

	tinyxml2::XMLElement* pSettingElement = xml.FirstChildElement("VISION");
	m_nUseCameraInitial = pSettingElement->FirstChildElement("SETTING")->FirstChildElement("CAM_INITIAL")->IntText();

	tinyxml2::XMLElement* pElement = xml.FirstChildElement("VISION")->FirstChildElement("CAM2");

	nType = pElement->FirstChildElement("TYPE")->IntText();
	nBoardNo = pElement->FirstChildElement("BOARD_NO")->IntText();
	dResol[_DX] = pElement->FirstChildElement("RESOL_X0")->DoubleText();
	dResol[_DY] = pElement->FirstChildElement("RESOL_Y0")->DoubleText();

	gParAd2D_set(UN_PREVISION, NS_VISION::m_dPixelResol, 0, _DX, dResol[_DX]);
	gParAd2D_set(UN_PREVISION, NS_VISION::m_dPixelResol, 0, _DY, dResol[_DY]);

	m_nSizeX = pElement->FirstChildElement("CAM_SIZE_X")->IntText();
	m_nSizeY = pElement->FirstChildElement("CAM_SIZE_Y")->IntText();
	m_nRealSizeX = m_nSizeX;
	m_nRealSizeY = m_nSizeY;
	m_nViewerSizeX = pElement->FirstChildElement("VIEWER_SIZE_X")->IntText();
	m_nViewerSizeY = pElement->FirstChildElement("VIEWER_SIZE_Y")->IntText();

	m_nFlipX = pElement->FirstChildElement("FLIP_X")->IntText();
	m_nFlipY = pElement->FirstChildElement("FLIP_Y")->IntText();
	m_nRotate = pElement->FirstChildElement("ROTATE")->IntText();
	m_nRotateDegree = pElement->FirstChildElement("ROTATE_DEGREE")->IntText();

	if (m_nRotate)
	{
		m_nSizeX = m_nViewerSizeX;
		m_nSizeY = m_nViewerSizeY;
	}

	chCamFile = pElement->FirstChildElement("CAM_FILE")->GetText();
	chCannel = pElement->FirstChildElement("CHANNEL")->GetText();
	chCamID = pElement->FirstChildElement("CAMID")->GetText();
	chCommChannel = pElement->FirstChildElement("COMM_CHANNEL")->GetText();
	nCommPort = pElement->FirstChildElement("COMM_PORT")->IntText();

	strRecipePath = strPath + StringFormat("\\Data\\CamFile\\%s", chCamFile);
	sprintf_s(cBuffer, "%s", strRecipePath.c_str());

	// MultiCam 초기화
	MCSTATUS status;

	status = McOpenDriver(NULL);
	Sleep(200);

	status = McCreate(MC_CHANNEL, &m_Channel);
	status = McSetParamInt(m_Channel, MC_DriverIndex, nBoardNo);
	status = McSetParamStr(m_Channel, MC_Connector, chCannel);
	status = McSetParamStr(m_Channel, MC_CamFile, cBuffer);

	status = McSetParamInt(m_Channel, MC_Expose_us, 1000);
	status = McSetParamInt(m_Channel, MC_ColorFormat, MC_ColorFormat_Y8);
	status = McSetParamInt(m_Channel, MC_TrigLine, MC_TrigLine_NOM);
	status = McSetParamInt(m_Channel, MC_TrigEdge, MC_TrigEdge_GOHIGH);
	status = McSetParamInt(m_Channel, MC_TrigFilter, MC_TrigFilter_ON);
	status = McSetParamInt(m_Channel, MC_TrigCtl, MC_TrigCtl_TTL);

	status = McSetParamInt(m_Channel, MC_TrigMode, MC_TrigMode_SOFT);
	status = McSetParamInt(m_Channel, MC_NextTrigMode, MC_TrigMode_SOFT);
	status = McSetParamInt(m_Channel, MC_SeqLength_Fr, MC_INDETERMINATE);

	status = McGetParamInt(m_Channel, MC_ImageSizeX, &SizeX);
	status = McGetParamInt(m_Channel, MC_ImageSizeY, &SizeY);
	status = McGetParamInt(m_Channel, MC_BufferPitch, &BufferPitch);

	status = McSetParamInt(m_Channel, MC_SurfaceCount, 3);

	if (nType == 1)
	{
		InitialCrevisCam();
	}

	status = McSetParamInt(m_Channel, MC_SignalEnable + MC_SIG_SURFACE_FILLED, MC_SignalEnable_ON);
	status = McSetParamInt(m_Channel, MC_SignalEnable + MC_SIG_ACQUISITION_FAILURE, MC_SignalEnable_ON);
	status = McSetParamInt(m_Channel, MC_SignalEnable + MC_SIG_END_CHANNEL_ACTIVITY, MC_SignalEnable_ON);

	// ✅ FITO 전용 콜백 등록
	status = McRegisterCallback(m_Channel, GlobalCallbackPre_FITO, this);

	status = McSetParamInt(m_Channel, MC_ChannelState, MC_ChannelState_ACTIVE);

	m_bCameraOpen = TRUE;

	// 공유 메모리 및 이미지 버퍼 생성
	int nSize = sizeof(BYTE) * m_nSizeX * m_nSizeY;
	smViewImage.CreateMemory("SM_CAM_2_VIEW", nSize);
	smViewImage.CreateSync("SYNC_CAM_2_VIEW");

	smImageStructure.CreateMemory("SM_CAM2_STRUCTURE", sizeof(stVisionData));
	smImageStructure.CreateSync("SYNC_CAM2_STRUCTURE");

	pOriginalImg = new BYTE[m_nSizeX * m_nSizeY];
	pCamImg = new BYTE[m_nSizeX * m_nSizeY];
	pGainImg = new BYTE[m_nSizeX * m_nSizeY];
	pBinImg = new BYTE[m_nSizeX * m_nSizeY];

	memset(pOriginalImg, 0, m_nSizeX * m_nSizeY);
	memset(pCamImg, 0, m_nSizeX * m_nSizeY);
	memset(pGainImg, 0, m_nSizeX * m_nSizeY);
	memset(pBinImg, 0, m_nSizeX * m_nSizeY);

	pViewImg = smViewImage.GetMemoryPointerByte();
	stImage = smImageStructure.GetMemoryPointerVisionData();

	// ✅ OpenCV 캘리브레이션 로드
	std::string strCalibPath = strPath + "\\dat\\CutterConfig\\OpenCV_Calibration_PreVision.xml";

	if (LoadCalibration(strCalibPath))
	{
		m_bUseCalibration = true;
		WriteFunctionLogI(StringFormat("OpenCV Calibration Loaded: Error=%.3f pixels", m_dReprojectionError));
	}
	else
	{
		m_bUseCalibration = false;
		WriteFunctionLogW("OpenCV Calibration Not Found - Raw Image Mode");
	}

	return PASS;
}

// ========================================================================
// Grab - 이미지 획득 + OpenCV 왜곡 보정
// ========================================================================
bool Fnc_Vision_Pre_FITO::Grab()
{
	// 이미지 파일 모드
	if (m_bImageMode)
	{
		CV_ImgProcess_Prepare(pCamImg, pGainImg, pBinImg);

		if (VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionViewMode, 0) == 0) // Raw Image
		{
			memcpy(pViewImg, pCamImg, m_nSizeX * m_nSizeY);
		}
		else if (VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionViewMode, 0) == 1) // Gain Image
		{
			memcpy(pViewImg, pGainImg, m_nSizeX * m_nSizeY);
		}
		else if (VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionViewMode, 0) == 2) // Binary Image
		{
			memcpy(pViewImg, pBinImg, m_nSizeX * m_nSizeY);
		}

		Paint();
		return true;
	}

	// 카메라 미오픈
	if (!m_bCameraOpen) return false;

	// Frame Grabber 트리거
	ResetEvent(m_hProssed);
	McSetParamInt(m_Channel, MC_ForceTrig, MC_ForceTrig_TRIG);

	// 이미지 수신 대기
	DWORD dwRet;
	dwRet = WaitForSingleObject(m_hProssed, 1000);
	if (dwRet == WAIT_TIMEOUT)
	{
		ResetEvent(m_hProssed);
		return FAIL;
	}

	ResetEvent(m_hProssed);

	// ✅ 1단계: OpenCV 왜곡 보정 (회전 전에 먼저 수행)
	BYTE* pUndistortedImg = nullptr;

	if (m_bUseCalibration && m_bCalibrated)
	{
		pUndistortedImg = new BYTE[m_nRealSizeX * m_nRealSizeY];

		// 원본 이미지 왜곡 보정
		bool bSuccess = UndistortImage(pOriginalImg, pUndistortedImg, m_nRealSizeX, m_nRealSizeY);

		if (!bSuccess)
		{
			WriteFunctionLogW("Image Undistortion Failed - Using Raw Image");
			memcpy(pUndistortedImg, pOriginalImg, m_nRealSizeX * m_nRealSizeY);
		}
	}
	else
	{
		// 캘리브레이션 없으면 원본 사용
		pUndistortedImg = pOriginalImg;
	}

	// ✅ 2단계: 회전 처리 (보정된 이미지를 회전)
	if (m_nRotate)
	{
		SwapCoordinate(pUndistortedImg, pCamImg, m_nRealSizeX, m_nRealSizeY, (double)m_nRotateDegree);
	}
	else
	{
		memcpy(pCamImg, pUndistortedImg, m_nSizeX * m_nSizeY);
	}

	// 임시 버퍼 정리
	if (m_bUseCalibration && m_bCalibrated && pUndistortedImg != nullptr)
	{
		delete[] pUndistortedImg;
	}

	// 전처리 (Gain, Binary)
	CV_ImgProcess_Prepare(pCamImg, pGainImg, pBinImg);

	// 뷰 모드에 따라 이미지 선택
	if (VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionViewMode, 0) == 0) // Raw Image
	{
		memcpy(pViewImg, pCamImg, m_nSizeX * m_nSizeY);
	}
	else if (VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionViewMode, 0) == 1) // Gain Image
	{
		memcpy(pViewImg, pGainImg, m_nSizeX * m_nSizeY);
	}
	else if (VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionViewMode, 0) == 2) // Binary Image
	{
		memcpy(pViewImg, pBinImg, m_nSizeX * m_nSizeY);
	}

	return true;
}

// ========================================================================
// Paint - GUI에 이미지 전달
// ========================================================================
void Fnc_Vision_Pre_FITO::Paint(bool GrabContinue /* false*/)
{
	auto ConvertToWString = [&](const char* str) -> std::wstring {
		size_t len = strlen(str) + 1;
		wchar_t* buffer = new wchar_t[len];
		size_t sn;
		mbstowcs_s(&sn, buffer, len, str, len);

		std::wstring result(buffer);
		delete[] buffer;

		return result;
	};

	auto GetProcessIdByName = [&](const std::wstring& processName) -> DWORD {
		DWORD processId = 0;
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (hSnapshot != INVALID_HANDLE_VALUE) {
			PROCESSENTRY32 pe32;
			pe32.dwSize = sizeof(PROCESSENTRY32);

			if (Process32First(hSnapshot, &pe32)) {
				do {
					if (ConvertToWString(pe32.szExeFile) == processName) {
						processId = pe32.th32ProcessID;
						break;
					}
				} while (Process32Next(hSnapshot, &pe32));
			}
			CloseHandle(hSnapshot);
		}
		return processId;
	};

	if (stMain.hwView == NULL)
	{
		HWND wView = NULL;

		if (FindWindow(NULL, "CutterWindow"))
		{
			std::wstring processName = UI_LAUNCHER_CTRL_W();
			DWORD processId = GetProcessIdByName(processName);
			if (processIdOld != processId)
			{
				if (m_hProssed != NULL)
				{
					::CloseHandle(m_hGuiProcess);
					m_hGuiProcess = NULL;
				}
				if (processId != 0)
				{
					processIdOld = processId;
					m_hGuiProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
					wView = ::FindWindow(NULL, "CutterWindow");
				}
			}
		}

		if (wView != NULL)
		{
			::PostMessage(wView, WM_MY_GRAB_DONE2, 0, GrabContinue);
		}
	}
	else
	{
		::PostMessage(stMain.hwView, WM_MY_GRAB_DONE2, 0, GrabContinue);
	}
}

// ========================================================================
// SetImageMode
// ========================================================================
void Fnc_Vision_Pre_FITO::SetImageMode(bool bMode)
{
	EnterCriticalSection(&cs_Grab);
	m_bImageMode = bMode;
	LeaveCriticalSection(&cs_Grab);
}

// ========================================================================
// ImageOpen - 이미지 파일 열기 (OpenCV 사용)
// ========================================================================
void Fnc_Vision_Pre_FITO::ImageOpen(std::string strFile)
{
	EnterCriticalSection(&cs_Grab);

	try
	{
		// OpenCV로 이미지 로드
		cv::Mat image = cv::imread(strFile, cv::IMREAD_GRAYSCALE);

		if (image.empty())
		{
			WriteFunctionLogE("Failed to load image file");
			LeaveCriticalSection(&cs_Grab);
			return;
		}

		// 크기 조정
		cv::Mat resized;
		cv::resize(image, resized, cv::Size(m_nSizeX, m_nSizeY), 0, 0, cv::INTER_LINEAR);

		// 버퍼에 복사
		memcpy(pOriginalImg, resized.data, m_nSizeX * m_nSizeY);
		memcpy(pCamImg, resized.data, m_nSizeX * m_nSizeY);
		memcpy(pViewImg, resized.data, m_nSizeX * m_nSizeY);

		m_bImageMode = true;
		Grab();
	}
	catch (const cv::Exception& e)
	{
		WriteFunctionLogE(StringFormat("OpenCV Error: %s", e.what()));
	}

	LeaveCriticalSection(&cs_Grab);
}

// ========================================================================
// InitialCrevisCam - Crevis 카메라 초기화 (주석 처리)
// ========================================================================
void Fnc_Vision_Pre_FITO::InitialCrevisCam()
{
	// Crevis 카메라 초기화 코드 (주석 처리됨)
}

// ========================================================================
// OpenCV 캘리브레이션 기능
// ========================================================================

// LoadCalibration - XML 파일에서 캘리브레이션 파라미터 로드
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

// SaveCalibration - XML 파일로 캘리브레이션 파라미터 저장
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

// InitUndistortMaps - 왜곡 보정 맵 생성 (초기화 시 한 번만)
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

// UndistortImage - BYTE 배열 이미지의 왜곡 보정
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

// UndistortPoint - 좌표 왜곡 보정
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
			outX = static_cast<double>(dstPoints[0].x);
			outY = static_cast<double>(dstPoints[0].y);
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

// SetCalibrationParams - 캘리브레이션 파라미터 직접 설정
void Fnc_Vision_Pre_FITO::SetCalibrationParams(const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs)
{
	m_cvCameraMatrix = cameraMatrix.clone();
	m_cvDistCoeffs = distCoeffs.clone();
	m_bCalibrated = true;
	m_bMapsInitialized = false;

	InitUndistortMaps();
}

// ========================================================================
// Edge 추출 기능 (왜곡된 이미지 대응)
// ========================================================================

/**
 * @brief Canny Edge 검출
 * @details 왜곡된 이미지에서 edge를 추출합니다.
 *          캘리브레이션된 이미지에서 사용하면 더 정확한 직선 에지를 얻을 수 있습니다.
 *
 * @param pSrc 입력 이미지 (BYTE 배열, 8-bit grayscale)
 * @param pDst 출력 Edge 이미지 (BYTE 배열, 8-bit binary)
 * @param nWidth 이미지 너비
 * @param nHeight 이미지 높이
 * @param dLowThreshold Canny 하위 임계값 (기본: 50.0)
 * @param dHighThreshold Canny 상위 임계값 (기본: 150.0)
 * @param nApertureSize Sobel 커널 크기 (3, 5, 7, 기본: 3)
 * @param bL2Gradient L2 norm 사용 여부 (기본: false = L1 norm)
 * @return 성공 시 true, 실패 시 false
 *
 * @note 권장 파라미터:
 *       - 일반적인 경우: dLowThreshold=50, dHighThreshold=150
 *       - 노이즈가 많은 경우: dLowThreshold=100, dHighThreshold=200
 *       - 약한 에지도 검출: dLowThreshold=30, dHighThreshold=100
 */
bool Fnc_Vision_Pre_FITO::CV_ExtractEdgesCanny(const unsigned char* pSrc, unsigned char* pDst,
											   int nWidth, int nHeight,
											   double dLowThreshold, double dHighThreshold,
											   int nApertureSize, bool bL2Gradient)
{
	if (pSrc == nullptr || pDst == nullptr || nWidth <= 0 || nHeight <= 0)
	{
		return false;
	}

	// Aperture size 유효성 검사 (3, 5, 7만 가능)
	if (nApertureSize != 3 && nApertureSize != 5 && nApertureSize != 7)
	{
		nApertureSize = 3;
	}

	try
	{
		// BYTE 배열 → cv::Mat (복사 없이 래핑)
		cv::Mat srcMat(nHeight, nWidth, CV_8UC1, const_cast<unsigned char*>(pSrc));
		cv::Mat dstMat(nHeight, nWidth, CV_8UC1, pDst);

		// Gaussian Blur로 노이즈 제거 (Canny 전처리)
		cv::Mat blurred;
		cv::GaussianBlur(srcMat, blurred, cv::Size(5, 5), 1.4);

		// Canny Edge 검출
		cv::Canny(blurred, dstMat, dLowThreshold, dHighThreshold, nApertureSize, bL2Gradient);

		return true;
	}
	catch (const cv::Exception& e)
	{
		// 오류 시 원본 복사
		memcpy(pDst, pSrc, nWidth * nHeight);
		return false;
	}
}

/**
 * @brief LSD (Line Segment Detector) 직선 검출
 * @details OpenCV LSD 알고리즘으로 왜곡된 이미지에서 직선을 검출합니다.
 *          Halcon의 LinesGauss와 유사한 기능을 제공합니다.
 *
 * @param pSrc 입력 이미지 (BYTE 배열, 8-bit grayscale)
 * @param lines 출력 직선 벡터 (Vec4f: [x1, y1, x2, y2])
 * @param nWidth 이미지 너비
 * @param nHeight 이미지 높이
 * @param dScale 이미지 스케일 (기본: 0.8, 범위: 0.5~1.0)
 * @param dSigmaScale Gaussian blur sigma (기본: 0.6)
 * @param dMinLength 최소 직선 길이 (픽셀, 기본: 10.0)
 * @return 성공 시 true, 실패 시 false
 *
 * @note LSD는 왜곡된 직선도 어느 정도 검출하지만, 캘리브레이션 후 사용 권장
 * @note Halcon LinesGauss 대응:
 *       - LinesGauss(Sigma=1.2) ≈ LSD(dSigmaScale=0.6)
 *       - LinesGauss(Low=1, High=3) ≈ LSD(dMinLength=10)
 */
bool Fnc_Vision_Pre_FITO::CV_ExtractLinesLSD(const unsigned char* pSrc, std::vector<cv::Vec4f>& lines,
											  int nWidth, int nHeight,
											  double dScale, double dSigmaScale, double dMinLength)
{
	if (pSrc == nullptr || nWidth <= 0 || nHeight <= 0)
	{
		return false;
	}

	try
	{
		// BYTE 배열 → cv::Mat (복사 없이 래핑)
		cv::Mat srcMat(nHeight, nWidth, CV_8UC1, const_cast<unsigned char*>(pSrc));

		// LSD 검출기 생성
		cv::Ptr<cv::LineSegmentDetector> lsd = cv::createLineSegmentDetector(
			cv::LSD_REFINE_STD,  // Refinement: NONE, STD, ADV
			dScale,              // Scale
			dSigmaScale,         // Sigma for Gaussian filter
			2.0,                 // Quantization error
			22.5,                // Angle tolerance (degrees)
			0.0,                 // Detection threshold (log-eps)
			0.7,                 // Density threshold
			1024                 // Number of bins
		);

		// 직선 검출
		std::vector<cv::Vec4f> detectedLines;
		lsd->detect(srcMat, detectedLines);

		// 최소 길이 필터링
		lines.clear();
		for (const auto& line : detectedLines)
		{
			float x1 = line[0], y1 = line[1];
			float x2 = line[2], y2 = line[3];
			float length = std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));

			if (length >= dMinLength)
			{
				lines.push_back(line);
			}
		}

		return true;
	}
	catch (const cv::Exception& e)
	{
		lines.clear();
		return false;
	}
}

/**
 * @brief Robust Line Fitting (RANSAC 기반)
 * @details 점들의 집합에서 robust하게 직선을 피팅합니다.
 *          Halcon의 FitLineContourXld("tukey")와 유사한 기능을 제공합니다.
 *
 * @param points 입력 점 벡터 (Point2f)
 * @param line 출력 직선 (Vec4f: [vx, vy, x0, y0])
 *             vx, vy: 직선의 방향 벡터 (정규화됨)
 *             x0, y0: 직선 위의 한 점
 * @param nDistType 거리 타입 (기본: DIST_HUBER)
 *                  - DIST_L2: 최소제곱법 (outlier에 약함)
 *                  - DIST_HUBER: Huber robust estimator (권장)
 *                  - DIST_FAIR: Fair robust estimator
 * @param dParam 파라미터 (DIST_HUBER: 1.345, DIST_FAIR: 1.3998)
 * @param dRadiusEps 직선까지의 거리 오차 (기본: 0.01)
 * @param dAngleEps 각도 오차 (기본: 0.01)
 * @return 성공 시 true, 실패 시 false
 *
 * @note 출력 직선을 두 점으로 변환하려면:
 *       x1 = x0, y1 = y0
 *       x2 = x0 + vx * length, y2 = y0 + vy * length
 *
 * @note Halcon FitLineContourXld("tukey") 대응:
 *       - "tukey" ≈ DIST_HUBER
 *       - "huber" ≈ DIST_HUBER
 *       - "regression" ≈ DIST_L2
 */
bool Fnc_Vision_Pre_FITO::CV_FitLineRobust(const std::vector<cv::Point2f>& points,
										   cv::Vec4f& line,
										   int nDistType, double dParam,
										   double dRadiusEps, double dAngleEps)
{
	if (points.size() < 2)
	{
		return false;
	}

	try
	{
		// OpenCV fitLine 함수 사용
		// distType: DIST_L2, DIST_HUBER, DIST_FAIR 등
		cv::fitLine(points, line, nDistType, dParam, dRadiusEps, dAngleEps);

		// line 형식: [vx, vy, x0, y0]
		// vx, vy: 방향 벡터 (정규화됨)
		// x0, y0: 직선 위의 한 점

		return true;
	}
	catch (const cv::Exception& e)
	{
		return false;
	}
}

// ========================================================================
// Hough 변환 직선 검출
// ========================================================================

/**
 * @brief 표준 Hough 변환으로 직선 검출
 * @details 극좌표계(ρ, θ)로 직선을 검출합니다.
 *          Edge 이미지(Canny 결과)를 입력으로 받습니다.
 *
 * @param pSrc 입력 Edge 이미지 (BYTE 배열, 8-bit binary, Canny 결과)
 * @param lines 출력 직선 벡터 (Vec2f: [ρ, θ])
 *              ρ: 원점에서 직선까지의 거리 (픽셀)
 *              θ: 직선의 각도 (라디안, 0~π)
 * @param nWidth 이미지 너비
 * @param nHeight 이미지 높이
 * @param dRho ρ 해상도 (픽셀, 기본: 1.0)
 * @param dTheta θ 해상도 (라디안, 기본: π/180 = 1도)
 * @param nThreshold 누적기 임계값 (기본: 100)
 * @param dMinTheta 최소 각도 (라디안, 기본: 0.0)
 * @param dMaxTheta 최대 각도 (라디안, 기본: π)
 * @return 성공 시 true, 실패 시 false
 *
 * @note 직선 방정식: x*cos(θ) + y*sin(θ) = ρ
 * @note 수평선: θ ≈ 0° 또는 180°
 *       수직선: θ ≈ 90°
 *
 * @note 사용 순서:
 *       1. CV_ExtractEdgesCanny() → Edge 이미지 생성
 *       2. CV_HoughLinesStandard() → 극좌표 직선 검출
 *       3. CV_ConvertHoughLinesToCartesian() → 직교좌표 변환
 */
bool Fnc_Vision_Pre_FITO::CV_HoughLinesStandard(const unsigned char* pSrc, std::vector<cv::Vec2f>& lines,
												 int nWidth, int nHeight,
												 double dRho, double dTheta,
												 int nThreshold, double dMinTheta, double dMaxTheta)
{
	if (pSrc == nullptr || nWidth <= 0 || nHeight <= 0)
	{
		return false;
	}

	try
	{
		// BYTE 배열 → cv::Mat (복사 없이 래핑)
		cv::Mat srcMat(nHeight, nWidth, CV_8UC1, const_cast<unsigned char*>(pSrc));

		// 표준 Hough 변환
		cv::HoughLines(srcMat, lines, dRho, dTheta, nThreshold, 0, 0, dMinTheta, dMaxTheta);

		return true;
	}
	catch (const cv::Exception& e)
	{
		lines.clear();
		return false;
	}
}

/**
 * @brief 확률적 Hough 변환으로 직선 세그먼트 검출
 * @details 직교좌표계(x1, y1, x2, y2)로 직선 세그먼트를 직접 검출합니다.
 *          표준 Hough 변환보다 빠르고 직선 세그먼트를 바로 얻을 수 있습니다.
 *
 * @param pSrc 입력 Edge 이미지 (BYTE 배열, 8-bit binary, Canny 결과)
 * @param lines 출력 직선 세그먼트 벡터 (Vec4f: [x1, y1, x2, y2])
 * @param nWidth 이미지 너비
 * @param nHeight 이미지 높이
 * @param dRho ρ 해상도 (픽셀, 기본: 1.0)
 * @param dTheta θ 해상도 (라디안, 기본: π/180 = 1도)
 * @param nThreshold 누적기 임계값 (기본: 50)
 * @param dMinLineLength 최소 직선 길이 (픽셀, 기본: 50.0)
 * @param dMaxLineGap 직선 세그먼트 간 최대 갭 (픽셀, 기본: 10.0)
 * @return 성공 시 true, 실패 시 false
 *
 * @note 확률적 Hough 변환은 표준 Hough보다:
 *       - 빠름 (랜덤 샘플링)
 *       - 직선 세그먼트 직접 반환 (변환 불필요)
 *       - 짧은 직선도 검출 가능
 *
 * @note 권장 파라미터:
 *       - 긴 직선만: nThreshold=100, dMinLineLength=100, dMaxLineGap=5
 *       - 짧은 직선도: nThreshold=50, dMinLineLength=30, dMaxLineGap=10
 *       - 매우 민감하게: nThreshold=30, dMinLineLength=20, dMaxLineGap=15
 *
 * @note 사용 순서:
 *       1. CV_ExtractEdgesCanny() → Edge 이미지 생성
 *       2. CV_HoughLinesP() → 직선 세그먼트 검출 (변환 불필요)
 */
bool Fnc_Vision_Pre_FITO::CV_HoughLinesP(const unsigned char* pSrc, std::vector<cv::Vec4f>& lines,
										 int nWidth, int nHeight,
										 double dRho, double dTheta,
										 int nThreshold, double dMinLineLength, double dMaxLineGap)
{
	if (pSrc == nullptr || nWidth <= 0 || nHeight <= 0)
	{
		return false;
	}

	try
	{
		// BYTE 배열 → cv::Mat (복사 없이 래핑)
		cv::Mat srcMat(nHeight, nWidth, CV_8UC1, const_cast<unsigned char*>(pSrc));

		// 확률적 Hough 변환
		cv::HoughLinesP(srcMat, lines, dRho, dTheta, nThreshold, dMinLineLength, dMaxLineGap);

		return true;
	}
	catch (const cv::Exception& e)
	{
		lines.clear();
		return false;
	}
}

/**
 * @brief Hough 극좌표 직선을 직교좌표로 변환
 * @details 표준 Hough 변환 결과(ρ, θ)를 직교좌표(x1, y1, x2, y2)로 변환합니다.
 *
 * @param polarLines 입력 극좌표 직선 벡터 (Vec2f: [ρ, θ])
 * @param cartesianLines 출력 직교좌표 직선 벡터 (Vec4f: [x1, y1, x2, y2])
 * @param nWidth 이미지 너비 (직선 연장 범위)
 * @param nHeight 이미지 높이 (직선 연장 범위)
 * @return 성공 시 true, 실패 시 false
 *
 * @note 극좌표 → 직교좌표 변환:
 *       직선 방정식: x*cos(θ) + y*sin(θ) = ρ
 *
 *       θ ≈ 0° (수평선):
 *         x1 = 0, y1 = ρ/sin(θ)
 *         x2 = width, y2 = (ρ - width*cos(θ))/sin(θ)
 *
 *       θ ≈ 90° (수직선):
 *         x1 = ρ/cos(θ), y1 = 0
 *         x2 = (ρ - height*sin(θ))/cos(θ), y2 = height
 *
 * @note 변환된 직선은 이미지 경계까지 연장됩니다.
 */
bool Fnc_Vision_Pre_FITO::CV_ConvertHoughLinesToCartesian(const std::vector<cv::Vec2f>& polarLines,
														   std::vector<cv::Vec4f>& cartesianLines,
														   int nWidth, int nHeight)
{
	if (polarLines.empty() || nWidth <= 0 || nHeight <= 0)
	{
		return false;
	}

	try
	{
		cartesianLines.clear();
		cartesianLines.reserve(polarLines.size());

		for (const auto& polarLine : polarLines)
		{
			float rho = polarLine[0];
			float theta = polarLine[1];

			double a = std::cos(theta);
			double b = std::sin(theta);
			double x0 = a * rho;
			double y0 = b * rho;

			// 직선을 이미지 경계까지 연장
			// (x0, y0)는 직선 위의 한 점
			// (a, b)는 직선에 수직인 벡터
			// 직선 방향 벡터: (-b, a)

			float x1, y1, x2, y2;

			if (std::abs(b) > std::abs(a))  // 수평선에 가까운 경우
			{
				// y = 0, y = height일 때의 x 좌표 계산
				x1 = static_cast<float>(x0 + 2000 * (-b));
				y1 = static_cast<float>(y0 + 2000 * (a));
				x2 = static_cast<float>(x0 - 2000 * (-b));
				y2 = static_cast<float>(y0 - 2000 * (a));
			}
			else  // 수직선에 가까운 경우
			{
				// x = 0, x = width일 때의 y 좌표 계산
				x1 = static_cast<float>(x0 + 2000 * (-b));
				y1 = static_cast<float>(y0 + 2000 * (a));
				x2 = static_cast<float>(x0 - 2000 * (-b));
				y2 = static_cast<float>(y0 - 2000 * (a));
			}

			cartesianLines.push_back(cv::Vec4f(x1, y1, x2, y2));
		}

		return true;
	}
	catch (const std::exception& e)
	{
		cartesianLines.clear();
		return false;
	}
}
