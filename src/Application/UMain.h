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

#include "Task.h"

//---------------------------------------------------------------------------

#define ERR_NOERR       0
#define ERR_NOSOCKCONN  1
#define ERR_NOSOCKPARAM 2

#define MODULENAME      "Application"
#define DEFAULTPRMNAME  "default.prm"

class TfMain : public TForm
{
protected:
    void __fastcall HandleMessage(TMessage &Message);
    void __fastcall ResetMessage(TMessage &Message);
    BEGIN_MESSAGE_MAP
      MESSAGE_HANDLER(HANDLE_MESSAGE, TMessage, HandleMessage)
      MESSAGE_HANDLER(RESET_MESSAGE, TMessage, ResetMessage)
    END_MESSAGE_MAP(TControl)
__published:	// IDE-managed Components
        TEdit *eOperatorIP;
        TLabel *Label1;
        TLabel *Label2;
        TEdit *eOperatorPort;
        TButton *bConnect;
        TButton *bDisconnect;
        TServerSocket *ReceivingSocket;
        TEdit *eReceivingPort;
        TEdit *eReceivingIP;
        TLabel *Label3;
        TLabel *Label4;
        TLabel *Label5;
        TEdit *eSendingIP;
        TLabel *Label6;
        TEdit *eSendingPort;
        TCheckBox *rReceivingConnected;
        TCheckBox *rSendingConnected;
        void __fastcall bConnectClick(TObject *Sender);
        void __fastcall bDisconnectClick(TObject *Sender);
        void __fastcall ReceivingSocketClientDisconnect(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall ReceivingSocketGetThread(TObject *Sender,
          TServerClientWinSocket *ClientSocket,
          TServerClientThread *&SocketThread);
        void __fastcall FormCreate(TObject *Sender);
private:	// User declarations
        bool    GUImode;
public:		// User declarations
        __fastcall TfMain(TComponent* Owner);
        __fastcall ~TfMain( void );
        void   HandleCoreMessage(COREMESSAGE *coremessage);
        int    InitializeConnections();
        void   SendStatus(char *line, TCustomWinSocket *Socket);
        void   ShutdownSystem();
        void   ShutdownConnections();
        int    StartupApplication(AnsiString connectto);
        void   SendSysCommand(char *syscmdbuf, CORECOMM *corecomm);
        PARAMLIST       paramlist;
        STATELIST       statelist;
        CORECOMM        *corecomm, *sendingcomm;
        TTask  *Task;
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif
