//---------------------------------------------------------------------------
#ifndef UStateH
#define UStateH

#include <vcl.h>

#define MAX_STATEVECTORLENGTH   30

#define ERRSTATE_NOERR          0
#define ERRSTATE_INVALIDSTATE   1

#define ERRSTATEVEC_NOSTATE     -1
#define ERRSTATEVEC_NOERR       0

#define LENGTH_NAME             30
#define LENGTH_STATELINE        512


class STATE
{
  friend class STATEVECTOR;
  friend class STATELIST; // calls GetValue() which is to be omitted
  
private: 	// User declarations
        mutable char    buffer[LENGTH_STATELINE];
        int     length;
        char    name[LENGTH_NAME];
        unsigned short value; // will be renamed writeCache if GetValue can be omitted
        int     byteloc;
        int     bitloc;
        int     get_argument(int ptr, char *buf, const char *line, int maxlen) const;
        bool    valid;
        bool    modified;
        int     ConstructStateLine() const;
        void    SetByteLocation(int loc);
        void    SetBitLocation(int loc);
protected:
        unsigned short GetValue() const;
        void Commit( STATEVECTOR* );

public:		// User declarations
        STATE::STATE();
        const char    *GetName() const;
        int     GetLength() const;
        void    SetValue(unsigned short new_value);
        int     GetByteLocation() const;
        int     GetBitLocation() const;
        const char    *GetStateLine() const;
        int     ParseState(const char *line, int length);
        bool    Valid() const { return valid; }
};


class STATELIST
{
private: 	// User declarations
        TList   *state_list;
public:		// User declarations
        STATELIST::STATELIST();
        STATELIST::~STATELIST();
        STATE   *GetStatePtr(int idx) const;
        STATE   *GetStatePtr(const char *name) const;
        void    AddState2List(const char *statestring);
        void    AddState2List(const STATE *state);
        int     GetNumStates() const;
        void    ClearStateList();
        void    DeleteState(const char *name);
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
        unsigned short GetStateValue(const char *statename) const;
        unsigned short GetStateValue(int byteloc, int bitloc, int length) const;
        STATELIST *GetStateListPtr();
        int     SetStateValue(const char *statename, unsigned short value);
        int     SetStateValue(int byteloc, int bitloc, int length, unsigned short value);
        int     GetStateVectorLength() const;
        BYTE    *GetStateVectorPtr();
        const BYTE* GetStateVectorPtr() const;
        void    CommitStateChanges();
};

//---------------------------------------------------------------------------
#endif

