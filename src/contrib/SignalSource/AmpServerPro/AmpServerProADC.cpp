/******************************************************************************
 * Module:    AmpServerProADC.CPP                                             *
 * Comment:   Definition for the AmpServerProADC class that facilitates       *
 *  communication with EGI's Amp Server Pro SDK                               *
 * Version:   1.00                                                            *
 * Author:    Joshua Fialkoff, Robert Bell (EGI)                              *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V1.00 - Updated Joshua's code to work with Amp Server Pro SDK GM release   *
 *         version 1.0.
 ******************************************************************************/
/* $BEGIN_BCI2000_LICENSE$
 *
 * This file is part of BCI2000, a platform for real-time bio-signal research.
 * [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
 *
 * BCI2000 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * BCI2000 is distributed in the hope that it will be useful, but
 *                         WITHOUT ANY WARRANTY
 * - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_BCI2000_LICENSE$
/*/

//#include "PCHIncludes.h"

#pragma hdrstop

#include "AmpServerProADC.h"

#include <stdio.h>
#include <assert.h>
#include <iostream>
//#include <winsock.h>

#ifdef STANDALONE
#define bcierr cerr
#define bcidbg cout
#else

#endif
using namespace std;

#ifndef STANDALONE
RegisterFilter( AmpServerProADC, 1 );
#endif

// **************************************************************************
// Function:   AmpServerProADC
// Purpose:    The constructor for the AmpServerProADC
//             it fills the provided list of parameters and states
//             with the parameters and states it requests from the operator
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
AmpServerProADC::AmpServerProADC()
{
  InitGlobalVars();

  m_oCmdConn.socket = NULL;
  m_oCmdConn.stream = NULL;
  m_oNotifConn.socket = NULL;
  m_oNotifConn.stream = NULL;
  m_oDataConn.socket = NULL;
  m_oDataConn.stream= NULL;
  m_aLastDataPack = NULL;

  strcpy(m_sServerIP, ASP_DEF_IP);
  strcpy(m_sAmpState,ASP_CMD_DEFAULTACQUISITIONSTATE);

#ifndef STANDALONE
  //TODO: Replace max values with actual max values
  // add all the parameters that this ADC requests to the parameter list
  BEGIN_PARAMETER_DEFINITIONS
    "Source int SourceCh=      16 16 1 280 "
        "// number of digitized channels",
    "Source int SampleBlockSize= 32 32 1 128 "
        "// number of samples per block",
    "Source int SamplingRate=    256 256 1 40000 "
        "// the signal sampling rate",
    "Source string ServerIP= 127.0.0.1 127.0.0.1"
		"// Address of Amp Server Pro",
    "Source int CommandPort= 9877 9877 1 65535"
        "// port number corresponding to the command layer",
    "Source int NotificationPort= 9878 9878 1 65535"
        "// port number corresponding to the notification layer",
    "Source int StreamPort= 9879 9879 1 65535"
        "// port number corresponding to the stream layer",
    "Source string AmplifierID=  Auto Auto"
        "// the ID of the Amplifier from which data should be collected",
	"Source int AmpState= 0 0 0 1"
		"// command to set amp state: 0 DefaultAcquisitionState, 1 DefaultSignalGeneration (enumeration)",
  END_PARAMETER_DEFINITIONS
#endif
}

// **************************************************************************
// Function:   InitGlobalVars
// Purpose:    This function initializes global variables to a default value
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void AmpServerProADC::InitGlobalVars()
{
  m_nAmpId = 0;
  m_nNumChans = 0;
  m_nBlockSize = 0;

  // If you change these, change param defaults - in constructor - as well
  m_nCmdPort = ASP_DEF_CMD_PORT;
  m_nNotifPort = ASP_DEF_NOTIF_PORT;
  m_nDataPort = ASP_DEF_DATA_PORT;

  m_bListening = false;
  m_nLastDataPackOffset = 0;
  m_nLastDataPackSize = 0;
}

// **************************************************************************
// Function:   Connect
// Purpose:    This function creates the three connections to the Amp Server
//             that are necessary for communicating via the command,
//             notification and data layers
// Parameters: N/A
// Returns:    True if all connections were made successfully, false
//             otherwise
// **************************************************************************
bool AmpServerProADC::Connect()
{
  return Connect(m_sServerIP, m_nCmdPort, m_nNotifPort, m_nDataPort, &m_oCmdConn, &m_oNotifConn, &m_oDataConn);
}

