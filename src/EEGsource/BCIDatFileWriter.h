////////////////////////////////////////////////////////////////////////////////
// $Id$
// File: BCIDatFileWriter.cpp
//
// Date: Jun 22, 2005
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A filter that stores data into a BCI2000 dat file.
//
// $Log$
// Revision 1.4  2006/04/25 18:05:21  mellinger
// Some changes for gcc compatibility.
//
// Revision 1.3  2006/02/18 12:11:00  mellinger
// Support for EDF and GDF data formats.
//
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef BCIDatFileWriterH
#define BCIDatFileWriterH

#include "FileWriterBase.h"

class BCIDatFileWriter: public FileWriterBase
{
 public:
          BCIDatFileWriter();
  virtual ~BCIDatFileWriter();
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
  virtual const char* DataFileExtension() const { return ".dat"; }

  template<SignalType::Type T> void PutBlock( const GenericSignal&,
                                              const STATEVECTOR& );
  SignalProperties mInputProperties;
};

#endif // BCIDatFileWriterH
