//---------------------------------------------------------------------------

#ifndef USysCommandH
#define USysCommandH
//---------------------------------------------------------------------------

#define LENGTH_SYSCMD          256

#define ERRSYSCMD_NOERR        0

class SYSCMD
{
private: 	// User declarations
        char    buffer[LENGTH_SYSCMD];
public:		// User declarations
        SYSCMD::SYSCMD();
        SYSCMD::~SYSCMD();
        int     ParseSysCmd(const char *line, int length);
        const char* SYSCMD::GetSysCmd();
        void WriteToStream( class std::ostream& ) const;
        void ReadFromStream( class std::istream& );
        class std::ostream& WriteBinary( class std::ostream& ) const;
        class std::istream& ReadBinary( class std::istream& );
};

inline class std::ostream& operator<<( class std::ostream& os, const SYSCMD& s )
{
  s.WriteToStream( os );
  return os;
}

inline class std::istream& operator>>( class std::istream& is, SYSCMD& s )
{
  s.ReadFromStream( is );
  return is;
}
#endif