// **************************************************************************
// Function:   Connect
// Purpose:    This function creates the three connections to the Amp Server
//             that are necessary for communicating via the command,
//             notification and data layers
// Parameters: sServerIP - IP address of the server (port not included)
//             nCmdPort - the port number corresponding to the command layer
//             nNotifPort - the port number corresponding to the notification layer
//             nDataPort - the port number corresponding to the data stream layer
//             pCmdConn - (out) a pointer to the command connection
//             pNotifPort - (out) a pointer to the notification connection
//             pDataConn - (out) a pointer to the data stream connection
// Returns:    True if all connections were made successfully, false
//             otherwise
// **************************************************************************
bool AmpServerProADC::Connect(const char *sServerIP, unsigned int nCmdPort,
  unsigned int nNotifPort, unsigned int nDataPort, Connection *pCmdConn,
  Connection *pNotifConn, Connection *pDataConn) const
{
  // Establish a connection with all three communication ports.
  int nAddrLen, nTmp;
  char *sAddress;

  // Allocate memory for storage of host address.
  nAddrLen = static_cast<int>(ceil(log10((double)nCmdPort) + 1));
  nTmp = static_cast<int>(ceil(log10((double)nNotifPort) + 1));
  if( nTmp > nAddrLen)
    nAddrLen= nTmp;
  nTmp = static_cast<int>(ceil(log10((double)nDataPort) + 1));
  if( nTmp > nAddrLen)
    nAddrLen= nTmp;

  nAddrLen += static_cast<int>(strlen(sServerIP) + 5);
  sAddress = new char[nAddrLen];

  // Initialize command layer.
  sprintf(sAddress, "%s:%d", sServerIP, nCmdPort);
  pCmdConn->socket = new client_tcpsocket(sAddress);
  (pCmdConn->socket)->set_tcpnodelay( true );
  pCmdConn->stream = new sockstream(*(pCmdConn->socket));

  if (!(pCmdConn->stream))
  {
    bcierr << "Could not create TCP stream for the command layer." << endl;
    return false;
  }
  if (!((pCmdConn->stream)->is_open()))
  {
    bcierr << "Could not establish command layer connection to the server.  Make sure that the server address and command port have been entered correctly." << endl;
    return false;
  }

  (pCmdConn->stream)->set_timeout(ASP_TIMEOUT);

  // Initialize notification layer.
  sprintf(sAddress, "%s:%d", sServerIP, nNotifPort);
  pNotifConn->socket = new client_tcpsocket(sAddress);
  (pNotifConn->socket)->set_tcpnodelay( true );
  pNotifConn->stream = new sockstream(*(pNotifConn->socket));

  if (!(pNotifConn->stream))
  {
    bcierr << "Could not create TCP stream for the notification layer." << endl;
    return false;
  }
  if (!((pNotifConn->stream)->is_open()))
  {
    bcierr << "Could not establish notification layer connection to the server.  Make sure that the server address and notification port have been entered correctly." << endl;
    return false;
  }

  (pNotifConn->stream)->set_timeout(ASP_TIMEOUT);

  // Initialize data stream layer.
  sprintf(sAddress, "%s:%d", sServerIP, nDataPort);
  pDataConn->socket = new client_tcpsocket(sAddress);
  (pDataConn->socket)->set_tcpnodelay( true );
  pDataConn->stream = new sockstream(*(pDataConn->socket));

  if (!(pDataConn->stream))
  {
    bcierr << "Could not create TCP stream for the data stream layer." << endl;
    return false;
  }
  if (!((pDataConn->stream)->is_open()))
  {
    bcierr << "Could not establish data stream layer connection to the server.  Make sure that the server address and data stream port have been entered correctly." << endl;
    return false;
  }

  (pDataConn->stream)->set_timeout(ASP_TIMEOUT);

  delete[] sAddress;

  return true;
}


