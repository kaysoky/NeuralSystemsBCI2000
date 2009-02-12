////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: A filter that takes a unit variance, zero mean signal and
//              scales it according to a desired minimum trial duration, and
//              system update rate.
//              The purpose of this filter is to connect old-style application
//              modules to the new normalizer filter without changing their
//              behavior.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef TASK_ADAPTER_H
#define TASK_ADAPTER_H

#include "GenericFilter.h"
#include <vector>

class TaskAdapter : public GenericFilter
{
 public:
   TaskAdapter();
   ~TaskAdapter();

   void Preflight( const SignalProperties&, SignalProperties& ) const;
   void Initialize( const SignalProperties&, const SignalProperties& );
   void Process( const GenericSignal&, GenericSignal& );

 private:
   std::vector<float> mGains;
};
#endif // TASK_ADAPTER_H
