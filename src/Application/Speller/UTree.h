//---------------------------------------------------------------------------

#ifndef UTreeH
#define UTreeH
//---------------------------------------------------------------------------

#define MAX_TREESIZE    65535
#define MAX_BRANCHES    16             // maximum number of branches from one node

class TREE
{
private: 	// User declarations
        int     parentID[MAX_TREESIZE], displaypos[MAX_TREESIZE], targetID[MAX_TREESIZE];
        int     get_argument(int ptr, char *buf, char *line, int maxlen);
        int     treesize;
public:		// User declarations
        TREE::TREE();
        TREE::~TREE();
        int     LoadTree(char *filename);
        int     DetermineTargetID(int parentID, int displaypos);
        bool    DoesLeadTo(int cur_parentID, int cur_targetID);
        bool    HasChildren(int cur_parentID);
};
#endif
