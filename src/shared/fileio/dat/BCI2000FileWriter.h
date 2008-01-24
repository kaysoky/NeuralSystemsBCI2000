////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A FileWriter filter that stores data into a BCI2000 dat file.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef BCI2000_FILE_WRITER_H
#define BCI2000_FILE_WRITER_H

#include "FileWriterBase.h"

class BCI2000FileWriter: public FileWriterBase
{
 public:
          BCI2000FileWriter();
  virtual ~BCI2000FileWriter();
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
  virtual const char* DataFileExtension() const { return ".dat"; }

  template<SignalType::Type T> void PutBlock( const GenericSignal&,
                                              const StateVector& );
  SignalProperties mInputProperties;
};

#endif // BCI2000_FILE_WRITER_H
