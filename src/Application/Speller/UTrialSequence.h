//---------------------------------------------------------------------------

#ifndef UTrialSequenceH
#define UTrialSequenceH
//---------------------------------------------------------------------------
#include <vector>

#include "UGenericVisualization.h"
#include "UserDisplay.h"

#define SEQ_ITI                 1
#define SEQ_PTP                 2
#define SEQ_FEEDBACK            3
#define SEQ_OUTCOME             4
#define SEQ_CONGRATULATIONS     5

class TRIALSEQUENCE
{
private: 	// User declarations
        int     ititime, max_ititime;                   // in units of SampleBlocks
        int     pretrialtime, max_pretrialtime;         // in units of SampleBlocks
        int     feedbacktime, max_feedbacktime;         // in units of SampleBlocks
        int     outcometime, max_outcometime;           // in units of SampleBlocks
        int     congrattime, max_congrattime;           // in units of SampleBlocks
        int     cur_sequence;                           // current sequence
        USERDISPLAY     *userdisplay;
        STATEVECTOR     *statevector;
        GenericVisualization    *vis;
        unsigned short oldrunning;
        TARGET  *selectedtarget;
        bool    highlightcorrecttarget;
        void    SuspendTrial();
public:		// User declarations
        TRIALSEQUENCE::TRIALSEQUENCE(PARAMLIST *plist, STATELIST *slist);
        TRIALSEQUENCE::~TRIALSEQUENCE();
        int     Initialize( PARAMLIST *, STATEVECTOR *, USERDISPLAY *, int);
        int     correcttargetID;                        // targetID of the next correct target
        void    ITI();
        void    PTP();
        void    Feedback(const std::vector<float>& controlsignal);
        TARGET  *Outcome();
        void    Congratulations();
        TARGET  *Process(const std::vector<float>&);
        void    ResetTrialSequence();
        void    Switch2Congratulations();
        /*shidong starts*/
        int     NUM_TARGETS;
        /*shidong ends*/
};
#endif
