////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A base class that implements functionality common to all
//              file writer classes that output into a file.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef FILE_WRITER_BASE_H
#define FILE_WRITER_BASE_H

#include "GenericFileWriter.h"
#include "GenericOutputFormat.h"

#include <string>
#include <fstream>

class FileWriterBase: public GenericFileWriter
{
 protected:
          FileWriterBase( GenericOutputFormat& );
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

 private:
  GenericOutputFormat&     mrOutputFormat;
  std::string              mFileName;
  std::ofstream            mOutputFile;
};

#endif // FILE_WRITER_BASE_H
