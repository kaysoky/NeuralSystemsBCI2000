//---------------------------------------------------------------------------
#ifndef UStateH
#define UStateH

#define MAX_STATEVECTORLENGTH   30

#define ERRSTATE_NOERR          0
#define ERRSTATE_INVALIDSTATE   1

#define ERRSTATEVEC_NOSTATE     -1
#define ERRSTATEVEC_NOERR       0

#define LENGTH_NAME             30
#define LENGTH_STATELINE        255

#include <vector>
#include <string>

typedef unsigned char BYTE;

class STATE
{
  friend class STATEVECTOR;
  friend class STATELIST; // calls GetValue()
  friend class CoreModule; // calls GetValue()
  friend class TfMain;     // calls GetValue()

private:    // User declarations
        int     length;
        char    name[LENGTH_NAME];
        unsigned short value;
        int     byteloc;
        int     bitloc;
        bool    valid;
        bool    modified;
        void    SetByteLocation(int loc);
        void    SetBitLocation(int loc);
private:
        unsigned short GetValue() const;
        void Commit( STATEVECTOR* );

public:     // User declarations
        STATE::STATE();
        const char    *GetName() const;
        int     GetLength() const;
        void    SetValue(unsigned short new_value);
        int     GetByteLocation() const;
        int     GetBitLocation() const;

        void WriteToStream( std::ostream& ) const;
        void ReadFromStream( std::istream& );
        std::ostream& WriteBinary( std::ostream& ) const;
        std::istream& ReadBinary( std::istream& );

#if 1 // Changed return type to a copied string value to avoid multithreading trouble.
      // In the future, this function should be replaced by using stream i/o.
        std::string GetStateLine() const;
#endif
        int     ParseState(const char *line, int length);
        bool    Valid() const { return valid; }
};


class STATELIST
{
private:    // User declarations
        typedef std::vector<STATE*> _state_list;
        _state_list state_list;
        _state_list::size_type GetStateIndex( const char* name ) const;
public:     // User declarations
        STATELIST::STATELIST();
        STATELIST::~STATELIST();
        STATE   *GetStatePtr(int idx) const;
        STATE   *GetStatePtr(const char *name) const;
        void    AddState2List(const char *statestring);
        void    AddState2List(const STATE *state);
        int     GetNumStates() const;
        void    ClearStateList();
        void    DeleteState(const char *name);
        
        void WriteToStream( std::ostream& ) const;
        void ReadFromStream( std::istream& );
};


class STATEVECTOR
{
 private: // User declarations
  BYTE    state_vector[MAX_STATEVECTORLENGTH];    // the actual state vector
  int     state_vector_length;                    // the length of the actual state vector
  STATELIST   *state_list;            // a pointer to the list responsible for this vector
 public:  // User declarations
  STATEVECTOR(STATELIST *);          // constructor takes the pointer to the state list
  STATEVECTOR(STATELIST *list, bool usepositions);
  ~STATEVECTOR();
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
  void WriteToStream( std::ostream& ) const;
  void ReadFromStream( std::istream& );
  std::ostream& WriteBinary( std::ostream& ) const;
  std::istream& ReadBinary( std::istream& );

 // A class that allows for convenient automatic type conversions when
 // accessing state values.
 public:
  class type_adapter
  {
   private:
    type_adapter& operator=( const type_adapter& );
   public:
    type_adapter()
    : p( NULL ), byte( 0 ), bit( 0 ), length( 0 ), defVal( 0 ) {}
    type_adapter( STATEVECTOR* inStatevector, int inByte, int inBit, int inLength, short inDefval = 0 )
    : p( inStatevector ), byte( inByte ), bit( inBit ), length( inLength ), defVal( inDefval ) {}
    const type_adapter& operator=( int i )
    { p && p->SetStateValue( byte, bit, length, i ); return *this; }
    operator int() const
    { return p ? p->GetStateValue( byte, bit, length ) : defVal; }
   private:
    STATEVECTOR* p;
    int byte, bit, length;
    short defVal;
  };
};

//---------------------------------------------------------------------------

inline std::ostream& operator<<( std::ostream& os, const STATE& s )
{
  s.WriteToStream( os );
  return os;
}

inline std::istream& operator>>( std::istream& is, STATE& s )
{
  s.ReadFromStream( is );
  return is;
}

inline std::ostream& operator<<( std::ostream& os, const STATELIST& s )
{
  s.WriteToStream( os );
  return os;
}

inline std::istream& operator>>( std::istream& is, STATELIST& s )
{
  s.ReadFromStream( is );
  return is;
}

inline std::ostream& operator<<( std::ostream& os, const STATEVECTOR& s )
{
  s.WriteToStream( os );
  return os;
}

inline std::istream& operator>>( std::istream& is, STATEVECTOR& s )
{
  s.ReadFromStream( is );
  return is;
}
#endif

