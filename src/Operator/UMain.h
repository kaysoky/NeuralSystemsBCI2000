#ifndef UMainH
#define UMainH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>

#include <map>
#include "TCPStream.h"
#include "MessageHandler.h"
#include "UParameter.h"
#include "UState.h"

#include "UVisual.h"
#include "USysLog.h"
#include "UScript.h"
#include "USysStatus.h"
#include "UPreferences.h"

#define TXT_WINDOW_CAPTION      "BCI2000/Operator"
#define TXT_OPERATOR_VERSION    "V1.31"
#define TXT_OPERATOR_SUSPENDED  "Suspended"
#define TXT_OPERATOR_RUNNING    "Running"

class TfMain : public TForm
{
 __published:	// IDE-managed Components
  TStatusBar *StatusBar;
  TButton *bQuit;
  TButton *bRunSystem;
  TButton *bConfig;
  TButton *bSetConfig;
  TLabel *Label7;
  TLabel *Label8;
  TButton *bShowConnectionInfo;
  TMainMenu *MainMenu1;
  TMenuItem *File1;
  TMenuItem *N1;
  TMenuItem *Exit1;
  TMenuItem *Help1;
  TMenuItem *About1;
  TMenuItem *View1;
  TMenuItem *States1;
  TMenuItem *OperatorLog1;
  TMenuItem *ConnectionInfo1;
  TButton *bFunction1;
  TButton *bFunction2;
  TButton *bFunction3;
  TButton *bFunction4;
  void __fastcall bQuitClick(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall bRunSystemClick(TObject *Sender);
  void __fastcall bConfigClick(TObject *Sender);
  void __fastcall bSetConfigClick(TObject *Sender);
  void __fastcall bShowConnectionInfoClick(TObject *Sender);
  void __fastcall Exit1Click(TObject *Sender);
  void __fastcall N1Click(TObject *Sender);
  void __fastcall About1Click(TObject *Sender);
  void __fastcall States1Click(TObject *Sender);
  void __fastcall OperatorLog1Click(TObject *Sender);
  void __fastcall ConnectionInfo1Click(TObject *Sender);
  void __fastcall bFunction1Click(TObject *Sender);
  void __fastcall bFunction2Click(TObject *Sender);
  void __fastcall bFunction3Click(TObject *Sender);
  void __fastcall bFunction4Click(TObject *Sender);
    void __fastcall View1Click(TObject *Sender);

 public:
  __fastcall TfMain( TComponent* );
  __fastcall ~TfMain();
  int  UpdateState( const char* Name, unsigned short Value );

 private:
  void __fastcall ApplicationIdleHandler( TObject*, bool& );
  void Terminate();

  void __fastcall ProcessBCIMessages();

  void EEGSourceSocketAccept();
  void EEGSourceSocketDisconnect();
  void SigProcSocketAccept();
  void SigProcSocketDisconnect();
  void AppSocketAccept();
  void AppSocketDisconnect();
  MessageOrigin Origin( std::istream& );

  void EnterState( SYSSTATUS::State );
  void BroadcastParameters();
  void BroadcastStates();
  void QuitOperator();
  void UpdateDisplay();
  void SetFunctionButtons();

 private:
  PARAMLIST        mParameters;
  STATELIST        mStates;
  SYSSTATUS        mSysstatus;
  SYSLOG           mSyslog;
  PREFERENCES      mPreferences;
  SCRIPT           mScript;
  bool             mTerminated;
  TDateTime        mStarttime;
  enum
  {
    EEGSourcePort = 4000,
    SigProcPort = 4001,
    AppPort = 4002,
  };
  server_tcpsocket  mEEGSourceSocket,
                    mSigProcSocket,
                    mAppSocket;
  tcpstream         mEEGSource,
                    mSigProc,
                    mApp;

  typedef void ( TfMain::*SocketHandler )();
  // mSockets and the m<Module>Socket members are accessed from ReceivingThread
  // without a lock. This is OK as long as they are not accessed outside
  // TfMain::ProcessBCIMessages().
  tcpsocket::set_of_instances         mSockets;
  std::map<tcpsocket*, SocketHandler> mAcceptHandlers,
                                      mDisconnectHandlers;
  std::map<tcpsocket*, tcpstream*>    mStreams;
  std::map<tcpsocket*, bool>          mConnectionStates;

  class ReceivingThread;
  friend class ReceivingThread;
  class ReceivingThread : public TThread
  {
   public:
    ReceivingThread( TfMain* parent )
    : TThread( true ), mParent( *parent )
    {}
   private:
    virtual void __fastcall Execute();
    TfMain& mParent;
  } *mpReceivingThread;

  bool HandleSTATUS(    std::istream& );
  bool HandleSYSCMD(    std::istream& );
  bool HandlePARAM(     std::istream& );
  bool HandleSTATE(     std::istream& );
  bool HandleVisSignal( std::istream& );
  bool HandleVisCfg(    std::istream& );
  bool HandleVisMemo(   std::istream& );

  class _MessageHandler;
  friend class _MessageHandler;
  class _MessageHandler : public MessageHandler
  {
    public:
      _MessageHandler( TfMain& parent ) : mParent( parent ) {}
    private:
      virtual bool HandleSTATUS(    std::istream& is ) { return mParent.HandleSTATUS( is ); }
      virtual bool HandleSYSCMD(    std::istream& is ) { return mParent.HandleSYSCMD( is ); }
      virtual bool HandlePARAM(     std::istream& is ) { return mParent.HandlePARAM( is ); }
      virtual bool HandleSTATE(     std::istream& is ) { return mParent.HandleSTATE( is ); }
      virtual bool HandleVisSignal( std::istream& is ) { return mParent.HandleVisSignal( is ); }
      virtual bool HandleVisCfg(    std::istream& is ) { return mParent.HandleVisCfg( is ); }
      virtual bool HandleVisMemo(   std::istream& is ) { return mParent.HandleVisMemo( is ); }
      TfMain& mParent;
  } mMessageHandler;
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif
