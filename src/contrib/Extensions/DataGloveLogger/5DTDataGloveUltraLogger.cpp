/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIEvent.h"
#include "5DTDataGloveUltraLogger.h"
#include <cstdio>
#include <iomanip>

Extension( DataGloveLogger );

DataGloveLogger::DataGloveLogger()
{
	m_hinstLib                = NULL;
	m_fdOpenCall              = NULL;
	m_fdCloseCall             = NULL;
	m_fdScanUSBCall           = NULL;
	m_fdGetGloveTypeCall      = NULL;
	m_fdGetGloveHandCall      = NULL;
	m_fdGetNumSensorsCall     = NULL;
	m_fdGetSensorRawAllCall   = NULL;
	m_fdGetSensorRawCall      = NULL;
	m_enabled                 = false;
	m_deriv                   = false;
}
DataGloveLogger::~DataGloveLogger()
{
	EmptyList(mGloves);
}
void
DataGloveLogger::OpenInterface()
{
	m_hinstLib = NULL;
	m_hinstLib = LoadLibrary(TEXT("fglove.dll"));
	if (m_hinstLib == NULL) {
		if(m_enabled) bcierr << "Failed to find 5DT DataGlove Ultra dynamic link library glove.dll" << endl;
		return;
	}

	bool good = true;
	good &= ( m_fdOpenCall            = (FDOPEN)            GetProcAddress(m_hinstLib, TEXT("?fdOpen@@YAPAUfdGlove@@PAD@Z")) ) != NULL;
	good &= ( m_fdCloseCall           = (FDCLOSE)           GetProcAddress(m_hinstLib, TEXT("?fdClose@@YAHPAUfdGlove@@@Z")) ) != NULL;
	good &= ( m_fdScanUSBCall         = (FDSCANUSB)         GetProcAddress(m_hinstLib, TEXT("?fdScanUSB@@YAHPAGAAH@Z")) ) != NULL;
	good &= ( m_fdGetGloveTypeCall    = (FDGETGLOVETYPE)    GetProcAddress(m_hinstLib, TEXT("?fdGetGloveType@@YAHPAUfdGlove@@@Z")) ) != NULL;
	good &= ( m_fdGetGloveHandCall    = (FDGETGLOVEHAND)    GetProcAddress(m_hinstLib, TEXT("?fdGetGloveHand@@YAHPAUfdGlove@@@Z")) ) != NULL;
	good &= ( m_fdGetNumSensorsCall   = (FDGETNUMSENSORS)   GetProcAddress(m_hinstLib, TEXT("?fdGetNumSensors@@YAHPAUfdGlove@@@Z")) ) != NULL;
	good &= ( m_fdGetSensorRawAllCall = (FDGETSENSORRAWALL) GetProcAddress(m_hinstLib, TEXT("?fdGetSensorRawAll@@YAXPAUfdGlove@@PAG@Z")) ) != NULL;
	good &= ( m_fdGetSensorRawCall    = (FDGETSENSORRAW)    GetProcAddress(m_hinstLib, TEXT("?fdGetSensorRaw@@YAGPAUfdGlove@@H@Z")) ) != NULL; 

	if(!m_enabled) {
		if(CountGloves()) bciout << "One or more datagloves are connected, but disabled. To enable acquisition, start the source module with --LogDataGlove=1" << endl;
		return;
	}
	if(!good) {
		if(m_enabled) bcierr << "not all the required functions were found in the 5DT DataGlove Ultra dynamic link library fglove.dll" << endl;
		return;
	}
}

int
DataGloveLogger::CountGloves() const
{
	unsigned short aPID[5];
	int nGloves, nNumMax = 5;
	if(m_fdScanUSBCall == NULL) nGloves = 0;
	else nGloves = m_fdScanUSBCall(aPID, nNumMax);
	return nGloves;
}

void
DataGloveLogger::GetGloves(DataGloveThreadList & available) const
{
	EmptyList(available);
	int nGloves = CountGloves();
	for(int iGlove = 0; iGlove < nGloves; iGlove++) {
		DataGloveThread *g = new DataGloveThread(this, iGlove, m_deriv);
		available.push_back(g);
		string err = g->GetError();
		if(err.size()) {
			EmptyList(available);
			bcierr << err.c_str() << endl;
			return;
		}
	}
}

