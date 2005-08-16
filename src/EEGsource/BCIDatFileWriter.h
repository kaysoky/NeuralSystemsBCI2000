////////////////////////////////////////////////////////////////////////////////
//
// File: BCIDatFileWriter.h
//
// Date: Nov 11, 2003
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A filter that handles data acquisition from a GenericADC
//              and storing into a file.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef BCIDatFileWriterH
#define BCIDatFileWriterH

#include "GenericFileWriter.h"
#include "UGenericVisualization.h"

#include <fstream>

class BCIDatFileWriter: public GenericFileWriter
{
 public:
          BCIDatFileWriter();
  virtual ~BCIDatFileWriter();
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const;
  virtual void Initialize2( const SignalProperties& Input,
                            const SignalProperties& Output );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Write( const GenericSignal& Signal,
                      const STATEVECTOR&   Statevector );

 private:
  template<SignalType::Type T> void PutBlock( const GenericSignal&, const STATEVECTOR& );
  
  SignalProperties         mInputProperties;
  std::string              mFileName;
  std::ofstream            mOutputFile;
};

#endif // BCIDatFileWriterH