// **************************************************************************
// Function:   BeginListening
// Purpose:    This function indicates to the Amp Server that it should
//             begin streaming data.
// Parameters: N/A
// Returns:    True if message was sent successfully, false otherwise
// **************************************************************************
bool AmpServerProADC::BeginListening()
{
  // Make sure amp is powered on.
  char *sCmd = BuildCmdString(ASP_CMD_SETPOWER, 0, 1, 0);
  if(!SendCommand(sCmd))
  {
    delete[] sCmd;
    bcierr << "Unable to power on the amplifier" << endl;
    return false;
  }
  delete[] sCmd;

  // Start the amplifier.
  sCmd = BuildCmdString(ASP_CMD_START, 0, 0);
  if(!SendCommand(sCmd))
  {
    delete[] sCmd;
    bcierr << "Unable to start the amplifier" << endl;
    return false;
  }
  delete[] sCmd;

  // Always set the amp to the default acquisition state first.
  sCmd = BuildCmdString(ASP_CMD_DEFAULTACQUISITIONSTATE, 0, 0, 0);
  SendCommand(sCmd);
  delete[] sCmd;

  // Now send the desired start state (if not the already set default acquisition state).
  if(m_nAmpState != 0){
	switch(m_nAmpState){
		case 1:
			{
            	sCmd = BuildCmdString(ASP_CMD_DEFAULTSIGNALGENERATION, 0, 0, 0);
				SendCommand(sCmd);
				delete[] sCmd;
			}
			break;
		default:
			break;
	}
  }

  // Make sure we haven't lost connection.
  if (!m_oDataConn.stream)
  {
    bcierr << "Data layer stream not created." << endl;
    return false;
  }
  if (!m_oDataConn.stream->is_open())
  {
    bcierr << "Data layer connection to the server not established." << endl;
    return false;
  }

  sCmd = BuildCmdString(ASP_DATAMSG_LISTENTOAMP, 0, 0, m_nAmpId);

  m_oDataConn.stream->write(sCmd, strlen(sCmd));
  m_oDataConn.stream->flush();

  delete[] sCmd;

  Sleep(3000);

  m_bListening = true;

  return true;
}

// **************************************************************************
// Function:   ReorderToNetworkOrder
// Purpose:    This function converts a string of 64-bit integers from host
//             order to network order.
// Parameters: sData - a pointer to the data to be reordered
//             nLen - the number of 64 bit ints that should be reordered
// Returns:    N/A
// **************************************************************************
inline void AmpServerProADC::ReorderToNetworkOrder(char *sData, int nLen)
{
  __int32 *tmpData = reinterpret_cast<__int32 *>(sData);
  for(int i=0;i<nLen;i++)
  {
    int location = i*2;
    __int32 tmpValue = -1;
    tmpData[location] =htonl(tmpData[location]);
    tmpData[location+1] =htonl(tmpData[location+1]);
    tmpValue = tmpData[location];
    tmpData[location] = tmpData[location+1];
    tmpData[location+1] = tmpValue;
  }
}

// **************************************************************************
// Function:   ReorderToHostOrder
// Purpose:    This function converts a string of 64-bit integers from network
//             order to host order.
// Parameters: sData - a pointer to the data to be reordered
//             nLen - the number of 64 bit ints that should be reordered
// Returns:    N/A
// **************************************************************************
inline void AmpServerProADC::ReorderToHostOrder(char *sData, int nLen)
{
  __int32 *tmpData = reinterpret_cast<__int32 *>(sData);
  for(int i=0;i<nLen;i++)
  {
    int location = i*2;
    __int32 tmpValue = -1;
    tmpData[location] =ntohl(tmpData[location]);
    tmpData[location+1] =ntohl(tmpData[location+1]);
    tmpValue = tmpData[location];
    tmpData[location] = tmpData[location+1];
    tmpData[location+1] = tmpValue;
  }
}

// **************************************************************************
// Function:   ~AmpServerProADC
// Purpose:    The destructor for AmpServerProADC
//             which closes all connections and frees up memory
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
AmpServerProADC::~AmpServerProADC()
{
  Halt();
  if(m_aLastDataPack)
    delete[] m_aLastDataPack;
}


