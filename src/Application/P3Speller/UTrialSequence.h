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

#define NUM_STIMULI     12      // in this case, we have 12 stimuli (6 columns and 6 rows)

class TRIALSEQUENCE
{
private: 	// User declarations
        int     ontime, offtime;                   // in units of SampleBlocks
        bool    cur_on;
        int     cur_trialsequence;                 // current sequence within trial
        int     cur_sequence;                      // how many total intensifications did we have
        CORECOMM        *corecomm;
        USERDISPLAY     *userdisplay;
        STATEVECTOR     *statevector;
        GenericVisualization    *vis;
        unsigned short oldrunning;
        TARGET  *selectedtarget;
        TColor  TextColor, TextColorIntensified;
        int     get_argument(int ptr, char *buf, const char *line, int maxlen) const;
        int     cur_stimuluscode;
public:		// User declarations
        TRIALSEQUENCE::TRIALSEQUENCE(PARAMLIST *plist, STATELIST *slist);
        TRIALSEQUENCE::~TRIALSEQUENCE();
        int     Initialize( PARAMLIST *, STATEVECTOR *, CORECOMM *, USERDISPLAY *);
        int     Process(const std::vector<float>&);
        void    ResetTrialSequence();
        bool    onlinemode;
        TARGETLIST      *GetActiveTargets();           // returns targets given a specific parentID (i.e., targetID of selection)
        int             LoadPotentialTargets(const char *targetdeffilename);
        int             Initialize(PARAMLIST *plist);
        TARGETLIST      *targets;                                       // all the potential targets
        int             GetRandomStimulusCode();
        short           IntensifyTargets(int stimuluscode, bool intensify);
        void            SuspendTrial();
        void            GetReadyForTrial();
        void            SetUserDisplayTexts();
        int             char2spellidx;
        AnsiString      TextToSpell, chartospell;
};
#endif
