/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef UStatusH
#define UStatusH

#include <iostream>

class STATUS
{
 public:
  enum
  {
    ERRSTATUS_NOERR = 0,
    LENGTH_STATUSLINE = 512,
  };

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

  void                ReadFromStream( std::istream& );
  void                WriteToStream( std::ostream& ) const;
  std::istream& ReadBinary( std::istream& );
  std::ostream& WriteBinary( std::ostream& ) const;

  static const STATUS Fail;

 private:
  int     ParseStatus( const char* line, int length );
  
  char    mStatus[ LENGTH_STATUSLINE ];
  int     mCode;
};


inline
std::ostream& operator<<( std::ostream& os, const STATUS& s )
{
  s.WriteToStream( os );
  return os;
}

inline
std::istream& operator>>( std::istream& is, STATUS& s )
{
  s.ReadFromStream( is );
  return is;
}
#endif // UStatusH