// **************************************************************************
// Function:   Preflight
// Purpose:    Checks parameters for availability and consistence with
//             input signal properties; requests minimally needed properties for
//             the output signal; checks whether resources are available.
// Parameters: Input and output signal properties pointers.
// Returns:    N/A
// **************************************************************************
#ifdef STANDALONE
void AmpServerProADC::Preflight() const
#else
void AmpServerProADC::Preflight( const SignalProperties&, SignalProperties& outSignalProperties ) const
#endif
{
  bool bResp;
  char sServerIP[25];
  unsigned int nCmdPort, nNotifPort, nDataPort;
  unsigned int nAmpId = -1;
  Connection oCmdConn, oNotifConn, oDataConn;
  unsigned int nBlockSize;
  char sCmdResp[ASP_CMD_RESP_SIZE];

#ifdef STANDALONE
  strcpy(sServerIP, ASP_DEF_IP);
  nCmdPort = ASP_DEF_CMD_PORT;
  nNotifPort = ASP_DEF_NOTIF_PORT;
  nDataPort = ASP_DEF_DATA_PORT;
  nAmpState = 0; // This corresponds to the default acquisition state in the display.
#else
  // Get connection info.
  strcpy(sServerIP, Parameter("ServerIP").c_str());
  sscanf(Parameter("CommandPort").c_str(), "%d", &nCmdPort);
  sscanf(Parameter("NotificationPort").c_str(), "%d", &nNotifPort);
  sscanf(Parameter("StreamPort").c_str(), "%d", &nDataPort);

  // Amp state.
  sscanf(Parameter("AmpState").c_str(), "%d", &m_nAmpState);

  // Samples are always 4-byte floats.
  outSignalProperties = SignalProperties(
    Parameter( "SourceCh" ), Parameter( "SampleBlockSize" ), SignalType::float32);
  nBlockSize = Parameter( "SampleBlockSize" );
  if( nBlockSize < 1 )
  {
    bcierr << "Invalid value specified for SampleBlockSize.  Please choose a positive integer value." << endl;
    Halt(&oCmdConn, &oNotifConn, &oDataConn, nAmpId);
    return;
  }
#endif

  // Connect to the amp server.
  if(!Connect(sServerIP, nCmdPort, nNotifPort, nDataPort, &oCmdConn, &oNotifConn, &oDataConn))
  {
    return;
  }

  // Make sure the amp is powered on.
  char* sCmd = BuildCmdString(ASP_CMD_SETPOWER, 0, 1, 0);
  bResp = SendCommand(sCmd, &oCmdConn, sCmdResp);
  delete[] sCmd;

  // Allow amp to settle into stable state.
  Sleep(3000);

  if(!bResp)
  {
    bcierr << "Unable to communicate via the command layer." << endl;
    Halt(&oCmdConn, &oNotifConn, &oDataConn, nAmpId);
    return;
  }


  // Figure out what the AmpId is.
  if(!GetAmpId(&nAmpId, &oCmdConn))
  {
    Halt(&oCmdConn, &oNotifConn, &oDataConn, nAmpId);
    return;
  }

  // Done talking for now, stop server and close connections.
  Halt(&oCmdConn, &oNotifConn, &oDataConn, nAmpId);


#ifndef STANDALONE
  // Parameter consistency checks: Existence/Ranges and mutual Ranges.
  // cross check the parameters as received from the server with the BCI2000 parameters
  if( Parameter( "SamplingRate" ).InHertz() != 1000.0 )
    bcierr << "Parameter \"SamplingRate\" needs to be set to 1000" << endl;

  // also cross check SourceChGain and SourceChOffset
  for (int ch=0; ch<Parameter( "SourceCh" ); ch++)
  {
	PreflightCondition( Parameter( "SourceChOffset" )( ch ) == 0 );
	PreflightCondition( fabs(Parameter( "SourceChGain" )( ch )-0.0238)<(1E-3) );

	// EGI NA300 amps have a range of +/- 200,000uV that is digitized into
	// a 24bit signed value. The values received in BCI 2000 are in float,
	// but use these values when converting back to uV.ss
  }

#endif
}

// **************************************************************************
// Function:   GetAmpId
// Purpose:    This function determines the Amplifier Id based on what was
//             specified by the user
// Parameters: pAmpId - (out) the AmpId as determined by the function
//             pCmdConn - a pointer to the command connection
// Returns:    True if function was able to determine amp ID, false otherwise
// **************************************************************************
bool AmpServerProADC::GetAmpId(unsigned int *pAmpId, Connection *pCmdConn) const
{
  int nAmpId = 0;
//  bool bResp;
  char sCmdResp[ASP_CMD_RESP_SIZE];
  int nScanRes, nNumAmps;

  // Query server for number of amplifiers.
  char* sCmd = BuildCmdString(ASP_CMD_NUMBEROFAMPS, 0, 0, nAmpId);
  if(!SendCommand(sCmd, pCmdConn, sCmdResp))
  {
    delete[] sCmd;
    bcierr << "Unable to determine the number of amplifiers connected." << endl;
    return false;
  }
  delete[] sCmd;

  //read response
  char *sNumAmps = GetCmdRespValue("number_of_amps", sCmdResp);
  if(sNumAmps == NULL || sNumAmps[0] == '\0')
  {
    delete[] sNumAmps;
    bcierr << "Received invalid response from server.  Unable to determine the number of amplifiers connected." << endl;
    return false;
  }

  nScanRes = sscanf(sNumAmps, "%d", &nNumAmps);
  if(nScanRes <= 0 || nScanRes == EOF)
  {
    delete[] sNumAmps;
    bcierr << "Received invalid response from server" << endl;
    return false;
  }

  delete[] sNumAmps;

#ifndef STANDALONE
  //find out what the user specified for amp ID and ensure it's valid
  if(stricmp(Parameter("AmplifierID").c_str(), "auto") == 0)
  {
    //user entered "auto" - only valid if 1 amp is connected
    if( nNumAmps == 1 )
      nAmpId = 0;
    else
    {
      bcierr << "More than 1 amplifier available at the specified server.  Please specify a valid amplifier ID" << endl;
      return false;
    }
  }
  else
  {
    //user entered arbitrary value, ensure it's a valid number.
    nScanRes = sscanf(Parameter("AmplifierID").c_str(), "%d", &nAmpId);
    if(nScanRes != 1)
    {
      bcierr << "Invalid AmplifierID was specified.  Please choose an integer greater than or equal to 0." << endl;
      return false;
    }
    if( nAmpId > nNumAmps - 1 )
    {
      bcierr << "Amplifier ID " << nAmpId << " does not refer to a valid amplifier for the specified server." << endl;
      return false;
    }
  }
#endif

  *pAmpId = nAmpId;

  return true;
}

