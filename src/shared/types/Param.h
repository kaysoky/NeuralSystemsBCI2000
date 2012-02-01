////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: gschalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A type that represents a single BCI2000 parameter.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#ifndef PARAM_H
#define PARAM_H

#include <locale>
#include <string>
#include <vector>
#include <map>
#include "EncodedString.h"
#include "LabelIndex.h"
#include "HierarchicalLabel.h"

class Param
{
  friend class ParamList;
  // A class that represents a single parameter value entry, accommodating
  // strings and subparameters.
  public:
   class ParamValue
   {
    public:
     enum
     {
       Null,
       Single,
       List,
       Matrix
     };

     ParamValue()
       : mpString( new EncodedString ), mpParam( NULL )
       {}
     ParamValue( const ParamValue& p )
       : mpString( NULL ), mpParam( NULL )
       { Assign( p ); }
     ParamValue( const char* s )
       : mpString( new EncodedString( s ) ), mpParam( NULL )
       {}
     ParamValue( const std::string& s )
       : mpString( new EncodedString( s ) ), mpParam( NULL )
       {}
     ParamValue( const Param& p )
       : mpString( NULL ), mpParam( new Param( p ) )
       {}
     ~ParamValue()
       { delete mpString; delete mpParam; }

     const ParamValue& operator=( const ParamValue& p )
       { Assign( p ); return *this; }
     const ParamValue& operator=( const char* s )
       { Assign( std::string( s ) ); return *this; }
     const ParamValue& operator=( const std::string& s )
       { Assign( s ); return *this; }
     const ParamValue& operator=( const Param& p )
       { Assign( p ); return *this; }

     operator const std::string&() const
       { return ToString(); }
     const char* c_str() const
       { return ToString().c_str(); }
     operator Param*()
       { return ToParam(); }
     operator const Param*() const
       { return ToParam(); }
     Param* operator->()
       { return ToParam(); }
     const Param* operator->() const
       { return ToParam(); }

     int Kind() const;

     void Assign( const ParamValue& );
     void Assign( const std::string& );
     void Assign( const Param& );
     const std::string& ToString() const;
     const Param*       ToParam() const;
     Param*             ToParam();

     std::ostream& WriteToStream( std::ostream& os ) const;
     std::istream& ReadFromStream( std::istream& is );

    private:
     void ConstructParamBuf() const;

     EncodedString* mpString;
     Param*         mpParam;

     static Param       sParamBuf;
     static std::string sStringBuf;
  };

 public:
  // Construction, destruction, assignment
                     Param();
                     Param( const std::string& name,
                            const std::string& section,
                            const std::string& type = "int",
                            const std::string& value = "0",
                            const std::string& defaultvalue = "0",
                            const std::string& lowrange = "0",
                            const std::string& highrange = "0",
                            const std::string& comment = "" );
            explicit Param( const std::string& parameterDefinition );
                    ~Param() {}
  Param&             operator=( const Param& );
  Param&             AssignValues( const Param& );

  // Sections
  HierarchicalLabel& Sections()
                     { mChanged = true; return mSections; }
  const HierarchicalLabel& Sections() const
                     { return mSections; }

  Param&             SetSection( const std::string& s );
  std::string        Section() const
                     { return mSections.empty() ? cEmptyString : mSections[ 0 ]; }

  // Type
  Param&             SetType( const std::string& s )
                     { mChanged = true; mType = s; tolower( mType ); return *this; }
  const std::string& Type() const
                     { return mType; }

  // Name
 private: // Changing the name without changing its index in the list would be
          // a bad idea, so this function is private.
  Param&             SetName( const std::string& s )
                     { mChanged = true; mName = s; return *this; }
 public:
  const std::string& Name() const
                     { return mName; }

  // Default, LowRange, HighRange, Comment
  Param&             SetDefaultValue( const std::string& s )
                     { mChanged = true; mDefaultValue = s; return *this; }
  const std::string& DefaultValue() const
                     { return mDefaultValue; }

  Param&             SetLowRange( const std::string& s )
                     { mChanged = true; mLowRange = s; return *this; }
  const std::string& LowRange() const
                     { return mLowRange; }

  Param&             SetHighRange( const std::string& s )
                     { mChanged = true; mHighRange = s; return *this; }
  const std::string& HighRange() const
                     { return mHighRange; }

  Param&             SetComment( const std::string& s )
                     { mChanged = true; mComment = s; return *this; }
  const std::string& Comment() const
                     { return mComment; }

  // Changed flag
  Param&             Unchanged()
                     { mChanged = false; return *this; }
  bool               Changed() const
                     { return mChanged; }

  // Dimensions
  Param&             SetNumValues( size_t n );
  int                NumValues() const
                     { return static_cast<int>( mValues.size() ); }

