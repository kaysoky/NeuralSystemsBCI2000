////////////////////////////////////////////////////////////////////////////////
//
// File:   LogFile.h
//
// Date:   Feb 8, 2004
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A std::ofstream descendant that centralizes/encapsulates details
//         of a log file.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef LogFileH
#define LogFileH

#include <fstream>
#include <string>
#include "UEnvironment.h"

class LogFile: public std::ofstream, Environment
{
  public:
    //LogFile() : mExtension( ".log" ) {}
    explicit LogFile( const char* extension ) : mExtension( extension ) {}

  private:
    LogFile( const LogFile& );
    LogFile& operator=( const LogFile& );

  public:
    void        Preflight() const;
    void        Initialize();

  private:
    std::string FilePath() const;

    std::string mExtension;
};

#endif // LogFileH
