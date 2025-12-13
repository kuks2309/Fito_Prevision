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

	// TEED 공유 메모리 초기화
	m_hTeedInputMapping = NULL;
	m_hTeedOutputMapping = NULL;
	m_pTeedInputBuffer = nullptr;
	m_pTeedOutputBuffer = nullptr;
	m_bTeedConnected = false;
	m_nTeedHeight = 0;
	m_nTeedWidth = 0;
	m_hTeedProcess = NULL;
	m_strTeedServerPath = "D:\\FITO_2026\\TEED\\teed_shared_memory.py";

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

	// TEED 공유 메모리 해제 및 프로세스 종료
	TEED_Disconnect();

	// Python 서버 프로세스 종료
	if (m_hTeedProcess != NULL)
	{
		TerminateProcess(m_hTeedProcess, 0);
		CloseHandle(m_hTeedProcess);
		m_hTeedProcess = NULL;
	}

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

// ========================================================================
// ✅ TEED_LaunchServer - Python 서버 실행
// ========================================================================
bool Fnc_Vision_Pre_FITO::TEED_LaunchServer(int height, int width)
{
	if (m_hTeedProcess != NULL)
	{
		// 이미 실행 중인지 확인
		DWORD exitCode;
		if (GetExitCodeProcess(m_hTeedProcess, &exitCode) && exitCode == STILL_ACTIVE)
		{
			return true;  // 이미 실행 중
		}
		CloseHandle(m_hTeedProcess);
		m_hTeedProcess = NULL;
	}

	// Python 명령어 구성
	char cmdLine[512];
	snprintf(cmdLine, sizeof(cmdLine),
		"python \"%s\" --height %d --width %d",
		m_strTeedServerPath.c_str(), height, width);

	STARTUPINFOA si = { sizeof(si) };
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;  // 창 숨김

	PROCESS_INFORMATION pi;

	if (!CreateProcessA(
		NULL,           // 실행 파일 (NULL이면 cmdLine에서 파싱)
		cmdLine,        // 명령줄
		NULL,           // 프로세스 보안
		NULL,           // 스레드 보안
		FALSE,          // 핸들 상속
		CREATE_NO_WINDOW,  // 콘솔 창 없음
		NULL,           // 환경변수
		NULL,           // 작업 디렉토리
		&si,
		&pi))
	{
		return false;
	}

	m_hTeedProcess = pi.hProcess;
	CloseHandle(pi.hThread);

	// 모델 로딩 대기 (약 2~3초)
	Sleep(3000);

	return true;
}

