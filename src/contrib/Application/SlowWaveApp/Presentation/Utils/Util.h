/////////////////////////////////////////////////////////////////////////////
//
// File: Util.h
//
// Date: Oct 26, 2001
//
// Author: Juergen Mellinger
//
// Description: A collection of useful platform independent macros
//              and algorithms.
//
// Changes:
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef UTILH
#define UTILH

#include <string>
#include <exception>
#include <math.h>

#define EPS 1e-5
#define MAX(a,b)    ((a)>(b)?(a):(b))
#define MIN(a,b)    ((a)<(b)?(a):(b))
#define ROUND(a)    (floor((a)+0.5))
// A modulo function that always returns positive arguments.
#define MOD(a,b)    (((b)+(a)%(b))%(b))
// An integer division that does not round towards zero but towards minus infinity.
#define DIV(a,b)    ( ((a)<0) ? (-(-(a)-1)/(b)-1) : ((a)/(b)) )


class Util
{
  public:
    static std::string ConvertLiterals( const std::string& inString );

    class TPath : public std::string
    {
      public:
        TPath();
        TPath( const std::string &inString );
        ~TPath() {}

        TPath        operator+(  const std::string &inString );
        const TPath& operator+=( const std::string &inString );

        static void  Initialize( const char *inResourceDirectory );

      private:
        static std::string resourceDirectory;
    };
};

inline
const Util::TPath&
Util::TPath::operator+=( const std::string &inString )
{
  *this = *this + inString;
  return *this;
}

#endif // UTILH
