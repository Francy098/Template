#pragma once
#include "LEEffect.h"
#include "ipp.h"
#include "math.h"
#include "myLib.h"


#define PLAYBUFFER_TXT "PlayBuffer"
#define	SCALAR_TXT "Scalar"
#define VECTOR_TXT "Vector"
#define VARLEN_TXT "VarLen"
			
#define INT16_TXT "Integer 16bit"
#define INT32_TXT "Integer 32bit"
#define FLT32_TXT "Float 32bit"
#define DBL64_TXT "Double 64bit"

#define DATATYPEBITMASK VECTOR|SCALAR|BUFFSTREAMING|VARLEN
#define DATALENBITMASK DATAINT32|DATAINT16|DATAINT8|DATAFLOAT32|DATADOUBLE64|CUSTOM

#define MAX_FILE_NAME_LENGTH 255
#define WIDTHDEF 75

#define NUTS_NAME	"CBBANC"

#define filterOrder 400 //Number of filter taps
#define half_filterOrder 200 

class PlugIn :	public LEEffect
{
public:
	PlugIn(InterfaceType _CBFunction,void * _PlugRef,HWND ParentDlg);

	~PlugIn(void);
		
	void __stdcall LESetName(char *Name);

	void __stdcall LEPlugin_Init();
	int  __stdcall LEPlugin_Process(PinType **Input,PinType **Output,LPVOID ExtraInfo);
	void __stdcall LEPlugin_Delete(void);

	bool __stdcall LEInfoIO(int index,int type, char *StrInfo);

	int __stdcall LEGetNumInput(){return LEEffect::LEGetNumInput();};  
	int __stdcall LEGetNumOutput(){return LEEffect::LEGetNumOutput();};
	int __stdcall LESetNumInput(int Val,PinType *TypeNewIn=0){ return LEEffect::LESetNumInput(Val,TypeNewIn); }; 
	int __stdcall LESetNumOutput(int Val,PinType *TypeNewOut=0){ return LEEffect::LESetNumOutput(Val, TypeNewOut);};

	void __stdcall LESetParameter(int Index,void *Data,LPVOID bBroadCastInfo);
	int  __stdcall LEGetParameter(int Index,void *Data);

	int __stdcall LESetDefPin(int index,int type, PinType *Info);

	HWND __stdcall LEGetWndSet(){return 0;};
	int __stdcall  LEWinSetStatusChange(int NewStatus){return 0;};		

	void __stdcall LESaveSetUp();
	void __stdcall LELoadSetUp();

	void __stdcall LESampleRateChange(int NewVal,int TrigType); 
	void __stdcall LEFrameSizeChange (int NewVal,int TrigType); 
	void __stdcall LEConnectionChange(int IOType,int Index,bool Connected){};
	int  __stdcall LEConnectionRequest(int IOType,int Index,PinType *NewType);
	int  __stdcall LEExtraInfoPinChange(int IOType,int Index,PinExtraInfoType ExInfo){return 0;};

	void __stdcall LERTWatchInit();
	LPVOID __stdcall LEOnNUTechMessage(int MessageType,int MessageID,WPARAM wParam,LPARAM lParam);



private:

	int FrameSize, SampleRate;

	int Fs; // Freq. di campionamento 

	double mu; //Step size for FxNLMS Algorithm

	int paths_len, bufSize, specSize, one_sample;

	// ------ Right channel
	// 
	double* xR; // Ingresso xR (reference mic) e uscita canale (error mic)) 
	double* an_R, * an2_R; // Audio Source input
	double* dR, *eR;

	// Parametri wR 
	double* wR_Taps;

	// Primary Path parameters
	IppsFIRSpec_64f* PR_pSpec;
	Ipp8u* PR_pBuf;
	double* PR_dly_dst, * PR_dly_src;
	double* PR_Taps;

	// Secondary Path parameters
	IppsFIRSpec_64f* SR_pSpec;
	Ipp8u* SR_pBuf;
	double* SR_dly_dst, * SR_dly_src;
	double* SR_Taps;

