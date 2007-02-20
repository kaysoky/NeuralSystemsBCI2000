////////////////////////////////////////////////////////////////////////////////
// $Id$
// File:        TaskAdapter.h
// Date:        Dec 30, 2005
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that takes a unit variance, zero mean signal and
//              scales it according to a desired minimum trial duration, and
//              system update rate.
//              The purpose of this filter is to connect old-style application
//              modules to the new normalizer filter without changing their
//              behavior.
// $Log$
// Revision 1.1  2006/01/13 15:04:46  mellinger
// Initial version.
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef TaskAdapterH
#define TaskAdapterH

#include "UGenericFilter.h"
#include <vector>

class TaskAdapter : public GenericFilter
{
 public:
   TaskAdapter();
   ~TaskAdapter();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize2( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal*, GenericSignal* );

 private:
   std::vector<float> mGains;
};
#endif // TaskAdapterH
