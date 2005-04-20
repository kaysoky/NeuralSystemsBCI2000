//---------------------------------------------------------------------------
#ifndef UStatusH
#define UStatusH

#define ERRSTATUS_NOERR          0

#define LENGTH_STATUSLINE        512

class STATUS
{
 public:
  typedef enum
  {
    unknown,
    initialized,
    running,
    suspended,
    warning,
    error,
  } ContentType;

  STATUS();
  STATUS( const char*, int );

  const char* GetStatus() const;
  int         GetCode()   const;
  ContentType Content()   const;

  void                ReadFromStream( class std::istream& );
  void                WriteToStream( class std::ostream& ) const;
  class std::istream& ReadBinary( class std::istream& );
  class std::ostream& WriteBinary( class std::ostream& ) const;

  static const STATUS Fail;

 private:
  int     ParseStatus( const char* line, int length );
  
  char    mStatus[ LENGTH_STATUSLINE ];
  int     mCode;
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

