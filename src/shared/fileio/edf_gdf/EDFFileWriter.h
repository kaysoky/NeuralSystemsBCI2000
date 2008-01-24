////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that stores data into a EDF data file.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef EDF_FILE_WRITER_H
#define EDF_FILE_WRITER_H

#include "EDFFileWriterBase.h"
#include <vector>
#include <string>

class EDFFileWriter: public EDFFileWriterBase
{
 public:
          EDFFileWriter();
  virtual ~EDFFileWriter();
  virtual void Publish() const;
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const;
  virtual void Initialize( const SignalProperties& Input,
                           const SignalProperties& Output );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Write( const GenericSignal& Signal,
                      const StateVector&   Statevector );

 private:
  virtual const char* DataFileExtension() const { return ".edf"; }
};

#endif // EDF_FILE_WRITER_H
