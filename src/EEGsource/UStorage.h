//---------------------------------------------------------------------------

#ifndef UStorageH
#define UStorageH
//---------------------------------------------------------------------------

#include "UParameter.h"
#include "UState.h"
#include "UGenericSignal.h"

#include <Classes.hpp>
#include <scktcomp.hpp>
#include <forms.hpp>
#include "BCIDirectry.h"

#define MAX_BUFFERS     20

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
public:
        __fastcall TDataStorage::TDataStorage(PARAMLIST *paramlist);
        __fastcall TDataStorage::~TDataStorage();
        void TDataStorage::Initialize(PARAMLIST *Newparamlist, STATELIST *NewStateList, STATEVECTOR *Newstatevector);
        void TDataStorage::CreateFileName( BCIDtry * );
        char *TDataStorage::GetFileName();
        void TDataStorage::WriteHeader();
        bool TDataStorage::Write2Disk(GenericIntSignal *StoreSignal);
};
//---------------------------------------------------------------------------
#endif
