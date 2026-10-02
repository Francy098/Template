#pragma once
#include "StdAfx.h"
#include ".\plugin.h"



PlugIn::PlugIn(InterfaceType _CBFunction,void * _PlugRef,HWND ParentDlg): LEEffect(_CBFunction,_PlugRef,ParentDlg)
{
	FrameSize = CBFunction(this,NUTS_GET_FS_SR,0,(LPVOID)AUDIOPROC);
	SampleRate = CBFunction(this,NUTS_GET_FS_SR,1,(LPVOID)AUDIOPROC);	

	//We need two inputs and two outputs
	LESetNumInput(6); // Ref mic R/L and Err mic R/L
	LESetNumOutput(2);

	// Setting char vairiables
	memset(save_name1, 0, MAX_FILE_NAME_LENGTH * sizeof(char));
	memset(save_name2, 0, MAX_FILE_NAME_LENGTH * sizeof(char));

	// Coping the Secondary paths taps
	strcpy(save_name1, "C:\\UNIVPM\\paths_taps\\MOTU_M4\\prova_Err_L.f64"); // Path from Error L mic to L speaker
	strcpy(save_name2, "C:\\UNIVPM\\paths_taps\\MOTU_M4\\prova_Err_R.f64"); // Path from Error R mic to R speaker

	//Inizialize parameters
	Fs = 0;
	mu = 0;


	xR = 0;					xL = 0;
	dR = 0;					dL = 0;
	eR = 0;					eL = 0;
	an_R = 0;				an_L = 0;
	an2_R = 0;				an2_L = 0;
	wR_Taps = 0;			wL_Taps = 0;
	PR_Taps = 0;			PL_Taps = 0;
	SR_Taps = 0;			SL_Taps = 0;
	SR1_est_Taps = 0;		SL1_est_Taps = 0;
	SR2_est_Taps = 0;		SL2_est_Taps = 0;
	SR3_est_Taps = 0;		SL3_est_Taps = 0;
	PR_dly_src = 0;			PL_dly_src = 0;
	SR_dly_src = 0;			SL_dly_src = 0;
	SR1_est_dly_src = 0;	SL1_est_dly_src = 0;
	SR2_est_dly_src = 0;	SL2_est_dly_src = 0;
	SR3_est_dly_src = 0;	SL3_est_dly_src = 0;
	PR_dly_dst = 0;			PL_dly_dst = 0;
	SR_dly_dst = 0;			SL_dly_dst = 0;
	SR1_est_dly_dst = 0;	SL1_est_dly_dst = 0;
	SR2_est_dly_dst = 0;	SL2_est_dly_dst = 0;
	SR3_est_dly_dst = 0;	SL3_est_dly_dst = 0;

	xTaps = 0;
	xBil = 0;				//xLTaps = 0;
	xRR = 0;				xRL = 0;
	xLR = 0;				xLL = 0;
	xBin_R = 0;				xBin_L = 0;

	xR_buf_Bil = 0;			xL_buf_Bil = 0;
	xR_buf_Bin = 0;			xL_buf_Bin = 0;
	xfR_buf = 0;			xfL_buf = 0;
	xRR_buf = 0;			xRL_buf = 0;
	xLR_buf = 0;			xLL_buf = 0;

	xR_old = 0;				xL_old = 0;
//	xR_old_Bin = 0;			xL_old_Bin = 0;
//	xfR_old = 0;			xfL_old = 0;
	xRR_old = 0;			xRL_old = 0;
	xLR_old = 0;			xLL_old = 0;

	delta = 0;
	update = 0;
	div = 0;

	// ----- FFT parameters
	fft_pSpecSize = 0;
	fft_pSpecBufferSize = 0;
	fft_pBufferSize = 0;
	fft_pSpec = 0;
	fft_pSpecBuf = 0;
	fft_pBuf = 0;

	// ----- Coherence parameters
	x_seg = 0; y_seg = 0; x_im = 0; y_im = 0;
	X_Re = 0; Y_Re = 0; X_Im = 0; Y_Im = 0;
	Pxx = 0; Pyy = 0; Pxy_Re = 0; Pxy_Im = 0; 
	Coh_temp = 0;

}