// **************************************************************************
// Function:   Initialize
// Purpose:    This function parameterizes the AmpServerProADC.
//             It is called each time the operator first starts,
//             or suspends and then resumes, the system
//             (i.e., each time the system goes into the main
//             data acquisition loop (fMain->MainDataAcqLoop())
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
#ifdef STANDALONE
void AmpServerProADC::Initialize()
#else
void AmpServerProADC::Initialize( const SignalProperties&, const SignalProperties& )
#endif
{
  char *sCmd;
//  bool bResp;
//  char sCmdResp[ASP_CMD_RESP_SIZE];

  Halt();

#ifdef STANDALONE
  m_nNumChans = 280;
  m_nBlockSize = 40;
#else
  //get some values set by the user in BCI2000 config.  The validity of the values was
  //already confirmed in Preflight
  strcpy(m_sServerIP, Parameter("ServerIP").c_str());
  sscanf(Parameter("CommandPort").c_str(), "%d", &m_nCmdPort);
  sscanf(Parameter("NotificationPort").c_str(), "%d", &m_nNotifPort);
  sscanf(Parameter("StreamPort").c_str(), "%d", &m_nDataPort);
  sscanf(Parameter("SourceCh").c_str(), "%d", &m_nNumChans);
  m_nBlockSize = Parameter( "SampleBlockSize" );
  sscanf(m_sAmpState, Parameter("AmpState").c_str());
#endif

  if(m_aLastDataPack != NULL)
    delete[] m_aLastDataPack;
  m_aLastDataPack = new char[ASP_SAMP_SIZE * m_nBlockSize];

  // Connect to the amplifier
  if(!Connect())
  {
    bcierr << "Unable to connect to amp server" << endl;
    Halt();
    return;
  }

  //Make sure the amp is powered on
  sCmd = BuildCmdString(ASP_CMD_SETPOWER, 0, 1, 0);
  if(!SendCommand(sCmd))
  {
    delete[] sCmd;
    bcierr << "Unable to power on the amplifier." << endl;
    return;
  }
  delete[] sCmd;

  //Get the amp ID
  if(!GetAmpId(&m_nAmpId, &m_oCmdConn))
  {
    bcierr << "Unable to set amplifier ID." << endl;
    Halt();
    return;
  }

  //Tell amplifier to start streaming data
  BeginListening();
}

// **************************************************************************
// Function:   GetCmdRespValue
// Purpose:    This function finds the value in an s-string that corresponds
//             to a particular name
// Parameters: sParamName - parameter name (e.g., num_channels)
// Returns:    A string with the value corresponding to the name in
//             sParamName
// **************************************************************************
char *AmpServerProADC::GetCmdRespValue(const char *sParamName)
{
  return GetCmdRespValue(sParamName, m_sCmdResp);
}

