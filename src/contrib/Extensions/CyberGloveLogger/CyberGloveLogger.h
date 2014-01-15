//////////////////////////////////////////////////////////////////////
// $Id: CyberGloveUltraLogger.h 3360 2011-10-20 19:24:24Z tblakely $
// Authors: Tim Blakely (tim.blakely@gmail.com)
// Description: BCI2000 v3.0+ Logger for CyberGlove
// 
// Version History
// 
//  10/20/2011 tblakely: Initial version 
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
///////////////////////////////////////////////////////////////////////

#ifndef CyberGloveLoggerH
#define CyberGloveLoggerH

#include "Environment.h"
#include "OSThread.h"

#include <list>
#include <string>
using namespace std;

const int MAX_SENSORS = 22;
const int SENSOR_PRECISION = 12;

class CyberGloveLogger;
class CyberGloveThread;

class CyberGloveThread : public OSThread
{
	public:
		CyberGloveThread(const CyberGloveLogger *pLogger, std::string pComPortName);
		virtual         ~CyberGloveThread();
		virtual int     Execute();

		virtual			std::string GetError();
		virtual char    GetHandedness();
		virtual void    Cleanup();

	private:
		HANDLE						pHandle;
		const CyberGloveLogger      *mLogger;
		std::stringstream           m_err;
		std::string                 m_portString;
		char                        m_hand;
		unsigned char               m_sensors;
		int                         m_index;
		unsigned short              mPreviousReading[MAX_SENSORS];
		unsigned short              mPreviousOutput[MAX_SENSORS];

		bool SetComParams(HANDLE pHandle, int pBaud, int pTimeoutMS);
		bool ReadByte(HANDLE pPort, unsigned char *pDest, bool pEcho);
		bool WriteByte(HANDLE pPort, unsigned char pValue, bool pEcho);
		bool IsGlove( HANDLE pPort );
		bool DetectHandedness(HANDLE pPort, char* pHand);
		bool ResetGlove(HANDLE pPort, bool pRightHanded);
		bool GetNumSensors(HANDLE pPort, unsigned char *pNumSensors);
		bool GetSample(HANDLE pPort, unsigned char *pValues, int pNumValues, bool pRightHanded);
};
typedef std::list<CyberGloveThread*> CyberGloveThreadList;

class CyberGloveLogger : public EnvironmentExtension
{
	public:
		        CyberGloveLogger();
		virtual ~CyberGloveLogger();
		virtual void Publish();
		virtual void Preflight() const;
		virtual void Initialize();
		virtual void StartRun();
		virtual void StopRun();
		virtual void Halt();

	private:
		CyberGloveThreadList	mGloves;
		void FindGlovePorts();

		bool SetComParams(HANDLE pHandle, int pBaud, int pTimeoutMS);
		bool ReadByte(HANDLE pPort, unsigned char *pDest, bool pEcho);
		bool WriteByte(HANDLE pPort, unsigned char pValue, bool pEcho);
		bool IsGlove( HANDLE pPort );
		bool DetectHandedness(HANDLE pPort, char* pHand);
		bool ResetGlove(HANDLE pPort, bool pRightHanded);
		bool GetNumSensors(HANDLE pPort, unsigned char *pNumSensors);
		bool GetSample(HANDLE pPort, unsigned char *pValues, int pNumValues, bool pRightHanded);

		virtual void GetGloveThreads(CyberGloveThreadList & mAvailable) ;
		virtual void EmptyList(CyberGloveThreadList & gloves);
		virtual void AssignGloves(CyberGloveThreadList & assigned);

		bool mEnabled;

		std::vector<std::string> mGoodPorts;

		/*virtual void OpenInterface();
		virtual int  CountGloves() const;
		virtual void GetGloves(DataGloveThreadList & available) const;
		virtual void AssignGloves(DataGloveThreadList & assigned) const;
		

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
		DataGloveThreadList         mGloves;*/

	friend class CyberGloveThread;
};

#endif // DataGloveLoggerH
