/******************************************************************************
 * $Id$                                                                       *
 * Program:   BCI2000                                                         *
 * File:      UState.h                                                        *
 * Comment:   This unit provides support for system-wide states,              *
 *            lists of states, and the state vector                           *
 * Version:   0.10                                                            *
 * Author:    Gerwin Schalk                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.08 - 03/30/2000 - First commented version                               *
 * V0.09 - 07/22/2003 - Replaced VCL's TList with STL's std::vector<>         *
 *                      to avoid linking against the VCL in command line      *
 *                      tools using the STATELIST class, jm                   *
 * V0.10 - 07/24/2003 - Introduced stream based i/o, jm                       *
 * $Log$
 * Revision 1.19  2006/01/12 20:19:18  mellinger
 * Various fixes.
 *
 * Revision 1.18  2006/01/11 19:07:28  mellinger
 * Revision of interface style to match corresponding parameter classes.
 *
 ******************************************************************************/
#ifndef UStateH
#define UStateH

#include <vector>
#include <string>
#include "UParameter.h"

class STATE
{
  friend class STATEVECTOR;
  friend class STATELIST;  // calls GetValue()
  friend class CoreModule; // calls GetValue()
  friend class TfMain;     // calls GetValue()

 public:
  typedef unsigned short value_type;

 public:
  STATE();
  ~STATE() {}

  const char*   GetName() const            { return mName.c_str(); }
  size_t        GetLocation() const        { return mLocation; }
  size_t        GetLength() const          { return mLength; }

  void          SetValue( value_type );

  std::ostream& WriteToStream( std::ostream& ) const;
  std::istream& ReadFromStream( std::istream& );
  std::ostream& WriteBinary( std::ostream& ) const;
  std::istream& ReadBinary( std::istream& );

  class namecmp
  {
   public:
    bool operator()( const std::string& a, const std::string& b ) const
    { return ::stricmp( a.c_str(), b.c_str() ) < 0; }
  };

  size_t     GetByteLocation() const
             { return mLocation / 8; }
  size_t     GetBitLocation() const
             { return mLocation % 8; }
 private:
  value_type GetValue() const
             { return mValue; }
  void       SetLocation( size_t location )
             { mLocation = location; }
  void       SetByteLocation( size_t location )
             { SetLocation( location * 8 + GetBitLocation() ); }
  void       SetBitLocation( size_t location )
             { SetLocation( GetByteLocation() * 8 + location ); }

  void       Commit( STATEVECTOR* );

 private:
  std::string mName;
  value_type  mValue;
  size_t      mLocation,
              mLength;
  bool        mModified;
};


typedef std::vector<STATE> state_container;

class STATELIST : private state_container
{
 public:
  const STATE& operator[]( const std::string& ) const;
  STATE&       operator[]( const std::string& );
  const STATE& operator[]( size_t idx ) const
               { return at( idx ); }
  STATE&       operator[]( size_t idx )
               { return at( idx ); }

  size_t Size() const
         { return size(); }
  bool   Empty() const
         { return empty(); }
  void   Clear();

  bool   Exists( const std::string& name ) const
         { return mIndex.find( name ) != mIndex.end(); }
  void   Add( const STATE& s )
         { ( *this )[ s.mName ] = s; }
  bool   Add( const std::string& stateDefinition );
  void   Delete( const std::string& name );

  std::ostream& WriteToStream( std::ostream& ) const;
  std::istream& ReadFromStream( std::istream& );
  std::ostream& WriteBinary( std::ostream& ) const;
  std::istream& ReadBinary( std::istream& );

  // Backward compatibility
  STATE* GetStatePtr( size_t idx )
         { return idx < size() ? &at( idx ) : NULL; }
  STATE* GetStatePtr( const char* name ) 
         { return Exists( name ) ? &operator[]( name ) : NULL; }
  bool   AddState2List( const char* statestring )
         { return Add( statestring ); }
  void   AddState2List( const STATE* state )
         { Delete( state->GetName() ); Add( *state ); }
  int    GetNumStates() const
         { return Size(); }
  void   ClearStateList()
         { Clear(); }
  void   DeleteState( const char* name )
         { Delete( name ); }

 private:
  void   RebuildIndex();

  typedef std::map<std::string, int, STATE::namecmp> state_index;
  state_index mIndex;
};


class STATEVECTOR
{
 public:
  STATEVECTOR( const STATEVECTOR& );
  explicit STATEVECTOR( STATELIST& list, bool use_positions = false );
  ~STATEVECTOR();
  const STATEVECTOR& operator=( const STATEVECTOR& );

 private:
  void           Initialize( bool use_assigned_positions = false );
 public:
  int            Length() const
                 { return mByteLength; }
  unsigned char* Data()
                 { return mpData; }
  const unsigned char* Data() const
                 { return mpData; }
  STATELIST&     Statelist()
                 { return *mpStatelist; }

  STATE::value_type GetStateValue( const std::string& name ) const;
  STATE::value_type GetStateValue( size_t location, size_t length) const;
  void              SetStateValue( const std::string& name, STATE::value_type value );
  void              SetStateValue( size_t location, size_t length, STATE::value_type value );
  void              PostStateChange( const std::string& name, STATE::value_type value );
  void              CommitStateChanges();

  std::ostream&  WriteToStream( std::ostream& ) const;
  std::istream&  ReadFromStream( std::istream& );
  std::ostream&  WriteBinary( std::ostream& ) const;
  std::istream&  ReadBinary( std::istream& );

  // Backward compatibility
  STATEVECTOR( STATELIST* list, bool use_positions = false );
  int            GetStateVectorLength() const
                 { return Length(); }
  unsigned char* GetStateVectorPtr()
                 { return Data(); }
  const unsigned char* GetStateVectorPtr() const
                 { return Data(); }
  STATELIST*     GetStateListPtr()
                 { return &Statelist(); }
  STATE::value_type GetStateValue( int byteLocation, int bitLocation, size_t length) const
                 { return GetStateValue( byteLocation * 8 + bitLocation, length ); }
  void           SetStateValue(  int byteLocation, int bitLocation, size_t length, STATE::value_type value )
                 { SetStateValue( byteLocation * 8 + bitLocation, length, value ); }

 private:
  unsigned char* mpData;      // the actual state vector
  size_t         mByteLength; // the length of the actual state vector
  STATELIST*     mpStatelist; // a pointer to the list responsible for this vector
};


inline std::ostream& operator<<( std::ostream& os, const STATE& s )
{
  return s.WriteToStream( os );
}

inline std::istream& operator>>( std::istream& is, STATE& s )
{
  return s.ReadFromStream( is );
}

inline std::ostream& operator<<( std::ostream& os, const STATELIST& s )
{
  return s.WriteToStream( os );
}

inline std::istream& operator>>( std::istream& is, STATELIST& s )
{
  return s.ReadFromStream( is );
}

inline std::ostream& operator<<( std::ostream& os, const STATEVECTOR& s )
{
  return s.WriteToStream( os );
}

inline std::istream& operator>>( std::istream& is, STATEVECTOR& s )
{
  return s.ReadFromStream( is );
}
#endif // UStateH