void
DataGloveLogger::AssignGloves(DataGloveThreadList & assigned) const
{
	DataGloveThreadList available;
	GetGloves(available);
	ParamRef handedness = Parameter("DataGloveHandedness"); 
	int nGlovesRequested = handedness->NumValues();
	for(int iRequest = 0; iRequest < nGlovesRequested; iRequest++) {
		string sWanted = handedness(iRequest);
		int hWanted;
		DataGloveThread *found = NULL;
		if(     sWanted == "L" || sWanted == "l" || sWanted == "left"  || sWanted == "LEFT"  || sWanted == "0") hWanted = FD_HAND_LEFT;
		else if(sWanted == "R" || sWanted == "r" || sWanted == "right" || sWanted == "RIGHT" || sWanted == "1") hWanted = FD_HAND_RIGHT;
		else {
			EmptyList(available);
			EmptyList(assigned);
			bcierr << "unrecognized DataGloveHandedness string \"" << sWanted << "\"" << endl;
		}
		for(DataGloveThreadList::iterator g=available.begin(); g != available.end(); g++) {
			if( (*g)->GetHandedness() == hWanted ) { found = *g; break; }
		}
		if(found) {
			available.remove(found);
			assigned.push_back(found);
			found->SetIndex(iRequest+1);
		}
		else {
			EmptyList(available);
			EmptyList(assigned);
			bcierr << "could not find a match for requested glove #" << iRequest+1 << " (handedness requested: " << sWanted << ")" << endl; 
		}
	}
	EmptyList(available);
}

void
DataGloveLogger::EmptyList(DataGloveThreadList & gloves) const
{
	while(gloves.size()) {delete gloves.back(); gloves.pop_back();}
}

void
DataGloveLogger::Publish()
{
	char definition[32];

	m_enabled = ( ( int )OptionalParameter( "LogDataGlove" ) != 0 );
	if( !m_enabled ) return;

	OpenInterface();
	int nGloves = CountGloves(); // needed in order to know how many states to declare.
	string h;
	if(nGloves == 1) { // if 1 glove is connected, since we have this info, helpfully set the "default" value of DataGloveHandedness to the correct value
		DataGloveThreadList g;
		GetGloves(g);
		h = ((g.back()->GetHandedness() == FD_HAND_LEFT) ? "L" : "R");
		EmptyList(g);
	}

	for(int iGlove = 0; iGlove < nGloves; iGlove++) {
		if(nGloves > 1) h = ((iGlove%0)?" L ":" R ") + h; // If an even number of gloves, set the default to L R L R... If odd, set to R L R .... 
		for(int iSensor = 0; iSensor < MAX_SENSORS; iSensor++) {
			sprintf(definition, "Glove%dSensor%02d %d 0 0 0", iGlove+1, iSensor+1, SENSOR_PRECISION);
			BEGIN_EVENT_DEFINITIONS
				definition,
			END_EVENT_DEFINITIONS
		}
	}
	sprintf(definition, "%d", nGloves);
	h = "Source:Log%20Input list    DataGloveHandedness= " + (definition+h) + " // DataGlove handedness: L or R for each glove";
	BEGIN_PARAMETER_DEFINITIONS
		"Source:Log%20Input int     DataGloveDerivative=   0    0  0 1 // measure changes in glove signals?: 0: no - measure position, 1:yes - measure velocity (enumeration)",
		"Source:Log%20Input int     LogDataGlove=          0    0  0 1 // record DataGlove to states (boolean)",
		h.c_str(), // "Source:Log%20Input list    DataGloveHandedness= 1 L    R  % % // DataGlove handedness: L or R for each glove",
	END_PARAMETER_DEFINITIONS
}

void
DataGloveLogger::Preflight() const
{
	bool enabled = ( ( int )OptionalParameter( "LogDataGlove" ) != 0 );
	if(enabled) {
		DataGloveThreadList assigned;
		AssignGloves(assigned);
		EmptyList(assigned); // <sigh>
	}

}

void
DataGloveLogger::Initialize()
{
	m_enabled = ( ( int )OptionalParameter( "LogDataGlove" ) != 0 );
	m_deriv = m_enabled && ( (int)Parameter("DataGloveDerivative") != 0 ); 
}

void
DataGloveLogger::StartRun()
{
	if(m_enabled) {
		AssignGloves(mGloves);
	}
	for(DataGloveThreadList::iterator g = mGloves.begin(); g != mGloves.end(); g++)
		(*g)->Start();
}

void
DataGloveLogger::StopRun()
{
	for(DataGloveThreadList::iterator g = mGloves.begin(); g != mGloves.end(); g++)
		(*g)->Terminate();
	for(DataGloveThreadList::iterator g = mGloves.begin(); g != mGloves.end(); g++)
		while( !(*g)->IsTerminated() )
			::Sleep(1);
	EmptyList(mGloves);
}

void
DataGloveLogger::Halt()
{
	StopRun();
}