// ========================================================================
// ✅ TEED_Connect - 공유 메모리 연결
// ========================================================================
bool Fnc_Vision_Pre_FITO::TEED_Connect(int height, int width)
{
	printf("[TEED] TEED_Connect called\n");
	fflush(stdout);

	if (m_bTeedConnected)
	{
		// 이미 연결된 경우, 크기가 같으면 재사용
		if (m_nTeedHeight == height && m_nTeedWidth == width)
		{
			printf("[TEED] Already connected with same size\n");
			fflush(stdout);
			return true;
		}

		// 크기가 다르면 재연결
		TEED_Disconnect();
	}

	m_nTeedHeight = height;
	m_nTeedWidth = width;

	// 입력 버퍼 크기: header + 3*H*W*4 (float32, CHW)
	size_t inputSize = TEED_HEADER_SIZE + 3 * height * width * sizeof(float);

	// 출력 버퍼 크기: header + H*W (uint8)
	size_t outputSize = TEED_HEADER_SIZE + height * width;

	printf("[TEED] Trying to connect: %dx%d, inputSize=%zu, outputSize=%zu\n",
		width, height, inputSize, outputSize);
	fflush(stdout);

	// 입력 공유 메모리 열기 시도 (Local namespace)
	std::string inputName = std::string("Local\\") + TEED_INPUT_NAME;
	std::string outputName = std::string("Local\\") + TEED_OUTPUT_NAME;

	m_hTeedInputMapping = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, inputName.c_str());
	if (m_hTeedInputMapping == NULL)
	{
		DWORD err = GetLastError();
		printf("[TEED] OpenFileMapping failed for '%s', error=%lu\n", inputName.c_str(), err);
		fflush(stdout);

		// Local 없이도 시도
		m_hTeedInputMapping = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, TEED_INPUT_NAME);
		if (m_hTeedInputMapping == NULL)
		{
			err = GetLastError();
			printf("[TEED] OpenFileMapping failed for '%s', error=%lu\n", TEED_INPUT_NAME, err);
			fflush(stdout);

			// 공유 메모리가 없음 → Python 서버 실행
			printf("[TEED] Launching Python server...\n");
			fflush(stdout);
			if (!TEED_LaunchServer(height, width))
			{
				printf("[TEED] Failed to launch Python server\n");
				fflush(stdout);
				return false;
			}

			// 다시 연결 시도 (3초 대기 후)
			Sleep(3000);
			m_hTeedInputMapping = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, inputName.c_str());
			if (m_hTeedInputMapping == NULL)
			{
				m_hTeedInputMapping = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, TEED_INPUT_NAME);
			}

			if (m_hTeedInputMapping == NULL)
			{
				printf("[TEED] Still cannot open shared memory after server launch\n");
				fflush(stdout);
				// 직접 생성
				m_hTeedInputMapping = CreateFileMappingA(
					INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
					0, static_cast<DWORD>(inputSize), TEED_INPUT_NAME
				);
				printf("[TEED] Created input shared memory directly\n");
				fflush(stdout);
			}
		}
	}

	if (m_hTeedInputMapping == NULL)
	{
		printf("[TEED] Failed to get input mapping handle\n");
		fflush(stdout);
		return false;
	}

	printf("[TEED] Input mapping handle obtained\n");
	fflush(stdout);

	m_pTeedInputBuffer = static_cast<BYTE*>(
		MapViewOfFile(m_hTeedInputMapping, FILE_MAP_ALL_ACCESS, 0, 0, inputSize)
	);

	if (m_pTeedInputBuffer == nullptr)
	{
		DWORD err = GetLastError();
		printf("[TEED] MapViewOfFile failed for input, error=%lu\n", err);
		fflush(stdout);
		CloseHandle(m_hTeedInputMapping);
		m_hTeedInputMapping = NULL;
		return false;
	}

	printf("[TEED] Input buffer mapped\n");
	fflush(stdout);

	// 출력 공유 메모리 열기/생성
	m_hTeedOutputMapping = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, outputName.c_str());
	if (m_hTeedOutputMapping == NULL)
	{
		m_hTeedOutputMapping = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, TEED_OUTPUT_NAME);
	}
	if (m_hTeedOutputMapping == NULL)
	{
		m_hTeedOutputMapping = CreateFileMappingA(
			INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
			0, static_cast<DWORD>(outputSize), TEED_OUTPUT_NAME
		);
	}

	if (m_hTeedOutputMapping == NULL)
	{
		printf("[TEED] Failed to get output mapping handle\n");
		fflush(stdout);
		UnmapViewOfFile(m_pTeedInputBuffer);
		CloseHandle(m_hTeedInputMapping);
		m_pTeedInputBuffer = nullptr;
		m_hTeedInputMapping = NULL;
		return false;
	}

	printf("[TEED] Output mapping handle obtained\n");
	fflush(stdout);

	m_pTeedOutputBuffer = static_cast<BYTE*>(
		MapViewOfFile(m_hTeedOutputMapping, FILE_MAP_ALL_ACCESS, 0, 0, outputSize)
	);

	if (m_pTeedOutputBuffer == nullptr)
	{
		printf("[TEED] MapViewOfFile failed for output\n");
		fflush(stdout);
		UnmapViewOfFile(m_pTeedInputBuffer);
		CloseHandle(m_hTeedInputMapping);
		CloseHandle(m_hTeedOutputMapping);
		m_pTeedInputBuffer = nullptr;
		m_hTeedInputMapping = NULL;
		m_hTeedOutputMapping = NULL;
		return false;
	}

	printf("[TEED] Output buffer mapped - Connection successful!\n");
	fflush(stdout);
	m_bTeedConnected = true;
	return true;
}

