#ifndef USysCommandH
#define USysCommandH

#define LENGTH_SYSCMD          256
#define ERRSYSCMD_NOERR        0

class SYSCMD
{
  private:
    char    mBuffer[ LENGTH_SYSCMD ];

  public:
    SYSCMD();
    explicit SYSCMD( const char* );
    ~SYSCMD();
    
    int           ParseSysCmd( const char* line, int length );
    const char*   GetSysCmd() const;
    void          WriteToStream( class std::ostream& ) const;
    void          ReadFromStream( class std::istream& );
    class std::ostream& WriteBinary( class std::ostream& ) const;
    class std::istream& ReadBinary( class std::istream& );
    bool          operator<( const SYSCMD& ) const;
    bool          operator==( const SYSCMD& ) const;

    static const  SYSCMD EndOfState,
                         EndOfParameter,
                         Start,
                         Reset,
                         Run,
                         Suspend,
                         Success,
                         Recoverable,
                         Fatal;
};

inline
class std::ostream& operator<<( class std::ostream& os, const SYSCMD& s )
{
  s.WriteToStream( os );
  return os;
}

inline
class std::istream& operator>>( class std::istream& is, SYSCMD& s )
{
  s.ReadFromStream( is );
  return is;
}
#endif // USysCommandH
