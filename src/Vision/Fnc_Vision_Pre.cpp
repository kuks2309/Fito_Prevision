#include "Fnc_Vision_Pre.h"
#include "../AllInclude.h"
#include <atlimage.h>
#include <windows.h>
#include <tlhelp32.h>
#include "../Crevis/VirtualFG40CL.h"
#include "../DataDefine/GlobalVariable.h"

#pragma comment(lib, "VirtualFG40CL.lib")
IMPLEMENT_RUNTIMECLASS(Fnc_Vision_Pre)

using namespace tinyxml2;

Fnc_Vision_Pre::Fnc_Vision_Pre()
{
	m_hDevice = 0;
	m_Channel = 0;

	pCamImg = nullptr;
	pOriginalImg = nullptr;
	pViewImg = nullptr;

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
	// 	m_hv_PrevisionModelID[0] = -1;//X, Y  ch
	// 	m_hv_PrevisionModelID[1] = -1;//X, Y  ch
	// 	m_hv_PrevisionModelID[2] = -1;//X, Y  ch
	// 	m_hv_PrevisionModelID[3] = -1;//X, Y  ch


	m_hGuiProcess = NULL;
	
	processIdOld = 0;
	m_nIsInitialize = 0;

	
	CreateCS();
	::InitializeCriticalSection(&cs_Grab);
}
Fnc_Vision_Pre::~Fnc_Vision_Pre()
{
	DeleteCS();
	::DeleteCriticalSection(&cs_Grab);
	::CloseHandle(m_hProssed);
	::CloseHandle(m_hGuiProcess);
}
bool Fnc_Vision_Pre::RegistCommand() {
	// Default Value
	ADDVALUEACTION("GetResult", enumReturnType::Int, "", COMMAND_FUNCTION(Fnc_Vision_Pre::Act_GetResult)); // 모든 Class는 해당 기능을 반드시 추가한다. ( Return 타입은 Int로 )
	ADDVALUEACTION("SetResult", enumReturnType::Int, "", COMMAND_FUNCTION(Fnc_Vision_Pre::Act_SetResult)); // 모든 Class는 해당 기능을 반드시 추가한다. ( Return 타입은 Int로 )
	ADDVALUEACTION("ResetResult", enumReturnType::Int, "", COMMAND_FUNCTION(Fnc_Vision_Pre::Act_ResetResult)); // 모든 Class는 해당 기능을 반드시 추가한다. ( Return 타입은 Int로 )

	// Action Command
	// 반드시 해당 규칙으로 추가할것.
	ADDACTION("Initial", enumReturnType::None, "", COMMAND_FUNCTION(Fnc_Vision_Pre::Act_Initial));
	ADDACTION("ModelChange", enumReturnType::None, "", COMMAND_FUNCTION(Fnc_Vision_Pre::Act_ModelChange));
	ADDACTION("Grab", enumReturnType::None, "", COMMAND_FUNCTION(Fnc_Vision_Pre::Act_Grab));
	ADDACTION("GrabContinous", enumReturnType::None, "", COMMAND_FUNCTION(Fnc_Vision_Pre::Act_GrabContinous));

	ADDACTION("FindPreVision", enumReturnType::None, "", COMMAND_FUNCTION(Fnc_Vision_Pre::Act_FindPreVision));

	// Value Command
	ADDVALUEACTION("StopGrabContinous", enumReturnType::Int, "", COMMAND_FUNCTION(Fnc_Vision_Pre::Act_StopGrabContinous));
	ADDVALUEACTION("TestResultClear", enumReturnType::Int, "", COMMAND_FUNCTION(Fnc_Vision_Pre::Act_TestResultClear));
	//Non Blocking 용 Value Command

    return true;                                                                                                
}
bool Fnc_Vision_Pre::InitCode() {
	SetAlarmCategory("CUTTERAlarm");
	InitCommonFunction(); // 반드시 추가
	m_Unit = ProductStep::Loader; // loader 유닛에 종속
	UnitName = "Loader";
	SetSEMLogCallUnitName(UnitName);
    return true;
}
bool Fnc_Vision_Pre::LoadIOConfig(std::string &strIOInfo) { return true; }
bool Fnc_Vision_Pre::PreCode(std::string& strCommandName, std::string& strArgs, bool bRetry, void* pLogPath) { GetSEMLogData(pLogPath);  return true; }
void Fnc_Vision_Pre::PostCode(std::string &strCommandName, std::string &strArgs, enumFunctionResult &eResult) {}
void Fnc_Vision_Pre::FunctionAbort() {
    WriteLogI("Function Aborting");
    WriteLogI("Function Aborted");
}
void WINAPI GlobalCallbackPre(PMCSIGNALINFO SigInfo)
{
	if (SigInfo && SigInfo->Context)
	{
		Fnc_Vision_Pre* pDoc = (Fnc_Vision_Pre*)SigInfo->Context;

		pDoc->Callback(SigInfo);
	}
}
void Fnc_Vision_Pre::Callback(PMCSIGNALINFO SigInfo)
{
	MCHANDLE hSurface = NULL;
	switch (SigInfo->Signal)
	{
	case MC_SIG_SURFACE_FILLED:
	case MC_SIG_SURFACE_PROCESSING:
		McGetParamPtr(SigInfo->SignalInfo, MC_SurfaceAddr, &m_pCurrent);

		hSurface = (MCHANDLE)SigInfo->SignalInfo;

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
bool Fnc_Vision_Pre::Grab()
{
	if (m_bImageMode)
	{
		
		HV_ImgProcess_Prepare2(pCamImg, pGainImg, pBinImg);

		if (VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionViewMode, 0) == 0) // Raw Image
		{
			memcpy(pViewImg, pCamImg, m_nSizeX * m_nSizeY);
		}
		else if (VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionViewMode, 0) == 1) // Gain Image
		{
			memcpy(pViewImg, pGainImg, m_nSizeX * m_nSizeY);

		}
		else if (VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionViewMode, 0) == 2) // Mopology Image
		{
			memcpy(pViewImg, pBinImg, m_nSizeX * m_nSizeY);
		}

		Paint();

		return true;
	}

	if (!m_bCameraOpen) return false;

	ResetEvent(m_hProssed);

	McSetParamInt(m_Channel, MC_ForceTrig, MC_ForceTrig_TRIG);

	DWORD dwRet;
	dwRet = WaitForSingleObject(m_hProssed, 1000);
	if (dwRet == WAIT_TIMEOUT)
	{
		ResetEvent(m_hProssed);
		return FAIL;
	}

	ResetEvent(m_hProssed);

	if (m_nRotate)
	{
		SwapCoordinate(pOriginalImg, pCamImg, m_nRealSizeX, m_nRealSizeY, (double)m_nRotateDegree);
	}
	else
	{
		memcpy(pCamImg, pOriginalImg, m_nSizeX * m_nSizeY);
	}

	RECT fullRect;
	fullRect.left = 0;
	fullRect.top = 0;
	fullRect.right = m_nSizeX - 1;
	fullRect.bottom = m_nSizeY - 1;

	HV_ImgProcess_Prepare2(pCamImg, pGainImg, pBinImg);
	if (VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionViewMode, 0) == 0) // Raw Image
	{
		memcpy(pViewImg, pCamImg, m_nSizeX * m_nSizeY);
	}
	else if (VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionViewMode, 0) == 1) // Gain Image
	{
		memcpy(pViewImg, pGainImg, m_nSizeX * m_nSizeY);
	}
	else if (VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionViewMode, 0) == 2) // Mopology Image
	{
		memcpy(pViewImg, pBinImg, m_nSizeX * m_nSizeY);
	}

	return true;
}
void Fnc_Vision_Pre::Paint(bool GrabContinue /* false*/)
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
			std::wstring processName = UI_LAUNCHER_CTRL_W(); // 프로세스 이름
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
void Fnc_Vision_Pre::SetImageMode(bool bMode)
{
	EnterCriticalSection(&cs_Grab);

	m_bImageMode = bMode;

	LeaveCriticalSection(&cs_Grab);
}
void Fnc_Vision_Pre::ImageOpen(std::string strFile)
{
	EnterCriticalSection(&cs_Grab);
	HObject ho_Image;
	ReadImage(&ho_Image, strFile.c_str());
	ZoomImageSize(ho_Image, &ho_Image, m_nSizeX, m_nSizeY, "constant");
	HTuple hv_ptr, hv_typ, hv_w, hv_h;
	GetImagePointer1(ho_Image, &hv_ptr, &hv_typ, &hv_w, &hv_h);
	memcpy(pOriginalImg, (BYTE*)hv_ptr.L(), m_nSizeX * m_nSizeY);
	memcpy(pCamImg, (BYTE*)hv_ptr.L(), m_nSizeX * m_nSizeY);
	memcpy(pViewImg, (BYTE*)hv_ptr.L(), m_nSizeX * m_nSizeY);
	ho_Image.Clear();
	m_bImageMode = true;
	Grab();
	LeaveCriticalSection(&cs_Grab);
}
void Fnc_Vision_Pre::HV_PreVisionCreateTeamplate(int nCorner)
{
	HObject ho_RegionLinesH, ho_RegionLinesV, ho_RegionBase;
	HObject ho_BinImage, ho_Lines;
	HTuple hv_nSize = 300;
	
	GenImageConst(&ho_BinImage, "byte", 512, 512);

	if (nCorner == _RU)
	{
		GenRectangle1(&ho_RegionLinesH, 0, 0, 0, hv_nSize);
		GenRectangle1(&ho_RegionLinesV, 0, hv_nSize, hv_nSize, hv_nSize);

	}
	else if (nCorner == _RL)
	{
		GenRectangle1(&ho_RegionLinesH, hv_nSize, 0, hv_nSize, hv_nSize);
		GenRectangle1(&ho_RegionLinesV, 0, hv_nSize, hv_nSize, hv_nSize);
	
	}
	else if (nCorner == _LU)
	{
		GenRectangle1(&ho_RegionLinesH, 0, 0, 0, hv_nSize);
		GenRectangle1(&ho_RegionLinesV, 0, 0, hv_nSize, 0);
	
	}
	else if (nCorner == _LL)
	{
		GenRectangle1(&ho_RegionLinesH, hv_nSize, 0, hv_nSize, hv_nSize);
		GenRectangle1(&ho_RegionLinesV, 0, 0, hv_nSize, 0);
		
	}
	
	Union2(ho_RegionLinesH, ho_RegionLinesV, &ho_RegionBase);
	RegionToBin(ho_RegionBase, &ho_BinImage, 255, 0, 512, 512);
	LinesGauss(ho_BinImage, &ho_Lines, 1.5, 3, 8, "light", "true", "bar-shaped", "true");
	CreateShapeModelXld(ho_Lines, "auto", -0.39, 0.79, "auto", "auto", "ignore_local_polarity", 20, &m_hv_ModelID[nCorner]);
	ho_RegionLinesH.Clear();
	ho_RegionLinesV.Clear();
	ho_RegionBase.Clear();
	ho_BinImage.Clear();
	ho_Lines.Clear();
}
VisionResult Fnc_Vision_Pre::HV_PreVisionFindCornerTeamplate(int nCorner)
{
	VisionResult stResult;
	stResult.ResetVariable(m_nVision);
	int nStep = GetPointer(Mnt_Data, "Mnt_Data")->GetPreVisionChannel() / 2;

	_FPOINT dPoint;
	_FPOINT dPointRet;
	//SavePreImage(pCamImg, m_nSizeX, m_nSizeY, 0, nCorner);

	if (parAi2D(UN_PREVISION, NS_VISION::nOpenValueX, nStep, 0) < 1)
	{
		parAi2D_set(UN_PREVISION, NS_VISION::nOpenValueX, nStep, 0, 1);
	}
	if (parAi2D(UN_PREVISION, NS_VISION::nOpenValueY, nStep, 0) < 1)
	{
		parAi2D_set(UN_PREVISION, NS_VISION::nOpenValueY, nStep, 0, 1);
	}

	double dGain = parAd2D(UN_PREVISION, NS_VISION::dGainValue, nStep, 0);
	int nOffset = parAi2D(UN_PREVISION, NS_VISION::nOffsetValue, nStep, 0);

	int nBinaryThreshold = parAi2D(UN_PREVISION, NS_VISION::nBinaryLevel, nStep, 0);
	int nopenx = parAi2D(UN_PREVISION, NS_VISION::nOpenValueX, nStep, 0);
	int nopeny = parAi2D(UN_PREVISION, NS_VISION::nOpenValueY, nStep, 0);


	HObject  Ho_Processimage, Ho_OrgImage;
	HObject  ho_RectangleRow, ho_RectangleCol, ho_RegionBase;
	HObject  ho_ModelContours, ho_ContoursAffineTrans;
	HObject  ho_Cross1, ho_RegionCross, ho_RegionIntersection1;
	HObject  ho_RegionLinesRow, ho_RegionIntersection2, ho_RegionLinesCol;
	HObject  ho_Lines1, ho_Region4, ho_RegionUnion2, ho_RegionMoved;
	HObject  ho_RegionIntersection, ho_RegionMovedRow, ho_RegionMovedCol;
	HObject  ho_Contours, ho_RegionLinesTapeRow, ho_RegionLinesTapeCol;
	HObject  ho_Image, ho_InspectionROI, ho_DomainImage;;
	
	// Local control variables
	HTuple  hv_Files, hv_Length1, hv_Index1, hv_Length;
	HTuple  hv_Substring, hv_Substring1, hv_Width, hv_Height;
	HTuple  hv_nSize, hv_Type, hv_AX, hv_AY, hv_StartX;
	HTuple  hv_EndX, hv_StepX, hv_StartY, hv_EndY, hv_StepY;
	HTuple  hv_ModelID, hv_Row, hv_Column, hv_Angle, hv_Score;
	HTuple  hv_HomMat2DIdentity, hv_HomMat2DTranslate, hv_HomMat2DRotate;
	HTuple  hv_Qx, hv_Qy, hv_Area1, hv_Row_Row, hv_Column_Row;
	HTuple  hv_Area2, hv_Row_Col, hv_Column_Col, hv_MaxVal, hv_MaxVal2;
	HTuple  hv_maxIndex, hv_IndexX, hv_Area, hv_Row1, hv_Column1;
	HTuple  hv_maxIndex2, hv_IndexY, hv_RowBegin, hv_ColBegin;
	HTuple  hv_RowEnd, hv_ColEnd, hv_Nr, hv_Nc, hv_Dist, hv_dInter;
	HTuple  hv_dSlope, hv_dRowStartR, hv_dRowEndR, hv_dColumnStartR;
	HTuple  hv_dColumnEndR, hv_dRowStartC, hv_dRowEndC, hv_dColumnStartC;
	HTuple  hv_dColumnEndC, hv_Result_Row, hv_Result_Col, hv_IsOverlapping;
	HTuple  hv_error, hv_maxIntersection;
	int nTabpeInspection = pari(UN_LOADER, NS_LOADER::bUseTapeInspection);
	GenImage1Extern(&Ho_OrgImage, "byte", m_nSizeX, m_nSizeY, (Hlong)pCamImg, (Hlong)NULL);




	CopyImage(Ho_OrgImage, &Ho_Processimage);

	GrayOpeningRect(Ho_Processimage, &Ho_Processimage, 11, 11);
	GrayClosingRect(Ho_Processimage, &Ho_Processimage, 11, 11);


	GetImageSize(Ho_Processimage, &hv_Width, &hv_Height);
	hv_nSize = 300;
	
	hv_AX = hv_nSize / 2;
	hv_AY = hv_nSize / 2;

	hv_maxIntersection = VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionTapeInspectionMaxIntersection, 0);//30;
	int iStartGap = VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionTapeInspectionStartGap, 0);//10;
	int iSrartEndLimite = VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionTapeInspectionSrartEndLimite, 0);//200;
	int iCropSize = VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionTapeInspectionCropSize, 0);//200;
	double dSigma = VARD(UN_SYSTEM, NS_VARIABLE::dPreVisionTapeInspectionSigma,0);//1.2;
	int iLow = VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionTapeInspectionLow, 0);//1;
	int iHigh = VARI(UN_SYSTEM, NS_VARIABLE::nPreVisionTapeInspectionHigh, 0);//3;
	double dMoveStep = VARD(UN_SYSTEM, NS_VARIABLE::dPreVisionTapeInspectionMoveStep, 0);//0.5;

	int  nROISize = VARI(UN_SYSTEM, NS_VARIABLE::nPreInspectionRegionSize, 0);
	GenRectangle1(&ho_InspectionROI, (m_nSizeY / 2) - nROISize, (m_nSizeX / 2) - nROISize, (m_nSizeY / 2) + nROISize, (m_nSizeX / 2) + nROISize);
	ReduceDomain(Ho_Processimage, ho_InspectionROI, &ho_DomainImage);
	if (nCorner == _RU)
	{

	//	WriteImage(Ho_OrgImage, "bmp", 0, "D:\\Pre_2.bmp");
		GenRectangle1(&ho_RectangleRow, hv_Height - 1, 0, hv_Height - 1, hv_Width);
		GenRectangle1(&ho_RectangleCol, 0, 0, hv_Height, 0);

		hv_AX = hv_nSize / 2;
		hv_AY = (-hv_nSize) / 2;

		hv_StartX = iStartGap;
		hv_EndX = iSrartEndLimite;
		hv_StepX = dMoveStep;

		hv_StartY = -iStartGap;
		hv_EndY = -iSrartEndLimite;
		hv_StepY = -dMoveStep;
	}
	else if (nCorner == _RL)
	{
		GenRectangle1(&ho_RectangleRow, 0, 0, 0, hv_Width - 1);
		GenRectangle1(&ho_RectangleCol, 0, 0, hv_Height - 1, 0);
	//	WriteImage(Ho_OrgImage, "bmp", 0, "D:\\Pre_3.bmp");


		hv_AX = hv_nSize / 2;
		hv_AY = hv_nSize / 2;


		hv_StartX = iStartGap;
		hv_EndX = iSrartEndLimite;
		hv_StepX = dMoveStep;

		hv_StartY = iStartGap;
		hv_EndY = iSrartEndLimite;
		hv_StepY = dMoveStep;
	}
	else if (nCorner == _LU)
	{
		GenRectangle1(&ho_RectangleRow, hv_Height - 1, 0, hv_Height - 1, hv_Width);
		GenRectangle1(&ho_RectangleCol, 0, hv_Width - 1, hv_Height, hv_Width - 1);
	//	WriteImage(Ho_OrgImage, "bmp", 0, "D:\\Pre_0.bmp");
		hv_AX = (-hv_nSize) / 2;
		hv_AY = (-hv_nSize) / 2;

		hv_StartX = -iStartGap;
		hv_EndX = -iSrartEndLimite;
		hv_StepX = -dMoveStep;

		hv_StartY = -iStartGap;
		hv_EndY = -iSrartEndLimite;
		hv_StepY = -dMoveStep;
	}
	else if (nCorner == _LL)
	{
		GenRectangle1(&ho_RectangleRow, 0, 0, 0, hv_Width);
		GenRectangle1(&ho_RectangleCol, 0, hv_Width - 1, hv_Height, hv_Width - 1);
	//	WriteImage(Ho_OrgImage, "bmp", 0, "D:\\Pre_1.bmp");
		hv_AX = (-hv_nSize) / 2;
		hv_AY = hv_nSize / 2;


		hv_StartX = -iStartGap;
		hv_EndX = -iSrartEndLimite;
		hv_StepX = -dMoveStep;

		hv_StartY = iStartGap;
		hv_EndY = iSrartEndLimite;
		hv_StepY = dMoveStep;
	}

	try
	{
		

	//	WriteImage(Ho_OrgImage, "bmp", 0, "D:\\Ho_Processimage.bmp");
	//	WriteShapeModel(m_hv_ModelID[nCorner], "D:\\m_hv_ModelID.shm");
		double MatchingRate = VARD(UN_SYSTEM, NS_VARIABLE::dMatchingRate, 0) / 100;
		FindShapeModel(ho_DomainImage, m_hv_ModelID[nCorner], -0.39, 0.79, 0.1, 1, 0.5, "least_squares", 0, 0.9, &hv_Row, &hv_Column, &hv_Angle, &hv_Score);
		
		if (hv_Score< MatchingRate)
		{
			stImage->nObjectCount = 1;
			stImage->stObject[0].nType = 5;
			stImage->stObject[0].nColorCode = 1;
			sprintf_s(stImage->stObject[0].chText, "%s,  Score %0.3lf: ", "Align false", hv_Score.D()*100.0);
			Write_SEMLog_Event(StringFormat("Fnc_Vision_Pre_Align_Result [%d] : ", nCorner), stImage->stObject[0].chText);
			stImage->stObject[0].nTextX = 100;
			stImage->stObject[0].nTextY = 100;
			Ho_OrgImage.Clear();
			Ho_Processimage.Clear();
			ho_RectangleRow.Clear();
			ho_RectangleCol.Clear();
			ho_ModelContours.Clear();
			ho_Cross1.Clear();
			ho_RegionCross.Clear();
			ho_RegionIntersection1.Clear();
			ho_RegionLinesRow.Clear();
			ho_RegionIntersection2.Clear();
			ho_RegionLinesCol.Clear();
			ho_Lines1.Clear();
			ho_Region4.Clear();
			ho_RegionUnion2.Clear();
			ho_RegionMoved.Clear();
			ho_RegionIntersection.Clear();
			ho_RegionMovedRow.Clear();
			ho_RegionMovedCol.Clear();
			ho_Contours.Clear();
			ho_RegionLinesTapeCol.Clear();
			ho_RegionLinesTapeRow.Clear();
			ho_InspectionROI.Clear();
			ho_DomainImage.Clear();
			ho_Image.Clear();
			return stResult;
		}
		
		GetShapeModelContours(&ho_ModelContours, m_hv_ModelID[nCorner], 1);
		
		HomMat2dIdentity(&hv_HomMat2DIdentity);
		HomMat2dTranslate(hv_HomMat2DIdentity, hv_AY, hv_AX, &hv_HomMat2DTranslate);
		HomMat2dRotate(hv_HomMat2DTranslate, hv_Angle, hv_Row, hv_Column, &hv_HomMat2DRotate);
		AffineTransPoint2d(hv_HomMat2DRotate, hv_Row, hv_Column, &hv_Result_Row, &hv_Result_Col);
		GenCrossContourXld(&ho_Cross1, hv_Result_Row, hv_Result_Col, 8000, hv_Angle);

		HTuple hv_bInside;
		TestRegionPoint(ho_InspectionROI, hv_Result_Row, hv_Result_Col, &hv_bInside);
		if (hv_bInside.L()==0)
		{
			stImage->nObjectCount = 1;
			stImage->stObject[0].nType = 1;
			stImage->stObject[0].nColorCode = 1;
			stImage->stObject[0].nLineWidth = 2;
			stImage->stObject[0].nCrossX = hv_Result_Col.D();
			stImage->stObject[0].nCrossY = hv_Result_Row.D();
			stImage->stObject[0].nCrossSize = 100;



			stImage->nObjectCount = 2;
			stImage->stObject[1].nColorCode = 1;
			stImage->stObject[1].nType = 5;
			sprintf_s(stImage->stObject[1].chText, "%s, Score : %0.3lf", "Align false", hv_Score.D() * 100);
			Write_SEMLog_Event(StringFormat("Fnc_Vision_Pre_Align_Result [%d] : ", nCorner), stImage->stObject[1].chText);
			stImage->stObject[1].nTextX = 100;
			stImage->stObject[1].nTextY = 100;
			Ho_OrgImage.Clear();
			Ho_Processimage.Clear();
			ho_RectangleRow.Clear();
			ho_RectangleCol.Clear();
			ho_ModelContours.Clear();
			ho_Cross1.Clear();
			ho_RegionCross.Clear();
			ho_RegionIntersection1.Clear();
			ho_RegionLinesRow.Clear();
			ho_RegionIntersection2.Clear();
			ho_RegionLinesCol.Clear();
			ho_Lines1.Clear();
			ho_Region4.Clear();
			ho_RegionUnion2.Clear();
			ho_RegionMoved.Clear();
			ho_RegionIntersection.Clear();
			ho_RegionMovedRow.Clear();
			ho_RegionMovedCol.Clear();
			ho_Contours.Clear();
			ho_RegionLinesTapeCol.Clear();
			ho_RegionLinesTapeRow.Clear();
			ho_InspectionROI.Clear();
			ho_DomainImage.Clear();
			ho_Image.Clear();
			return stResult;
			
		}



		if (nTabpeInspection)
		{
			GenRegionContourXld(ho_Cross1, &ho_RegionCross, "margin");
			
			Intersection(ho_RectangleRow, ho_RegionCross, &ho_RegionIntersection1);
			AreaCenter(ho_RegionIntersection1, &hv_Area1, &hv_Row_Row, &hv_Column_Row);
		//	WriteRegion(ho_RectangleRow, "D:\\ho_RectangleRow.reg");
		//	WriteRegion(ho_RegionCross, "D:\\ho_RegionCross.reg");

			GenRegionLine(&ho_RegionLinesRow, hv_Row_Row, hv_Column_Row, hv_Result_Row, hv_Result_Col);

			Intersection(ho_RectangleCol, ho_RegionCross, &ho_RegionIntersection2);
			AreaCenter(ho_RegionIntersection2, &hv_Area2, &hv_Row_Col, &hv_Column_Col);
			GenRegionLine(&ho_RegionLinesCol, hv_Row_Col, hv_Column_Col, hv_Result_Row, hv_Result_Col);

			HObject rrect;
			GenRectangle1(&rrect, hv_Result_Row - iCropSize, hv_Result_Col - iCropSize, hv_Result_Row + iCropSize, hv_Result_Col + iCropSize);
			ReduceDomain(Ho_OrgImage, rrect, &ho_DomainImage);
		//	GrayClosingRect(ho_DomainImage, &ho_DomainImage, 1, 5);
			//LinesGauss(Ho_OrgImage, &ho_Lines1, 1.5, 1, 2, "light", "false", "bar-shaped", "true");
			LinesGauss(ho_DomainImage, &ho_Lines1, dSigma, iLow, iHigh, "light", "false", "bar-shaped", "false");
			//EdgesSubPix(ho_DomainImage, &ho_Lines1, "canny", 4, 3, 8);
			//EdgesSubPix(Ho_OrgImage, &ho_Lines1, "canny", 4, 7, 10);
			GenRegionContourXld(ho_Lines1, &ho_Region4, "margin");
			Union1(ho_Region4, &ho_RegionUnion2);

		//	WriteRegion(ho_RegionUnion2, "D:\\ho_RegionUnion2.reg");

			hv_MaxVal = 0;
			hv_maxIndex = 0;


			HTuple end_val147 = hv_EndX;
			HTuple step_val147 = hv_StepX;
			
			vector<DPOINT>vList1;


			HTuple LastIndex = -1;
			for (hv_IndexX = hv_StartX; hv_IndexX.Continue(end_val147, step_val147); hv_IndexX += step_val147)
			{
				MoveRegion(ho_RegionLinesRow, &ho_RegionMoved, 0, hv_IndexX);
				Intersection(ho_RegionMoved, ho_RegionUnion2, &ho_RegionIntersection);
				AreaCenter(ho_RegionIntersection, &hv_Area, &hv_Row1, &hv_Column1);

			
			
				if (0 != (hv_Area > hv_maxIntersection))
				{
					LastIndex = hv_IndexX;
					
				}


				if (0 != (hv_Area > hv_MaxVal))
				{
					hv_maxIndex = hv_IndexX;
					hv_MaxVal = hv_Area;
				}

			}
		
			if (LastIndex <0)
			{
				LastIndex = hv_maxIndex;
			}

			MoveRegion(ho_RegionLinesRow, &ho_RegionMovedRow, 0, LastIndex);

			hv_MaxVal2 = 0;
			hv_maxIndex2 = 0;
			LastIndex = -1;
			vector<DPOINT>vList2;

			HTuple end_val162 = hv_EndY;
			HTuple step_val162 = hv_StepY;
			for (hv_IndexY = hv_StartY; hv_IndexY.Continue(end_val162, step_val162); hv_IndexY += step_val162)
			{
				MoveRegion(ho_RegionLinesCol, &ho_RegionMoved, hv_IndexY, 0);
				Intersection(ho_RegionMoved, ho_RegionUnion2, &ho_RegionIntersection);
				AreaCenter(ho_RegionIntersection, &hv_Area, &hv_Row1, &hv_Column1);

			
					if (0 != (hv_Area > hv_maxIntersection))
					{
						LastIndex = hv_IndexY;

					}
				if (0 != (hv_Area > hv_MaxVal2))
				{
					hv_maxIndex2 = hv_IndexY;

					hv_MaxVal2 = hv_Area;
				}

			}
			if (LastIndex > 0)
			{
				LastIndex = hv_maxIndex2;
			}

			MoveRegion(ho_RegionLinesCol, &ho_RegionMovedCol, LastIndex, 0);


			//dev_display (RegionMoved3)

			GenContourRegionXld(ho_RegionMovedRow, &ho_Contours, "center");
			FitLineContourXld(ho_Contours, "tukey", -1, 0, 5, 2, &hv_RowBegin, &hv_ColBegin, &hv_RowEnd, &hv_ColEnd, &hv_Nr, &hv_Nc, &hv_Dist);
			if (hv_Nr==0.0)
			{
				hv_dInter = hv_Dist;
				hv_dSlope =0;
				hv_dRowStartR = 0;
				hv_dRowEndR = hv_Height;
				hv_dColumnStartR = hv_ColBegin;
				hv_dColumnEndR = hv_ColBegin;
			}
			else
			{
				hv_dInter = hv_Dist / hv_Nr;
				hv_dSlope = (-hv_Nc) / hv_Nr;
				hv_dRowStartR = 0;
				hv_dRowEndR = hv_Height;
				hv_dColumnStartR = (-hv_dInter) / hv_dSlope;
				hv_dColumnEndR = (hv_dRowEndR - hv_dInter) / hv_dSlope;
				
			}
			GenRegionLine(&ho_RegionLinesTapeRow, hv_dRowStartR, hv_dColumnStartR, hv_dRowEndR, hv_dColumnEndR);

			GenContourRegionXld(ho_RegionMovedCol, &ho_Contours, "center");
			FitLineContourXld(ho_Contours, "tukey", -1, 0, 5, 2, &hv_RowBegin, &hv_ColBegin, &hv_RowEnd, &hv_ColEnd, &hv_Nr, &hv_Nc, &hv_Dist);
			hv_dInter = hv_Dist / hv_Nr;
			hv_dSlope = (-hv_Nc) / hv_Nr;
			hv_dRowStartC = hv_dInter;
			hv_dRowEndC = (hv_dSlope * hv_Width) + hv_dInter;
			hv_dColumnStartC = 0;
			hv_dColumnEndC = hv_Width;
			GenRegionLine(&ho_RegionLinesTapeCol, hv_dRowStartC, hv_dColumnStartC, hv_dRowEndC, hv_dColumnEndC);


			IntersectionLines(hv_dRowStartR, hv_dColumnStartR, hv_dRowEndR, hv_dColumnEndR, hv_dRowStartC, hv_dColumnStartC, hv_dRowEndC, hv_dColumnEndC, &hv_Result_Row, &hv_Result_Col, &hv_IsOverlapping);
			
			
			stImage->nObjectCount = 1;
			stImage->stObject[0].nType = 3;
			stImage->stObject[0].nColorCode = 2;
			stImage->stObject[0].nLineWidth = 2;
			stImage->stObject[0].nStartX = hv_dColumnStartR.D();
			stImage->stObject[0].nStartY = hv_dRowStartR.D();
			stImage->stObject[0].nEndX = hv_dColumnEndR.D();
			stImage->stObject[0].nEndY = hv_dRowEndR.D();

			stImage->nObjectCount = 2;
			stImage->stObject[1].nType = 3;
			stImage->stObject[1].nColorCode = 1;
			stImage->stObject[1].nLineWidth = 2;
			stImage->stObject[1].nStartX = hv_dColumnStartC.D();
			stImage->stObject[1].nStartY = hv_dRowStartC.D();
			stImage->stObject[1].nEndX = hv_dColumnEndC.D();
			stImage->stObject[1].nEndY = hv_dRowEndC.D();



			stImage->nObjectCount = 3;
			stImage->stObject[2].nType = 1;
			stImage->stObject[2].nColorCode = 1;
			stImage->stObject[2].nLineWidth = 2;
			stImage->stObject[2].nCrossX = hv_Result_Col.D();
			stImage->stObject[2].nCrossY = hv_Result_Row.D();
			stImage->stObject[2].nCrossSize = 100;

	

			stImage->nObjectCount = 4;
			stImage->stObject[3].nType = 5;
			stImage->stObject[3].nColorCode = 1;
			sprintf_s(stImage->stObject[3].chText, "%s, Score : %0.3lf,  %d, %d", "Tape Align Ok ", hv_Score.D() * 100, hv_MaxVal.L(), hv_MaxVal2.L());
			Write_SEMLog_Event(StringFormat("Fnc_Vision_Pre_Align_Result [%d] : ", nCorner), stImage->stObject[3].chText);
			stImage->stObject[3].nTextX = 100;
			stImage->stObject[3].nTextY = 100;


			/*	stImage->nObjectCount = 1;
				stImage->stObject[0].nType = 1;
				stImage->stObject[0].nColorCode = 1;
				stImage->stObject[0].nLineWidth = 2;
				stImage->stObject[0].nCrossX = hv_Result_Col.D();
				stImage->stObject[0].nCrossY = hv_Result_Row.D();
				stImage->stObject[0].nCrossSize = 100;



				stImage->nObjectCount = 2;
				stImage->stObject[1].nColorCode = 1;
				stImage->stObject[1].nType = 5;
				sprintf_s(stImage->stObject[1].chText, "%s, Score : %0.3lf", "Align Ok", hv_Score.D() * 100);
				stImage->stObject[1].nTextX = 100;
				stImage->stObject[1].nTextY = 100;*/


		}
		else
		{

			stImage->nObjectCount = 1;
			stImage->stObject[0].nType = 1;
			stImage->stObject[0].nColorCode = 1;
			stImage->stObject[0].nLineWidth = 2;
			stImage->stObject[0].nCrossX = hv_Result_Col.D();
			stImage->stObject[0].nCrossY = hv_Result_Row.D();
			stImage->stObject[0].nCrossSize = 100;



			stImage->nObjectCount = 2;
			stImage->stObject[1].nColorCode = 1;
			stImage->stObject[1].nType = 5;
			sprintf_s(stImage->stObject[1].chText, "%s, Score : %0.3lf", "Bar Align Ok", hv_Score.D()*100);
			Write_SEMLog_Event(StringFormat("Fnc_Vision_Pre_Align_Result [%d] : ", nCorner), stImage->stObject[1].chText);
			stImage->stObject[1].nTextX = 100;
			stImage->stObject[1].nTextY = 100;

			}





	}
	catch (HException& e)
	{
		Ho_OrgImage.Clear();
		Ho_Processimage.Clear();
		ho_RectangleRow.Clear();
		ho_RectangleCol.Clear();
		ho_ModelContours.Clear();
		ho_Cross1.Clear();
		ho_RegionCross.Clear();
		ho_RegionIntersection1.Clear();
		ho_RegionLinesRow.Clear();
		ho_RegionIntersection2.Clear();
		ho_RegionLinesCol.Clear();
		ho_Lines1.Clear();
		ho_Region4.Clear();
		ho_RegionUnion2.Clear();
		ho_RegionMoved.Clear();
		ho_RegionIntersection.Clear();
		ho_RegionMovedRow.Clear();
		ho_RegionMovedCol.Clear();
		ho_Contours.Clear();
		ho_RegionLinesTapeCol.Clear();
		ho_RegionLinesTapeRow.Clear();
		ho_InspectionROI.Clear();
		ho_DomainImage.Clear();
		ho_Image.Clear();
	
		e.ToHTuple(&hv_error);
		//	pImgView->InsertString(100, 100, _T("Align false"), RGB_GREEN, RGB_GREEN, 20, TRUE);
// 		pImgView->InsertString(10, 10, _T("Align false"), RGB_RED, RGB_RED, 20, TRUE);

		stImage->nObjectCount = 1;
		stImage->stObject[0].nType = 5;
		stImage->stObject[0].nColorCode = 1;
		sprintf_s(stImage->stObject[0].chText, "%s", "error Align false");
		Write_SEMLog_Event(StringFormat("Fnc_Vision_Pre_Align_Result [%d] : ", nCorner), stImage->stObject[0].chText);
		stImage->stObject[0].nTextX = 100;
		stImage->stObject[0].nTextY = 100;

		return stResult;
	}
	Ho_OrgImage.Clear();
	Ho_Processimage.Clear();
	ho_RectangleRow.Clear();
	ho_RectangleCol.Clear();
	ho_ModelContours.Clear();
	ho_Cross1.Clear();
	ho_RegionCross.Clear();
	ho_RegionIntersection1.Clear();
	ho_RegionLinesRow.Clear();
	ho_RegionIntersection2.Clear();
	ho_RegionLinesCol.Clear();
	ho_Lines1.Clear();
	ho_Region4.Clear();
	ho_RegionUnion2.Clear();
	ho_RegionMoved.Clear();
	ho_RegionIntersection.Clear();
	ho_RegionMovedRow.Clear();
	ho_RegionMovedCol.Clear();
	ho_Contours.Clear();
	ho_RegionLinesTapeCol.Clear();
	ho_RegionLinesTapeRow.Clear();
	ho_InspectionROI.Clear();
	ho_DomainImage.Clear();
	ho_Image.Clear();

	//reduceImage.Clear();
	//subRect.Clear();

	HTuple  hv_len;
	TupleLength(hv_Result_Row, &hv_len);
	if (hv_len < 1)
	{
		// 		pImgView->InsertString(10, 10, _T("Align false"), RGB_RED, RGB_RED, 20, TRUE);
		stImage->nObjectCount = 1;
		stImage->stObject[0].nType = 5;
		sprintf_s(stImage->stObject[0].chText, "%s", "row error Align false");
		Write_SEMLog_Event(StringFormat("Fnc_Vision_Pre_Align_Result [%d] : ", nCorner), stImage->stObject[0].chText);
		stImage->stObject[0].nTextX = 100;
		stImage->stObject[0].nTextY = 100;

		return stResult;

	}

	dPoint.x = hv_Result_Col.D();
	dPoint.y = hv_Result_Row.D();

	dPointRet.x = dPoint.x - (m_nSizeX / 2);
	dPointRet.y = dPoint.y - (m_nSizeY / 2);


	stResult.bExist = true;
	stResult.bResult = true;
	stResult.dPositionX = dPointRet.x;
	stResult.dPositionY = dPointRet.y;

		
	

	// 		pImgView->InsertCross(float(dPoint.x), float(dPoint.y), 200, RGB_GREEN);
	// 		pImgView->InsertString(10, 10, _T("Align ok"), RGB_GREEN, RGB_GREEN, 20, TRUE);



	return stResult;
}
void Fnc_Vision_Pre::CreateFolder(CString strPath)
{
	CString Root;
	Root.Format(_T("%s"), strPath.GetString());
	CreateDirectory(Root, NULL);
}

