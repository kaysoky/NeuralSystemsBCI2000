////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: A BCI2000 filter performing temporal alignment of its input
//   data using linear interpolation between subsequent samples.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef ALIGNMENT_FILTER_H
#define ALIGNMENT_FILTER_H

#include "GenericFilter.h"
#include <vector>

class AlignmentFilter : public GenericFilter
{
 public:
          AlignmentFilter();
  virtual ~AlignmentFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );

 private:
  bool                 mAlign;
  std::vector<float>   mWeightPrev,
                       mWeightCur,
                       mPrevSample;
};

#endif // ALIGNMENT_FILTER_H


