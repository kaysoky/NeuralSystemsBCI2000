//---------------------------------------------------------------------------

#ifndef AppManagerH
#define AppManagerH
//---------------------------------------------------------------------------
#include <math.h>
#include "UGenericFilter.h"
//--------------------------------------------------------------------------------------------------
//------------------ Unit for Decision-, Run- and Trial-Management ---------------------------------
//-------------------- programed by Dr. Thilo Hinterberger 2001 ------------------------------------
//--------------------------------------------------------------------------------------------------


// for source status
#define NONE     0
#define BCIFILE  1
#define SWTRANS  2
#define DAS1402  3


// for system status
#define PAUSE         0
#define RUNNING       1
#define INIT_REQUIRED 2
#define START         3
#define STOP          4

// for rec status
#define NOSTORAGE     0
#define READY         1
#define STORING       2

class TSTATUS {
private:
public:
        TSTATUS();
        int SysStatus;             // contains the system status
        int SourceStatus;          // contains the source status
        int RecStatus;             // contains the recording status
        int TrialStatus;           // contains the trial status
        int OldSysStatus;          // contains the previous status
        bool FBFlag;               // true if feedback is on 
        char WorkingDir[128];      // contains the working directory
        char DataDir[128];         // contains the data directory
};



class TDecider {
private:
        STATEVECTOR *statevector;
        int DecisionMode;
        float ResultValue;
        int Initialized;
        int Position;
        float ZeroInt;
        int DecisionTime;
        int DecisionCh;
        int NegThreshold;
        int PosThreshold;
        int CoeffNum;
        void SetResultStates(float Res);
     // voting variables
        void Vote();
        int Right, Wrong;
        int Invalid;
        int PosOK, Posf;
        int NegOK, Negf;
        int IDFOK, IDFf;
public:
        TDecider(PARAMLIST *paramlist, STATELIST *statelist);
        void Initialize(PARAMLIST *paramlist, STATEVECTOR *Newstatevector);
    // data related functions
        float Process(short *signals );
        void GetVote(int &GetRight, int &GetWrong, int &GetInvalid);
        void GetVote(int &GetPosOK, int &GetPosf, int &GetNegOK, int &GetNegf, int &GetIDFOK, int &GetIDFf);
        void ResetVote();
};

class TTaskManager {
private:
        STATEVECTOR *statevector;
        PARAMLIST *paramlist;
        int TaskMode;
        int PosInTrial;
        int TaskBegin;
        int SeqPosition;
        int SeqLength;
        void SetTask();
public:
        TTaskManager(PARAMLIST *Newparamlist, STATELIST *statelist);
        void Initialize(PARAMLIST *Newparamlist, STATEVECTOR *NewStateVector);
        void Process(bool StartFlag);
};

class TSessionManager {
private:
        STATEVECTOR *statevector;
        PARAMLIST *paramlist;
        TSTATUS *status;
        int TrialsARun;
        int BIPts, FIPts, ITIPts;
        int PosInTrial;
        int PosAfterTrig;
        int StartAfterTrig;
        int TrialStartMode;
public:
        TSessionManager(PARAMLIST *paramlist, STATELIST *statelist);
        void Initialize(PARAMLIST *paramlist, STATEVECTOR *Newstatevector, TSTATUS *NewStatus);
        void Process(bool StartFlag);
};


class TClassSequencer
{
 private:
        STATEVECTOR *statevector;
        int PosInTrial;
        int ClBeg, ClEnd;
        bool Initialized;
public:
        TClassSequencer(PARAMLIST *paramlist, STATELIST *statelist);
        void Initialize(PARAMLIST *paramlist, STATEVECTOR *Newstatevector);
        void Process(bool StartFlag);
};

//---------------------------------------------------------------------------
#endif

