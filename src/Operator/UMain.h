#ifndef UMainH
#define UMainH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>

#include <set>
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
#define TXT_OPERATOR_VERSION    "V1.41"
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

  void EnterState( SYSSTATUS::State );
  void BroadcastParameters();
  void BroadcastEndOfParameter();
  void BroadcastStates();
  void BroadcastEndOfState();
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

  class CoreConnection;
  friend class CoreConnection;
  class CoreConnection : public MessageHandler
  {
    public:
      CoreConnection( TfMain&, MessageOrigin, short port );
      ~CoreConnection();

      void ProcessBCIMessages();
      template<typename T> bool PutMessage( const T& t )
      { return MessageHandler::PutMessage<T>( mStream, t ).flush(); }
      MessageOrigin Origin() const { return mOrigin; }

    private:
      virtual bool HandleSTATUS(    std::istream& );
      virtual bool HandleSYSCMD(    std::istream& );
      virtual bool HandlePARAM(     std::istream& );
      virtual bool HandleSTATE(     std::istream& );
      virtual bool HandleVisSignal( std::istream& );
      virtual bool HandleVisCfg(    std::istream& );
      virtual bool HandleVisMemo(   std::istream& );

      void OnAccept();
      void OnDisconnect();

      TfMain&          mParent;
      MessageOrigin    mOrigin;
      short            mPort;
      server_tcpsocket mSocket;
      tcpstream        mStream;
      bool             mConnected;

  };

  typedef std::set<CoreConnection*> SetOfConnections;
  SetOfConnections mCoreConnections;
  tcpsocket::set_of_instances mSockets;
  // Note that these members must be declared after the containers
  // because their constructors require already-initialized containers.
  CoreConnection mEEGSource,
                 mSigProc,
                 mApp;


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
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif
