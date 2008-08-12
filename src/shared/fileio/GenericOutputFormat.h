////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A virtual class interface for output data formats.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef GENERIC_OUTPUT_FORMAT_H
#define GENERIC_OUTPUT_FORMAT_H

#include "Environment.h"
#include "GenericSignal.h"
#include <iostream>

class GenericOutputFormat : public Environment
{
 protected: // No instantiation outside derived classes.
  GenericOutputFormat() {}

 public:
  virtual ~GenericOutputFormat() {}
  // Request parameters and states from the Publish() function.
  virtual void Publish() const = 0;
  // Test parameters and states for consistency from the Preflight() function.
  virtual void Preflight( const SignalProperties&, const StateVector& ) const = 0;
  // Apply parameters and states from Initialize().
  virtual void Initialize( const SignalProperties&, const StateVector& ) = 0;
  // Write a header in StartRun(), and a footer in StopRun().
  virtual void StartRun( std::ostream&, const std::string& ) {}
  virtual void StopRun( std::ostream& ) {}
  // The Write() function takes as argument the state vector
  // that existed at the time of the signal argument's time stamp.
  virtual void Write( std::ostream&, const GenericSignal&, const StateVector& ) = 0;
  // Stop all asynchronous activity in Halt().
  virtual void Halt() {}
  // A descendant reports the file extension through the DataFileExtension()
  // function.
  virtual const char* DataFileExtension() const = 0;
};

#endif // GENERIC_OUTPUT_FORMAT_H

