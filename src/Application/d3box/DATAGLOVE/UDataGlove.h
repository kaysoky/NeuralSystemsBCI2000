/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef UDataGloveH
#define UDataGloveH
//---------------------------------------------------------------------------

#include <windows.h>
#include <system.hpp>
#include <Classes.hpp>
#include <SyncObjs.hpp>

#define GLOVE_ERR_NOERR         0
#define GLOVE_ERR_RESYNCERR     -1
#define GLOVE_ERR_SENSORNUMERR  -2

#define MAX_GLOVESENSORS        7

// these two variables have to be static, otherwise BuildCommDB does not work. No clue why
static HANDLE        hCOM;
static DCB           dcb;
static COMMTIMEOUTS ctmoNew = {0}, ctmoOld;

class DataGlove : public TThread
{
protected:
//  HANDLE        hCOM;
//  DCB           dcb;
  void __fastcall Execute();
  TCriticalSection *critsec;
  bool  streaming;
  bool  resyncerr;
  int   gloveerr;
  AnsiString COMport;
private:
  unsigned char sensorval[MAX_GLOVESENSORS];
  unsigned char sensorval_old[MAX_GLOVESENSORS];
  bool OpenComPort(AnsiString COMport);
  bool CloseComPort();
  void ReSyncGlove();
public:		// User declarations
  __fastcall DataGlove::DataGlove();
  __fastcall DataGlove::~DataGlove();

  int  GetGloveErr();
  bool GlovePresent(AnsiString COMport);
  bool StartStreaming(AnsiString COMport);
  bool ReadSensorsFromGlove(AnsiString COMport);
  unsigned char GetSensorVal(int sensor);
  unsigned char GetSensorValOld(int sensor);
};

#endif














