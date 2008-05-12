////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for protocol version matching.
//   Compatibility-breaking modifications to the protocol will be reflected
//   by an increment of the VersionID constant.
//   Upon connecting to the operator module, core modules report their
//   protocol version, and incompatible versions are rejected.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
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
