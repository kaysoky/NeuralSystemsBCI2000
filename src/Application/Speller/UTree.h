//---------------------------------------------------------------------------

#ifndef UTreeH
#define UTreeH
//---------------------------------------------------------------------------

#define MAX_TREESIZE    65535
#define MAX_BRANCHES    16             // maximum number of branches from one node

#include "UEnvironment.h"

class TREE : public Environment
{
private: 	// User declarations
        int     parentID[MAX_TREESIZE], displaypos[MAX_TREESIZE], targetID[MAX_TREESIZE];
        int     get_argument(int ptr, char *buf, const char *line, int maxlen) const;
        int     treesize;
public:		// User declarations
        TREE::TREE();
        TREE::~TREE();
        /*shidong starts*/
        int     LoadTree();
        FILE            *a ;
        bool    debug;
        //int     LoadTree(const int treeRow, const int treeCol);
        /*shidong ends*/
        int     DetermineTargetID(int parentID, int displaypos);
        bool    DoesLeadTo(int cur_parentID, int cur_targetID);
        bool    HasChildren(int cur_parentID);
};
#endif
