//---------------------------------------------------------------------------

#ifndef UCoreCommH
#define UCoreCommH
//---------------------------------------------------------------------------

#include "UGenericSignal.h"
#include "UParameter.h"
#include "UState.h"

#include <Classes.hpp>
#include <scktcomp.hpp>
#include <forms.hpp>

//---------------------------------------------------------------------------
class CORECOMM : public TThread
{
private:
protected:
        void __fastcall Execute();
        TClientSocket   *CoreSocket;
        TForm   *main_form;
        int     moduleID;               // destination module ID (as defined in ..\shared\defines.h)
        bool    sendingparameters;
public:
        __fastcall CORECOMM::CORECOMM(bool CreateSuspended);
        __fastcall CORECOMM::~CORECOMM();
        TClientWinSocket *GetSocket();
        int     Initialize(AnsiString destIP, int destPort, TForm *, int);
        int     PublishStates(STATELIST *statelist);
        int     PublishParameters(PARAMLIST *paramlist);
        int     PublishParameter(PARAM *param);
        void    StartSendingParameters();
        void    StopSendingParameters();
        int     SendStatus(char *line);
        bool    Connected();
        bool    SendData2CoreModule(GenericIntSignal *my_signal, PARAM *channellistparam);
        bool    SendStateVector2CoreModule(STATEVECTOR *statevector);
        void    SendSysCommand(char *syscmdbuf);
};
//---------------------------------------------------------------------------
#endif
