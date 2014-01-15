//////////////////////////////////////////////////////////////////////
// $Id: 5DTDataGloveUltraLogger.cpp 3470 2011-08-18 13:18:58Z mellinger $
// Authors: Peter Brunner (pbrunner@wadsworth.org), Jeremy Hill (jezhill@gmail.com)
// Description: BCI2000 v3.0+ Logger for 5DT DataGlove Ultra
// 
// Version History
// 
//  05/17/2007 pbrunner: Initial version (as Filter, not Logger);
//  01/14/2009 pbrunner: Added support for the 14 sensor data glove.
//  01/20/2011 jhill:    Revamped into 5DTDataGloveUltraLogger.cpp
//                       Converted from Filter to Logger
//                       Made v.3.0-compatible
//                       Added support for multiple gloves
//                          (NB: This entailed changes to Parameter and State names) 
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

#include "PCHIncludes.h"
#pragma hdrstop

#include "BCIEvent.h"
#include "CyberGloveLogger.h"
#include <cstdio>
#include <iomanip>

//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>

Extension( CyberGloveLogger );

bool CyberGloveLogger::SetComParams(HANDLE pHandle, int pBaud, int pTimeoutMS) {
	DCB params;
	GetCommState(pHandle, &params);
	params.DCBlength = sizeof(DCB);
	params.BaudRate  = pBaud;
	params.ByteSize  = 8;
	params.StopBits  = ONESTOPBIT;
	params.Parity    = NOPARITY;

	if(!SetCommState(pHandle, &params))
	{
		CloseHandle(pHandle);
		return false;
	}

	COMMTIMEOUTS timeouts; // In milliseconds
	timeouts.ReadIntervalTimeout         = pTimeoutMS;
	timeouts.ReadTotalTimeoutConstant    = pTimeoutMS;
	timeouts.ReadTotalTimeoutMultiplier  = pTimeoutMS;
	timeouts.WriteTotalTimeoutConstant   = pTimeoutMS;
	timeouts.WriteTotalTimeoutMultiplier = pTimeoutMS;

	if(!SetCommTimeouts(pHandle, &timeouts))
	{
		CloseHandle(pHandle);
		return false;
	}

	return true;
}

CyberGloveLogger::CyberGloveLogger()
{
	mEnabled = false;
}
CyberGloveLogger::~CyberGloveLogger()
{
}

bool CyberGloveLogger::WriteByte(HANDLE pPort, unsigned char pValue, bool pEcho = false) {
	DWORD bytesWritten;
	if(pEcho) {
		wprintf(L"WRITING %c (%i)\n", pValue, pValue);
	}
	if(!WriteFile(pPort, &pValue, 1, &bytesWritten, NULL))
		return false;
	return true;
}

bool CyberGloveLogger::ReadByte(HANDLE pPort, unsigned char *pDest, bool pEcho = false){
	DWORD bytesRead;
	if(ReadFile(pPort, pDest, 1, &bytesRead, NULL) && bytesRead == 1 ) {
		if(pEcho) {
			wprintf(L"READ %c (%i)\n", *pDest, *pDest);
		}
		return true;
	}
	return false;
}

bool CyberGloveLogger::ResetGlove(HANDLE pPort, bool pRightHanded) {
	
	unsigned char readByte;
	
	if(!WriteByte(pPort, 18)) {
		return false;
	}

	/*for(int i = 0; i < 1; i++) {
		if(!WriteByte(pPort, cmd[i])) {
			return false;
		}
	}*/
	
	// Assume we reset, flush results
	readByte = 1;
	if(pRightHanded) {
		while(readByte != 0){
			ReadByte(pPort, &readByte);
		}
	}
	else {
		while(readByte != 10){
			ReadByte(pPort, &readByte);
		}
	}
	

	return true;
}

