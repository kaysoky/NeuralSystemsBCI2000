//---------------------------------------------------------------------------
#ifndef UStateH
#define UStateH

#define MAX_STATEVECTORLENGTH   30

#define ERRSTATE_NOERR          0
#define ERRSTATE_INVALIDSTATE   1

#define ERRSTATEVEC_NOSTATE     -1
#define ERRSTATEVEC_NOERR       0

#define LENGTH_NAME             30
#define LENGTH_STATELINE        512

#include <ScktComp.hpp>


class STATE
{
private: 	// User declarations
        char    buffer[LENGTH_STATELINE];
        int     length;
        char    name[LENGTH_NAME];
        unsigned short value;
        int     byteloc;
        int     bitloc;
        int     get_argument(int ptr, char *buf, char *line, int maxlen);
public:		// User declarations
        STATE::STATE();
        STATE::~STATE();
        char    *GetName();
        int     GetLength();
        unsigned short GetValue();
        void    SetValue(unsigned short new_value);
        int     GetByteLocation();
        int     GetBitLocation();
        void    SetByteLocation(int loc);
        void    SetBitLocation(int loc);
        int     ConstructStateLine();
        char    *GetStateLine();
        int     ParseState(char *line, int length);
        bool    valid;
        bool    modified;
};


class STATELIST
{
private: 	// User declarations
        TList   *state_list;
        TList   *GetListPtr();
public:		// User declarations
        STATELIST::STATELIST();
        STATELIST::~STATELIST();
        STATE   *GetStatePtr(int idx);
        STATE   *GetStatePtr(char *name);
        void    AddState2List(STATE *state);
        int     GetNumStates();
        void    ClearStateList();
        void    DeleteState(char *name);
        void    AddState2List(char *statestring);
        int     PublishStates(TCustomWinSocket *Socket);
        int     UpdateState(char *statename, unsigned short newvalue, TCustomWinSocket *socket);
};


class STATEVECTOR
{
private: 	// User declarations
        BYTE    state_vector[MAX_STATEVECTORLENGTH];    // the actual state vector
        int     state_vector_length;                    // the length of the actual state vector
        STATELIST   *state_list;            // a pointer to the list responsible for this vector
public:		// User declarations
        STATEVECTOR::STATEVECTOR(STATELIST *);          // constructor takes the pointer to the state list
        STATEVECTOR::STATEVECTOR(STATELIST *list, bool usepositions);
        STATEVECTOR::~STATEVECTOR();
        void    Initialize_StateVector();
        void    Initialize_StateVector(bool use_assigned_positions);
//        void    SetStateVector(BYTE *src, int length, STATELIST *src_list);
        unsigned short GetStateValue(char *statename);
        unsigned short GetStateValue(int byteloc, int bitloc, int length);
        STATELIST *GetStateListPtr();
        int     SetStateValue(char *statename, unsigned short value);
        int     SetStateValue(int byteloc, int bitloc, int length, unsigned short value);
        int     GetStateVectorLength();
        BYTE    *GetStateVectorPtr();
};

//---------------------------------------------------------------------------
#endif

