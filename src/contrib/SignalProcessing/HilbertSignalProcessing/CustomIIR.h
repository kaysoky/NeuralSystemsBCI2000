////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author:      jezhill@gmail.com
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#ifndef CUSTOM_IIR_H
#define CUSTOM_IIR_H

#include "GenericSignal.h"
#include "IIRFilter.h"

class CustomIIR
{
 typedef double          Real;

 public:
  CustomIIR();
  ~CustomIIR();

  CustomIIR&     SetHP( Real, unsigned int );
  Real           HPCorner() const { return mHPCorner; }
  unsigned int   HPOrder()  const { return mHPOrder; }
  CustomIIR&     SetLP( Real, unsigned int );
  Real           LPCorner() const { return mLPCorner; }
  unsigned int   LPOrder()  const { return mLPOrder; }
  CustomIIR&     SetNotch( Real, unsigned int );
  Real           NotchCenter() const { return mNotchCenter; }
  unsigned int   NotchOrder()  const { return mNotchOrder; }

  void Reset();
  void Process( const GenericSignal&, GenericSignal& );

 private:
  void DesignFilter();

 private:
  Real            mHPCorner,
                  mLPCorner,
                  mNotchCenter;
  unsigned int    mHPOrder,
                  mLPOrder,
                  mNotchOrder;
  IIRFilter<Real> mFilter;
};

#endif // CUSTOM_IIR_H
