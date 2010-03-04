/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef AmpServerProADCH
#define AmpServerProADCH

//#define STANDALONE 1 

#include "SockStream.h"

#ifdef STANDALONE
#include <math.h>
#include <ctime>
#else
#include "GenericADC.h"     
#include "GenericSignal.h"
#include "BCIError.h"
#endif

#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>

#define ASP_DEF_IP "172.16.2.249"
#define ASP_DEF_CMD_PORT 9877
#define ASP_DEF_NOTIF_PORT 9878
#define ASP_DEF_DATA_PORT 9879
#define ASP_SAMP_SIZE 1152

#define ASP_CMD_SYNTAX "(sendCommand cmd_%s %d %d %d)\n"
#define ASP_CMD_START "Start"
#define ASP_CMD_STOP "Stop"
#define ASP_CMD_SETLOWPASS "SetLowpass"
#define ASP_CMD_SETPOINT01HIGHPASS "SetPoint01HighPass"
#define ASP_CMD_SETSUBJECTGROUND "SetSubjectGround"
#define ASP_CMD_SETELLIPTICALFILTER "SetEllipticalFilter"
#define ASP_CMD_SETPOWER "SetPower"
#define ASP_CMD_RESET "Reset"
#define ASP_CMD_GETSTARTTIME "GetStartTime"
#define ASP_CMD_GETCURRENTTIME "GetCurrentTime"
#define ASP_CMD_NUMBEROFAMPS "NumberOfAmps"
#define ASP_CMD_DEFAULTACQUISITIONSTATE "DefaultAcquisitionState"
#define ASP_CMD_DEFAULTSIGNALGENERATION "DefaultSignalGeneration"

#define ASP_SEXP_NUMBEROFAMPS "number_of_amps \d+"

#define ASP_DATAMSG_LISTENTOAMP "ListenToAmp"

#define ASP_CMD_RESP_SIZE 4096
#define ASP_DATA_MSG_SIZE 4096

#define ASP_TIMEOUT 2000
#define ASP_NUM_CHANS 280

class AmpServerProADC 
#ifndef STANDALONE
  : public GenericADC
#endif
{
 public:
               AmpServerProADC();
  virtual      ~AmpServerProADC();

#ifndef STANDALONE
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& signal );
#else
  virtual void Preflight() const;
  void Process(float sampBlock[40][280]);
  void Initialize();
#endif
  virtual void Halt();   

 private:
  struct Connection 
  {
    sockstream *stream;
    client_tcpsocket *socket;
  } m_oCmdConn, m_oNotifConn, m_oDataConn; 

  unsigned int m_nNumChans;
  unsigned int m_nAmpId;
  unsigned int m_nBlockSize;
  char m_sServerIP[25];
  char m_sAmpState[25];
  int m_nAmpState;
  unsigned int m_nCmdPort, m_nNotifPort, m_nDataPort;
  bool m_bListening;

  time_t m_nLastEnd;

  char m_sCmdResp[ASP_CMD_RESP_SIZE];

  bool Connect();
  bool SendCommand(char *sCmd);
  char *BuildCmdString(char *sCmd, int nChanId, int nArg);
  char *GetCmdRespValue(char *sParamName);
  void InitGlobalVars();

  // A number of functions must have const entry points for use from Preflight
  bool Connect(char *sServerIP, unsigned int nCmdPort, unsigned int nNotifPort, unsigned int nDataPort, Connection *pCmdConn, Connection *pNotifConn, Connection *pDataConn) const;
  bool SendCommand(char *sCmd, Connection *pCmdConn, char *sCmdResp) const;
  char *BuildCmdString(char *sCmd, int nChanId, int nArg, unsigned int nAmpId) const;
  char *GetCmdRespValue(char *sParamName, char *sCmdResp) const;
  void Halt(Connection *pCmdConn, Connection *pNotifConn, Connection *pDataConn, unsigned int nAmpId) const;
  bool GetAmpId(unsigned int *pAmpId, Connection *pCmdConn) const;

  inline void ReorderToNetworkOrder(char *sData, int nLen);
  inline void ReorderToHostOrder(char *sData, int nLen);
  bool BeginListening();
  inline bool ReadDataBlock(unsigned int nMaxBytes, unsigned int *nBytesRead);
  inline bool ReadDataHeader();


  char *m_aLastDataPack;
  unsigned int m_nLastDataPackOffset;
  unsigned int m_nLastDataPackSize;

  struct {
    __int64 ampID;
    unsigned __int64 length;
  } m_oLastHeader;
};

#endif // AmpServerProADCH

