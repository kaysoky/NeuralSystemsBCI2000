//---------------------------------------------------------------------------

#ifndef UTargetSequenceH
#define UTargetSequenceH
//---------------------------------------------------------------------------

#include "UTree.h"
#include "UParameter.h"
#include "UState.h"
#include "UDictionary.h"
#include "UTarget.h"

#define MAX_TARGETHISTORY       5000
#define MAX_TEXTHISTORY         MAX_TARGETHISTORY


#define NUM_TARGETS             4               // number of targets

class TARGETSEQUENCE
{
private: 	// User declarations
        int     get_argument(int ptr, char *buf, char *line, int maxlen);
        int     activetargets_historynum, text_historynum;
        TARGETLIST      *activetargets_history[MAX_TARGETHISTORY];
        AnsiString      text_history[MAX_TEXTHISTORY];
        DICTIONARY      *dictionary;
        bool            prediction;
public:		// User declarations
        TARGETSEQUENCE::TARGETSEQUENCE(PARAMLIST *plist, STATELIST *slist);
        TARGETSEQUENCE::~TARGETSEQUENCE();
        TARGETLIST      *GetActiveTargets(int, BYTE, char *);           // returns targets given a specific parentID (i.e., targetID of selection)
        TARGETLIST      *GetActiveTargetsPrediction(int, BYTE, char *); // returns targets in the case of word prediction
        TARGETLIST      *GetPreviousTargets();                          // returns the previous list of targets
        AnsiString      GetPreviousText();                              // returns the previous text
        int             AddDictionary2PotentialTargets();               // adds the dictionary to the list of potential targets
        void            PushTargetsOnHistory(TARGETLIST *activetargets);
        void            PushTextOnHistory(AnsiString text);
        int             LoadPotentialTargets(char *targetdeffilename, char *treedeffilename);
        void            DeleteHistory();
        int             Initialize(PARAMLIST *plist);
        TARGETLIST      *targets;                                       // all the potential targets
        TREE            *tree;                                          // tree that is being traversed down if the user makes selections
};
#endif


