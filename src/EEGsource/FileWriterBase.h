////////////////////////////////////////////////////////////////////////////////
// $Id$
// File: FileWriterBase.cpp
//
// Date: Feb 17, 2006
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A base class that implements functionality common to all
//              file writer classes that output into a file.
//
// $Log$
// Revision 1.3  2006/04/25 18:05:21  mellinger
// Some changes for gcc compatibility.
//
// Revision 1.2  2006/02/23 19:34:08  mellinger
// Moved OutputStream() accessor definition into cpp file.
//
// Revision 1.1  2006/02/18 12:11:00  mellinger
// Support for EDF and GDF data formats.
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef FileWriterBaseH
#define FileWriterBaseH

#include "GenericFileWriter.h"

#include <fstream>

class FileWriterBase: public GenericFileWriter
{
 protected:
          FileWriterBase();
 public:
  virtual ~FileWriterBase();
  virtual void Publish() const;
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const;
  virtual void Initialize2( const SignalProperties& Input,
                            const SignalProperties& Output );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Write( const GenericSignal& Signal,
                      const STATEVECTOR&   Statevector );

 protected:
  std::ostream& OutputStream();

 private:
  // A descendant reports the file extension through the DataFileExtension()
  // function.
  virtual const char* DataFileExtension() const = 0;

  std::string              mFileName;
  std::ofstream            mOutputFile;
};

#endif // FileWriterBaseH
