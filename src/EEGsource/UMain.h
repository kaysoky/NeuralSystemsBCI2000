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
//---------------------------------------------------------------------------

#define ERR_NOERR       0
#define ERR_NOSOCKCONN  1
#define ERR_NOSOCKPARAM 2

#define MODULENAME      "EEGsource"
#define DEFAULTPRMNAME  "default.prm"

class TfMain : public TForm
{
protected:
    void __fastcall HandleMessage(TMessage &Message);
    void __fastcall StartDaqMessage(TMessage &Message);
    void __fastcall ResetMessage(TMessage &Message);
    BEGIN_MESSAGE_MAP
      MESSAGE_HANDLER(HANDLE_MESSAGE, TMessage, HandleMessage)
      MESSAGE_HANDLER(STARTDAQ_MESSAGE, TMessage, StartDaqMessage)
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
        void   HandleCoreMessage(COREMESSAGE *coremessage);
        int    InitializeConnections();
        void   SendStatus(char *line, TCustomWinSocket *Socket);
        int    Write2SignalProc(GenericIntSignal *, STATEVECTOR *, PARAM *);
        void   SendSysCommand(char *syscmdbuf, TCustomWinSocket *socket);
        void   SendSysCommand(char *syscmdbuf, CORECOMM *corecomm);
        PARAMLIST       paramlist;
        STATELIST       statelist;
        void    ShutdownConnections();
        void    ShutdownSystem();
        void    MainDataAcqLoop();
        int     StartupDataAcquisition(AnsiString connectto);
        void    UpdateStateVector();
        int     ParametersConsistent();
        void    SetEEGDisplayProperties();
        int     resetrequest;
        TEvent  *statevectorupdate;
        #ifdef ADC_DTADC
         DTADC               *adc;
        #else
         RandomNumberADC     *adc;
        #endif
        GenericVisualization *vis, *roundtripvis;
        TDataStorage         *tds;
        CORECOMM             *corecomm, *sendingcomm;
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif

