#pragma once
#include "../FunctionBaseCustom.h"
#include "multicam.h"
#include "../Inc/SCD/Inc/tinyxml/tinyxml2.h"
#include <memory>
#include "internal/common/h/SharedMemory.h"

class Fnc_Vision_Pre : public FunctionBaseCustom {
    DECLARE_DYNAMIC(Fnc_Vision_Pre)

public:
	bool RegistCommand() override;
	bool InitCode() override;
	bool LoadIOConfig(std::string& strIOInfo) override;
	bool PreCode(std::string& strCommandName, std::string& strArgs, bool bRetry, void* pLogPath) override;
	void PostCode(std::string& strCommandName, std::string& strArgs, enumFunctionResult& eResult) override;
	void FunctionAbort() override;


public:
	int					InitialBoard();
	void				InitialCrevisCam();
	bool				SetCameraGain(int nGain = 1);
	bool				SetPole(int nMode);
	bool				GetPole();

public:
	bool				Saveparameter();
	bool				LoadParameter();

public:
	void				Callback(PMCSIGNALINFO SigInfo);
	bool				Grab();
	void				Paint(bool GrabContinue = false);

public:
	void				SetImageMode(bool bMode);
	void				ImageOpen(std::string strFile);
	void				SenGDIResult(int iStart, int iEnd);

public:
	VisionResult		HV_PreVisionFindCornerTeamplate(int nCorner);
	void				HV_PreVisionCreateTeamplate(int nCorner);
	HTuple				m_hv_ModelID[4];

	void             HV_ImgProcess_Prepare(BYTE* img, RECT rRoi, bool bOverPaint = true);
	void             HV_ImgProcess_Prepare2(BYTE* img, BYTE* Gainimg, BYTE* Binimg);

	BYTE*				CreateShareMemory(HANDLE* Handle, int Width, int Height, string sFileName, BOOL* bDone);
	VisionResult GetVisionResult();
	void				SwapCoordinate(BYTE* ORG, BYTE* DEST, int Width, int Height, double dAngle);

	void				CreateFolder(CString strPath);
	void				SavePreImage(BYTE* Image, int Width, int Height, int nResult, int nSaveChannel);

private:
	enumFunctionResult Act_Initial(std::string strArg, structFunctionResult* pstFunctionResult);
	enumFunctionResult Act_ModelChange(std::string strArg, structFunctionResult* pstFunctionResult);
	enumFunctionResult Act_Grab(std::string strArg, structFunctionResult* pstFunctionResult);
	enumFunctionResult Act_GrabContinous(std::string strArg, structFunctionResult* pstFunctionResult);
	enumFunctionResult Act_StopGrabContinous(std::string strArg, structFunctionResult* pstFunctionResult);

    enumFunctionResult Act_FindPreVision(std::string strArg, structFunctionResult* pstFunctionResult);
	enumFunctionResult Act_CropCreateShapeModel(std::string strArg, structFunctionResult* pstFunctionResult);
	enumFunctionResult Act_TestResultClear(std::string strArg, structFunctionResult* pstFunctionResult);

private:
	MARSLog m_MarsLog;

public:
	MCHANDLE			m_Channel;
	PVOID				m_pCurrent;
	int					m_hDevice;
	HANDLE				m_hProssed;

public:

	BYTE* pOriginalImg;
	BYTE* pCamImg;
	BYTE* pGainImg;
	BYTE* pBinImg;
	BYTE* pViewImg;


public:
	VisionResult			stVisionResult;

public:
	// 	HTuple              m_hv_PrevisionModelID[4];//X, Y  ch

public:
	int m_nVision;
	double dResol[2];
	int nType;
	int nBoardNo;
	const char* chCamFile;
	const char* chCannel;
	const char* chCamID;
	const char* chCommChannel;
	const char* chViewShareMemFile;
	int nCommPort;
	bool m_bCommConnect;
	bool m_bCameraOpen;

	int	m_nRealSizeX;
	int	m_nRealSizeY;
	int m_nSizeX;
	int m_nSizeY;
	int m_nViewerSizeX;
	int m_nViewerSizeY;

	int m_nFlipX;
	int m_nFlipY;
	int m_nRotate;
	int m_nRotateDegree;

	bool m_bImageMode;

	bool m_bGrabContinous;
	int m_nUseCameraInitial;

	int SizeX;
	int SizeY;
	int BufferPitch;

	int	m_nGrabPosition;
	int	m_nOffsetMode;
	
	private:
		CRITICAL_SECTION cs_Grab;
		SharedMemory smViewImage;
	
		SharedMemory smImageStructure;
		stVisionData* stImage;

		DWORD processIdOld;
		HANDLE m_hGuiProcess;

		int m_nIsInitialize;
};