void Fnc_Vision_Pre::SavePreImage(BYTE* Image, int Width, int Height, int nResult, int nSaveChannel)
{
	CString strPath;
	CString strTemp;
	strPath.Format(_T("D:\\RawData"));
	CreateFolder(strPath);

	strTemp.Format(_T("\\Image"));
	strPath += strTemp;

	CreateFolder(strPath);

	strTemp.Format(_T("\\PreVision"));
	strPath += strTemp;

	CreateFolder(strPath);

	HObject Ho_OrgImage;

	GenImage1Extern(&Ho_OrgImage, "byte", Width, Height, (Hlong)Image, (Hlong)NULL);

	SYSTEMTIME time_info;
	GetLocalTime(&time_info);
	CTime time_ff;
	time_ff = CTime::GetCurrentTime();

	CTime tTime;
	tTime = GetCurrTime();
	strTemp.Format(_T("\\%02d%02d_%02d%02d%02d%03d"), time_info.wMonth, time_info.wDay, time_info.wHour, time_info.wMinute, time_info.wSecond, time_info.wMilliseconds);
	strPath += strTemp;
	strTemp.Format(_T("_Pre_%d.bmp"), nSaveChannel);
	strPath += strTemp;

	HTuple hv_File(strPath);
	HTuple fillColor = 0;
	WriteImage(Ho_OrgImage, "bmp", 0, hv_File);
	Ho_OrgImage.Clear();
}
int Fnc_Vision_Pre::InitialBoard()
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
	chCannel =  pElement->FirstChildElement("CHANNEL")->GetText();
	chCamID = pElement->FirstChildElement("CAMID")->GetText();
	chCommChannel = pElement->FirstChildElement("COMM_CHANNEL")->GetText();
	nCommPort = pElement->FirstChildElement("COMM_PORT")->IntText();
	
	

	strRecipePath = strPath + StringFormat("\\Data\\CamFile\\%s", chCamFile);
	sprintf_s(cBuffer, "%s", strRecipePath.c_str());



	
	MCSTATUS status;

	status = McOpenDriver(NULL);

	Sleep(200);

	//	if (m_nVision < 4)
	//	{
	//		status = McSetParamStr(MC_BOARD + g_stVision.nBoardNo[m_nVision], MC_BoardTopology, "MONO_DECA");
	//	}

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
										   
	status = McRegisterCallback(m_Channel, GlobalCallbackPre, this);

	status = McSetParamInt(m_Channel, MC_ChannelState, MC_ChannelState_ACTIVE);

	m_bCameraOpen = TRUE;


	//m_nSizeX = m_nViewerSizeX; 
	//m_nSizeY = m_nViewerSizeY;

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


	HV_PreVisionCreateTeamplate(_RU);
	HV_PreVisionCreateTeamplate(_RL);
	HV_PreVisionCreateTeamplate(_LU);
	HV_PreVisionCreateTeamplate(_LL);


	return PASS;
}
void Fnc_Vision_Pre::InitialCrevisCam()
{
	// 	int status = MCAM_ERR_SUCCESS;
	// 
	// 	unsigned int portNum = 1;
	// 	unsigned int size = 256;
	// 
	// 	char szPortID[256] = { 0, };
	// 	char szDeviceID[256] = { 0, };
	// 	char szErrorDescription[256] = { 0, };
	// 
	// 	int nPortNo = -1;
	// 
	// 	int i = 0;
	// 
	// 	int nHasCommPort = 0;
	// 
	// 	int nLength = strlen(chCamID);
	// 
	// 	if (nLength < 1) return;
	// 
	// 	try
	// 	{
	// 		status = ST_GetPortIDNum(&portNum);
	// 		if (status != 0)
	// 		{
	// 			WriteFunctionLogE("Can Not Find Crevis Port ID Count [CAM3]");
	// 			throw status;
	// 		}
	// 
	// 		for (i = 0; i < portNum; i++)
	// 		{
	// 			size = 256;
	// 			status = ST_GetPortID(i, szPortID, &size);
	// 			if (status != 0)
	// 			{
	// 				WriteFunctionLogE("Can Not Get Port ID String [CAM3]");
	// 				throw status;
	// 			}
	// 
	// 			if (strstr(szPortID, chCamID) != NULL)
	// 			{
	// 				nPortNo = i;
	// 				break;
	// 			}
	// 		}
	// 
	// 		if (nPortNo < 0)
	// 		{
	// 			WriteFunctionLogE("Can Not Find COMM Port [CAM3]");
	// 			throw status;
	// 		}
	// 
	// 		status = ST_GetPortID(nPortNo, szPortID, &size);
	// 		if (status != 0)						throw status;
	// 
	// 		size = 256;
	// 		status = ST_GetDeviceID(0, szDeviceID, &size);
	// 		if (status != 0)						throw status;
	// 
	// 		status = ST_ConnectDevice(&m_hDevice, szPortID, szDeviceID);
	// 		if (status != 0)						throw status;
	// 	}
	// 	catch (int err)
	// 	{
	// 		return;
	// 	}
	// 
	// 	m_bCommConnect = true;
	// 
	// 	SetCameraGain(1);
	// 
	// 	GetPole();
}
bool Fnc_Vision_Pre::Saveparameter()
{
	return true;
}
bool Fnc_Vision_Pre::LoadParameter()
{
	//if (!HV_PreVisionCreateTemplateModle(_LU))
	//{
	//	parAi2D_set(UN_LCAM + m_nVision, NS_VISION::bHasPattern, _LU, 0, 0);
	//}
	//else
	//{
	//	parAi2D_set(UN_LCAM + m_nVision, NS_VISION::bHasPattern, _LU, 0, 1);
	//}

	//if (!HV_PreVisionCreateTemplateModle(_LL))
	//{
	//	parAi2D_set(UN_LCAM + m_nVision, NS_VISION::bHasPattern, _LL, 0, 0);
	//}
	//else
	//{
	//	parAi2D_set(UN_LCAM + m_nVision, NS_VISION::bHasPattern, _LL, 0, 1);
	//}

	//if (!HV_PreVisionCreateTemplateModle(_RU))
	//{
	//	parAi2D_set(UN_LCAM + m_nVision, NS_VISION::bHasPattern, _RU, 0, 0);
	//}
	//else
	//{
	//	parAi2D_set(UN_LCAM + m_nVision, NS_VISION::bHasPattern, _RU, 0, 1);
	//}

	//if (!HV_PreVisionCreateTemplateModle(_RL))
	//{
	//	parAi2D_set(UN_LCAM + m_nVision, NS_VISION::bHasPattern, _RL, 0, 0);
	//}
	//else
	//{
	//	parAi2D_set(UN_LCAM + m_nVision, NS_VISION::bHasPattern, _RL, 0, 1);
	//}

	g_pParameter->Save();

	return true;
}
enumFunctionResult Fnc_Vision_Pre::Act_Initial(std::string strArg, structFunctionResult* pstFunctionResult)
{
	WriteConsole_FunctionStart;
	char* cmdname = (char*)"Initial";
	SetReturnValue(cmdname, 0);

	m_hProssed = CreateEventA(NULL, TRUE, FALSE, NULL);
	ResetEvent(m_hProssed);

	if (!InitialBoard())
	{
		ReturnFailWithResult(cmdname);
	}

	LoadParameter();
	Sleep(10);
	ReturnSuccessWithResult(cmdname);
}
enumFunctionResult Fnc_Vision_Pre::Act_ModelChange(std::string strArg, structFunctionResult* pstFunctionResult)
{
	WriteConsole_FunctionStart;
	char* cmdname = (char*)"ModelChange";
	SetReturnValue(cmdname, 0);

	ReturnSuccessWithResult(cmdname);
}
enumFunctionResult Fnc_Vision_Pre::Act_Grab(std::string strArg, structFunctionResult* pstFunctionResult) {
	WriteConsole_FunctionStart;
	char* cmdname = (char*)"Grab";
	SetReturnValue(cmdname, 0);

	m_bImageMode = false;

	/*if (m_bImageMode)
	{
		Grab();

		ReturnSuccessWithResult(cmdname);
	}*/

	Grab();

	// Image Buffer 저장.
	Paint();

	ReturnSuccessWithResult(cmdname);
}
enumFunctionResult Fnc_Vision_Pre::Act_GrabContinous(std::string strArg, structFunctionResult* pstFunctionResult) 
{
	//WriteFunctionSEMLogStart;
	char* cmdname = (char*)"GrabContinous";
	SetReturnValue(cmdname, 0);
	if (ISRUNNING(Opr_Proc))
	{
		ReturnSuccessWithResult(cmdname);
	}
	m_bGrabContinous = true;
	m_bImageMode = false;
	while (1)
	{
		EnterCriticalSection(&cs_Grab);
		if (m_bImageMode /*|| !m_nUseCameraInitial*/)
		{
			if (!m_bGrabContinous) break;

			LeaveCriticalSection(&cs_Grab);

			//ImgCopy(&m_LoadedImage, &m_DestImage);

			//Paint();

			Sleep(30);

			continue;
		}

		if (!m_bGrabContinous) break;



		Grab();

		Paint(true);
		LeaveCriticalSection(&cs_Grab);
		Sleep(30);
	}
	LeaveCriticalSection(&cs_Grab);
	ReturnSuccessWithResult(cmdname);
}
enumFunctionResult Fnc_Vision_Pre::Act_StopGrabContinous(std::string strArg, structFunctionResult* pstFunctionResult)
{
	
	char* cmdname = (char*)"StopGrabContinous";
	SetReturnValue(cmdname, 0);

	EnterCriticalSection(&cs_Grab);

	m_bGrabContinous = false;

	LeaveCriticalSection(&cs_Grab);

	//Value 는 임시로
	SEMLogData.ProductID = IO_READ_STRING(GetIOName_ProductID(m_Unit));
	Write_SEMLog_Event("Fnc_Vision_Pre_Act_StopGrabContinous", strArg);
	ReturnSuccessWithResult(cmdname);
}
enumFunctionResult Fnc_Vision_Pre::Act_TestResultClear(std::string strArg, structFunctionResult* pstFunctionResult)
{
	
	char* cmdname = (char*)"TestResultClear";
	SetReturnValue(cmdname, 0);	
	for (int i = 0; i < 100; i++)
	{
		stImage->stObject[i].nType = 0;
	}
	Paint(false);
	//	Write_SEMLog_Event("Fnc_Vision_SideLeft_Act_TestResultClear", strArg);
	ReturnSuccessWithResult(cmdname);
}
enumFunctionResult Fnc_Vision_Pre::Act_FindPreVision(std::string strArg, structFunctionResult* pstFunctionResult) {
	WriteConsole_FunctionStart;
	char* cmdname = (char*)"FindPreVision";
	SetReturnValue(cmdname, 0);

	stVisionResult.ResetVariable(m_nVision);

	int nArgs = 0;
	if (strArg == "") {
		nArgs = 0;
	}
	else
	{
		PARSE_ARGS(strArg, ",");
		nArgs = stoi(GET_ARGS(1));
	}

	int nTimeoutFlag = 0;
	int nTimeout = 5000;

	if (!Grab())
	{
		ReturnFailWithResult(cmdname);
	}

	int nCorner = 0;
	if (nArgs == _LU)
	{
		nCorner = 2;
	}
	else if (nArgs == _LL)
	{
		nCorner = 3;
	}
	else if (nArgs == _RU)
	{
		nCorner = 0;
	}
	else
	{
		nCorner = 1;
	}
	//점검
	//	stVisionResult = HV_PreVisionFindCorner(nCorner);
	stVisionResult = HV_PreVisionFindCornerTeamplate(nArgs);

	Paint();

	ReturnSuccessWithResult(cmdname);
}
VisionResult Fnc_Vision_Pre::GetVisionResult()
{
	VisionResult stRet;

	stRet = stVisionResult;
	VisionResultLog(GetFunctionName(), stRet);

	return stRet;
}
void Fnc_Vision_Pre::HV_ImgProcess_Prepare(BYTE* img, RECT rRoi, bool bOverPaint/*= true*/)
{
	HObject Ho_Processimage;
	HObject Ho_BinImage;

	HObject Ho_Domain;
	HObject Ho_thregion;

	int nStep = pMntData->GetPreVisionChannel();
	int nMode = pMntData->GetCuttingStep();

	HTuple hv_typ;
	HTuple hv_w, hv_h;
	HTuple hv_ptr;
	int iTop = rRoi.top;
	int iLeft = rRoi.left;
	int iBottom = rRoi.bottom;
	int iRight = rRoi.right;


	GenImage1Extern(&Ho_Processimage, "byte", m_nSizeX, m_nSizeY, (Hlong)img, (Hlong)NULL);
	GenRectangle1(&Ho_Domain, HTuple(iTop), HTuple(iLeft), HTuple(iBottom), HTuple(iRight));
	ReduceDomain(Ho_Processimage, Ho_Domain, &Ho_Processimage);
	double dGain = parAd2D(UN_PREVISION, NS_VISION::dGainValue, nStep, 0);
	int nOffset = parAi2D(UN_PREVISION, NS_VISION::nOffsetValue, nStep, 0);
	int nUseGainOffset = parAi2D(UN_PREVISION, NS_VISION::bUseGainOffset, nStep, 0);




	if (nUseGainOffset)
	{
		ScaleImage(Ho_Processimage, &Ho_Processimage, dGain, nOffset);
	}
	else
	{
		ScaleImage(Ho_Processimage, &Ho_Processimage, 1, 0);
	}

	dGain = parAd2D(UN_PREVISION, NS_VISION::dGainValue, nStep, 1);
	nOffset = parAi2D(UN_PREVISION, NS_VISION::nOffsetValue, nStep, 1);
	nUseGainOffset = parAi2D(UN_PREVISION, NS_VISION::bUseGainOffset, nStep, 1);

	if (nUseGainOffset)
	{
		ScaleImage(Ho_Processimage, &Ho_Processimage, dGain, nOffset);
	}
	else
	{
		ScaleImage(Ho_Processimage, &Ho_Processimage, 1, 0);
	}


	int nThreshold = parAi2D(UN_PREVISION, NS_VISION::nBinaryLevel, nStep, 0);

	if (1)
	{
		HalconCpp::Threshold(Ho_Processimage, &Ho_thregion, nThreshold, 256);


		if (parAi2D(UN_PREVISION, NS_VISION::bUseOpen, nStep, 0))
		{
			int nX = max(parAi2D(UN_PREVISION, NS_VISION::nOpenValueX, nStep, 0), 1);
			int nY = max(parAi2D(UN_PREVISION, NS_VISION::nOpenValueY, nStep, 0), 1);
			OpeningRectangle1(Ho_thregion, &Ho_thregion, nX, nY);

		}
		if (parAi2D(UN_PREVISION, NS_VISION::bUseClose, nStep, 0))
		{
			int nX = max(parAi2D(UN_PREVISION, NS_VISION::nCloseValueX, nStep, 0), 1);
			int nY = max(parAi2D(UN_PREVISION, NS_VISION::nCloseValueY, nStep, 0), 1);
			ClosingRectangle1(Ho_thregion, &Ho_thregion, nX, nY);
		}
	

		RegionToBin(Ho_thregion, &Ho_BinImage, 255, 0, m_nSizeX, m_nSizeY);
	}

	if (bOverPaint)
	{
		GetImagePointer1(Ho_BinImage, &hv_ptr, &hv_typ, &hv_w, &hv_h);
		memcpy(img, (BYTE*)hv_ptr.L(), m_nSizeX * m_nSizeY);

	}


	Ho_Processimage.Clear();
	Ho_BinImage.Clear();
	Ho_Domain.Clear();
	Ho_thregion.Clear();
}
void Fnc_Vision_Pre::HV_ImgProcess_Prepare2(BYTE* img, BYTE* Gainimg, BYTE* Binimg)
{
	HObject Ho_Processimage;
	HObject Ho_BinImage;

	HObject Ho_thregion;

	int nStep = pMntData->GetPreVisionChannel();
	int nMode = pMntData->GetCuttingStep();

	HTuple hv_typ;
	HTuple hv_w, hv_h;
	HTuple hv_ptr;

	GenImage1Extern(&Ho_Processimage, "byte", m_nSizeX, m_nSizeY, (Hlong)img, (Hlong)NULL);
	double dGain = parAd2D(UN_PREVISION, NS_VISION::dGainValue, nStep, 0);
	int nOffset = parAi2D(UN_PREVISION, NS_VISION::nOffsetValue, nStep, 0);
	int nUseGainOffset = parAi2D(UN_PREVISION, NS_VISION::bUseGainOffset, nStep, 0);

	if (nUseGainOffset)
	{
		ScaleImage(Ho_Processimage, &Ho_Processimage, dGain, nOffset);
	}
	else
	{
		ScaleImage(Ho_Processimage, &Ho_Processimage, 1, 0);
	}

// 	dGain = parAd2D(UN_PREVISION, NS_VISION::dGainValue, nStep, 1);
// 	nOffset = parAi2D(UN_PREVISION, NS_VISION::nOffsetValue, nStep, 1);
// 	nUseGainOffset = parAi2D(UN_PREVISION, NS_VISION::bUseGainOffset, nStep, 1);
// 
// 	if (nUseGainOffset)
// 	{
// 		ScaleImage(Ho_Processimage, &Ho_Processimage, dGain, nOffset);
// 	}
// 	else
// 	{
// 		ScaleImage(Ho_Processimage, &Ho_Processimage, 1, 0);
// 	}

	GetImagePointer1(Ho_Processimage, &hv_ptr, &hv_typ, &hv_w, &hv_h);
	memcpy(Gainimg, (BYTE*)hv_ptr.L(), m_nSizeX * m_nSizeY);

	int nThreshold = parAi2D(UN_PREVISION, NS_VISION::nBinaryLevel, nStep, 0);

	if (1)
	{
		HalconCpp::Threshold(Ho_Processimage, &Ho_thregion, nThreshold, 256);

		if (nMode)
		{
			if (parAi2D(UN_PREVISION, NS_VISION::bUseOpenMark, nStep, 0))
			{
				int nX = max(parAi2D(UN_PREVISION, NS_VISION::nOpenValueXMark, nStep, 0), 1);
				int nY = max(parAi2D(UN_PREVISION, NS_VISION::nOpenValueYMark, nStep, 0), 1);

				OpeningRectangle1(Ho_thregion, &Ho_thregion, nX, nY);

			}
			if (parAi2D(UN_PREVISION, NS_VISION::bUseCloseMark, nStep, 0))
			{
				int nX = max(parAi2D(UN_PREVISION, NS_VISION::nCloseValueXMark, nStep, 0), 1);
				int nY = max(parAi2D(UN_PREVISION, NS_VISION::nCloseValueYMark, nStep, 0), 1);
				ClosingRectangle1(Ho_thregion, &Ho_thregion, nX, nY);

			}
		}
		else
		{
			if (parAi2D(UN_PREVISION, NS_VISION::bUseOpen, nStep, 0))
			{
				int nX = max(parAi2D(UN_PREVISION, NS_VISION::nOpenValueX, nStep, 0), 1);
				int nY = max(parAi2D(UN_PREVISION, NS_VISION::nOpenValueY, nStep, 0), 1);
				OpeningRectangle1(Ho_thregion, &Ho_thregion, nX, nY);

			}
			if (parAi2D(UN_PREVISION, NS_VISION::bUseClose, nStep, 0))
			{
				int nX = max(parAi2D(UN_PREVISION, NS_VISION::nCloseValueX, nStep, 0), 1);
				int nY = max(parAi2D(UN_PREVISION, NS_VISION::nCloseValueY, nStep, 0), 1);
				ClosingRectangle1(Ho_thregion, &Ho_thregion, nX, nY);
			}
		}

		RegionToBin(Ho_thregion, &Ho_BinImage, 255, 0, m_nSizeX, m_nSizeY);
	}


	GetImagePointer1(Ho_BinImage, &hv_ptr, &hv_typ, &hv_w, &hv_h);
	memcpy(Gainimg, (BYTE*)hv_ptr.L(), m_nSizeX * m_nSizeY);


	Ho_Processimage.Clear();
	Ho_BinImage.Clear();

	Ho_thregion.Clear();
}
void  Fnc_Vision_Pre::SwapCoordinate(BYTE* ORG, BYTE* DEST, int Width, int Height, double dAngle)
{
	// 	int iDestIndex;
	// 	int iOrgIndex;
	// 
	// 	if (dAngle < 100)//90 left
	// 	{
	// 		for (int i = 0; i < Width; ++i)
	// 		{
	// 			for (int j = 0; j < Height; ++j)
	// 			{
	// 				//   iOrgIndex = (i * Width) + j;
	// 				//   iDestIndex = ((Height-j-1) * Height) + i;
	// 
	// 				iOrgIndex = ((Height - i - 1) * Width) + j;
	// 				iDestIndex = (j * Height) + i;
	// 				DEST[iDestIndex] = ORG[iOrgIndex];
	// 			}
	// 		}
	// 
	// 	}
	// 	else
	// 	{
	// 		for (int i = 0; i < Width; ++i)
	// 		{
	// 			for (int j = 0; j < Height; ++j)
	// 			{
	// 				//   iOrgIndex = (i * Width) + j;
	// 				//   iDestIndex = (j * Height) + i;
	// 
	// 				iOrgIndex = (i * Width) + j;
	// 				iDestIndex = ((Width - j - 1) * Height) + i;
	// 				DEST[iDestIndex] = ORG[iOrgIndex];
	// 			}
	// 		}
	// 	}



	HTuple hv_typ;
	HTuple hv_w, hv_h;
	HTuple hv_ptr;

	HObject ho_Org, ho_rotate;
	GenImage1Extern(&ho_Org, "byte", Width, Height, (Hlong)ORG, (Hlong)NULL);

	RotateImage(ho_Org, &ho_rotate, dAngle, "constant");

	GetImagePointer1(ho_rotate, &hv_ptr, &hv_typ, &hv_w, &hv_h);
	memcpy(DEST, (BYTE*)hv_ptr.L(), Width * Height);
	ho_Org.Clear();
	ho_rotate.Clear();
}
