//---------------------------------------------------------------------------

#ifndef UDataGloveH
#define UDataGloveH
//---------------------------------------------------------------------------

#include <windows.h>
#include <system.hpp>

#define MAX_GLOVESENSORS        7

// these two variables have to be static, otherwise BuildCommDB does not work. No clue why
static HANDLE        hCOM;
static DCB           dcb;

class DataGlove
{
protected:
//  HANDLE        hCOM;
//  DCB           dcb;
private:
  char          sensorval[MAX_GLOVESENSORS];
  bool OpenComPort(AnsiString COMport);
  bool CloseComPort();
public:		// User declarations
  DataGlove::DataGlove();
  DataGlove::~DataGlove();

  bool ReadSensorsFromGlove(AnsiString COMport);
  unsigned char GetSensorVal(int sensor);
};

#endif