// ========================================================================
// ✅ TEED_Disconnect - 공유 메모리 해제
// ========================================================================
void Fnc_Vision_Pre_FITO::TEED_Disconnect()
{
	if (m_pTeedInputBuffer)
	{
		UnmapViewOfFile(m_pTeedInputBuffer);
		m_pTeedInputBuffer = nullptr;
	}

	if (m_pTeedOutputBuffer)
	{
		UnmapViewOfFile(m_pTeedOutputBuffer);
		m_pTeedOutputBuffer = nullptr;
	}

	if (m_hTeedInputMapping)
	{
		CloseHandle(m_hTeedInputMapping);
		m_hTeedInputMapping = NULL;
	}

	if (m_hTeedOutputMapping)
	{
		CloseHandle(m_hTeedOutputMapping);
		m_hTeedOutputMapping = NULL;
	}

	m_bTeedConnected = false;
}

// ========================================================================
// ✅ TEED_Preprocess - 이미지 전처리 (32배수 crop + 1/4 resize + 정규화)
// ========================================================================
void Fnc_Vision_Pre_FITO::TEED_Preprocess(const cv::Mat& inputBGR, float* outputTensor,
                                           int& outHeight, int& outWidth)
{
	// BGR mean values (TEED 학습 시 사용된 값)
	const float mean_b = 104.007f;
	const float mean_g = 116.669f;
	const float mean_r = 122.679f;

	int h_orig = inputBGR.rows;
	int w_orig = inputBGR.cols;

	// 1. 32의 배수로 crop (1/4 축소 후 8의 배수 보장)
	int h_crop = (h_orig / 32) * 32;
	int w_crop = (w_orig / 32) * 32;
	int y_start = (h_orig - h_crop) / 2;
	int x_start = (w_orig - w_crop) / 2;

	cv::Mat cropped = inputBGR(cv::Rect(x_start, y_start, w_crop, h_crop));

	// 2. 1/4 축소
	outHeight = h_crop / 4;
	outWidth = w_crop / 4;
	cv::Mat resized;
	cv::resize(cropped, resized, cv::Size(outWidth, outHeight), 0, 0, cv::INTER_AREA);

	// 3. float 변환 및 정규화, CHW 순서로 저장
	// CHW 순서: B채널 전체 -> G채널 전체 -> R채널 전체
	int planeSize = outHeight * outWidth;

	for (int y = 0; y < outHeight; y++)
	{
		for (int x = 0; x < outWidth; x++)
		{
			cv::Vec3b pixel = resized.at<cv::Vec3b>(y, x);
			int idx = y * outWidth + x;

			// Channel 0: Blue
			outputTensor[0 * planeSize + idx] = static_cast<float>(pixel[0]) - mean_b;
			// Channel 1: Green
			outputTensor[1 * planeSize + idx] = static_cast<float>(pixel[1]) - mean_g;
			// Channel 2: Red
			outputTensor[2 * planeSize + idx] = static_cast<float>(pixel[2]) - mean_r;
		}
	}
}

