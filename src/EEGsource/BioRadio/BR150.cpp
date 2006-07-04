/******************************************************************************
 * $Id$                                                                       *
 * Program:   BioRadio.exe                                                    *
 * Module:    BR150.CPP                                                       *
 * Comment:   Code using the SDK for the BioRadio150                          *
 * Version:   0.05                                                            *
 * Author:    Yvan Pearson Lecours                                            *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.01 - 12/15/2005 - First start                                           *
 * V0.02 - 1/1/2006   - Updated code for BCI integration                      *
 * V0.03 - 5/1/2006   - Mod for production, clean up                          *
 * $Log$
 * Revision 1.1  2006/07/04 18:44:25  mellinger
 * Put files into CVS.
 *                                                                      *
 ******************************************************************************/
#include "PCHIncludes.h"
#pragma hdrstop

#include "BR150.h"
#include "BioRadio150_SDK/BioRadio150DLL.h"
#include "bioutils.h"

#include <string>

using namespace std;

// Static vars
DWORD BR150::br150        = NULL;
bool  BR150::runningState = NOT_RUNNING;


// **************************************************************************
// Constructor: Class BR150
// **************************************************************************
BR150::BR150()
{
  numRead                = ZERO;
  flags[COM_STATUS]      = ZERO;
  flags[PING_STATUS]     = ZERO;
  flags[PROGRAM_SUCCESS] = ZERO;
  purge();
}

// **************************************************************************
// Deconstructor: Class BR150
// **************************************************************************
BR150::~BR150()
{
}

// **************************************************************************
// Method:      start
// Purpose:     (1) Changes running state (2) Creates bioradio object
//              (3) Starts commumnication (4) Pings bioradio
//              (5) Sets bad data value   (6) returns error if comm. failed
// Parameters:  port value  (char*)
// Returns:    error or no error (int)
// **************************************************************************
int BR150::start(char *pt)
{
  port = pt;
  if(runningState == RUNNING)
    stop();

  br150 = CreateBioRadio();
  flags[COM_STATUS] = StartAcq(br150,DISPLAY_PROGRESS,port);

  if(flags[COM_STATUS])
  {
    flags[PING_STATUS] = PingConfig(br150,DISPLAY_PROGRESS);
    if(flags[PING_STATUS])
    {
      SetBadDataValue(br150,BAD_DATA);
      SetFreqHoppingMode(br150,HOP);
      runningState = RUNNING;
      return XNO_ERROR;
    }
    else
      return XERROR;
   }
  else
    return XERROR;
}

// **************************************************************************
// Method:      stop
// Purpose:     (1) Changes running state (2) Destroys bioradio object
//              (3) Stops commumnication  (6) returns error if comm. failed
// Parameters:  void
//
// Returns:    error or no error (int)
// **************************************************************************
int    BR150::stop(void)
{
  if(runningState)
  {
    StopAcq(br150);
    DestroyBioRadio(br150);
    runningState = NOT_RUNNING;
    return XNO_ERROR;
  }
  else
    return XERROR;
}

// **************************************************************************
// Method:      program
// Purpose:     (1) Programs bioradio using config file
// Parameters:  config file  (string)
// Returns:    error or no error (int)
// **************************************************************************
int    BR150::program(const string& config)
{
  flags[PROGRAM_SUCCESS] = ProgramConfig(br150,DISPLAY_PROGRESS,config.c_str());
  if(flags[PROGRAM_SUCCESS])
    return XNO_ERROR;
  else
    return XERROR;

}

// **************************************************************************
// Method:      bufferMerge
// Purpose:     (1) Copies data from one buffer to a larger buffer
// Parameters:  buffer1 [First buffer] (double*),
//              buffer2 [Second buffer](double*)
//              start [Starting point of buffer] (int),
//              finish [end point of buffer] (int)
// Returns:    void
// **************************************************************************
void BR150::bufferMerge(double* buffer1,double* buffer2, int start, int finish)
{
  for(int i = 0; i < finish; i++)
    *(buffer1+start+i) = *(buffer2+i);
}

// **************************************************************************
// Method:      getData
// Purpose:     (1) Inits a samples counter (2) Inits a temp buffer
//              (3) Collects data in temp buffer
//              (4) Copies data from small buffer to larger buffer
//              (5) Adds all samples collected
// Parameters:  block [sample block size] (int), chans [channels enabled] (int)
// Returns:    returns pointer to filled data buffer (double*)
// **************************************************************************
double*    BR150::getData(int block, int chans)
{
 int countSamplesCollected = ZERO;
 double tempBuffer[BUFFER_SIZE];

  while(countSamplesCollected < block*chans)
  {
    while(TransferBuffer(br150) == ZERO)
      Sleep(SLEEP);

    ReadScaled(br150,tempBuffer,BUFFER_SIZE,&numRead);
    bufferMerge(data,tempBuffer,countSamplesCollected,numRead);
    countSamplesCollected = numRead + countSamplesCollected;
  }

  numRead = countSamplesCollected;
  return data;
}

// **************************************************************************
// Method:      getData
// Purpose:     (1) Collects data into a buffer
// Parameters:  void
// Returns:    returns pointer to filled data buffer (double*)
// **************************************************************************
double*    BR150::getData(void)
{
  while(TransferBuffer(br150) == ZERO)
    Sleep(SLEEP);

  ReadScaled(br150,data,BUFFER_SIZE,&numRead);
  return data;
}

// **************************************************************************
// Method:      samplesRead
// Purpose:     (1) Returns the number of samples collected
// Parameters:  void
// Returns:    Returns the number of samples collected (int)
// **************************************************************************
int BR150::samplesRead(void)
{
  return numRead;
}

// **************************************************************************
// Method:      purge
// Purpose:     (1) Sets all values in the data buffer to zero
// Parameters:  void
// Returns:     void
// **************************************************************************
void BR150::purge(void)
{
  for(int i = ZERO; i < BUFFER_SIZE; i++)
    data[i] = ZERO;
}


// **************************************************************************
// Method:      getState
// Purpose:     (1) Sets a static bool on whether or not the bioradio is
//              running
// Parameters:  void
// Returns:     runningState [State of the Bioradio150] (bool)
// **************************************************************************
bool BR150::getState(void)
{
  return runningState;
}