int __stdcall PlugIn::LEPlugin_Process(PinType **Input,PinType **Output,LPVOID ExtraInfo)
{ 
	double* InputData_Ref_mic_L = ((double*)Input[0]->DataBuffer); // Left channel mic
	double* InputData_Err_mic_L = ((double*)Input[1]->DataBuffer); // Left channel mic
	double* InputData_Ref_mic_R = ((double*)Input[2]->DataBuffer); // Right channel mic
	double* InputData_Err_mic_R = ((double*)Input[3]->DataBuffer); // Right channel mic
	double* InputData_audio_suorce_R = ((double*)Input[4]->DataBuffer); // Right channel mic
	double* InputData_audio_suorce_L = ((double*)Input[5]->DataBuffer); // Right channel mic

	// Error mic output R and L channels
	double* OutputData_R = ((double*)Output[0]->DataBuffer);
	double* OutputData_L = ((double*)Output[1]->DataBuffer);

	for (int i = 0; i < FrameSize; i++) {
		xR[i] = (double)InputData_Ref_mic_R[i] / 32767.0; // Reference mics
		xL[i] = (double)InputData_Ref_mic_L[i] / 32767.0;
		eR[i] = (double)InputData_Err_mic_R[i] / 32767.0; // Error mics
		eL[i] = (double)InputData_Err_mic_L[i] / 32767.0;
		an_R[i] = (double)InputData_audio_suorce_R[i] / 32767.0; // Audio source
		an_L[i] = (double)InputData_audio_suorce_L[i] / 32767.0;
	}

	// ------------------------ HP: Audio Source = 0 ----------------------------
	ippsSet_64f(0.0, an_R, FrameSize);
	ippsSet_64f(0.0, an_L, FrameSize);
	// --------------------------------------------------------------------------

	// Cleaning currrent Error Mic Signal
	//ippsZero_64f(eR, FrameSize);
	//ippsZero_64f(eL, FrameSize);

	//-------------------------------------------------------------------------------------------------
	//-------------------------------------- ONLY FOR TESTING -----------------------------------------
	//-------------------------------------------------------------------------------------------------

	/*ippsZero_64f(xL, FrameSize);
	for (int i = 0; i < (FrameSize-5); i++) { //Primi campioni li escludiamo per simulare una sorgente rumorosa a DX
		xL[i + 5] = xR[i] * 0.5;
		//an[i] = (double)InputData[i] / 32767.0; // Ridimensionto dei valori dell' Audio Source
	}*/
	//-------------------------------------------------------------------------------------------------


	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// -------------- Coherence calculation ---------------
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// 
	// Cab = |Gab|^2 / Gaa*Gbb con a = ref mic signal and b = err mic signal
	// First of all, we need the FFT trasform of current xR,xL and enC_R,enC_L

	double CdirR = 0, CdirL = 0, CcrossR = 0, CcrossL = 0;

	// Evaluate Direct coherence  
	CdirR = coherence(xR, eR); //Direct coherence for Right channel, reference mic(dx) < -- > error mic(dx)
	CdirL = coherence(xL, eL);
	// Evaluate Cross Coherence
	CcrossR = coherence(xL, eR);  //Right cross coherence, reference mic(sx) < -- > Error mic(dx)
	CcrossL = coherence(xR, eL);

	// Evaluating Sigma
	double ratioR = CdirR / CcrossR;
	double ratioL = CdirL / CcrossL;
	double sigma = min(ratioR, ratioL);

	// ------- Switching Algorithm -------
	lambda = (int)round(fabs(sigma - fabs(sigma - 1.0)));
	// -lambda = 1 --->Bilateral
	// -lambda = 0 --->Binaural


	lambda = 0; // Only for TESTING


	// Partiamo dalla condizione in cui x[1:FrameSize] è proprio l'intero frame in ingresso
	// cui abbiamo opportunamente ridimensionato in ampiezza per confronatare i valori con quelli di Matlab
	//
	// ---------------------- HYPOTHESIS to consider ----------------------------
	// Reference mic signal: xR = xL
	// Secondary Path S = S_est estimation of secondary path
	// Audio Source an = 1, già fatto nell'allocazione di memoria
	// Reference mic signal xn = white noise
	// --------------------------------------------------------------------------

	// Primary Path filtering --- entire Frame
	//ippsFIRSR_64f(xR, dR, FrameSize, PR_pSpec, PR_dly_src, PR_dly_dst, PR_pBuf);
	//ippsCopy_64f(PR_dly_dst, PR_dly_src, paths_len - 1);

	//ippsFIRSR_64f(xL, dL, FrameSize, PL_pSpec, PL_dly_src, PL_dly_dst, PL_pBuf);
	//ippsCopy_64f(PL_dly_dst, PL_dly_src, paths_len - 1);

	// Filtered reference vectors
	ippsFIRSR_64f(xR, xRR, FrameSize, SR1_est_pSpec, SR1_est_dly_src, SR1_est_dly_dst, SR1_est_pBuf);
	ippsCopy_64f(SR1_est_dly_dst, SR1_est_dly_src, paths_len - 1);

	ippsFIRSR_64f(xL, xRL, FrameSize, SR2_est_pSpec, SR2_est_dly_src, SR2_est_dly_dst, SR2_est_pBuf);
	ippsCopy_64f(SR2_est_dly_dst, SR2_est_dly_src, paths_len - 1);

	ippsFIRSR_64f(xR, xLR, FrameSize, SL1_est_pSpec, SL1_est_dly_src, SL1_est_dly_dst, SL1_est_pBuf);
	ippsCopy_64f(SL1_est_dly_dst, SL1_est_dly_src, paths_len - 1);

	ippsFIRSR_64f(xL, xLL, FrameSize, SL2_est_pSpec, SL2_est_dly_src, SL2_est_dly_dst, SL2_est_pBuf);
	ippsCopy_64f(SL2_est_dly_dst, SL2_est_dly_src, paths_len - 1);

	// Audio Source filtering by Estimation of Secondary Path
	ippsFIRSR_64f(an_R, an2_R, FrameSize, SR3_est_pSpec, SR3_est_dly_src, SR3_est_dly_dst, SR3_est_pBuf);
	ippsCopy_64f(SR3_est_dly_dst, SR3_est_dly_src, paths_len - 1);

	ippsFIRSR_64f(an_L, an2_L, FrameSize, SL3_est_pSpec, SL3_est_dly_src, SL3_est_dly_dst, SL3_est_pBuf);
	ippsCopy_64f(SL3_est_dly_dst, SL3_est_dly_src, paths_len - 1);

/*	// --- Setting buffers for BILATERAL
	for (int k = 0; k < filterOrder - 1; k++) // Fist of all, inserting the xR/L_old samples... (199 in total: [0-198])
	{
		xR_buf_Bil[k] = xR_old[k];
		xL_buf_Bil[k] = xL_old[k];
		xfR_buf[k] = xRR_old[k];
		xfL_buf[k] = xLL_old[k];
	}
	for (int k = 0; k < FrameSize; k++)  // ... and then the samples of the current frame xn (from 199 to FrameSize + half_filterOrder  )
	{
		xR_buf_Bil[k + filterOrder - 1] = xR[k];
		xL_buf_Bil[k + filterOrder - 1] = xL[k];
		xfR_buf[k + filterOrder - 1] = xRR[k];
		xfL_buf[k + filterOrder - 1] = xLL[k];
	}	*/

/*	// --- Setting buffers for BINAURAL
	int M = 0;
	for (int k = 0; k < half_filterOrder - 1; k++) // Fist of all, inserting the xR/L_old samples... (199 in total: [0-198])
	{
		M = half_filterOrder + k; //We need only 199 last samples of old parts.

		xR_buf_Bin[k] = xR_old[M];
		xL_buf_Bin[k] = xL_old[M];
		xRR_buf[k] = xRR_old[M];
		xRL_buf[k] = xRL_old[k]; // xRL used only for Binaural
		xLR_buf[k] = xLR_old[k]; // xLR used only for Binaural
		xLL_buf[k] = xLL_old[M];
	}
	for (int k = 0; k < FrameSize; k++)  // ... and then the samples of the current frame xn (from 199 to FrameSize + half_filterOrder  )
	{
		xR_buf_Bin[k + half_filterOrder - 1] = xR[k];
		xL_buf_Bin[k + half_filterOrder - 1] = xL[k];
		xRR_buf[k + half_filterOrder - 1] = xRR[k];
		xRL_buf[k + half_filterOrder - 1] = xRL[k];
		xLR_buf[k + half_filterOrder - 1] = xLR[k];
		xLL_buf[k + half_filterOrder - 1] = xLL[k];
	}	*/


	int idx_start = 0;

	if (lambda == 1) { // ------ Bilateral ANC ------

		PlugIn::Bilateral_R_ANC(xR_buf_Bil, xfR_buf, eR, OutputData_R); //Saving current error frame 
		PlugIn::Bilateral_L_ANC(xL_buf_Bil, xfL_buf, eL, OutputData_L); //Saving current error frame 
	}
	else {// (lamda == 0) ------ Binaural ANC ------

		PlugIn::Binaural_ANC(xR_buf_Bin, xL_buf_Bin, OutputData_R, OutputData_L);
	}

	// Error mic signal
	//ippsMulC_64f(eR, 32767.0, OutputData_R, FrameSize); // Right Error mic signal
	//ippsMulC_64f(eL, 32767.0, OutputData_L, FrameSize); // Left Error mic signal

	// ---------------- END CURRENT FRAME FILTERING ---------------
	// Saving old samples for BILATERAL

	idx_start = FrameSize + 1 - filterOrder;

	for (int k = 0; k < filterOrder - 1; k++)
	{
		xR_old[k] = xR[idx_start + k]; //399 samples: [398 - 0]
		xL_old[k] = xL[idx_start + k];
		xRR_old[k] = xRR[idx_start + k];   //399 samples: [398 - 0]
		xLL_old[k] = xLL[idx_start + k];
	}

	// Saving old samples for BINAURAL
	
	idx_start = FrameSize + 1 - half_filterOrder;

	for (int k = 0; k < half_filterOrder - 1; k++)
	{
		xRL_old[k] = xRL[idx_start + k]; //199 samples: [198 - 0]
		xLR_old[k] = xLR[idx_start + k];
	}


	return COMPLETED;
}