	// Secondary Path Estimation parameters
	IppsFIRSpec_64f* SR1_est_pSpec;
	Ipp8u* SR1_est_pBuf;
	double* SR1_est_dly_dst, * SR1_est_dly_src;
	double* SR1_est_Taps;

	IppsFIRSpec_64f* SR2_est_pSpec;
	Ipp8u* SR2_est_pBuf;
	double* SR2_est_dly_dst, * SR2_est_dly_src;
	double* SR2_est_Taps;

	IppsFIRSpec_64f* SR3_est_pSpec;
	Ipp8u* SR3_est_pBuf;
	double* SR3_est_dly_dst, * SR3_est_dly_src;
	double* SR3_est_Taps;

	// ------ Left channel
	// 
	double* xL; // Ingresso xR (reference mic) e uscita canale (error mic)) 
	double* an_L, * an2_L; //Audio Source 
	double* dL, * eL;;

	// Parametri wL
	double* wL_Taps;

	// Primary Path parameters
	IppsFIRSpec_64f* PL_pSpec;
	Ipp8u* PL_pBuf;
	double* PL_dly_dst, * PL_dly_src;
	double* PL_Taps;

	// Secondary Path parameters
	IppsFIRSpec_64f* SL_pSpec;
	Ipp8u* SL_pBuf;
	double* SL_dly_dst, * SL_dly_src;
	double* SL_Taps;

	// Secondary Path Estimation parameters
	IppsFIRSpec_64f* SL1_est_pSpec;
	Ipp8u* SL1_est_pBuf;
	double* SL1_est_dly_dst, * SL1_est_dly_src;
	double* SL1_est_Taps;

	IppsFIRSpec_64f* SL2_est_pSpec;
	Ipp8u* SL2_est_pBuf;
	double* SL2_est_dly_dst, * SL2_est_dly_src;
	double* SL2_est_Taps;

	IppsFIRSpec_64f* SL3_est_pSpec;
	Ipp8u* SL3_est_pBuf;
	double* SL3_est_dly_dst, * SL3_est_dly_src;
	double* SL3_est_Taps;


	double* xRR, * xRL, * xLR, * xLL; // Vettori di 200 campioni
	double* xBin_R, * xBin_L;
	double* xBil;

	// Updating coefficient parameters
	double* div;
	double* update;
	double delta; // per evitare che il denom sia 0;

	char save_name1[MAX_FILE_NAME_LENGTH]; // max 255 caratteri
	char save_name2[MAX_FILE_NAME_LENGTH]; // max 255 caratteri

	// Buffers
	double* xR_buf_Bil, * xL_buf_Bil, * xR_buf_Bin, * xL_buf_Bin;
	double* xfR_buf, * xfL_buf,* xRR_buf, * xRL_buf, * xLR_buf, * xLL_buf;
	// Array for saving old samples
	double* xR_old, * xL_old;
	double* xRR_old, * xRL_old, * xLR_old, * xLL_old;

	double* xTaps; // Finestra di 200 campioni
	//double* xBil;

	// Dichiarating a place where saving more informations

	void Bilateral_R_ANC(double* xn_buf, double* xf_buf, double* en, double* OutputData_R);
	void Bilateral_L_ANC(double* xn_buf, double* xf_buf, double* en, double* OutputData_L);

	void Binaural_ANC(double* xR_buf, double* xL_buf, double* OutputData_R, double* OutputData_L);

	// ------------ FFT parameters --------------
	int fft_pSpecSize, fft_pSpecBufferSize, fft_pBufferSize;
	//Ipp64f fft_status;
	Ipp8u* fft_pSpec;
	Ipp8u* fft_pSpecBuf;
	Ipp8u* fft_pBuf;

	int fftOrd, nfft;
	IppsFFTSpec_C_64f* fftState;

	//------------ Coherence parameters -------------
	int lambda;

	double coherence(double* x, double* y);

	double* Pxx, * Pyy, * Pxy_Re, * Pxy_Im, * Coh_temp;
	double* x_seg, * x_im, * y_seg, * y_im;
	double* X_Re, * Y_Re, * X_Im, * Y_Im;

	//Setting parameters
	int segmentLength, overlap, hopSize, numSegments;
};