// **************************************************************************
// Function:   GetCmdRespValue
// Purpose:    This function finds the value in an s-string that corresponds
//             to a particular name
// Parameters: sParamName - parameter name (e.g., num_channels)
//             sCmdResp - an s-expression response from the amp server
// Returns:    A string with the value corresponding to the name in
//             sParamName
// **************************************************************************
char *AmpServerProADC::GetCmdRespValue(const char *sParamName, const char *sCmdResp) const
{
  int nValStart = -1;
  unsigned int nCharsMatched = 0;
  int nStartIdx = -1, nEndIdx = -1;


  const char *sSExp = sCmdResp;

  //search for the string in sParamName in sCmdResp
  for(unsigned int i=0; i<strlen(sSExp); i++)
  {
    if(sParamName[nCharsMatched] == sSExp[i])
    {
      nCharsMatched++;

      if(nCharsMatched == strlen(sParamName))
      {
        nValStart = i + 1;
        break;
      }
    }
    else
      nCharsMatched = 0;
  }

  if( nValStart == -1 )
    return NULL;
  else
  {

    //find the value which might be after some spaces and will end at a ')'
    for(unsigned int i=nValStart; i<strlen(sSExp); i++)
    {
      if(sSExp[i] == ')')
      {
        if(nStartIdx < 0)
          nStartIdx = i-1;
        nEndIdx = i;
        break;
      }
      else if(nStartIdx == -1 && sSExp[i] != ' ')
        nStartIdx = i;
    }

    char *sRetVal;

    if( nEndIdx < 0 || nStartIdx < 0 )
    {
      return NULL;
    }
    else if(nEndIdx == 0 && nStartIdx == 0)
    {
      sRetVal = new char[1];
      sRetVal[0] = '\0';
      return sRetVal;
    }
    else
    {
      sRetVal = new char[nEndIdx - nStartIdx + 4];
      strncpy(sRetVal, sSExp + nStartIdx, nEndIdx - nStartIdx);
      sRetVal[nEndIdx - nStartIdx] = '\0';
      return sRetVal;
    }
  }
}

// **************************************************************************
// Function:   SendCommand
// Purpose:    This function sends a message to the Amp Server via the
//             command layer.
// Parameters: sCmd - the command string
// Returns:    True if the command was sent successfully, false otherwise
// **************************************************************************
bool AmpServerProADC::SendCommand(const char *sCmd)
{
  return SendCommand(sCmd, &m_oCmdConn, m_sCmdResp);
}

// **************************************************************************
// Function:   SendCommand
// Purpose:    This function sends a message to the Amp Server via the
//             command layer.
// Parameters: sCmd - the command string
//             pCmdConn - a pointer to the command layer connection
//             sCmdResp - (out) the amp server's response to the command
// Returns:    True if the command was sent successfully, false otherwise
// **************************************************************************
bool AmpServerProADC::SendCommand(const char *sCmd, Connection *pCmdConn, char *sCmdResp) const
{
  // Make sure connection is still valid.
  if (!(pCmdConn->stream))
  {
    bcierr << "Command layer stream not created." << endl;
    return false;
  }
  if (!(pCmdConn->stream)->is_open())
  {
    bcierr << "Command layer connection to the server not established." << endl;
    return false;
  }

  // Send command.
  (pCmdConn->stream)->write( sCmd, strlen(sCmd) );
  (pCmdConn->stream)->flush();

  // Get the response.
  int i;
  for(i=0; i<ASP_CMD_RESP_SIZE; i++)
  {
    (pCmdConn->stream)->get(*(sCmdResp + i));

	// Check for timeout.
    if( !( (pCmdConn->stream)->is_open() ) )
    {
      bcierr << "Lost connection to the server." << endl;
      return false;
    }

    if( sCmdResp[i] == '\n' )
      break;
  }
  sCmdResp[i] = '\0';

  return true;
}

// **************************************************************************
// Function:   BuildCmdString
// Purpose:    This function creates a command string in the format accepted
//             by the command layer of Amp Server Pro
// Parameters: sCmd - the command to be issued (e.g., SetPower)
//             nChanId - the number of the channel to which the command
//             corresponds.  Enter 0 if N/A.
//             nArg - the value pertaining to the request.  Enter 0 if N/A.
// Returns:    An s-expression compiled from the arguments
// **************************************************************************
char *AmpServerProADC::BuildCmdString(const char *sCmd, int nChanId, int nArg)
{
  return BuildCmdString(sCmd, nChanId, nArg, m_nAmpId);
}