bool CyberGloveLogger::DetectHandedness(HANDLE pPort, char* pHand) {
	bool success = true; 
	success &= WriteByte(pPort,'?');
	success &= WriteByte(pPort,'i');

	unsigned char expected[] = {'?','i',' ',13,10 };
	unsigned char byteRead;
	for(int i = 0; i < 5; i++) {
		if(!ReadByte(pPort, &byteRead)) { // ?
			return false;
		}
		if(byteRead != expected[i]) {
			return false;
		}
	}

	// Now we get in the lines
	unsigned char replyBuffer[1024];
	ReadByte(pPort, &replyBuffer[0]);
	ReadByte(pPort, &replyBuffer[1]);

	*pHand = 'r';

	if(replyBuffer[0] != 'C' || replyBuffer[1] != 'G') {
		*pHand = 'l';
	}

	//Flush the buffer
	byteRead = 1;
	while(byteRead != 0) {
		ReadByte(pPort, &byteRead);
	}

	return true;
}

bool CyberGloveLogger::IsGlove( HANDLE pPort ) {
	
	bool success = true;
	unsigned char byteRead;

	success &= WriteByte(pPort,'?');
	success &= WriteByte(pPort,'G');

	if(!success){
		return false;
	}

	char expected[] = {'?','G',3,0 };

	for(int i = 0; i < 4; i++) {
		// '?' 'G' 3 0
		if(!ReadByte(pPort, &byteRead)) { // ?
			return false;
		}
		if(byteRead != expected[i]) {
			return false;
		}
	}
	return true;
}

bool CyberGloveLogger::GetNumSensors(HANDLE pPort, unsigned char *pNumSensors) {
	bool success = true;
	*pNumSensors = 0;
	success &= WriteByte(pPort,'?');
	success &= WriteByte(pPort,'S');

	if(!success) {
		return false;
	}

	unsigned char readByte = 1;
	bool nextByteIsData = false;
	
	while(readByte != 0) {
		success = ReadByte(pPort, &readByte);
		if(nextByteIsData) {
			nextByteIsData = false;
			*pNumSensors = (unsigned char)readByte;
		}
		if(readByte == 'S') {
			nextByteIsData = true;
		}
	}
	return true;
}

bool CyberGloveLogger::GetSample(HANDLE pPort, unsigned char *pValues, int pNumValues, bool pRightHanded) {
	bool success = true;
	unsigned char a;

	success &= WriteByte(pPort, 'G');

	ReadByte(pPort,&a); // burn reply 'G'

	for(int i = 0; i < pNumValues; i++){
		ReadByte(pPort,&pValues[i]);
	}

	ReadByte(pPort,&a); // burn reply 0

	return true;
}

