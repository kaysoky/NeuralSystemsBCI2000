//---------------------------------------------------------------------------
#ifndef UStatusH
#define UStatusH

#define ERRSTATUS_NOERR          0

#define LENGTH_STATUSLINE        512

class STATUS
{
private: 	// User declarations
        char    status[LENGTH_STATUSLINE];
        int     code;
public:		// User declarations
        int     ParseStatus(const char *line, int length);
        const char *GetStatus();
        int     GetCode();
        
        void ReadFromStream( class std::istream& );
        void WriteToStream( class std::ostream& ) const;
        class std::istream& ReadBinary( class std::istream& );
        class std::ostream& WriteBinary( class std::ostream& ) const;
};

//---------------------------------------------------------------------------
inline class std::ostream& operator<<( class std::ostream& os, const STATUS& s )
{
  s.WriteToStream( os );
  return os;
}

inline class std::istream& operator>>( class std::istream& is, STATUS& s )
{
  s.ReadFromStream( is );
  return is;
}
#endif

