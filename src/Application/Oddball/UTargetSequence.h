//---------------------------------------------------------------------------

#ifndef UTargetSequenceH
#define UTargetSequenceH
//---------------------------------------------------------------------------

#include "UParameter.h"
#include "UState.h"
#include "UTarget.h"


#define NUM_TARGETS             4               // number of targets

class TARGETSEQUENCE
{
private: 	// User declarations
        int     get_argument(int ptr, char *buf, char *line, int maxlen);
        int     probability;          
public:		// User declarations
        TARGETSEQUENCE::TARGETSEQUENCE(PARAMLIST *plist, STATELIST *slist);
        TARGETSEQUENCE::~TARGETSEQUENCE();
        TARGETLIST      *GetActiveTargets();           // returns targets given a specific parentID (i.e., targetID of selection)
        int             LoadPotentialTargets(char *targetdeffilename);
        int             Initialize(PARAMLIST *plist);
        TARGETLIST      *targets;                                       // all the potential targets
};
#endif