// **************************************************************************
// Function:   BuildCmdString
// Purpose:    This function creates a command string in the format accepted
//             by the command layer of Amp Server Pro
// Parameters: sCmd - the command to be issued (e.g., SetPower)
//             nChanId - the number of the channel to which the command
//             corresponds.  Enter 0 if N/A.
//             nArg - the value pertaining to the request.  Enter 0 if N/A.
//             nAmpId - the amplifier ID
// Returns:    An s-expression compiled from the arguments
// **************************************************************************
char *AmpServerProADC::BuildCmdString(const char *sCmd, int nChanId, int nArg, unsigned int nAmpId) const
{
  // Estimate amount of memory to allocate.
  int nCmdLen = static_cast<int>(strlen(ASP_CMD_SYNTAX));
  nCmdLen += static_cast<int>(strlen(sCmd));
  if (nAmpId <= 0)
    nCmdLen += 1;
  else
    nCmdLen += static_cast<int>(ceil(log10((double)nAmpId) + 1));

  if (nChanId <= 0)
    nCmdLen += 1;
  else
    nCmdLen += static_cast<int>(ceil(log10((double)nChanId) + 1));

  if (nArg <= 0)
    nCmdLen += 1;
  else
    nCmdLen += static_cast<int>(ceil(log10((double)nArg) + 1));

  // Create cmd string.
  char* sCmdFull = new char[nCmdLen];
  sprintf(sCmdFull, ASP_CMD_SYNTAX, sCmd, nAmpId, nChanId, nArg);

  return sCmdFull;
}

// **************************************************************************
// Function:   Halt
// Purpose:    This routine tells the amplifier to stop sending data and
//             closes any connections
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
void AmpServerProADC::Halt()
{
  Halt(&m_oCmdConn, &m_oNotifConn, &m_oDataConn, m_nAmpId);
  InitGlobalVars();
}

// **************************************************************************
// Function:   Halt
// Purpose:    This routine tells the amplifier to stop sending data and
//             closes any connections
// Parameters: pCmdConn - A pointer to the command layer connection
//             pNotifConn - A pointer to the notification layer connection
//             pDataConn - A pointer to the data stream layer connection
//             nAmpId - The amplifier ID
// Returns:    N/A
// **************************************************************************
void AmpServerProADC::Halt(Connection *pCmdConn, Connection *pNotifConn,
  Connection *pDataConn, unsigned int nAmpId) const
{
  char *sCmd;
  char sCmdResp[ASP_CMD_RESP_SIZE];

   if (pCmdConn->stream && (pCmdConn->stream)->is_open())
   {
	 // Send the command to stop sending data.
     sCmd = BuildCmdString(ASP_CMD_STOP, 0, 0, nAmpId);
     SendCommand(sCmd, pCmdConn, sCmdResp);
     delete[] sCmd;

	 // Close command layer connection.
     (pCmdConn->stream)->close();
     (pCmdConn->socket)->close();
     delete pCmdConn->socket;
     delete pCmdConn->stream;
     pCmdConn->socket = NULL;
     pCmdConn->stream = NULL;
   }

  if (pNotifConn->stream && (pNotifConn->stream)->is_open())
  {
	 // Close notification layer connection.
    (pNotifConn->stream)->close();
    (pNotifConn->socket)->close();
    delete pNotifConn->socket;
    delete pNotifConn->stream;
    pNotifConn->socket = NULL;
    pNotifConn->stream = NULL;
  }

  if (pDataConn->stream && (pDataConn->stream)->is_open())
  {
	 // Close data stream layer connection.
    (pDataConn->stream)->close();
    (pDataConn->socket)->close();
    delete pDataConn->socket;
    delete pDataConn->stream;
    pDataConn->socket = NULL;
    pDataConn->stream = NULL;
  }
}

