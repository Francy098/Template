#pragma once
#include "stdafx.h"
#include "coherence.h"
#include "ipp.h"


double coherence(double* Coh, double* x, double* y, int FrameSize) {

	/*
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% IMPLEMENTAZIONE VECCHIA %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	//int one_sample = 1;
	//int FrameSize = fftLen;
	double* Gaa, * Gbb, * Gab;

	Gaa = 0; Gbb = 0; Gab = 0;

	if (Gaa == 0) { Gaa = ippsMalloc_64f(fftLen); ippsZero_64f(Gaa, fftLen); }
	if (Gbb == 0) { Gbb = ippsMalloc_64f(fftLen); ippsZero_64f(Gbb, fftLen); }
	if (Gab == 0) { Gab = ippsMalloc_64f(fftLen); ippsZero_64f(Gab, fftLen); }

	// Gaa : Auto-spectral density of x ------- Gaa(f) = X(f)*conj(X(f)) = normL2(X(f))^2
	//ippsNorm_L2_64f(Xn, fftLen, Gaa); // ------------------------ Gaa
	//ippsSqr_64f_I(Gaa, one_sample);
	//ippsCplxToReal_64fc(Xn, Re, Im, fftLen);
	//ippsPowerSpectr_64f(Re,Im,Gaa,fftLen);
	//Gaa = Xn;
	//
	ippsCopy_64f(Xn, Gaa, fftLen);
	ippsMulPackConj_64f_I(Gaa, Gaa, fftLen); 
	//ippsPowerSpectr_64fc(Xn, Gaa, fftLen);
	//
	// Gbb : Auto-spectral density of en //------------------------ Gbb
	//ippsNorm_L2_64f(En, fftLen, Gbb);  
	//ippsSqr_64f_I(Gbb, one_sample);
	//ippsCplxToReal_64fc(En, Re, Im, fftLen);
	//ippsPowerSpectr_64f(Re, Im, Gbb, fftLen);
	//Gbb = En;
	ippsCopy_64f(En, Gbb, fftLen);
	ippsMulPackConj_64f_I(Gbb, Gbb, fftLen);
	//ippsPowerSpectr_64fc(En, Gbb, fftLen);
	//
	//Cross-spectral density ------- // ------------------------ Gab
	//ippsConj_64fc_I(En, fftLen);
	//ippsMul_64f(Xn, En, Gab, fftLen);					
	//Gab = En;
	ippsCopy_64f(En, Gab, fftLen);
	ippsMulPackConj_64f_I(Xn, Gab, fftLen);
	//ippsAbs_64f_A53(,);
	//ippsConj_64fc_A53(En, En, fftLen); //En = conj(En)
	//ippsMul_64fc(Xn, En, Xn, fftLen); //
	//ippsPowerSpectr_64fc(Xn, Gab, fftLen);
	ippsSqr_64f_I(Gab, fftLen);
	//
	// Cab = |Gab|^2 / Gaa*Gbb
	ippsMul_64f_I(Gbb, Gaa, fftLen); //Gaa = Gaa*Gbb
	double delta = 0.000001; // per evitare valori assurdi alti
	ippsAddC_64f_I(delta,Gaa,fftLen);
	ippsDiv_64f(Gaa, Gab, Gab, fftLen);

	ippsMean_64f(Gab, fftLen, Cdir); //------------------------------------- PROBLEMA!!!

	if (Gaa != 0) { ippsFree(Gaa); }
	if (Gbb != 0) { ippsFree(Gbb); }
	if (Gab != 0) { ippsFree(Gab); }


	*/
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% IMPLEMENTAZIONE NUOVA %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	//Setting parameters
	int segmentLength = 1024; // FrameSize / 4
	int overlap = 512;
	int hopSize = segmentLength - overlap;
	int nfft = 1024; // FrameSize / 4

	//Hann window
	//win = hann(segmentLength, 'periodic').';

	//Number of segments
	int numSegments = round((FrameSize - overlap) / hopSize);

	// Spectral accomulators
	//Pxx = zeros(1, nfft); Pyy = zeros(1, nfft); Pxy = zeros(1, nfft);
	double* Pxx, * Pyy, * Pxy;
	Pxx = 0; Pyy = 0; Pxy = 0;

	if (Pxx == 0) { Pxx = ippsMalloc_64f(nfft); ippsZero_64f(Pxx, nfft); }
	if (Pyy == 0) { Pyy = ippsMalloc_64f(nfft); ippsZero_64f(Pyy, nfft); }
	if (Pxy == 0) { Pxy = ippsMalloc_64f(nfft); ippsZero_64f(Pxy, nfft); }

	double* x_seg, * y_seg;
	x_seg = 0; y_seg = 0;
	if (x_seg == 0) { x_seg = ippsMalloc_64f(segmentLength); ippsZero_64f(x_seg, segmentLength); }
	if (y_seg == 0) { y_seg = ippsMalloc_64f(segmentLength); ippsZero_64f(y_seg, segmentLength); }


	double* X, * Y;
	X = 0; Y = 0;
	if (X == 0) { X = ippsMalloc_64f(nfft); ippsZero_64f(X, nfft); }
	if (Y == 0) { Y = ippsMalloc_64f(nfft); ippsZero_64f(Y, nfft); }

	int idx = 1;

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
		ippsFFTFwd_RToPack_64f(x_seg, X, );
		Y = fft(y_win, nfft);

		//Auto Power Spectral Density
		Pxx_curr = abs(X). ^ 2;
		Pyy_curr = abs(Y). ^ 2;

		// Cross Power Spectral Density
		Pxy_curr = X.*conj(Y);

		//Accumulation
		Pxx = Pxx + Pxx_curr;
		Pyy = Pyy + Pyy_curr;
		Pxy = Pxy + Pxy_curr;

		idx = idx + hopSize;
	}

	Pxx = Pxx / numSegments;
	Pyy = Pyy / numSegments;
	Pxy = Pxy / numSegments;

	// Coherence calculation
	num = abs(Pxy). ^ 2;
	den = Pxx .* Pyy;

	Coh_temp = num ./ den;

	Coh = mean(Coh_temp);



	if (Pxx != 0) { ippsFree(Pxx); }
	if (Pyy != 0) { ippsFree(Pyy); }
	if (Pxy != 0) { ippsFree(Pxy); }
	if (x_seg != 0) { ippsFree(x_seg); }
	if (y_seg != 0) { ippsFree(y_seg); }
	if (X != 0) { ippsFree(X); }
	if (Y != 0) { ippsFree(Y); }


	return Coh[0] ; //Restituiamo valore della coerenza
}


