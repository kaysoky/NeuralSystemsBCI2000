////////////////////////////////////////////////////////////////////////////////
// $Id$
// File: EDFFileWriter.h
//
// Date: Feb 3, 2006
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A filter that stores data into a EDF data file.
//
// $Log$
// Revision 1.1  2006/02/18 12:11:00  mellinger
// Support for EDF and GDF data formats.
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef EDFFileWriterH
#define EDFFileWriterH

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
  virtual void Initialize2( const SignalProperties& Input,
                            const SignalProperties& Output );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Write( const GenericSignal& Signal,
                      const STATEVECTOR&   Statevector );

 private:
  virtual const char* DataFileExtension() const { return ".edf"; }
};

#endif // EDFFileWriterH