////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class that implements functionality common to all
//              file writer classes that output into a file.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef FILE_WRITER_BASE_H
#define FILE_WRITER_BASE_H

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
  virtual void Initialize( const SignalProperties& Input,
                           const SignalProperties& Output );
  virtual void StartRun();
  virtual void StopRun();
  virtual void Write( const GenericSignal& Signal,
                      const StateVector&   Statevector );

 protected:
  std::ostream& OutputStream();

 private:
  // A descendant reports the file extension through the DataFileExtension()
  // function.
  virtual const char* DataFileExtension() const = 0;

  std::string              mFileName;
  std::ofstream            mOutputFile;
};

#endif // FILE_WRITER_BASE_H
