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
#include "IIRFilter.h"

class DisplayFilter
{
 typedef double          Real;
 typedef IIRFilter<Real> ComplexVector;

 public:
  DisplayFilter();
  ~DisplayFilter();

  DisplayFilter& HPCorner( Real );
  Real           HPCorner() const;
  DisplayFilter& LPCorner( Real );
  Real           LPCorner() const;
  DisplayFilter& NotchCenter( Real );
  Real           NotchCenter() const;

  void Reset();
  void Process( const GenericSignal&, GenericSignal& );

 private:
  void DesignFilter();

 private:
  Real            mHPCorner,
                  mLPCorner,
                  mNotchCenter;
  IIRFilter<Real> mFilter;
};

#endif // DISPLAY_FILTER_H
