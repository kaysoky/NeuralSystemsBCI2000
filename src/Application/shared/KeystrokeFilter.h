////////////////////////////////////////////////////////////////////////////////
//
// File:   KeystrokeFilter.h
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Date:   Jul 28, 2004
//
// Description: A filter that watches a given state for changes, and simulates
//         a key press for the respective number key.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef KeystrokeFilterH
#define KeystrokeFilterH

#include "UGenericFilter.h"
#include <string>

class KeystrokeFilter : public GenericFilter
{
 public:
          KeystrokeFilter();
  virtual ~KeystrokeFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize();
  virtual void Process( const GenericSignal*, GenericSignal* );

 private:
  void SendKeystroke( short );

  std::string mKeystrokeStateName;
  short       mPreviousStateValue;
};

#endif // KeystrokeH