void __stdcall PlugIn::LEPlugin_Init()
{
	paths_len = 256; // ------ S_est length 
	one_sample = 1; 
	
	// Coherence paramters setup --- initializated here for memory allocations 
	nfft = 1024;
	segmentLength = 1024; // Lenght of x_seg, y_seg windows


	//Allocating vectors and setting them to zero
	if (xR == 0) { xR = ippsMalloc_64f(FrameSize); ippsZero_64f(xR, FrameSize); }
	if (xL == 0) { xL = ippsMalloc_64f(FrameSize); ippsZero_64f(xL, FrameSize); }
	if (an_R == 0) { an_R = ippsMalloc_64f(FrameSize); ippsZero_64f(an_R, FrameSize); }
	if (an_L == 0) { an_L = ippsMalloc_64f(FrameSize); ippsZero_64f(an_L, FrameSize); }
	if (an2_R == 0) { an2_R = ippsMalloc_64f(FrameSize); ippsZero_64f(an2_R, FrameSize); }
	if (an2_L == 0) { an2_L = ippsMalloc_64f(FrameSize); ippsZero_64f(an2_L, FrameSize); }
	if (dR == 0) { dR = ippsMalloc_64f(FrameSize); ippsZero_64f(dR, FrameSize); }
	if (dL == 0) { dL = ippsMalloc_64f(FrameSize); ippsZero_64f(dL, FrameSize); }

	// ----- FILTER TAPS
	if (wR_Taps == 0) { wR_Taps = ippsMalloc_64f(filterOrder); ippsZero_64f(wR_Taps, filterOrder); }
	if (wL_Taps == 0) { wL_Taps = ippsMalloc_64f(filterOrder); ippsZero_64f(wL_Taps, filterOrder); }
	if (PR_Taps == 0) { PR_Taps = ippsMalloc_64f(filterOrder); ippsZero_64f(PR_Taps, filterOrder); }
	if (PL_Taps == 0) { PL_Taps = ippsMalloc_64f(filterOrder); ippsZero_64f(PL_Taps, filterOrder); }
	if (SR_Taps == 0) { SR_Taps = ippsMalloc_64f(filterOrder); ippsZero_64f(SR_Taps, filterOrder); }
	if (SL_Taps == 0) { SL_Taps = ippsMalloc_64f(filterOrder); ippsZero_64f(SL_Taps, filterOrder); }
	if (SR1_est_Taps == 0) { SR1_est_Taps = ippsMalloc_64f(filterOrder); ippsZero_64f(SR1_est_Taps, filterOrder); }
	if (SL1_est_Taps == 0) { SL1_est_Taps = ippsMalloc_64f(filterOrder); ippsZero_64f(SL1_est_Taps, filterOrder); }
	if (SR2_est_Taps == 0) { SR2_est_Taps = ippsMalloc_64f(filterOrder); ippsZero_64f(SR2_est_Taps, filterOrder); }
	if (SL2_est_Taps == 0) { SL2_est_Taps = ippsMalloc_64f(filterOrder); ippsZero_64f(SL2_est_Taps, filterOrder); }
	if (SR3_est_Taps == 0) { SR3_est_Taps = ippsMalloc_64f(filterOrder); ippsZero_64f(SR3_est_Taps, filterOrder); }
	if (SL3_est_Taps == 0) { SL3_est_Taps = ippsMalloc_64f(filterOrder); ippsZero_64f(SL3_est_Taps, filterOrder); }

	// ----- WINDOWS
	if (xTaps == 0) { xTaps = ippsMalloc_64f(filterOrder); ippsZero_64f(xTaps, filterOrder); }
	//if (xRTaps == 0) { xRTaps = ippsMalloc_64f(half_filterOrder); ippsZero_64f(xRTaps, half_filterOrder); }
	//if (xLTaps == 0) { xLTaps = ippsMalloc_64f(half_filterOrder); ippsZero_64f(xLTaps, half_filterOrder); }

	// ----- FILTERED REFERENCES
	if (xRR == 0) { xRR = ippsMalloc_64f(FrameSize); ippsZero_64f(xRR, FrameSize); }
	if (xRL == 0) { xRL = ippsMalloc_64f(FrameSize); ippsZero_64f(xRL, FrameSize); }
	if (xLR == 0) { xLR = ippsMalloc_64f(FrameSize); ippsZero_64f(xLR, FrameSize); }
	if (xLL == 0) { xLL = ippsMalloc_64f(FrameSize); ippsZero_64f(xLL, FrameSize); }

	// ----- 
	if (xBin_R == 0) { xBin_R = ippsMalloc_64f(filterOrder); ippsZero_64f(xBin_R, filterOrder); }
	if (xBin_L == 0) { xBin_L = ippsMalloc_64f(filterOrder); ippsZero_64f(xBin_L, filterOrder); }
	if (xBil == 0) { xBil = ippsMalloc_64f(filterOrder); ippsZero_64f(xBil, filterOrder); }

	// ----- UPDATE VECTORS
	if (update == 0) { update = ippsMalloc_64f(filterOrder); ippsZero_64f(update, filterOrder); }
	if (div == 0) { div = ippsMalloc_64f(filterOrder); ippsZero_64f(div, filterOrder); }

	// ----- Buffers
	if (xR_buf_Bil == 0) { xR_buf_Bil = ippsMalloc_64f(FrameSize + filterOrder - 1); ippsZero_64f(xR_buf_Bil, FrameSize + filterOrder - 1); }
	if (xL_buf_Bil == 0) { xL_buf_Bil = ippsMalloc_64f(FrameSize + filterOrder - 1); ippsZero_64f(xL_buf_Bil, FrameSize + filterOrder - 1); }
	if (xR_buf_Bin == 0) { xR_buf_Bin = ippsMalloc_64f(FrameSize + half_filterOrder - 1); ippsZero_64f(xR_buf_Bin, FrameSize + half_filterOrder - 1); }
	if (xL_buf_Bin == 0) { xL_buf_Bin = ippsMalloc_64f(FrameSize + half_filterOrder - 1); ippsZero_64f(xL_buf_Bin, FrameSize + half_filterOrder - 1); }
	if (xfR_buf == 0) { xfR_buf = ippsMalloc_64f(FrameSize + filterOrder - 1); ippsZero_64f(xfR_buf, FrameSize + filterOrder - 1); }
	if (xfL_buf == 0) { xfL_buf = ippsMalloc_64f(FrameSize + filterOrder - 1); ippsZero_64f(xfL_buf, FrameSize + filterOrder - 1); }
	if (xRR_buf == 0) { xRR_buf = ippsMalloc_64f(FrameSize + half_filterOrder - 1); ippsZero_64f(xRR_buf, FrameSize + half_filterOrder - 1); }
	if (xRL_buf == 0) { xRL_buf = ippsMalloc_64f(FrameSize + half_filterOrder - 1); ippsZero_64f(xRL_buf, FrameSize + half_filterOrder - 1); }
	if (xLR_buf == 0) { xLR_buf = ippsMalloc_64f(FrameSize + half_filterOrder - 1); ippsZero_64f(xLR_buf, FrameSize + half_filterOrder - 1); }
	if (xLL_buf == 0) { xLL_buf = ippsMalloc_64f(FrameSize + half_filterOrder - 1); ippsZero_64f(xLL_buf, FrameSize + half_filterOrder - 1); }
	if (xR_old == 0) { xR_old = ippsMalloc_64f(filterOrder - 1); ippsZero_64f(xR_old, filterOrder - 1); }
	if (xL_old == 0) { xL_old = ippsMalloc_64f(filterOrder - 1); ippsZero_64f(xL_old, filterOrder - 1); }
	if (xRR_old == 0) { xRR_old = ippsMalloc_64f(filterOrder - 1); ippsZero_64f(xRR_old, filterOrder - 1); }
	if (xRL_old == 0) { xRL_old = ippsMalloc_64f(half_filterOrder - 1); ippsZero_64f(xRL_old, half_filterOrder - 1); }
	if (xLR_old == 0) { xLR_old = ippsMalloc_64f(half_filterOrder - 1); ippsZero_64f(xLR_old, half_filterOrder - 1); }
	if (xLL_old == 0) { xLL_old = ippsMalloc_64f(filterOrder - 1); ippsZero_64f(xLL_old, filterOrder - 1); }
	if (eR == 0) { eR = ippsMalloc_64f(FrameSize); ippsZero_64f(eR, FrameSize);}
	if (eL == 0) { eL = ippsMalloc_64f(FrameSize); ippsZero_64f(eL, FrameSize);}
	// --------- Coherence parameters --------
	if (Pxx == 0) { Pxx = ippsMalloc_64f(nfft); ippsZero_64f(Pxx, nfft); }
	if (Pyy == 0) { Pyy = ippsMalloc_64f(nfft); ippsZero_64f(Pyy, nfft); }
	if (Pxy_Re == 0) { Pxy_Re = ippsMalloc_64f(nfft); ippsZero_64f(Pxy_Re, nfft); }
	if (Pxy_Im == 0) { Pxy_Im = ippsMalloc_64f(nfft); ippsZero_64f(Pxy_Im, nfft); }
	if (Coh_temp == 0) { Coh_temp = ippsMalloc_64f(nfft); ippsZero_64f(Coh_temp, nfft); }
	if (x_seg == 0) { x_seg = ippsMalloc_64f(segmentLength); ippsZero_64f(x_seg, segmentLength); }
	if (y_seg == 0) { y_seg = ippsMalloc_64f(segmentLength); ippsZero_64f(y_seg, segmentLength); }
	if (x_im == 0) { x_im = ippsMalloc_64f(nfft); ippsZero_64f(x_im, nfft); }
	if (y_im == 0) { y_im = ippsMalloc_64f(nfft); ippsZero_64f(y_im, nfft); }
	if (X_Re == 0) { X_Re = ippsMalloc_64f(nfft); ippsZero_64f(X_Re, nfft); }
	if (Y_Re == 0) { Y_Re = ippsMalloc_64f(nfft); ippsZero_64f(Y_Re, nfft); }
	if (X_Im == 0) { X_Im = ippsMalloc_64f(nfft); ippsZero_64f(X_Im, nfft); }
	if (Y_Im == 0) { Y_Im = ippsMalloc_64f(nfft); ippsZero_64f(Y_Im, nfft); }


	Fs = 16000; // Frequenza di campionamento [Hz]
	mu = 0.01; // Step size for FxNLMS algorithm
	delta = 0.000001;

	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// ------ Loading filters taps ------
	//read_dat(save_name1, PR_Taps, paths_len);
	//read_dat(save_name2, SR_Taps, paths_len);
	read_dat(save_name1, SL_Taps, paths_len);
	read_dat(save_name2, SR_Taps, paths_len);


	// Normalize Taps
	double maxSL = 0.0, maxSR = 0.0;
	ippsMaxAbs_64f(SL_Taps, paths_len, &maxSL);
	ippsMaxAbs_64f(SR_Taps, paths_len, &maxSR);
	double max = max(maxSL, maxSR);

	ippsDivC_64f_I(max, SL_Taps, paths_len);
	ippsDivC_64f_I(max, SR_Taps, paths_len);

	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// -------------------------------------------- RIGHT CHANNEL -----------------------------------------------
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%	

	// -------------------- PR : Primary Path --- FIR filter ------------------------
	//ippsZero_64f(P_Taps, paths_len); 
	//PR_Taps[2] = 1.0; // ------------------ Hp. RITARDO
	// -----------------------------------------------------------------------------
	ippsFIRSRGetSize(paths_len, ipp64f, &specSize, &bufSize); // Calcola le informazioni necessarie per creare il filtro FIR lungo filterOrder e le salva in pSpec1 e buffSize1
	PR_pSpec = (IppsFIRSpec_64f*)ippsMalloc_8u(specSize);
	PR_pBuf = ippsMalloc_8u(bufSize);
	PR_dly_dst = ippsMalloc_64f(paths_len - 1); ippsZero_64f(PR_dly_dst, paths_len - 1);
	PR_dly_src = ippsMalloc_64f(paths_len - 1); ippsZero_64f(PR_dly_src, paths_len - 1);
	ippsFIRSRInit_64f(PR_Taps, paths_len, ippAlgDirect, PR_pSpec);

	// -------------------- SR : Secondary Path --- FIR filter ------------------------
	//ippsZero_64f(S_Taps, paths_len);
	//SR_Taps[0] = 1.0;
	// -------------------------------------------------------------------------------
	ippsFIRSRGetSize(paths_len, ipp64f, &specSize, &bufSize); // Calcola le informazioni necessarie per creare il filtro FIR lungo filterOrder e le salva in pSpec1 e buffSize1
	SR_pSpec = (IppsFIRSpec_64f*)ippsMalloc_8u(specSize);
	SR_pBuf = ippsMalloc_8u(bufSize);
	SR_dly_dst = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SR_dly_dst, paths_len - 1);
	SR_dly_src = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SR_dly_src, paths_len - 1);
	ippsFIRSRInit_64f(SR_Taps, paths_len, ippAlgDirect, SR_pSpec);

	// -------------------- SR_est : Secondary Path Estimation --- FIR filter ------------------------
	//ippsZero_64f(S_est_Taps, paths_len);
	// ------------------------- HP. S = S_est --------------------------------
	ippsCopy_64f(SR_Taps, SR1_est_Taps, paths_len);
	ippsFIRSRGetSize(paths_len, ipp64f, &specSize, &bufSize);
	SR1_est_pSpec = (IppsFIRSpec_64f*)ippsMalloc_8u(specSize);
	SR1_est_pBuf = ippsMalloc_8u(bufSize);
	SR1_est_dly_dst = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SR1_est_dly_dst, paths_len - 1);
	SR1_est_dly_src = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SR1_est_dly_src, paths_len - 1);
	ippsFIRSRInit_64f(SR1_est_Taps, paths_len, ippAlgDirect, SR1_est_pSpec);

	ippsCopy_64f(SR_Taps, SR2_est_Taps, paths_len);
	ippsFIRSRGetSize(paths_len, ipp64f, &specSize, &bufSize);
	SR2_est_pSpec = (IppsFIRSpec_64f*)ippsMalloc_8u(specSize);
	SR2_est_pBuf = ippsMalloc_8u(bufSize);
	SR2_est_dly_dst = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SR2_est_dly_dst, paths_len - 1);
	SR2_est_dly_src = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SR2_est_dly_src, paths_len - 1);
	ippsFIRSRInit_64f(SR2_est_Taps, paths_len, ippAlgDirect, SR2_est_pSpec);

	ippsCopy_64f(SR_Taps, SR3_est_Taps, paths_len); // HP. S3_est = S
	ippsFIRSRGetSize(paths_len, ipp64f, &specSize, &bufSize);
	SR3_est_pSpec = (IppsFIRSpec_64f*)ippsMalloc_8u(specSize);
	SR3_est_pBuf = ippsMalloc_8u(bufSize);
	SR3_est_dly_dst = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SR3_est_dly_dst, paths_len - 1);
	SR3_est_dly_src = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SR3_est_dly_src, paths_len - 1);
	ippsFIRSRInit_64f(SR3_est_Taps, paths_len, ippAlgDirect, SR3_est_pSpec);

	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// -------------------------------------------- LEFT CHANNEL -----------------------------------------------
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%	

	// --------- Hp: Left channel = Right Channel 
	//ippsCopy_64f(PR_Taps, PL_Taps, paths_len);
	//ippsCopy_64f(SR_Taps, SL_Taps, paths_len);

	// -------------------- PL : Primary Path --- FIR filter ------------------------
	//ippsZero_64f(P_Taps, paths_len); 
	//PL_Taps[2] = 1.0; // ------------------ Hp. RITARDO
	// -----------------------------------------------------------------------------
	ippsFIRSRGetSize(paths_len, ipp64f, &specSize, &bufSize); // Calcola le informazioni necessarie per creare il filtro FIR lungo filterOrder e le salva in pSpec1 e buffSize1
	PL_pSpec = (IppsFIRSpec_64f*)ippsMalloc_8u(specSize);
	PL_pBuf = ippsMalloc_8u(bufSize);
	PL_dly_dst = ippsMalloc_64f(paths_len - 1); ippsZero_64f(PL_dly_dst, paths_len - 1);
	PL_dly_src = ippsMalloc_64f(paths_len - 1); ippsZero_64f(PL_dly_src, paths_len - 1);
	ippsFIRSRInit_64f(PL_Taps, paths_len, ippAlgDirect, PL_pSpec);

	// -------------------- SL : Secondary Path --- FIR filter ------------------------
	//ippsZero_64f(S_Taps, paths_len);
	//SL_Taps[0] = 1.0;
	// -------------------------------------------------------------------------------
	ippsFIRSRGetSize(paths_len, ipp64f, &specSize, &bufSize); // Calcola le informazioni necessarie per creare il filtro FIR lungo filterOrder e le salva in pSpec1 e buffSize1
	SL_pSpec = (IppsFIRSpec_64f*)ippsMalloc_8u(specSize);
	SL_pBuf = ippsMalloc_8u(bufSize);
	SL_dly_dst = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SL_dly_dst, paths_len - 1);
	SL_dly_src = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SL_dly_src, paths_len - 1);
	ippsFIRSRInit_64f(SL_Taps, paths_len, ippAlgDirect, SL_pSpec);

	// -------------------- SL_est : Secondary Path Estimation --- FIR filter ------------------------
	//ippsZero_64f(SL_est_Taps, paths_len);
	// ------------------------- HP. S = S_est --------------------------------
	ippsCopy_64f(SL_Taps, SL1_est_Taps, paths_len); // HP. S1_est = S
	ippsFIRSRGetSize(paths_len, ipp64f, &specSize, &bufSize);
	SL1_est_pSpec = (IppsFIRSpec_64f*)ippsMalloc_8u(specSize);
	SL1_est_pBuf = ippsMalloc_8u(bufSize);
	SL1_est_dly_dst = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SL1_est_dly_dst, paths_len - 1);
	SL1_est_dly_src = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SL1_est_dly_src, paths_len - 1);
	ippsFIRSRInit_64f(SL1_est_Taps, paths_len, ippAlgDirect, SL1_est_pSpec);

	ippsCopy_64f(SL_Taps, SL2_est_Taps, paths_len); // HP. S2_est = S
	ippsFIRSRGetSize(paths_len, ipp64f, &specSize, &bufSize);
	SL2_est_pSpec = (IppsFIRSpec_64f*)ippsMalloc_8u(specSize);
	SL2_est_pBuf = ippsMalloc_8u(bufSize);
	SL2_est_dly_dst = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SL2_est_dly_dst, paths_len - 1);
	SL2_est_dly_src = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SL2_est_dly_src, paths_len - 1);
	ippsFIRSRInit_64f(SL2_est_Taps, paths_len, ippAlgDirect, SL2_est_pSpec);

	ippsCopy_64f(SL_Taps, SL3_est_Taps, paths_len); // HP. S3_est = S
	ippsFIRSRGetSize(paths_len, ipp64f, &specSize, &bufSize);
	SL3_est_pSpec = (IppsFIRSpec_64f*)ippsMalloc_8u(specSize);
	SL3_est_pBuf = ippsMalloc_8u(bufSize);
	SL3_est_dly_dst = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SL3_est_dly_dst, paths_len - 1);
	SL3_est_dly_src = ippsMalloc_64f(paths_len - 1); ippsZero_64f(SL3_est_dly_src, paths_len - 1);
	ippsFIRSRInit_64f(SL3_est_Taps, paths_len, ippAlgDirect, SL3_est_pSpec);

	//_________________________________________________
	// ------ Inizialize old part windows -------
	ippsZero_64f(xR_old, filterOrder - 1);
	ippsZero_64f(xL_old, filterOrder - 1);

	ippsZero_64f(xRR_old, filterOrder - 1);
	ippsZero_64f(xRL_old, half_filterOrder - 1);
	ippsZero_64f(xLR_old, half_filterOrder - 1);
	ippsZero_64f(xLL_old, filterOrder - 1);


	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// --------------------------- STARTING CONDITION ------------------------------
	//
	//At the start we don't have yet the enC_R and enc_L (error mic signal), in
	// fact we need to process the first frame choosing a "PRIORI" which one of
	// the two ANC algorithms will process the frame.
	// My choice is starting with BINAURAL ANC algorithm, because cover a large
	// amount of cases of good ANC.

	lambda = 0; // Starting with Binaural ANC


	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// --------------------------- Coherence set parameters ------------------------------
	//

	//nfft = 1024*2; // Double then Matlab, because I use Pack format

	fftOrd = (int)(log10((double)nfft) / log10(2.0));

	//Allocazione variabili per la FFT
	ippsFFTGetSize_C_64f(fftOrd, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone, &fft_pSpecSize, &fft_pSpecBufferSize, &fft_pBufferSize);
	fft_pSpec = ippsMalloc_8u(fft_pSpecSize);
	fft_pSpecBuf = ippsMalloc_8u(fft_pSpecBufferSize);
	fft_pBuf = ippsMalloc_8u(fft_pBufferSize);
	ippsFFTInit_C_64f(&fftState, fftOrd, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone, fft_pSpec, fft_pSpecBuf);
	
	//segmentLength = 1024; // Lenght of x_seg, y_seg windows 
	overlap = 512; // 50%
	hopSize = segmentLength - overlap;
	numSegments = round((FrameSize - overlap) / hopSize); //Number of segments
	
}


