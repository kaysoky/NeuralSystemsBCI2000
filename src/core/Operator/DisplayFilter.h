////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      juergen.mellinger@uni-tuebingen.de
// Description: An IIR filter class containing a HighPass, LowPass, and Notch
//              filter.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef DISPLAY_FILTER_H
#define DISPLAY_FILTER_H

#include "GenericSignal.h"
#include "FilterDesign.h"
#include <vector>

class DisplayFilter
{
 typedef double                               num_type;
 typedef std::vector<std::complex<num_type> > complex_vector;

 public:
  DisplayFilter();
  ~DisplayFilter();

  DisplayFilter& HPCorner( num_type );
  num_type       HPCorner() const;
  DisplayFilter& LPCorner( num_type );
  num_type       LPCorner() const;
  DisplayFilter& NotchCenter( num_type );
  num_type       NotchCenter() const;

  void Reset();
  void Process( const GenericSignal&, GenericSignal& );

 private:
  void Initialize( size_t numChannels );
  void DesignFilter();

 private:
  num_type                    mHPCorner,
                              mLPCorner,
                              mNotchCenter;

  num_type                    mGain;
  complex_vector              mZeros,
                              mPoles;
  std::vector<complex_vector> mDelays;
};

#endif // DISPLAY_FILTER_H