  Param&             SetNumRows( size_t rows )
                     { return SetDimensions( rows, NumColumns() ); }
  int                NumRows() const
                     { return NumValues() / NumColumns(); }

  Param&             SetNumColumns( size_t cols )
                     { return SetDimensions( NumRows(), cols ); }
  int                NumColumns() const
                     { return mDim2Index.Size(); }

  Param&             SetDimensions( size_t rows, size_t cols );

  // Accessors for bounds-checked, recursive access to parameter entries
  const ParamValue&  Value( size_t idx = 0 ) const;
  ParamValue&        Value( size_t idx = 0 );

  const ParamValue&  Value( size_t row, size_t col ) const;
  ParamValue&        Value( size_t row, size_t col );
 private:
  void               BoundsCheck( size_t row, size_t col ) const;

 public:
  const ParamValue&  Value( const std::string& label ) const
                     { return Value( mDim1Index[ label ] ); }
  ParamValue&        Value( const std::string& label )
                     { return Value( mDim1Index[ label ] ); }

  const ParamValue&  Value( size_t row, const std::string& colLabel  ) const
                     { return Value( row, mDim2Index[ colLabel ] ); }
  ParamValue&        Value( size_t row, const std::string& colLabel )
                     { return Value( row, mDim2Index[ colLabel ] ); }

  const ParamValue&  Value( const std::string& rowLabel, size_t col ) const
                     { return Value( mDim1Index[ rowLabel ], col ); }
  ParamValue&        Value( const std::string& rowLabel, size_t col )
                     { return Value( mDim1Index[ rowLabel ], col ); }

  const ParamValue&  Value( const std::string& rowLabel, const std::string& colLabel ) const
                     { return Value( mDim1Index[ rowLabel ], mDim2Index[ colLabel ] ); }
  ParamValue&        Value( const std::string& rowLabel, const std::string& colLabel )
                     { return Value( mDim1Index[ rowLabel ], mDim2Index[ colLabel ] ); }

  // Labels
  LabelIndex&         RowLabels()
                      { mChanged = true; return mDim1Index; }
  const LabelIndex&   RowLabels() const
                      { return mDim1Index; }

  LabelIndex&         ColumnLabels()
                      { mChanged = true; return mDim2Index; }
  const LabelIndex&   ColumnLabels() const
                      { return mDim2Index; }

  LabelIndex&         Labels()
                      { mChanged = true; return mDim1Index; }
  const LabelIndex&   Labels() const
                      { return mDim1Index; }

  // Stream io
  std::ostream&       WriteToStream( std::ostream& ) const;
  std::istream&       ReadFromStream( std::istream& );
  std::ostream&       WriteBinary( std::ostream& ) const;
  std::istream&       ReadBinary( std::istream& );

 private:
  HierarchicalLabel   mSections;
  EncodedString       mName,
                      mType,
                      mDefaultValue,
                      mLowRange,
                      mHighRange;
  std::string         mComment;
  LabelIndex          mDim1Index,
                      mDim2Index;
  typedef std::vector<ParamValue> ValueContainer;
  bool                mChanged;
  ValueContainer      mValues;
  const EncodedString cEmptyString;

 // A comparator that compares parameters by their section entries.
 public:
  static bool CompareBySection( const Param& p1, const Param& p2 )
    { return p1.mSections < p2.mSections; }

 // Case insensitive string handling components.
 private:
  static const std::ctype<char>& ct();

 public:
  static void tolower( std::string& );
  static void toupper( std::string& );

  static bool ciless( char a, char b )
  { return ct().toupper( a ) < ct().toupper( b ); }
  static bool ciequal( char a, char b )
  { return ct().toupper( a ) == ct().toupper( b ); }

  static bool strciless( const std::string& a, const std::string& b )
  { return NameCmp()( a, b ); }
  static bool strciequal( const std::string& a, const std::string& b )
  { return !strciless( a, b ) && !strciless( b, a ); }

  class NameCmp
  {
   public:
    bool operator()( const std::string& a, const std::string& b ) const
    { return std::lexicographical_compare(
                        a.begin(), a.end(), b.begin(), b.end(), ciless ); }
  };
};


inline
std::ostream& operator<<( std::ostream& s, const Param::ParamValue& p )
{ return p.WriteToStream( s ); }

inline
std::istream& operator>>( std::istream& s, Param::ParamValue& p )
{ return p.ReadFromStream( s ); }

inline
std::ostream& operator<<( std::ostream& s, const Param& p )
{ return p.WriteToStream( s ); }

inline
std::istream& operator>>( std::istream& s, Param& p )
{ return p.ReadFromStream( s ); }

#endif // PARAM_H

