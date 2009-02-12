////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for system commands.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef SYS_COMMAND_H
#define SYS_COMMAND_H

#include <string>
#include <iostream>

class SysCommand
{
  public:
    SysCommand()  {}
    ~SysCommand() {}

  private:
    // The constructor which specifies the string content of
    // a SysCommand should not be created ad hoc --
    // instead, all existing SysCommands should be listed
    // as static constants of this class.
    explicit SysCommand( const std::string& s )
      : mBuffer( s )
      {}

  public:
    bool          operator<( const SysCommand& ) const;
    bool          operator==( const SysCommand& ) const;

    std::ostream& WriteToStream( std::ostream& ) const;
    std::istream& ReadFromStream( std::istream& );
    std::ostream& WriteBinary( std::ostream& ) const;
    std::istream& ReadBinary( std::istream& );

    // This is a list of all SysCommands defined in the protocol.
    // No other SysCommands should be sent.
    static const  SysCommand EndOfState,
                             EndOfParameter,
                             Start,
                             Reset,
                             Suspend;
  private:
    std::string   mBuffer;
};


inline
std::ostream& operator<<( std::ostream& os, const SysCommand& s )
{ return s.WriteToStream( os ); }

inline
std::istream& operator>>( std::istream& is, SysCommand& s )
{ return s.ReadFromStream( is ); }

#endif // SYS_COMMAND_H
