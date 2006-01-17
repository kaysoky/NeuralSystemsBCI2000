#ifndef USysCommandH
#define USysCommandH

#include <string>
#include <iostream>

#define SysCommand SYSCMD

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
    explicit SysCommand( const char* s ) : mBuffer( s ) {}

  public:
    bool          operator<( const SysCommand& ) const;
    bool          operator==( const SysCommand& ) const;

    void          WriteToStream( std::ostream& ) const;
    void          ReadFromStream( std::istream& );
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
{
  s.WriteToStream( os );
  return os;
}

inline
std::istream& operator>>( std::istream& is, SysCommand& s )
{
  s.ReadFromStream( is );
  return is;
}
#endif // USysCommandH
