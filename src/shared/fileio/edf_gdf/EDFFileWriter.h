////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that stores data into a EDF data file.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef EDF_FILE_WRITER_H
#define EDF_FILE_WRITER_H

#include "FileWriterBase.h"
#include "EDFOutputFormat.h"

class EDFFileWriter: public FileWriterBase
{
 public:
  EDFFileWriter()
  : FileWriterBase( mOutputFormat )
  {}
 private:
  EDFOutputFormat mOutputFormat;
};

#endif // EDF_FILE_WRITER_H