// ========================================================================
// ✅ TEED_Inference - BGR 이미지로 TEED 추론
// ========================================================================
bool Fnc_Vision_Pre_FITO::TEED_Inference(const cv::Mat& inputBGR, cv::Mat& outputEdge, int timeoutMs)
{
	printf("[TEED] TEED_Inference called\n");
	fflush(stdout);

	if (!m_bTeedConnected)
	{
		printf("[TEED] Not connected, trying to connect...\n");
		fflush(stdout);
		// 자동 연결 시도
		if (!TEED_Connect())
		{
			printf("[TEED] Connection failed\n");
			fflush(stdout);
			return false;
		}
	}

	if (inputBGR.empty() || inputBGR.channels() != 3)
	{
		printf("[TEED] Invalid input: empty=%d, channels=%d\n",
			inputBGR.empty(), inputBGR.empty() ? 0 : inputBGR.channels());
		fflush(stdout);
		return false;
	}

	printf("[TEED] Input image: %dx%d\n", inputBGR.cols, inputBGR.rows);
	fflush(stdout);

	// 전처리
	int outHeight, outWidth;
	std::vector<float> tensor(3 * m_nTeedHeight * m_nTeedWidth);
	TEED_Preprocess(inputBGR, tensor.data(), outHeight, outWidth);

	printf("[TEED] After preprocess: %dx%d\n", outWidth, outHeight);
	fflush(stdout);

	// 크기 확인 (연결된 공유 메모리 크기와 맞는지)
	if (outHeight != m_nTeedHeight || outWidth != m_nTeedWidth)
	{
		printf("[TEED] Size mismatch, reconnecting: %dx%d vs %dx%d\n",
			outWidth, outHeight, m_nTeedWidth, m_nTeedHeight);
		fflush(stdout);
		// 크기가 다르면 재연결
		TEED_Disconnect();
		if (!TEED_Connect(outHeight, outWidth))
			return false;
	}

	// 입력 공유 메모리에 쓰기
	// Header: height(4) + width(4)
	int* header = reinterpret_cast<int*>(m_pTeedInputBuffer);
	header[0] = outHeight;
	header[1] = outWidth;

	// Data: float32 tensor (CHW)
	memcpy(m_pTeedInputBuffer + TEED_HEADER_SIZE, tensor.data(),
	       3 * outHeight * outWidth * sizeof(float));

	printf("[TEED] Wrote to shared memory: header=(%d,%d), waiting for Python...\n",
		header[0], header[1]);
	fflush(stdout);

	// Python 서버가 처리할 때까지 대기 (폴링)
	// Python이 처리 완료 후 입력 헤더를 (0, 0)으로 초기화함
	int elapsed = 0;
	const int pollInterval = 10;  // 10ms

	while (elapsed < timeoutMs)
	{
		Sleep(pollInterval);
		elapsed += pollInterval;

		// 입력 헤더 확인 (처리 완료 시 0, 0)
		if (header[0] == 0 && header[1] == 0)
		{
			printf("[TEED] Python processing complete!\n");
			fflush(stdout);
			// 출력 읽기
			int* outHeader = reinterpret_cast<int*>(m_pTeedOutputBuffer);
			int resultHeight = outHeader[0];
			int resultWidth = outHeader[1];

			printf("[TEED] Output: %dx%d\n", resultWidth, resultHeight);
			fflush(stdout);

			if (resultHeight > 0 && resultWidth > 0)
			{
				// Edge map 복사
				outputEdge = cv::Mat(resultHeight, resultWidth, CV_8UC1);
				memcpy(outputEdge.data,
				       m_pTeedOutputBuffer + TEED_HEADER_SIZE,
				       resultHeight * resultWidth);

				printf("[TEED] Inference successful!\n");
				fflush(stdout);
				return true;
			}

			printf("[TEED] Invalid output dimensions\n");
			fflush(stdout);
			return false;
		}

		// 1초마다 상태 출력
		if (elapsed % 1000 == 0)
		{
			printf("[TEED] Still waiting... (%d ms, header=%d,%d)\n",
				elapsed, header[0], header[1]);
			fflush(stdout);
		}
	}

	// 타임아웃
	printf("[TEED] Timeout after %d ms\n", timeoutMs);
	fflush(stdout);
	return false;
}

// ========================================================================
// ✅ TEED_InferenceFromGray - Grayscale 버퍼로 TEED 추론
// ========================================================================
bool Fnc_Vision_Pre_FITO::TEED_InferenceFromGray(const BYTE* pSrc, int nWidth, int nHeight,
                                                  cv::Mat& outputEdge, int timeoutMs)
{
	if (pSrc == nullptr || nWidth <= 0 || nHeight <= 0)
		return false;

	// Grayscale -> BGR 변환
	cv::Mat gray(nHeight, nWidth, CV_8UC1, const_cast<BYTE*>(pSrc));
	cv::Mat bgr;
	cv::cvtColor(gray, bgr, cv::COLOR_GRAY2BGR);

	return TEED_Inference(bgr, outputEdge, timeoutMs);
}

