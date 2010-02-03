/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef DataGlove5DTUFilterH
#define DataGlove5DTUFilterH

#include "GenericFilter.h"
#include "fglove.h"


typedef fdGlove*        (*FDOPEN)                   (char    *pPort);
typedef int             (*FDCLOSE)                  (fdGlove *pFG  );
typedef int             (*FDGETGLOVETYPE)           (fdGlove *pFG  );
typedef int             (*FDGETNUMSENSORS)          (fdGlove *pFG  );
typedef int             (*FDGETGLOVEHAND)           (fdGlove *pFG  );
typedef int             (*FDSCANUSB)                (unsigned short *aPID,  int &nNumMax);
typedef void            (*FDGETSENSORRAWALL)        (fdGlove *pFG,          unsigned short *pData);
typedef unsigned short  (*FDGETSENSORRAW)           (fdGlove *pFG,          int nSensor);


class DataGlove5DTUFilter : public GenericFilter
{
 public:
          DataGlove5DTUFilter();
  virtual ~DataGlove5DTUFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual bool AllowsVisualization() const { return false; }

 private:

  HINSTANCE                         hinstLib;
  FDOPEN                            fdOpenCall;
  FDCLOSE                           fdCloseCall;
  FDSCANUSB                         fdScanUSBCall;
  FDGETGLOVETYPE                    fdGetGloveTypeCall;
  FDGETGLOVEHAND                    fdGetGloveHandCall;
  FDGETNUMSENSORS                   fdGetNumSensorsCall;
  FDGETSENSORRAWALL                 fdGetSensorRawAllCall;
  FDGETSENSORRAW                    fdGetSensorRawCall;
	fdGlove                          *pGlove;
  int                               ret;
	char                             *szPort;
	char	                            szPortToOpen[6];
	int                               glovetype;
	int                               glovehand;
  int                               glovesensors;
  bool                              datagloveenable;

  unsigned short                    sensor_data[14];




};

#endif // DataGlove5DTUFilterH




