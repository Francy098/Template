#pragma once
#include <math.h>
#include "io.h"
#include <fstream>
#include <iostream>

using namespace std;

double coherence(double* Cdir, double* xn, double* en, int fftLen);