// ========================================================================
// ✅ TEED_ExtractLines - Edge map에서 수평/수직 직선 추출
// ========================================================================
bool Fnc_Vision_Pre_FITO::TEED_ExtractLines(const cv::Mat& edgeMap,
                                             std::vector<TEEDLineInfo>& hLines,
                                             std::vector<TEEDLineInfo>& vLines,
                                             int threshold, int minLength,
                                             int maxGap, float angleTolerance)
{
	if (edgeMap.empty() || edgeMap.type() != CV_8UC1)
	{
		printf("[TEED] ExtractLines: Invalid edge map\n");
		return false;
	}

	hLines.clear();
	vLines.clear();

	// 이진화
	cv::Mat binary;
	cv::threshold(edgeMap, binary, threshold, 255, cv::THRESH_BINARY);

	// HoughLinesP
	std::vector<cv::Vec4i> lines;
	cv::HoughLinesP(binary, lines, 1, CV_PI / 180.0, 30, minLength, maxGap);

	printf("[TEED] ExtractLines: Found %zu raw lines\n", lines.size());

	for (const auto& line : lines)
	{
		int x1 = line[0], y1 = line[1];
		int x2 = line[2], y2 = line[3];

		float length = std::sqrt(static_cast<float>((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)));
		float angle = std::atan2(static_cast<float>(y2 - y1), static_cast<float>(x2 - x1)) * 180.0f / static_cast<float>(CV_PI);

		TEEDLineInfo info;
		info.length = length;

		// 수평선: -angleTolerance ~ +angleTolerance
		if (std::abs(angle) <= angleTolerance)
		{
			if (x2 != x1)
			{
				info.coef1 = static_cast<float>(y2 - y1) / static_cast<float>(x2 - x1);  // a
				info.coef2 = static_cast<float>(y1) - info.coef1 * static_cast<float>(x1);  // b
			}
			else
			{
				info.coef1 = 0.0f;
				info.coef2 = static_cast<float>(y1 + y2) / 2.0f;
			}
			hLines.push_back(info);
		}
		// 수직선: 90-angleTolerance ~ 90+angleTolerance (또는 -90)
		else if (std::abs(angle) >= (90.0f - angleTolerance) && std::abs(angle) <= (90.0f + angleTolerance))
		{
			if (y2 != y1)
			{
				info.coef1 = static_cast<float>(x2 - x1) / static_cast<float>(y2 - y1);  // c
				info.coef2 = static_cast<float>(x1) - info.coef1 * static_cast<float>(y1);  // d
			}
			else
			{
				info.coef1 = 0.0f;
				info.coef2 = static_cast<float>(x1 + x2) / 2.0f;
			}
			vLines.push_back(info);
		}
	}

	// Python과 동일하게 길이순 정렬 (클러스터링 전 중요!)
	std::sort(hLines.begin(), hLines.end(),
	          [](const TEEDLineInfo& a, const TEEDLineInfo& b) {
	              return a.length > b.length;
	          });
	std::sort(vLines.begin(), vLines.end(),
	          [](const TEEDLineInfo& a, const TEEDLineInfo& b) {
	              return a.length > b.length;
	          });

	printf("[TEED] ExtractLines: H=%zu, V=%zu\n", hLines.size(), vLines.size());
	return true;
}

// ========================================================================
// ✅ TEED_ClusterLines - 직선들을 절편 기준으로 클러스터링
// ========================================================================
void Fnc_Vision_Pre_FITO::TEED_ClusterLines(const std::vector<TEEDLineInfo>& lines,
                                             std::vector<TEEDLineCluster>& clusters,
                                             float distThreshold)
{
	clusters.clear();

	if (lines.empty())
		return;

	for (const auto& line : lines)
	{
		bool merged = false;

		for (auto& cluster : clusters)
		{
			// 절편(coef2) 차이로 클러스터링
			if (std::abs(cluster.coef2 - line.coef2) < distThreshold)
			{
				// 길이 가중 평균으로 계수 업데이트 (Python과 동일한 로직)
				float oldTotal = cluster.totalLength;
				cluster.totalLength += line.length;
				cluster.coef1 = (cluster.coef1 * oldTotal + line.coef1 * line.length) / cluster.totalLength;
				cluster.coef2 = (cluster.coef2 * oldTotal + line.coef2 * line.length) / cluster.totalLength;
				merged = true;
				break;
			}
		}

		if (!merged)
		{
			TEEDLineCluster newCluster;
			newCluster.coef1 = line.coef1;
			newCluster.coef2 = line.coef2;
			newCluster.totalLength = line.length;
			clusters.push_back(newCluster);
		}
	}

	// 크기순 정렬 (totalLength 기준)
	std::sort(clusters.begin(), clusters.end(),
	          [](const TEEDLineCluster& a, const TEEDLineCluster& b) {
	              return a.totalLength > b.totalLength;
	          });

	printf("[TEED] ClusterLines: %zu lines -> %zu clusters\n", lines.size(), clusters.size());
}
