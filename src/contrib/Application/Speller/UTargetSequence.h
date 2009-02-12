/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef UTargetSequenceH
#define UTargetSequenceH
//---------------------------------------------------------------------------

#include "UTree.h"
#include "UDictionary.h"
#include "UTarget.h"
#include "Environment.h"
/*shidong starts*/


/*shidong ends*/
#define MAX_TARGETHISTORY       5000
#define MAX_TEXTHISTORY         MAX_TARGETHISTORY


//#define NUM_TARGETS             4               // number of targets


class TARGETSEQUENCE : private Environment
{
private: 	// User declarations
        int     get_argument(int ptr, char *buf, const char *line, int maxlen) const;
        int     activetargets_historynum, text_historynum;
        TARGETLIST      *activetargets_history[MAX_TARGETHISTORY];
        AnsiString      text_history[MAX_TEXTHISTORY];
        DICTIONARY      *dictionary;
        bool            prediction;
public:		// User declarations
        TARGETSEQUENCE::TARGETSEQUENCE();
        TARGETSEQUENCE::~TARGETSEQUENCE();
        /*shidong startS*/
        TARGETLIST      *GetActiveTargets(int, const char *);     // returns targets given a specific parentID (i.e., targetID of selection)
        TARGETLIST      *GetActiveTargetsPrediction(int, BYTE, const char *); // returns targets in the case of word prediction
        /*shidong ends*/
        TARGETLIST      *GetPreviousTargets();                          // returns the previous list of targets
        AnsiString      GetPreviousText();                              // returns the previous text
        int             AddDictionary2PotentialTargets();               // adds the dictionary to the list of potential targets
        void            PushTargetsOnHistory(TARGETLIST *activetargets);
        void            PushTextOnHistory(AnsiString text);
        /*shidong starts*/
        int             LoadPotentialTargets();
        FILE            *f;
        int             NUM_TARGETS;
        bool            debug;
       // int             LoadPotentialTargets(const int targetRow, const int targetCol, const int treeRow, const int treeCol);
        /*shidong ends*/
        void            DeleteHistory();
        int             Initialize(int numTargets);
        TARGETLIST      *targets;                                       // all the potential targets
        TREE            *tree;                                          // tree that is being traversed down if the user makes selections
};
#endif