void __stdcall PlugIn::LEPlugin_Delete()
{
	// Deallocating 
	if (update != 0) { ippsFree(update); update = 0; }
	if (div != 0) { ippsFree(div); div = 0; }
	if (eR != 0) { ippsFree(eR); eR = 0; }									if (eL != 0) { ippsFree(eL); eL = 0; }
	if (xR != 0) { ippsFree(xR); xR = 0; }									if (xL != 0) { ippsFree(xL); xL = 0; }
	if (an_R != 0) { ippsFree(an_R); an_R = 0; }								if (an_L != 0) { ippsFree(an_L); an_L = 0; }
	if (an2_R != 0) { ippsFree(an2_R); an2_R = 0; }							if (an2_L != 0) { ippsFree(an2_L); an2_L = 0; }
	if (dR != 0) { ippsFree(dR); dR = 0; }									if (dL != 0) { ippsFree(dL); dL = 0; }
	// ---
	if (wR_Taps != 0) { ippsFree(wR_Taps); wR_Taps = 0; }							if (wL_Taps != 0) { ippsFree(wL_Taps); wL_Taps = 0; }
	// ----------------------------------- Primary Path parameters ------------------------------------------
	if (PR_Taps != 0) { ippsFree(PR_Taps); PR_Taps = 0; }						if (PL_Taps != 0) { ippsFree(PL_Taps); PL_Taps = 0; }
	if (PR_pSpec != 0) { ippsFree(PR_pSpec); PR_pSpec = 0; }						if (PL_pSpec != 0) { ippsFree(PL_pSpec); PL_pSpec = 0; }
	if (PR_pBuf != 0) { ippsFree(PR_pBuf); PR_pBuf = 0; }						if (PL_pBuf != 0) { ippsFree(PL_pBuf); PL_pBuf = 0; }
	if (PR_dly_src != 0) { ippsFree(PR_dly_src); PR_dly_src = 0; }					if (PL_dly_src != 0) { ippsFree(PL_dly_src); PL_dly_src = 0; }
	if (PR_dly_dst != 0) { ippsFree(PR_dly_dst); PR_dly_dst = 0; }					if (PL_dly_dst != 0) { ippsFree(PL_dly_dst); PL_dly_dst = 0; }
	// ----------------------------------- Secondary Path parameters -----------------------------------------
	if (SR_Taps != 0) { ippsFree(SR_Taps); SR_Taps = 0; }						if (SL_Taps != 0) { ippsFree(SL_Taps); SL_Taps = 0; }
	if (SR_pSpec != 0) { ippsFree(SR_pSpec); SR_pSpec = 0; }						if (SL_pSpec != 0) { ippsFree(SL_pSpec); SL_pSpec = 0; }
	if (SR_pBuf != 0) { ippsFree(SR_pBuf); SR_pBuf = 0; }						if (SL_pBuf != 0) { ippsFree(SL_pBuf); SL_pBuf = 0; }
	if (SR_dly_src != 0) { ippsFree(SR_dly_src); SR_dly_src = 0; }					if (SL_dly_src != 0) { ippsFree(SL_dly_src); SL_dly_src = 0; }
	if (SR_dly_dst != 0) { ippsFree(SR_dly_dst); SR_dly_dst = 0; }					if (SL_dly_dst != 0) { ippsFree(SL_dly_dst); SL_dly_dst = 0; }
	// ------------------------------ Secondary Path Estimation parameters ------------------------------------------------
	if (SR1_est_Taps != 0) { ippsFree(SR1_est_Taps); SR1_est_Taps = 0; }				if (SL1_est_Taps != 0) { ippsFree(SL1_est_Taps); SL1_est_Taps = 0; }
	if (SR1_est_pSpec != 0) { ippsFree(SR1_est_pSpec); SR1_est_pSpec = 0; }			if (SL1_est_pSpec != 0) { ippsFree(SL1_est_pSpec); SL1_est_pSpec = 0; }
	if (SR1_est_pBuf != 0) { ippsFree(SR1_est_pBuf); SR1_est_pBuf = 0; }				if (SL1_est_pBuf != 0) { ippsFree(SL1_est_pBuf); SL1_est_pBuf = 0; }
	if (SR1_est_dly_src != 0) { ippsFree(SR1_est_dly_src); SR1_est_dly_src = 0; }		if (SL1_est_dly_src != 0) { ippsFree(SL1_est_dly_src); SL1_est_dly_src = 0; }
	if (SR1_est_dly_dst != 0) { ippsFree(SR1_est_dly_dst); SR1_est_dly_dst = 0; }		if (SL1_est_dly_dst != 0) { ippsFree(SL1_est_dly_dst); SL1_est_dly_dst = 0; }

	if (SR2_est_Taps != 0) { ippsFree(SR2_est_Taps); SR2_est_Taps = 0; }				if (SL2_est_Taps != 0) { ippsFree(SL2_est_Taps); SL2_est_Taps = 0; }
	if (SR2_est_pSpec != 0) { ippsFree(SR2_est_pSpec); SR2_est_pSpec = 0; }			if (SL2_est_pSpec != 0) { ippsFree(SL2_est_pSpec); SL2_est_pSpec = 0; }
	if (SR2_est_pBuf != 0) { ippsFree(SR2_est_pBuf); SR2_est_pBuf = 0; }				if (SL2_est_pBuf != 0) { ippsFree(SL2_est_pBuf); SL2_est_pBuf = 0; }
	if (SR2_est_dly_src != 0) { ippsFree(SR2_est_dly_src); SR2_est_dly_src = 0; }		if (SL2_est_dly_src != 0) { ippsFree(SL2_est_dly_src); SL2_est_dly_src = 0; }
	if (SR2_est_dly_dst != 0) { ippsFree(SR2_est_dly_dst); SR2_est_dly_dst = 0; }		if (SL2_est_dly_dst != 0) { ippsFree(SL2_est_dly_dst); SL2_est_dly_dst = 0; }

	if (SR3_est_Taps != 0) { ippsFree(SR3_est_Taps); SR3_est_Taps = 0; }				if (SL3_est_Taps != 0) { ippsFree(SL3_est_Taps); SL3_est_Taps = 0; }
	if (SR3_est_pSpec != 0) { ippsFree(SR3_est_pSpec); SR3_est_pSpec = 0; }			if (SL3_est_pSpec != 0) { ippsFree(SL3_est_pSpec); SL3_est_pSpec = 0; }
	if (SR3_est_pBuf != 0) { ippsFree(SR3_est_pBuf); SR3_est_pBuf = 0; }				if (SL3_est_pBuf != 0) { ippsFree(SL3_est_pBuf); SL3_est_pBuf = 0; }
	if (SR3_est_dly_src != 0) { ippsFree(SR3_est_dly_src); SR3_est_dly_src = 0; }		if (SL3_est_dly_src != 0) { ippsFree(SL3_est_dly_src); SL3_est_dly_src = 0; }
	if (SR3_est_dly_dst != 0) { ippsFree(SR3_est_dly_dst); SR3_est_dly_dst = 0; }		if (SL3_est_dly_dst != 0) { ippsFree(SL3_est_dly_dst); SL3_est_dly_dst = 0; }

	// ---
	if (xTaps != 0) { ippsFree(xTaps); xTaps = 0; }								//if (xLTaps != 0) { ippsFree(xLTaps); }
	if (xBil != 0) { ippsFree(xBil); xBil = 0; }
	if (xRR != 0) { ippsFree(xRR); xRR = 0; }									if (xRL != 0) { ippsFree(xRL); xRL = 0; }
	if (xLR != 0) { ippsFree(xLR); xLR = 0; }									if (xLL != 0) { ippsFree(xLL); xLL = 0; }
	if (xBin_R != 0) { ippsFree(xBin_R); xBin_R = 0; }							if (xBin_L != 0) { ippsFree(xBin_L); xBin_L = 0; }
	// --- Buffers and old parts windows
	if (xR_buf_Bil != 0) { ippsFree(xR_buf_Bil); xR_buf_Bil = 0; }				if (xL_buf_Bil != 0) { ippsFree(xL_buf_Bil); xL_buf_Bil = 0; }
	if (xR_buf_Bin != 0) { ippsFree(xR_buf_Bin); xR_buf_Bin = 0; }				if (xL_buf_Bin != 0) { ippsFree(xL_buf_Bin); xL_buf_Bin = 0; }
	if (xfR_buf != 0) { ippsFree(xfR_buf); xfR_buf = 0; }						if (xfL_buf != 0) { ippsFree(xfL_buf); xfL_buf = 0; }
	if (xRR_buf != 0) { ippsFree(xRR_buf); xRR_buf = 0; }						if (xRL_buf != 0) { ippsFree(xRL_buf); xRL_buf = 0; }
	if (xLR_buf != 0) { ippsFree(xLR_buf); xLR_buf = 0; }						if (xLL_buf != 0) { ippsFree(xLL_buf); xLL_buf = 0; }
	if (xR_old != 0) { ippsFree(xR_old); xR_old = 0; }							if (xL_old != 0) { ippsFree(xL_old); xL_old = 0; }
	if (xRR_old != 0) { ippsFree(xRR_old); xRR_old = 0; }						if (xRL_old != 0) { ippsFree(xRL_old); xRL_old = 0; }
	if (xLR_old != 0) { ippsFree(xLR_old); xLR_old = 0; }						if (xLL_old != 0) { ippsFree(xLL_old); xLL_old = 0; }
	//
	if (fft_pSpec != 0) { ippsFree(fft_pSpec); fft_pSpec = 0; }
	if (fft_pSpecBuf != 0) { ippsFree(fft_pSpecBuf); fft_pSpecBuf = 0; }
	if (fft_pBuf != 0) { ippsFree(fft_pBuf); fft_pBuf = 0; }
	//if (fftState != 0) { ippsFree(fftState); }

	// Deallogating Coherence parameters 
	if (Pxx != 0) { ippsFree(Pxx); Pxx = 0; }
	if (Pyy != 0) { ippsFree(Pyy); Pyy = 0; }
	if (Pxy_Re != 0) { ippsFree(Pxy_Re); Pxy_Re = 0; }							if (Pxy_Im != 0) { ippsFree(Pxy_Im); Pxy_Im = 0; }
	if (x_seg != 0) { ippsFree(x_seg); x_seg = 0; }								if (x_im != 0) { ippsFree(x_im); x_im = 0; }
	if (y_seg != 0) { ippsFree(y_seg); y_seg = 0; }								if (y_im != 0) { ippsFree(y_im); y_im = 0; }
	if (X_Re != 0) { ippsFree(X_Re); X_Re = 0; }								if (X_Im != 0) { ippsFree(X_Im); X_Im = 0; }
	if (Y_Re != 0) { ippsFree(Y_Re); Y_Re = 0; }								if (Y_Im != 0) { ippsFree(Y_Im); Y_Im = 0; }
	if (Coh_temp != 0) { ippsFree(Coh_temp); Coh_temp = 0; }
}