DataGloveThread::DataGloveThread(const DataGloveLogger *parent, int usbPortNumber, bool deriv)
{
	m_err.str("");
	for(int i = 0; i < MAX_SENSORS; i++) mPreviousReading[i] = mPreviousOutput[i] = 0;
	
	char ps[10];
	sprintf(ps, "USB%i", usbPortNumber); 
	
	mpLogger       = parent;
	m_portNumber   = usbPortNumber;
	m_portString   = ps;
	m_index        = 0;
	mpGlove        = parent->m_fdOpenCall(ps);
	m_deriv        = deriv;
	if(mpGlove == NULL) {
		m_type = FD_GLOVENONE;
		m_hand = -1;
		m_sensors = -1;
		m_err << "failed to find a data glove on " << m_portString;
	}
	else {
		m_type         = parent->m_fdGetGloveTypeCall(mpGlove);
		m_hand         = parent->m_fdGetGloveHandCall(mpGlove);
		m_sensors     = parent->m_fdGetNumSensorsCall(mpGlove);
		if (m_type != FD_GLOVE5U_USB && m_type != FD_GLOVE14U_USB) {
			m_err << "unrecognized data glove type on " << m_portString << endl;
		}
	}
}

void
DataGloveThread::Cleanup()
{
	if(mpGlove != NULL && mpLogger != NULL && mpLogger->m_fdCloseCall != NULL) {
		mpLogger->m_fdCloseCall(mpGlove);
		mpGlove = NULL;
	}
	m_type = FD_GLOVENONE;
	m_hand = -1;
	m_sensors = -1;
}

DataGloveThread::~DataGloveThread()
{
	Cleanup();
}

void
DataGloveThread::SetIndex(int i)
{
	m_index = i;
}

int
DataGloveThread::GetHandedness()
{
	return m_hand;
}

string
DataGloveThread::GetError()
{
	return m_err.str();
}

int
DataGloveThread::Execute()
{
	unsigned short currentData[MAX_SENSORS];
	while( !IsTerminating() ) {
		for(int i = 0; i < MAX_SENSORS; i++) currentData[i] = 0;

		currentData[0]  = mpLogger->m_fdGetSensorRawCall(mpGlove, FD_THUMBNEAR);
		currentData[1]  = mpLogger->m_fdGetSensorRawCall(mpGlove, FD_INDEXNEAR);
		currentData[2]  = mpLogger->m_fdGetSensorRawCall(mpGlove, FD_MIDDLENEAR);
		currentData[3]  = mpLogger->m_fdGetSensorRawCall(mpGlove, FD_RINGNEAR);
		currentData[4]  = mpLogger->m_fdGetSensorRawCall(mpGlove, FD_LITTLENEAR);

		if (m_type == FD_GLOVE14U_USB) {
			currentData[5]  = mpLogger->m_fdGetSensorRawCall(mpGlove, FD_THUMBFAR);
			currentData[6]  = mpLogger->m_fdGetSensorRawCall(mpGlove, FD_INDEXFAR);
			currentData[7]  = mpLogger->m_fdGetSensorRawCall(mpGlove, FD_MIDDLEFAR);
			currentData[8]  = mpLogger->m_fdGetSensorRawCall(mpGlove, FD_RINGFAR);
			currentData[9]  = mpLogger->m_fdGetSensorRawCall(mpGlove, FD_LITTLEFAR);
			currentData[10] = mpLogger->m_fdGetSensorRawCall(mpGlove, FD_THUMBINDEX);
			currentData[11] = mpLogger->m_fdGetSensorRawCall(mpGlove, FD_INDEXMIDDLE);
			currentData[12] = mpLogger->m_fdGetSensorRawCall(mpGlove, FD_MIDDLERING);
			currentData[13] = mpLogger->m_fdGetSensorRawCall(mpGlove, FD_RINGLITTLE);
		}
		if(m_deriv) {
			for(int i = 0; i < MAX_SENSORS; i++) {
				double diff = (double)currentData[i] - (double)mPreviousReading[i];
				mPreviousReading[i] = currentData[i];
				long min = -(1L << (SENSOR_PRECISION-1));
				long max =  (1L << (SENSOR_PRECISION-1)) - 1;
				diff = (diff < min ? min : diff > max ? max : diff);
				currentData[i] = (unsigned short)(diff - min);
			}
		}

		for(int i = 0; i < MAX_SENSORS; i++) {
			if(currentData[i] != mPreviousOutput[i])
				bcievent << "Glove" << m_index << "Sensor" << setfill('0') << setw(2) << (i+1) << " " << currentData[i];
			mPreviousOutput[i] = currentData[i];
		}
		::Sleep(1);
	}
	::Sleep(10);
	return 0;
}
