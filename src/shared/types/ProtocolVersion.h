////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for protocol version matching.
//   Compatibility-breaking modifications to the protocol will be reflected
//   by an increment of the VersionID constant.
//   Upon connecting to the operator module, core modules report their
//   protocol version, and incompatible versions are rejected.
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
#ifndef PROTOCOL_VERSION_H
#define PROTOCOL_VERSION_H

#include <iostream>

class ProtocolVersion
{
   //static const unsigned int CurrentVersionID = 1; // 2.0 Release
   static const unsigned int CurrentVersionID = 2;   // Multi-sample state vector

  public:
   ProtocolVersion()
     : mVersionID( 0 )
     {}
   ~ProtocolVersion()
     {}

  private:
   ProtocolVersion( unsigned int inVersionID ) : mVersionID( inVersionID ) {}

  public:
   bool          Matches( const ProtocolVersion& p ) const
     { return mVersionID == p.mVersionID; }
   bool          MoreRecentThan( const ProtocolVersion& p ) const
     { return mVersionID > p.mVersionID; }

   std::ostream& WriteToStream( std::ostream& os ) const
     { return os << mVersionID; }
   std::istream& ReadFromStream( std::istream& is )
     { return is >> mVersionID; }
   std::ostream& WriteBinary( std::ostream& os ) const
     { return ( os << mVersionID ).put( '\0' ); }
   std::istream& ReadBinary( std::istream& is )
     { return ( is >> mVersionID ).ignore(); }

   static ProtocolVersion None()
     { return ProtocolVersion( 0 ); }
   static ProtocolVersion Current()
     { return ProtocolVersion( CurrentVersionID ); }

  private:
   unsigned int mVersionID;
};


inline
std::ostream& operator<<( std::ostream& os, const ProtocolVersion& s )
{ return s.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, ProtocolVersion& s )
{ return s.ReadFromStream( is ); }

#endif // PROTOCOL_VERSION_H
