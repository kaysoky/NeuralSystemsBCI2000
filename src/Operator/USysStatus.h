//---------------------------------------------------------------------------
#ifndef USysStatusH
#define USysStatusH
//---------------------------------------------------------------------------

#include <ScktComp.hpp>

#define STATE_IDLE              0
#define STATE_PUBLISHING        1
#define STATE_INFORMATION       2
#define STATE_INITIALIZATION    3
#define STATE_RUNNING           4
#define STATE_OPERATING         5
#define STATE_SUSPENDED         6

class SYSSTATUS
{
private:	// User declarations
public:		// User declarations
        SYSSTATUS::SYSSTATUS();
        void    ResetSysStatus();
        bool    SourceConnected;
        bool    SigProcConnected;
        bool    ApplicationConnected;
        TCustomWinSocket *SourceSocket, *SigProcSocket, *ApplicationSocket;
        bool    EEGsourceEOS, SigProcEOS, ApplicationEOS;
        bool    EEGsourceINI, SigProcINI, ApplicationINI;
        AnsiString EEGsourceStatus, SigProcStatus, ApplicationStatus;
        int     SystemState;
        long    NumMessagesRecv1, NumMessagesRecv2, NumMessagesRecv3;
        long    NumStatesRecv1, NumStatesRecv2, NumStatesRecv3;
        long    NumStateVecsRecv1, NumStateVecsRecv2, NumStateVecsRecv3;
        long    NumParametersRecv1, NumParametersRecv2, NumParametersRecv3;
        long    NumDataRecv1, NumDataRecv2, NumDataRecv3;
        long    NumMessagesSent1, NumMessagesSent2, NumMessagesSent3;
        long    NumStatesSent1, NumStatesSent2, NumStatesSent3;
        long    NumStateVecsSent1, NumStateVecsSent2, NumStateVecsSent3;
        long    NumParametersSent1, NumParametersSent2, NumParametersSent3;
        bool    EEGsourceStatusReceived, SigProcStatusReceived, ApplicationStatusReceived;
};
#endif

