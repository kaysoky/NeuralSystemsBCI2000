//////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A BCIReader class for data file output in ASCII format.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////
#ifndef ASCII_CONVERTER_H
#define ASCII_CONVERTER_H

#include <fstream>
#include <string>
#include <map>
#include <vector>
#include "BCIReader.h"

std::string ExtractFileName( const std::string& );

class ASCIIConverter : public BCIReader
{
  enum
  {
    defaultPrecision = -1,
    highPrecision = 20,
   };

 public:
  ASCIIConverter( int precision = defaultPrecision );
  virtual ~ASCIIConverter();

  static BCIReader* CreateInstance()
    { return new ASCIIConverter; }
  static BCIReader* CreateInstanceHighPrecision()
    { return new ASCIIConverter( highPrecision ); }

 protected:
  virtual void  InitOutput( OutputInfo& );
  virtual void  ExitOutput();
  virtual void  OutputSignal( const GenericSignal&, long inSamplePos );
  virtual void  OutputStateChange( const State&, short, long inSamplePos );
  virtual void  OutputStateRange( const State&, short, long inBeginPos, long inEndPos )
      {}

 private:
  int                            mPrecision;
  std::ofstream                  mDataFile;
  std::vector<State::ValueType>  mStateValues;
  std::map<std::string, size_t>  mStateIndices;
};

#endif // ASCII_CONVERTER_H


