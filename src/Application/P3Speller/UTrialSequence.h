//---------------------------------------------------------------------------

#ifndef UTrialSequenceH
#define UTrialSequenceH
//---------------------------------------------------------------------------
#include <vector>

#include "UGenericVisualization.h"
#include "UserDisplay.h"
#include "UEnvironment.h"

#define SEQ_ITI                 1
#define SEQ_PTP                 2
#define SEQ_FEEDBACK            3
#define SEQ_OUTCOME             4
#define SEQ_CONGRATULATIONS     5

/*shidong starts*/
#define MAX_STIMULI     100      
/*shidong ends*/

class TRIALSEQUENCE : public Environment
{
private: 	// User declarations
        int     ontime, offtime;                   // in units of SampleBlocks
        bool    cur_on;
        int     cur_trialsequence;                 // current sequence within trial
        int     cur_sequence;                      // how many total intensifications did we have
        USERDISPLAY     *userdisplay;
        GenericVisualization    *vis;
        unsigned short oldrunning;
        TARGET  *selectedtarget;
        TColor  TextColor, TextColorIntensified;
        int     get_argument(int ptr, char *buf, const char *line, int maxlen) const;
        int     cur_stimuluscode;
public:		// User declarations
        TRIALSEQUENCE::TRIALSEQUENCE();
        TRIALSEQUENCE::~TRIALSEQUENCE();
        int     Initialize(USERDISPLAY *);
        int     Process(const std::vector<float>&);
        void    ResetTrialSequence();
        bool    onlinemode;
        TARGETLIST      *GetActiveTargets();           // returns targets given a specific parentID (i.e., targetID of selection)
        /*shidong starts*/
        int     LoadPotentialTargets(const int matrixColumn, const int matrixRow);
        int     NumMatrixColumns;
        int     NumMatrixRows;
        int     NUM_STIMULI;      // in this case, we have 12 stimuli (6 columns and 6 rows)
        FILE    *f;
        bool    debug;
        /*shidong ends*/
        int             Initialize();
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