// **************************************************************************
// Function:   Process
// Purpose:    This function is called within fMain->MainDataAcqLoop()
//             it fills the already initialized array RawEEG with values
//             and DOES NOT RETURN, UNTIL ALL DATA IS ACQUIRED
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
#ifdef STANDALONE
void AmpServerProADC::Process(float sampBlock[40][280])
#else
void AmpServerProADC::Process( const GenericSignal&, GenericSignal& signal )
#endif
{
  unsigned int nBytesRead = 0;
  unsigned int nOffset = 0;
  unsigned int nTotalSampsRead = 0;

  bcidbg << "Process" << endl;

  if(!m_bListening)
  {
    bcierr << "The amplifier must be initialized before data can be processed." << endl;
    Halt();
    return;
  }

  // Read a full block of data.
  while(nTotalSampsRead < m_nBlockSize)
  {
	// Read header if necessary.
    if( m_nLastDataPackOffset >= m_nLastDataPackSize )
    {
      bcidbg << "Reading header" << endl;
      if(!ReadDataHeader())
      {
        Halt();
        return;
      }
    }

	// Read as many samples are in the current group but no more than is needed to fill the current block.
    if(!ReadDataBlock((m_nBlockSize-nTotalSampsRead) * ASP_SAMP_SIZE, &nBytesRead))
    {
      bcierr << "Received invalid data from the server.  Data may be corrupted." << endl;
      Halt();
      return;
    }

    unsigned int nSampsRead = nBytesRead / ASP_SAMP_SIZE;

    bcidbg << nSampsRead << " samples read" << endl;

	// Load the samples into the return parameter.
    nOffset = 0;
    unsigned int loopCount = 0;
    for(unsigned int nSamp=nTotalSampsRead; nSamp< nTotalSampsRead + nSampsRead; nSamp++)
    {
	  // Reorder only the first m_nNumChans floats and ignore the preceding 32-byte header.
      //ReorderToHostOrder(m_aLastDataPack + nOffset, 32 + sizeof(float) * m_nNumChans);

	  // There's a header in front of each sample - ignore it.
	  nOffset = (loopCount * ASP_SAMP_SIZE) + 32;
	  //nOffset += 32;

      for(unsigned int nChan=0; nChan<m_nNumChans; nChan++)
      {
        __int32* p = reinterpret_cast<__int32*>( m_aLastDataPack + nOffset );
        *p = ntohl( *p );
#ifdef STANDALONE
        sampBlock[nSamp][nChan] = *reinterpret_cast<float *>( p );
#else
        signal(nChan, nSamp) = *reinterpret_cast<float *>( p );
#endif
        nOffset += sizeof(float);
      }

      loopCount++;
    }

    nTotalSampsRead += nSampsRead;

  }

  /*
  m_nLastEnd = clock();
  bcidbg << "Seconds for process to complete: " << (double(m_nLastEnd)-double(nStartTime))/CLOCKS_PER_SEC << endl;
  */
}

// **************************************************************************
// Function:   ReadDataHeader
// Purpose:    This function reads a data header from the data stream
// Parameters: N/A
// Returns:    True/False depending on success
// **************************************************************************
inline bool AmpServerProADC::ReadDataHeader()
{
  char sData[16];

  memset(sData, 0, 16);

  if(!m_oDataConn.stream->is_open())
  {
    bcierr << "Lost connection to the server." << endl;
    return false;
  }

  // Read the header.
  m_oDataConn.stream->read(sData, 15);
  sData[15] = m_oDataConn.stream->get();

  ReorderToHostOrder(sData, 2);

  // Save the header values into m_oLastHeader.
  union { char* asCharPtr; __int64* asInt64Ptr; } pointer;
  pointer.asCharPtr = sData;
  m_oLastHeader.ampID = *pointer.asInt64Ptr;
  pointer.asCharPtr += 8;
  m_oLastHeader.length = *pointer.asInt64Ptr;

  // Ensure data is valid.
  if(m_nAmpId != m_oLastHeader.ampID || m_oLastHeader.length == 0 )
  {
	bcierr << "Received invalid data from amplifier.  Data may have been corrupted. m_nAmpId: " << m_nAmpId
	<< " m_oLastHeader.ampID: " << m_oLastHeader.ampID << " length: " << m_oLastHeader.length << endl;
    return false;
  }

  // We're at the beginning of a new data pack, set offset and length.
  m_nLastDataPackOffset = 0;
  m_nLastDataPackSize = static_cast<unsigned int>( m_oLastHeader.length );

  return true;
}

// **************************************************************************
// Function:   ReadDataPacket
// Purpose:    This function reads a data packet from the stream layer and
//             makes the data available in m_aLastDataPack
// Parameters: nMaxBytes - the maximum number of bytes that should be read
//             nBytesRead - (out) the number of bytes that were actually read
// Returns:    True if the read was successful, false otherwise
// **************************************************************************
inline bool AmpServerProADC::ReadDataBlock(unsigned int nMaxBytes, unsigned int *nBytesRead)
{
  // Determine the number of bytes to read.
  unsigned int nBytesLeft = m_nLastDataPackSize - m_nLastDataPackOffset;

  if(nMaxBytes > nBytesLeft)
    *nBytesRead = nBytesLeft;
  else
    *nBytesRead = nMaxBytes;

  // Ensure connection is still open.
  if(!m_oDataConn.stream->is_open())
  {
    bcierr << "Lost connection to the server." << endl;
    return false;
  }

  // Read data.
  m_oDataConn.stream->read(m_aLastDataPack, *nBytesRead-1);
  m_aLastDataPack[*nBytesRead-1] = m_oDataConn.stream->get();

  m_nLastDataPackOffset += *nBytesRead;

  return true;
}
