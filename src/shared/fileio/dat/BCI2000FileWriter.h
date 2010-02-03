////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A FileWriter filter that stores data into a BCI2000 dat file.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef BCI2000_FILE_WRITER_H
#define BCI2000_FILE_WRITER_H

#include "FileWriterBase.h"
#include "BCI2000OutputFormat.h"

class BCI2000FileWriter : public FileWriterBase
{
 public:
  BCI2000FileWriter()
  : FileWriterBase( mOutputFormat )
  {}
  virtual void Publish() const;
  virtual void Preflight( const SignalProperties& Input,
                                SignalProperties& Output ) const;
  virtual void StartRun();

 private:
  BCI2000OutputFormat mOutputFormat;
};

#endif // BCI2000_FILE_WRITER_H