PlugIn::~PlugIn(void)
{

}


bool __stdcall PlugIn::LEInfoIO(int index,int type, char *StrInfo)
{
	if(type==INPUT) sprintf(StrInfo,"In[%d]",index);
	if(type==OUTPUT) sprintf(StrInfo,"Out[%d]",index);
	return true;
}

int __stdcall PlugIn::LESetDefPin(int index,int type, PinType *Info)
{
	if (type==OUTPUT) 
	{
		Info->DataType=PLAYBUFFER;
		Info->Exclusive=true; 
		Info->DataLen=FrameSize;
		Info->MaxDataLen=FrameSize;
		return OUTPUT; 
	}

	if (type==INPUT) 
	{
		Info->DataType=PLAYBUFFER;
		Info->Exclusive=true; 
		Info->DataLen=FrameSize;
		Info->MaxDataLen=FrameSize;
		return INPUT; 
	}

	return -1;
}

int  __stdcall PlugIn::LEConnectionRequest(int IOType,int Index,PinType *NewType)
{
	return 0;
}

LPVOID __stdcall PlugIn::LEOnNUTechMessage(int MessageType,int MessageID,WPARAM wParam,LPARAM lParam)
{
	return 0;
}

void __stdcall PlugIn::LESetName(char *Name)
{
	strncpy(Name, NUTS_NAME, MAXNAME);
}

