/******************************************************************************
 * Program:   EEGsource.EXE                                                   *
 * Module:    GenericADC.CPP                                                  *
 * Comment:   Definition for the GenericADC class                             *
 * Version:   0.01                                                            *
 * Author:    Gerwin Schalk                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.01 - 05/11/2000 - First start                                           *
 * V0.02 - 05/26/2000 - changed **RawEEG to *GenericIntSignal                 *
 ******************************************************************************/

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "GenericADC.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


GenericADC::GenericADC()
{
 signal=NULL;
}


GenericADC::~GenericADC()
{
 if (signal) delete signal;
}


// **************************************************************************
// Function:   GetSampleRate
// Purpose:    returns the sample rate that this ADC samples at
// Parameters: N/A
// Returns:    sample rate
// **************************************************************************
int GenericADC::GetSampleRate()
{
 return(samplerate);
}

