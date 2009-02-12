////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Output into an EDF data file.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef EDF_OUTPUT_FORMAT_H
#define EDF_OUTPUT_FORMAT_H

#include "EDFOutputBase.h"

class EDFOutputFormat: public EDFOutputBase
{
 public:
          EDFOutputFormat();
  virtual ~EDFOutputFormat();

  virtual void Publish() const;
  virtual void Preflight( const SignalProperties&, const StateVector& ) const;
  virtual void Initialize( const SignalProperties&, const StateVector& );
  virtual void StartRun( std::ostream&, const std::string& );
  virtual void StopRun( std::ostream& );
  virtual void Write( std::ostream&, const GenericSignal&, const StateVector& );

  virtual const char* DataFileExtension() const { return ".edf"; }
};

#endif // EDF_OUTPUT_FORMAT_H
