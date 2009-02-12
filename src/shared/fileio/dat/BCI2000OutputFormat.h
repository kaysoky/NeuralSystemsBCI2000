////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An output class for the BCI2000 dat format.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef BCI2000_OUTPUT_FORMAT_H
#define BCI2000_OUTPUT_FORMAT_H

#include "GenericOutputFormat.h"

class BCI2000OutputFormat : public GenericOutputFormat
{
 public:
          BCI2000OutputFormat() {}
  virtual ~BCI2000OutputFormat() {}

  virtual void Publish() const;
  virtual void Preflight( const SignalProperties&, const StateVector& ) const;
  virtual void Initialize( const SignalProperties&, const StateVector& );
  virtual void StartRun( std::ostream&, const std::string& );
  virtual void StopRun( std::ostream& );
  virtual void Write( std::ostream&, const GenericSignal&, const StateVector& );
  virtual void Halt() {}

  virtual const char* DataFileExtension() const { return ".dat"; }

 private:
  template<SignalType::Type T> void PutBlock( std::ostream&,
                                              const GenericSignal&,
                                              const StateVector& );
  SignalProperties mInputProperties;
  int              mStatevectorLength;
};

#endif // BCI2000_OUTPUT_FORMAT_H
