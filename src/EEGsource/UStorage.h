//---------------------------------------------------------------------------

#ifndef UStorageH
#define UStorageH
//---------------------------------------------------------------------------

#include "UParameter.h"
#include "UState.h"
#include "UGenericSignal.h"
#include "BCIDirectry.h"

#include <Classes.hpp>
#include <scktcomp.hpp>
#include <forms.hpp>

#define MAX_BUFFERS     100

//---------------------------------------------------------------------------
class TDataStorage : public TThread
{
private:
        BCIDtry *bcidtry;
        int useflag;            // code for use of filename
protected:
        void __fastcall Execute();
        TCriticalSection *critsec[MAX_BUFFERS];
        STATELIST       *statelist;
        PARAMLIST       *paramlist;
        STATEVECTOR     *statevector;
        AnsiString      AName;
        char    *buffer[MAX_BUFFERS];
        int     length[MAX_BUFFERS];
        char    *FName;
        int     max;
        TEvent  *event;
        bool    AlreadyIncremented;
        int     OldRunNo;
        int     NewRunNo;
        bool    saveprmfile;
        int     Channels, StateVectorLen;
        void    CreateFileName( BCIDtry * );
        char    *GetFileName();
public:
        __fastcall TDataStorage::TDataStorage(PARAMLIST *paramlist);
        __fastcall TDataStorage::~TDataStorage();
        int     Initialize(PARAMLIST *Newparamlist, STATELIST *NewStateList, STATEVECTOR *Newstatevector);
        void    WriteHeader();
        bool    Write2Disk(const GenericIntSignal *StoreSignal);
};
//---------------------------------------------------------------------------
#endif
