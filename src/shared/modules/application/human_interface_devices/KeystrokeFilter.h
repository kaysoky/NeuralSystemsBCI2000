////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that watches a given state for changes, and simulates
//         a key press for the respective number key.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef KEYSTROKE_FILTER_H
#define KEYSTROKE_FILTER_H

#include "GenericFilter.h"
#include <string>

class KeystrokeFilter : public GenericFilter
{
 public:
          KeystrokeFilter();
  virtual ~KeystrokeFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual bool AllowsVisualization() const { return false; }

 private:
  void SendKeystroke( short );

  std::string mKeystrokeStateName;
  short       mPreviousStateValue;
};

#endif // KEYSTROKE_FILTER_H


