////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: gschalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: ParamList is a BCI2000 type that represents a collection of
//   parameters.
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
#ifndef PARAM_LIST_H
#define PARAM_LIST_H

#include <string>
#include <vector>
#include <map>
#include "Param.h"
#include "EncodedString.h"

class ParamList
{
 public:
        Param&  operator[]( const std::string& name );
  const Param&  operator[]( const std::string& name ) const;
        Param&  operator[]( size_t index )
                { return mParams.at( index ).Param; }
  const Param&  operator[]( size_t index ) const
                { return mParams.at( index ).Param; }

        int     Size() const
                { return static_cast<int>( mParams.size() ); }
        bool    Empty() const
                { return mParams.empty(); }
        void    Clear();

        bool    Exists( const std::string& name ) const
                { return mNameIndex.find( name ) != mNameIndex.end(); }
        int     Index( const std::string& name ) const
                { return Exists( name ) ? mNameIndex.find( name )->second : Size(); };
        void    Add( const Param& p, float sortingHint = 0.0 );
        void    Add( const Param& p, int sortingHint )
                { Add( p, static_cast<float>( sortingHint ) ); }
        bool    Add( const std::string& paramDefinition );
        void    Delete( const std::string& name );

        bool    Save( const std::string& filename ) const;
        bool    Load( const std::string& filename,
                      bool importNonexisting = true );

        void    Sort();

  // These contain all formatted I/O functionality.
        std::ostream& WriteToStream( std::ostream& ) const;
        std::istream& ReadFromStream( std::istream& );

  // These define binary I/O.
        std::ostream& WriteBinary( std::ostream& ) const;
        std::istream& ReadBinary( std::istream& );

 private:
        void    RebuildIndex();

  struct ParamEntry
  {
    ParamEntry()
      : SortingHint( 0.0 )
      {}
    class Param Param;
    float SortingHint;
    static bool Compare( const ParamEntry& p, const ParamEntry& q )
      { return p.SortingHint < q.SortingHint; }
  };
  typedef std::vector<ParamEntry> ParamContainer;
  ParamContainer mParams;

  typedef std::map<std::string, int, Param::NameCmp> NameIndex;
  NameIndex mNameIndex;
};


inline
std::ostream& operator<<( std::ostream& s, const ParamList& p )
{ return p.WriteToStream( s ); }

inline
std::istream& operator>>( std::istream& s, ParamList& p )
{ return p.ReadFromStream( s ); }

#endif // PARAM_LIST_H