void CyberGloveLogger::FindGlovePorts()
{
	std::string portNameString;
	char portStr[64];
	HANDLE port;
	for(int portNum = 0; portNum < 20; portNum++) {
		sprintf(portStr, "COM%i",portNum);
		
		
		portNameString = portStr;
		//wprintf(L"[%s]", portNameString);
		port = CreateFile(portNameString.c_str(),GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
		if(port == INVALID_HANDLE_VALUE) {
			//wprintf(L" [NONE]\n", portNameString);
			//bciout<<"Nothing on COM Port#"<<portNameString<<endl;
			
			continue;
		}

		//wprintf(L" [OPEN]");

		if(!SetComParams(port, 115200, 1000)) {
			//wprintf(L" [SETTING FAIL]\n");
			//bciout<<"Setting Fail on Port #"<<portNum<<endl;
			CloseHandle(port);continue;
		}

		wprintf(L" [PARAMS]");

		//FlushBuffer(pPort);
		//wprintf(L" [FLUSH]");

		if(!IsGlove(port)) {
			//wprintf(L" [NOT GLOVE]\n");
			bciout<<"Not glove on port #"<<portNum<<endl;
			CloseHandle(port);continue;
		}

		

		char hand;

		if (!DetectHandedness(port, &hand)) {
			//wprintf(L" [GLOVE DETECT FAILED]\n");
			CloseHandle(port);continue;
		}
		portNameString += hand;

		switch(hand){
		case 'r':
			//wprintf(L" [RIGHT]");
			break;
		case 'l':
			//wprintf(L" [LEFT]");
			break;
		}

		if(!ResetGlove(port, hand=='r')) {
			//wprintf(L" [RESET FAILED]\n");
			CloseHandle(port);continue;
		}
		
		unsigned char numSensors;
		if(!GetNumSensors(port, &numSensors)) {
			//wprintf(L" [NO SENSORS\n]");
			CloseHandle(port);
			continue;
		}

		//wprintf(L" [SENSORS: %i]", numSensors);

		unsigned char values[22];

		GetSample(port, values,22, hand=='r');
		//wprintf(L" [DATA: %i,%i,%i...]", values[0], values[1], values[2]);

		GetSample(port, values,22, hand=='r');
		//wprintf(L" [DATA: %i,%i,%i...]", values[0], values[1], values[2]);


		//wprintf(L" [SUCCESS]\n");

		CloseHandle(port);

		mGoodPorts.push_back(portNameString);
	}
	return;
}

void
CyberGloveLogger::GetGloveThreads(CyberGloveThreadList & available)
{
	EmptyList(available);
	for(unsigned int iGlove = 0; iGlove < mGoodPorts.size(); iGlove++) {
		CyberGloveThread *g = new CyberGloveThread(this, mGoodPorts[iGlove]);
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
CyberGloveLogger::AssignGloves(CyberGloveThreadList & assigned)
{
	CyberGloveThreadList available;
	GetGloveThreads(available);
	ParamRef handedness = Parameter("CyberGloveHandedness"); 
	int nGlovesRequested = handedness->NumValues();
	for(int iRequest = 0; iRequest < nGlovesRequested; iRequest++) {
		string sWanted = handedness(iRequest);
		unsigned char hWanted;
		CyberGloveThread *found = NULL;
		if(     sWanted == "L" || 
			sWanted == "l" || 
			sWanted == "left"  || 
			sWanted == "LEFT"  || 
			sWanted == "0") hWanted = 'l';
		else if(sWanted == "R" || 
			sWanted == "r" || 
			sWanted == "right" || 
			sWanted == "RIGHT" || 
			sWanted == "1") hWanted = 'r';
		else {
			EmptyList(available);
			EmptyList(assigned);
			bcierr << "unrecognized DataGloveHandedness string \"" << sWanted << "\"" << endl;
		}
		for(CyberGloveThreadList::iterator g=available.begin(); g != available.end(); g++) {
			if( (*g)->GetHandedness() == hWanted ) { found = *g; break; }
		}
		if(found) {
			available.remove(found);
			assigned.push_back(found);
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
CyberGloveLogger::EmptyList(CyberGloveThreadList & gloves)
{
	while(gloves.size()) {delete gloves.back(); gloves.pop_back();}
}

void
CyberGloveLogger::Publish()
{
	char definition[64];

	FindGlovePorts();

	if(mGoodPorts.size() <= 0) {
		bciout << " No cyberglove detected. Tear" << endl;
		return;
	}

	string handednessLine;

	for(unsigned int i = 0; i < mGoodPorts.size(); i++) {
		handednessLine = handednessLine + (mGoodPorts[i][mGoodPorts[i].length()-1] == 'l' ? "L " : "R ");
		for(int iSensor = 0; iSensor < 22; iSensor++) {
			sprintf(definition, "%cCyber%d %d 0 0 0", mGoodPorts[i][mGoodPorts[i].length()-1], iSensor+1, 16);
			BEGIN_EVENT_DEFINITIONS
				definition,
			END_EVENT_DEFINITIONS
		}
	}

	sprintf(definition,"%d", mGoodPorts.size());
	handednessLine = "HumanInterfaceDevices:Cyberglove list    CyberGloveHandedness= " + (string(definition) + " " + handednessLine) + " // CyberGlove handedness: L or R for each glove";

	//handednessLine = "Source:Log%20Input list    CyberGloveHandedness= 1 L R % % // CyberGlove handedness: L or R for each glove";
	BEGIN_PARAMETER_DEFINITIONS
		"HumanInterfaceDevices:Cyberglove int     LogCyberGlove=          1    0  0 1 // record CyberGlove to states (boolean)",
		handednessLine.c_str(), // "Source:Log%20Input list    DataGloveHandedness= 1 L    R  % % // DataGlove handedness: L or R for each glove",
	END_PARAMETER_DEFINITIONS
}

void
CyberGloveLogger::Preflight() const
{
	bool enabled = ( ( int )OptionalParameter( "LogCyberGlove", 0 ) != 0 );
	OptionalParameter("CyberGloveHandedness");
	/*if(enabled) {
		CyberGloveThreadList assigned;
		AssignGloves(assigned);
		EmptyList(assigned); // <sigh>
	}*/

}

void
CyberGloveLogger::Initialize()
{
	mEnabled = ( ( int )OptionalParameter( "LogCyberGlove" ) != 0 );
	//m_deriv = m_enabled && ( (int)Parameter("DataGloveDerivative") != 0 ); 
}

void
CyberGloveLogger::StartRun()
{
	if(mEnabled) {
		AssignGloves(mGloves);
	}
	for(CyberGloveThreadList::iterator g = mGloves.begin(); g != mGloves.end(); g++)
		(*g)->Start();
}

void
CyberGloveLogger::StopRun()
{
	for(CyberGloveThreadList::iterator g = mGloves.begin(); g != mGloves.end(); g++)
		(*g)->TerminateWait();
	EmptyList(mGloves);
}

void
CyberGloveLogger::Halt()
{
	StopRun();
}



CyberGloveThread::CyberGloveThread(const CyberGloveLogger *pLogger, std::string pComPortName)
{
	m_err.str("");
	for(int i = 0; i < MAX_SENSORS; i++) mPreviousReading[i] = mPreviousOutput[i] = 0;
	
	char ps[10];

	for(unsigned int i = 0; i < pComPortName.length()-1; i++) {
		ps[i] = pComPortName[i];
	}
	ps[pComPortName.length()-1] = 0; 
	
	mLogger       = pLogger;
	m_portString   = ps;
	pHandle = CreateFile(m_portString.c_str(),GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(pHandle == NULL) {
		m_hand = -1;
		m_sensors = -1;
		m_err << "failed to find a data glove on " << m_portString;
	}
	else {
		if(!SetComParams(pHandle, 115200,0)) {
			m_err << "Unable to set cyberglove com parameters for " << m_portString << endl;
		}
		if(!DetectHandedness(pHandle, &m_hand)) {
			m_err << "Unable to set cyberglove com handedness for " << m_portString << endl;
		}
		if(!GetNumSensors(pHandle, &m_sensors)) {
			m_err << "Unable to get cyberglove com sensor count for " << m_portString << endl;
		}
	}
}

void
CyberGloveThread::Cleanup()
{
	if(pHandle != NULL && mLogger != NULL) {
		CloseHandle(pHandle);
		pHandle = NULL;
	}
	m_hand = -1;
	m_sensors = -1;
}

CyberGloveThread::~CyberGloveThread()
{
	Cleanup();
}

char
CyberGloveThread::GetHandedness()
{
	return m_hand;
}

string
CyberGloveThread::GetError()
{
	return m_err.str();
}

int
CyberGloveThread::Execute()
{
	unsigned char *currentData = new unsigned char[m_sensors];
	while( !IsTerminating() ) {
		for(int i = 0; i < MAX_SENSORS; i++) currentData[i] = 0;

		if(!GetSample(pHandle, currentData, m_sensors, m_hand == 'r')) {
			bcierr << "GS FAILED" << endl;
		}

		for(int i = 0; i < MAX_SENSORS; i++) {
			if(currentData[i] != mPreviousOutput[i])
				bcievent << m_hand << "Cyber" << (i+1) << " " << short(currentData[i]);
			mPreviousOutput[i] = currentData[i];
		}
		::Sleep(1);
	}
	::Sleep(10);

	delete[] currentData;
	return 0;
}

bool CyberGloveThread::WriteByte(HANDLE pPort, unsigned char pValue, bool pEcho = false) {
	DWORD bytesWritten;
	if(pEcho) {
		wprintf(L"WRITING %c (%i)\n", pValue, pValue);
	}
	if(!WriteFile(pPort, &pValue, 1, &bytesWritten, NULL))
		return false;
	return true;
}

bool CyberGloveThread::ReadByte(HANDLE pPort, unsigned char *pDest, bool pEcho = false){
	DWORD bytesRead;
	if(ReadFile(pPort, pDest, 1, &bytesRead, NULL) && bytesRead == 1 ) {
		if(pEcho) {
			wprintf(L"READ %c (%i)\n", *pDest, *pDest);
		}
		return true;
	}
	return false;
}

bool CyberGloveThread::ResetGlove(HANDLE pPort, bool pRightHanded) {
	
	unsigned char readByte;
	
	if(!WriteByte(pPort, 18)) {
		return false;
	}

	/*for(int i = 0; i < 1; i++) {
		if(!WriteByte(pPort, cmd[i])) {
			return false;
		}
	}*/
	
	// Assume we reset, flush results
	readByte = 1;
	if(pRightHanded) {
		while(readByte != 0){
			ReadByte(pPort, &readByte);
		}
	}
	else {
		while(readByte != 10){
			ReadByte(pPort, &readByte);
		}
	}
	

	return true;
}

bool CyberGloveThread::DetectHandedness(HANDLE pPort, char* pHand) {
	bool success = true; 
	success &= WriteByte(pPort,'?');
	success &= WriteByte(pPort,'i');

	unsigned char expected[] = {'?','i',' ',13,10 };
	unsigned char byteRead;
	for(int i = 0; i < 5; i++) {
		if(!ReadByte(pPort, &byteRead)) { // ?
			return false;
		}
		if(byteRead != expected[i]) {
			return false;
		}
	}

	// Now we get in the lines
	unsigned char replyBuffer[1024];
	ReadByte(pPort, &replyBuffer[0]);
	ReadByte(pPort, &replyBuffer[1]);

	*pHand = 'r';

	if(replyBuffer[0] != 'C' || replyBuffer[1] != 'G') {
		*pHand = 'l';
	}

	//Flush the buffer
	byteRead = 1;
	while(byteRead != 0) {
		ReadByte(pPort, &byteRead);
	}

	return true;
}

bool CyberGloveThread::IsGlove( HANDLE pPort ) {
	
	bool success = true;
	unsigned char byteRead;

	success &= WriteByte(pPort,'?');
	success &= WriteByte(pPort,'G');

	if(!success){
		return false;
	}

	char expected[] = {'?','G',3,0 };

	for(int i = 0; i < 4; i++) {
		// '?' 'G' 3 0
		if(!ReadByte(pPort, &byteRead)) { // ?
			return false;
		}
		if(byteRead != expected[i]) {
			return false;
		}

	}

	return true;
}

bool CyberGloveThread::GetNumSensors(HANDLE pPort, unsigned char *pNumSensors) {
	bool success = true;
	*pNumSensors = 0;
	success &= WriteByte(pPort,'?');
	success &= WriteByte(pPort,'S');

	if(!success) {
		return false;
	}

	unsigned char readByte = 1;
	bool nextByteIsData = false;
	
	while(readByte != 0) {
		success = ReadByte(pPort, &readByte);
		if(nextByteIsData) {
			nextByteIsData = false;
			*pNumSensors = (unsigned char)readByte;
		}
		if(readByte == 'S') {
			nextByteIsData = true;
		}
	}

	return true;

}

bool CyberGloveThread::GetSample(HANDLE pPort, unsigned char *pValues, int pNumValues, bool pRightHanded) {
	bool success = true;
	unsigned char a;

	success &= WriteByte(pPort, 'G');

	ReadByte(pPort,&a); // burn reply 'G'

	for(int i = 0; i < pNumValues; i++){
		ReadByte(pPort,&pValues[i]);
	}

	ReadByte(pPort,&a); // burn reply 0

	return true;
}


bool CyberGloveThread::SetComParams(HANDLE pHandle, int pBaud, int pTimeoutMS) {
	DCB params;
	GetCommState(pHandle, &params);
	params.DCBlength = sizeof(DCB);
	params.BaudRate  = pBaud;
	params.ByteSize  = 8;
	params.StopBits  = ONESTOPBIT;
	params.Parity    = NOPARITY;

	if(!SetCommState(pHandle, &params))
	{
		CloseHandle(pHandle);
		return false;
	}

	COMMTIMEOUTS timeouts; // In milliseconds
	timeouts.ReadIntervalTimeout         = pTimeoutMS;
	timeouts.ReadTotalTimeoutConstant    = pTimeoutMS;
	timeouts.ReadTotalTimeoutMultiplier  = pTimeoutMS;
	timeouts.WriteTotalTimeoutConstant   = pTimeoutMS;
	timeouts.WriteTotalTimeoutMultiplier = pTimeoutMS;

	if(!SetCommTimeouts(pHandle, &timeouts))
	{
		CloseHandle(pHandle);
		return false;
	}

	return true;
}