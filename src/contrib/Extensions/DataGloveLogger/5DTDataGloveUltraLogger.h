/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef DataGloveLoggerH
#define DataGloveLoggerH

#include "Environment.h"
#include "OSThread.h"

#include "fglove.h"

#include <list>
#include <string>
using namespace std;

typedef fdGlove*        (*FDOPEN)                   (char    *pPort);
typedef int             (*FDCLOSE)                  (fdGlove *pFG  );
typedef int             (*FDGETGLOVETYPE)           (fdGlove *pFG  );
typedef int             (*FDGETNUMSENSORS)          (fdGlove *pFG  );
typedef int             (*FDGETGLOVEHAND)           (fdGlove *pFG  );
typedef int             (*FDSCANUSB)                (unsigned short *aPID,  int &nNumMax);
typedef void            (*FDGETSENSORRAWALL)        (fdGlove *pFG,          unsigned short *pData);
typedef unsigned short  (*FDGETSENSORRAW)           (fdGlove *pFG,          int nSensor);

const int MAX_SENSORS = 14;
const int SENSOR_PRECISION = 12;

class DataGloveLogger;
class DataGloveThread;

class DataGloveThread : public OSThread
{
	public:
				            DataGloveThread(const DataGloveLogger *logger, int usbPortNumber, bool deriv);
		virtual             ~DataGloveThread();
		virtual int         Execute();

		virtual std::string GetError();
		virtual int         GetHandedness();
		virtual void        SetIndex(int i);
		virtual void        Cleanup();

	private:
		const DataGloveLogger      *mpLogger;
		fdGlove                    *mpGlove;
		std::stringstream           m_err;
		std::string                 m_portString;
		int                         m_portNumber;
		int                         m_type;
		int                         m_hand;
		int                         m_sensors;
		int                         m_index;
		bool                        m_deriv;
		unsigned short              mPreviousReading[MAX_SENSORS];
		unsigned short              mPreviousOutput[MAX_SENSORS];
};
typedef std::list<DataGloveThread*> DataGloveThreadList;

class DataGloveLogger : public EnvironmentExtension
{
	public:
		        DataGloveLogger();
		virtual ~DataGloveLogger();
		virtual void Publish();
		virtual void Preflight() const;
		virtual void Initialize();
		virtual void StartRun();
		virtual void StopRun();
		virtual void Halt();

		virtual void OpenInterface();
		virtual int  CountGloves() const;
		virtual void GetGloves(DataGloveThreadList & available) const;
		virtual void AssignGloves(DataGloveThreadList & assigned) const;
		virtual void EmptyList(DataGloveThreadList & gloves) const;

	private:

		HINSTANCE                   m_hinstLib;
		FDOPEN                      m_fdOpenCall;
		FDCLOSE                     m_fdCloseCall;
		FDSCANUSB                   m_fdScanUSBCall;
		FDGETGLOVETYPE              m_fdGetGloveTypeCall;
		FDGETGLOVEHAND              m_fdGetGloveHandCall;
		FDGETNUMSENSORS             m_fdGetNumSensorsCall;
		FDGETSENSORRAWALL           m_fdGetSensorRawAllCall;
		FDGETSENSORRAW              m_fdGetSensorRawCall;

		bool                        m_enabled;
		bool                        m_deriv;
		DataGloveThreadList         mGloves;

	friend class DataGloveThread;
};

#endif // DataGloveLoggerH
