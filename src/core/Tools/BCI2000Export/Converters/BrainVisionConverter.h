//////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A BCIReader class for data file output in
//   BrainVision GDR format.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////
#ifndef BRAIN_VISION_CONVERTERS_H
#define BRAIN_VISION_CONVERTERS_H

#include <fstream>
#include <string>
#include "BCIReader.h"

std::string ExtractFileName( const std::string& );

// This one uses BrainVision's Generic Data Reader.
class BrainVisionGDRConverter : public BCIReader
{
 public:
  BrainVisionGDRConverter();
  virtual ~BrainVisionGDRConverter();

  static BCIReader* CreateInstance()
    { return new BrainVisionGDRConverter; }

 protected:
  virtual void InitOutput( OutputInfo& );
  virtual void ExitOutput();
  virtual void OutputSignal( const GenericSignal&, long inSamplePos );
  virtual void OutputStateChange( const State&, short, long inSamplePos );
  virtual void OutputStateRange( const State&, short, long inBeginPos, long inEndPos );

 private:
  long          mCurMarker;
  std::ofstream mDataFile,
                mHeaderFile,
                mMarkerFile;
};

#endif // BRAIN_VISION_CONVERTERS_H


