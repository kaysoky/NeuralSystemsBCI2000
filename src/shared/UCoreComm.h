//---------------------------------------------------------------------------

#ifndef UCoreCommH
#define UCoreCommH
//---------------------------------------------------------------------------

#include <Classes.hpp>
#include <scktcomp.hpp>
#include <forms.hpp>

//---------------------------------------------------------------------------
class STATELIST;
class STATEVECTOR;
class PARAM;
class PARAMLIST;
class GenericIntSignal;

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
        TClientWinSocket *GetSocket() const;
        int     Initialize(AnsiString destIP, int destPort, TForm *, int);
        int     PublishStates(const STATELIST *statelist) const;
        int     PublishParameters(const PARAMLIST *paramlist);
        int     PublishParameter(const PARAM *param) const;
        void    StartSendingParameters();
        void    StopSendingParameters();
        int     SendStatus(const char *line) const;
        bool    Connected() const;
        bool    SendData2CoreModule(const GenericIntSignal *my_signal, const PARAM *channellistparam) const;
        bool    SendStateVector2CoreModule(const STATEVECTOR *statevector) const;
        void    SendSysCommand(const char *syscmdbuf) const;
};
//---------------------------------------------------------------------------
#endif
