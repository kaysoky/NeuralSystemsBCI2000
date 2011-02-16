#ifndef _Check_Consistency_Files_h
#define _Check_Consistency_Files_h

#include <iostream>
#include <string>
#include <vector>
#include "BCI2000FileReader.h"
#include "BCIError.h"

using namespace std;

int Check_Consistency_Files(vector<string> fPathArr, int &numSamples, int &numChannels);

#endif
