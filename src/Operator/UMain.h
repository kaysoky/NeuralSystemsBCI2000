//---------------------------------------------------------------------------
#ifndef UMainH
#define UMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ScktComp.hpp>
#include <ExtCtrls.hpp>
#include <Chart.hpp>
#include <Series.hpp>
#include <TeEngine.hpp>
#include <ComCtrls.hpp>
#include <TeeProcs.hpp>
#include <AppEvnts.hpp>
#include "Trayicon.h"
#include <Menus.hpp>
#include "trayicon.h"
#include "UVisual.h"

#define PANEL_SYSSTATUS         0
#define PANEL_EEGSOURCE         1
#define PANEL_SIGPROC           2
#define PANEL_APPLICATION       3

#define TXT_WINDOW_CAPTION      "BCI2000/Operator"
#define TXT_OPERATOR_VERSION    "V1.10"
#define TXT_OPERATOR_SUSPENDED  "Suspended"
#define TXT_OPERATOR_RUNNING    "Running"

//---------------------------------------------------------------------------
class TfMain : public TForm
{
protected:
#if 0
    void __fastcall OpenVisual(TMessage &Message);
#endif
    void __fastcall DoResetOperator(TMessage &Message);
#ifndef NEW_DOUBLEBUF_SCHEME
    void __fastcall Render(TMessage &Message);
#endif // NEW_DOUBLEBUF_SCHEME
    BEGIN_MESSAGE_MAP
#if 0
      MESSAGE_HANDLER(WINDOW_OPEN, TMessage, OpenVisual)
#endif
      MESSAGE_HANDLER(RESET_OPERATOR, TMessage, DoResetOperator)
#ifndef NEW_DOUBLEBUF_SCHEME
      MESSAGE_HANDLER(WINDOW_RENDER, TMessage, Render)
#endif // NEW_DOUBLEBUF_SCHEME
    END_MESSAGE_MAP(TControl)
__published:	// IDE-managed Components
        TServerSocket *SourceSocket;
        TServerSocket *SigProcSocket;
        TServerSocket *ApplicationSocket;
        TTimer *ScrUpdateTimer;
        TStatusBar *StatusBar;
        TButton *bReset;
        TButton *bRunSystem;
        TButton *bConfig;
        TButton *bSetConfig;
        TLabel *Label7;
        TLabel *Label8;
        TButton *bShowConnectionInfo;
        TTrayIcon *TrayIcon;
        TTimer *ActiveTimer;
        TMainMenu *MainMenu1;
        TMenuItem *File1;
        TMenuItem *N1;
        TMenuItem *Exit1;
        TMenuItem *Help1;
        TMenuItem *About1;
        TMenuItem *View1;
        TMenuItem *States1;
        TMenuItem *OperatorLog1;
        TButton *bFunction1;
        TButton *bFunction2;
        TButton *bFunction3;
        TButton *bFunction4;
        void __fastcall ApplicationSocketClientDisconnect(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall SigProcSocketClientDisconnect(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall SourceSocketClientDisconnect(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall ScrUpdateTimerTimer(TObject *Sender);
        void __fastcall bResetClick(TObject *Sender);
        void __fastcall SourceSocketClientError(TObject *Sender,
          TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
          int &ErrorCode);
        void __fastcall SigProcSocketClientError(TObject *Sender,
          TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
          int &ErrorCode);
        void __fastcall ApplicationSocketClientError(TObject *Sender,
          TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
          int &ErrorCode);
        void __fastcall SourceSocketGetThread(TObject *Sender,
          TServerClientWinSocket *ClientSocket,
          TServerClientThread *&SocketThread);
        void __fastcall SourceSocketAccept(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall SigProcSocketGetThread(TObject *Sender,
          TServerClientWinSocket *ClientSocket,
          TServerClientThread *&SocketThread);
        void __fastcall SigProcSocketAccept(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall ApplicationSocketAccept(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall ApplicationSocketGetThread(TObject *Sender,
          TServerClientWinSocket *ClientSocket,
          TServerClientThread *&SocketThread);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall bRunSystemClick(TObject *Sender);
        void __fastcall bConfigClick(TObject *Sender);
        void __fastcall bSetConfigClick(TObject *Sender);
        void __fastcall ApplicationEvents1Idle(TObject *Sender,
          bool &Done);
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall bShowConnectionInfoClick(TObject *Sender);
        void __fastcall SetTrayIcon(TObject *Sender);        
        void __fastcall RemoveTrayIcon(TObject *Sender);
        void __fastcall ActiveTimerTimer(TObject *Sender);
        void __fastcall Exit1Click(TObject *Sender);
        void __fastcall N1Click(TObject *Sender);
        void __fastcall About1Click(TObject *Sender);
        void __fastcall States1Click(TObject *Sender);
        void __fastcall OperatorLog1Click(TObject *Sender);
        void __fastcall bFunction1Click(TObject *Sender);
        void __fastcall bFunction2Click(TObject *Sender);
        void __fastcall bFunction3Click(TObject *Sender);
        void __fastcall bFunction4Click(TObject *Sender);
private:	// User declarations
        bool    firsttime;
        TDateTime       starttime;
public:		// User declarations
        __fastcall TfMain(TComponent* Owner);
        int     HandleCoreMessage(COREMESSAGE *message, int module);
        int     ReceiveBufBytes(TCustomWinSocket *Socket, char *buf, int length);
        void    ResetOperator();
        int     BroadcastParameters();
        int     BroadcastStates();
        int     BroadcastStateVector(STATEVECTOR *my_state_vector);
        int     UpdateState(char *statename, unsigned short newvalue);
        void    SendSysCommand(char *syscmdbuf, TCustomWinSocket *socket);
        void    ShutdownSystem();
        void    StartSuspendSystem(bool update);
        void    QuitOperator();
        void    SetFunctionButtons();
        PARAMLIST       paramlist;
        STATELIST       statelist;
#if 0
        VISCFGLIST      viscfglist;
#endif
        SYSLOG          *syslog;
        PREFERENCES     preferences;
        SCRIPT          script;
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif
