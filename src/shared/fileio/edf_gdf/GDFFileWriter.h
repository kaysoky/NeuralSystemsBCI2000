////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that stores data into a GDF data file.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef GDF_FILE_WRITER_H
#define GDF_FILE_WRITER_H

#include "FileWriterBase.h"
#include "GDFOutputFormat.h"

class GDFFileWriter: public FileWriterBase
{
 public:
  GDFFileWriter()
  : FileWriterBase( mOutputFormat )
  {}
 private:
  GDFOutputFormat mOutputFormat;
};

#endif // GDF_FILE_WRITER_H