void __stdcall PlugIn::LESetParameter(int Index,void *Data,LPVOID bBroadCastInfo)
{

}

int  __stdcall PlugIn::LEGetParameter(int Index,void *Data)
{
	return 0;
}

void __stdcall PlugIn::LESaveSetUp()
{
	
}

void __stdcall PlugIn::LELoadSetUp()
{

}

void __stdcall PlugIn::LERTWatchInit()
{

}

void __stdcall PlugIn::LESampleRateChange(int NewVal,int TrigType)
{
	if(TrigType==AUDIOPROC)
	{
		if(NewVal!=SampleRate)
		{
			SampleRate = NewVal;
		}
	}

} 

void __stdcall PlugIn::LEFrameSizeChange (int NewVal,int TrigType)
{
	if(TrigType==AUDIOPROC)
	{
		if(NewVal!=FrameSize)
		{
			FrameSize = NewVal;
		}
	}
} 

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////     LOADER      ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

LEEffect * __stdcall LoadEffect(InterfaceType _CBFunction,void *PlugRef,HWND ParentDlg)
{
	return new PlugIn(_CBFunction,PlugRef,ParentDlg);
}

int __stdcall UnLoadEffect(PlugIn *effect)
{
	delete effect;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////     GetStartUpInfo      //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void __stdcall LENUTSDefProps(char *NameEffect,int *Width, void *data)
{
	
	strncpy(NameEffect,NUTS_NAME,MAXNAME);
	*Width=WIDTHDEF;

	if (data!=0)
	{
		StartUpNUTSProps *Info=(StartUpNUTSProps *)data;
		Info->NumInStartUp=1;
		Info->NumOutStartUp=1;
		Info->BitMaskProc = AUDIOPROC;
		Info->BitMaskDriver = OFFLINEDRIVER | DIRECTDRIVER | ASIODRIVER;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////     MY_NEW_FUNCTIONS      ////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void PlugIn::Bilateral_R_ANC(double* xn_buf, double* xf_buf, double* en, double* OutputData_R)
{

	// --- Setting buffers for BILATERAL
	for (int k = 0; k < filterOrder - 1; k++) // Fist of all, inserting the xR/L_old samples... (199 in total: [0-198])
	{
		xn_buf[k] = xR_old[k];
		xf_buf[k] = xRR_old[k];
	}
	for (int k = 0; k < FrameSize; k++)  // ... and then the samples of the current frame xn (from 199 to FrameSize + half_filterOrder  )
	{
		xn_buf[k + filterOrder - 1] = xR[k];
		xf_buf[k + filterOrder - 1] = xRR[k];
	}

	int idx_start;
	double yRn, zn, sn1, e2n, den;

	// Start filtering

	for (int n = 0; n < FrameSize; n++)
	{

		idx_start = n + filterOrder - 1;
		//idx_end = n;

		// xTaps Adaptive filter input
		for (int k = 0; k < filterOrder; k++) // Process sample by sample
		{
			xTaps[k] = xn_buf[idx_start - k]; //400 samples: [399 - 0]
		}

		// Adaptive filter output
		ippsDotProd_64f(wR_Taps, xTaps, filterOrder, &yRn); // proprio come in Matlab

		// Anti-noise signal --- Signal sent to secondary path
		zn = yRn + an_R[n];
		OutputData_R[n] = zn * 32767.0;
		
		// Secondary Path filtering
		//ippsFIRSR_64f(&zn, &sn1, one_sample, SR_pSpec, SR_dly_src, SR_dly_dst, SR_pBuf);
		//ippsCopy_64f(SR_dly_dst, SR_dly_src, paths_len - 1);

		// Error signal ---- received from err mic, so we don't need to calculate it
		//en = dR[n] - sn1; // but we want the dR signal
		//dR[n] = en[n] + sn1;

		// Pseudo error signal
		e2n = en[n] - an2_R[n];

		// Windowing filtered-reference vector 
		for (int k = 0; k < filterOrder; k++) // Process sample by sample
		{
			xBil[k] = xf_buf[idx_start - k]; //400 samples: [399 - 0]
		}

		// --------------------------- Update Adaptive filter coefficients for the current channel ------------------------------- 
		//
		// NLMS denominator
		ippsNorm_L2_64f(xBil, filterOrder, &den);
		den = den * den + delta; //den = den^2 
		// FxNLMS
		ippsDivC_64f(xBil, den, div, filterOrder); // --- div = xBil/den
		e2n = e2n * mu; // ___ e2n = mu * e2n
		ippsMulC_64f(div, e2n, update, filterOrder); // --- update = e2n * div
		ippsAdd_64f_I(update, wR_Taps, filterOrder); // --- w = w + update

	}

	return;
}



void PlugIn::Bilateral_L_ANC(double* xn_buf, double* xf_buf, double* en, double* OutputData_L)
{

	// --- Setting buffers for BILATERAL
	for (int k = 0; k < filterOrder - 1; k++) // Fist of all, inserting the xR/L_old samples... (199 in total: [0-198])
	{
		xn_buf[k] = xL_old[k];
		xf_buf[k] = xLL_old[k];
	}
	for (int k = 0; k < FrameSize; k++)  // ... and then the samples of the current frame xn (from 199 to FrameSize + half_filterOrder  )
	{
		xn_buf[k + filterOrder - 1] = xL[k];
		xf_buf[k + filterOrder - 1] = xLL[k];
	}

	int idx_start;
	double yLn, zn, sn1, e2n, den;

	// Start filtering

	for (int n = 0; n < FrameSize; n++)
	{

		idx_start = n + filterOrder - 1;
		//idx_end = n;

		// xTaps Adaptive filter input
		for (int k = 0; k < filterOrder; k++) // Process sample by sample
		{
			xTaps[k] = xn_buf[idx_start - k]; //400 samples: [399 - 0]
		}

		// Adaptive filter output
		ippsDotProd_64f(wL_Taps, xTaps, filterOrder, &yLn); // proprio come in Matlab

		// Anti-noise signal ---- Signal sent to secondary path
		zn = yLn + an_L[n];

		// Secondary Path filtering
		//ippsFIRSR_64f(&zn, &sn1, one_sample, SR_pSpec, SR_dly_src, SR_dly_dst, SR_pBuf);
		//ippsCopy_64f(SR_dly_dst, SR_dly_src, paths_len - 1); 
		 
		// Error signal ---- received from err mic, so we don't need to calculate it
		//en = dR[n] - sn1; // but we want the dR signal
		//dL[n] = eL[n] + sn1;

		OutputData_L[n] = zn * 32767.0;
		//OutputData_L[n] = sn1 * 32767.0; // anti-noise return to the headphones

		// Pseudo error signal
		e2n = en[n] - an2_L[n];

		// Windowing filtered-reference vector 
		for (int k = 0; k < filterOrder; k++) // Process sample by sample
		{
			xBil[k] = xf_buf[idx_start - k]; //400 samples: [399 - 0]
		}

		// --------------------------- Update Adaptive filter coefficients for the current channel ------------------------------- 
		//
		// NLMS denominator
		ippsNorm_L2_64f(xBil, filterOrder, &den);
		den = den * den + delta; //den = den^2 
		// FxNLMS
		ippsDivC_64f(xBil, den, div, filterOrder); // --- div = xBil/den
		e2n = e2n * mu; // ___ e2n = mu * e2n
		ippsMulC_64f(div, e2n, update, filterOrder); // --- update = e2n * div
		ippsAdd_64f_I(update, wL_Taps, filterOrder); // --- w = w + update
	}

	return;
}

void PlugIn::Binaural_ANC(double* xR_buf, double*xL_buf, double* OutputData_R, double* OutputData_L)
{

	// --- Setting buffers for BINAURAL
	int M = 0;
	for (int k = 0; k < half_filterOrder - 1; k++) // Fist of all, inserting the xR/L_old samples... (199 in total: [0-198])
	{
		M = half_filterOrder + k; //We need only 199 last samples of old parts.

		xR_buf[k] = xR_old[M];
		xL_buf[k] = xL_old[M];
		xRR_buf[k] = xRR_old[M];
		xRL_buf[k] =	xRL_old[k]; // xRL used only for Binaural
		xLR_buf[k] =	xLR_old[k]; // xLR used only for Binaural
		xLL_buf[k] = xLL_old[M];
	}
	for (int k = 0; k < FrameSize; k++)  // ... and then the samples of the current frame xn (from 199 to FrameSize + half_filterOrder  )
	{
		xR_buf[k + half_filterOrder - 1] = xR[k];
		xL_buf[k + half_filterOrder - 1] = xL[k];
		xRR_buf[k + half_filterOrder - 1] = xRR[k];
		xRL_buf[k + half_filterOrder - 1] = xRL[k];
		xLR_buf[k + half_filterOrder - 1] = xLR[k];
		xLL_buf[k + half_filterOrder - 1] = xLL[k];
	}


	double eRn = 0.0, eLn = 0.0, yRn = 0.0, yLn = 0.0;
	double zRn = 0.0, zLn = 0.0;
	double sRn1 = 0.0, sLn1 = 0.0;
	double eRn2 = 0.0, eLn2 = 0.0;
	double den = 0.0;
	int idx_start = 0;

	for (int n = 0; n < FrameSize; n++) // Process sample by sample
	{
		idx_start = n + half_filterOrder - 1;

		// Creating xTaps = [xL , xR]  
		for (int k = 0; k < half_filterOrder; k++) // Process sample by sample
		{
			xTaps[k] = xL_buf[idx_start - k]; //200 campioni --- Exanple: xRTaps [0:199] = xR[399:-1:200] 
			xTaps[k + half_filterOrder] = xR_buf[idx_start - k];
		}

		// Adaptive filter output
		ippsDotProd_64f(wR_Taps, xTaps, filterOrder, &yRn); // proprio come in Matlab
		ippsDotProd_64f(wL_Taps, xTaps, filterOrder, &yLn);

		//Anti-noise signal --- Signal sent to secondary path
		zRn = yRn + an_R[n];
		zLn = yLn + an_L[n];

		OutputData_R[n] = zRn * 32767.0;
		OutputData_L[n] = zLn * 32767.0;

		// Secondary Path filtering
		//ippsFIRSR_64f(&zRn, &sRn1, one_sample, SR_pSpec, SR_dly_src, SR_dly_dst, SR_pBuf);
		//ippsCopy_64f(SR_dly_dst, SR_dly_src, paths_len - 1);
		//ippsFIRSR_64f(&zLn, &sLn1, one_sample, SL_pSpec, SL_dly_src, SL_dly_dst, SL_pBuf);
		//ippsCopy_64f(SL_dly_dst, SL_dly_src, paths_len - 1);

		// Error signal
		//eRn = dR[n] - sRn1;
		//eLn = dL[n] - sLn1;

		// Pseudo error signal
		eRn2 = eRn - an2_R[n];
		eLn2 = eLn - an2_L[n];

		// ---  Calculate xBin_R and xBin_L

		//idx_start = n + half_filterOrder - 1;

		for (int k = 0; k < half_filterOrder; k++) //--vettori da 400 campioni
		{
			xBin_R[k] = xRR_buf[idx_start - k]; // -------------------  xBin_R = [xRR,xRL]; 
			xBin_R[k + half_filterOrder] = xRL_buf[idx_start - k];

			xBin_L[k] = xLL_buf[idx_start - k]; // ---------------  xBin_L = [xLL,xLR];
			xBin_L[k + half_filterOrder] = xLR_buf[idx_start - k];
		}

		// --------------------------- Update Adaptive filter coefficients for the current channel -------------------------------
		// NLMS denominator // -------------------------- RIGHT CHANNEL --------------------
		ippsNorm_L2_64f(xBin_R, filterOrder, &den);
		den = den * den + delta; //den = den^2 
		// FxNLMS
		ippsDivC_64f(xBin_R, den, div, filterOrder); // --- div = xBil/den
		eRn2 = eRn2 * mu; // ___ e2n = mu * e2n
		ippsMulC_64f(div, eRn2, update, filterOrder); // --- update = e2n * div
		ippsAdd_64f_I(update, wR_Taps, filterOrder); // --- w = w + update

		// NLMS denominator // -------------------------- LEFT CHANNEL --------------------
		ippsNorm_L2_64f(xBin_L, filterOrder, &den);
		den = den * den + delta; //den = den^2 
		// FxNLMS
		ippsDivC_64f(xBin_L, den, div, filterOrder); // --- div = xBil/den
		eLn2 = eLn2 * mu; // ___ e2n = mu * e2n
		ippsMulC_64f(div, eLn2, update, filterOrder); // --- update = e2n * div
		ippsAdd_64f_I(update, wL_Taps, filterOrder); // --- wL = wL + update

		//eR[n] = eRn; 
		//eL[n] = eLn;
	}

	return ;
}



double PlugIn :: coherence(double* x, double* y) {

	//Hann window
	//win = hann(segmentLength, 'periodic').';

	//Setting to zero all the array
	//Coh[0] = 0.0;
	ippsZero_64f(Pxx, nfft);
	ippsZero_64f(Pyy, nfft);
	ippsZero_64f(Pxy_Re, nfft); ippsZero_64f(Pxy_Im, nfft);

	int idx = 0;
	int numBin = nfft/2+1; // = 513
	double Pxx_curr = 0.0, Pyy_curr = 0.0;
	double Pxy_Re_curr = 0.0, Pxy_Im_curr = 0.0;

	for (int i = 1; i <= numSegments; i++) {

		// x and y segnments
		//x_seg = x(idx : idx + segmentLength - 1);
		//y_seg = y(idx : idx + segmentLength - 1);

		for (int k = 0; k < segmentLength; k++)
		{
			x_seg[k] = x[idx + k];
			y_seg[k] = y[idx + k];
		}

		//Windowing
		//x_win = x_seg.*win;
		//y_win = y_seg.*win;

		// FFT of X and Y signal
		ippsZero_64f(x_im, nfft);
		ippsZero_64f(y_im, nfft);
		ippsFFTFwd_CToC_64f(x_seg, x_im, X_Re, X_Im, fftState, fft_pBuf); //X = fft(x_win, nfft);
		ippsFFTFwd_CToC_64f(y_seg, y_im, Y_Re, Y_Im, fftState, fft_pBuf); //Y = fft(y_win, nfft);
		// ------------------
		// With this configuration we are considerating separate real ad immaginary parts:
		// - X = X_Re + i X_Im
		// - Y = Y_Re + i Y_Im
		// I'm calculating 
		
		for (int k = 0; k < numBin; k++){

			// Auto Power Spectral Density 
			Pxx_curr = X_Re[k] * X_Re[k] + X_Im[k] * X_Im[k]; //Pxx_curr = abs(X). ^ 2;
			Pyy_curr = Y_Re[k] * Y_Re[k] + Y_Im[k] * Y_Im[k]; //Pyy_curr = abs(Y). ^ 2;
			// Cross Power Spectral Density
			Pxy_Re_curr = X_Re[k] * Y_Re[k] + X_Im[k] * Y_Im[k]; //Pxy_curr = X.*conj(Y);
			Pxy_Im_curr = X_Im[k] * Y_Re[k] - X_Re[k] * Y_Im[k];
			
			//Accomulating values
			Pxx[k] = Pxx[k] + Pxx_curr;
			Pyy[k] = Pyy[k] + Pyy_curr;
			Pxy_Re[k] = Pxy_Re[k] + Pxy_Re_curr;
			Pxy_Im[k] = Pxy_Im[k] + Pxy_Im_curr;
		}
		
		// At this point in Matlab, we can see Pxx --> Pxx_curr, Pyy --> Pyy_curr and Pxy --> Pxy_curr  

		idx = idx + hopSize;
	}

	// Meaning values
	for (int k = 0; k < numBin; k++) {
		Pxx[k] = Pxx[k] / numSegments; //Pxx = Pxx / numSegments;
		Pyy[k] = Pyy[k] / numSegments; //Pyy = Pyy / numSegments;
		Pxy_Re[k] = Pxy_Re[k] / numSegments; //Pxy = Pxy / numSegments;
		Pxy_Im[k] = Pxy_Im[k] / numSegments;
	}
	
	

	// Coherence calculation
	double num = 0.0, den = 0.0;
	double Coh = 0.0;

	for (int k = 0; k < numBin; k++) {

		num = Pxy_Re[k] * Pxy_Re[k] + Pxy_Im[k] * Pxy_Im[k]; //Pxy = abs(Pxy). ^ 2; ---> in Matlab  è num
		den = Pxx[k] * Pyy[k]; //Pyy = Pxx.*Pyy; ---> den in Matlab

		// Necessary only for debugging
		Coh_temp[k] = num / den; //Coh_temp = num . / den;

		Coh = Coh + Coh_temp[k];
	}


	//Coh = mean(Coh_temp);
	Coh = Coh / numBin;
	

	return Coh;
}