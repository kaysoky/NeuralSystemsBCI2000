////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A statistical observer that combines a PowerSumObserver with
//   a HistogramObserver.
//   It provides accurate mean, variance, and covariance, as well as support
//   for quantiles and moments.
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2011: BCI2000 team and many external contributors ]
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
#ifndef CONFIGURABLE_OBSERVER_H
#define CONFIGURABLE_OBSERVER_H

#include "HistogramObserver.h"
#include "PowerSumObserver.h"

namespace StatisticalObserver
{

class CombinedObserver : public virtual PowerSumObserver, public virtual HistogramObserver
{
  // This class implements the ObserverBase interface. For documentation, see ObserverBase.h.
 public:
  enum { Supports =
     StatisticalObserver::Variance |
     StatisticalObserver::Covariance |
     StatisticalObserver::Quantile |
     StatisticalObserver::CentralMoment
  };

  CombinedObserver( int inConfig = DefaultConfig )
    : ObserverBase( inConfig, Supports ),
      PowerSumObserver( inConfig & PowerSumObserver::Supports ),
      HistogramObserver( inConfig & HistogramObserver::Supports ),
      mUseHistogram( ImpliedConfig( inConfig ) & ( StatisticalObserver::Quantile | StatisticalObserver::CentralMoment ) )
    {
    }

 protected:
  virtual void DoChange()
    {
      PowerSumObserver::DoChange();
      if( mUseHistogram )
        HistogramObserver::DoChange();
    }
  virtual void DoAgeBy( unsigned int count )
    {
      PowerSumObserver::DoAgeBy( count );
      if( mUseHistogram )
        HistogramObserver::DoAgeBy( count );
    }
  virtual void DoObserve( const Vector& v, Number w )
    {
      PowerSumObserver::DoObserve( v, w );
      if( mUseHistogram )
        HistogramObserver::DoObserve( v, w );
    }
  virtual void DoClear()
    {
      PowerSumObserver::DoClear();
      if( mUseHistogram )
        HistogramObserver::DoClear();
    }

 public:
  virtual Number PowerSum0() const
    {
      return PowerSumObserver::PowerSum0();
    }
  virtual Vector PowerSum1() const
    {
      return PowerSumObserver::PowerSum1();
    }
  virtual Vector PowerSum2Diag() const
    {
      return PowerSumObserver::PowerSum2Diag();
    }
  virtual Matrix PowerSum2Full() const
    {
      return PowerSumObserver::PowerSum2Full();
    }
  virtual Vector PowerSumDiag( unsigned int i ) const
    {
      return HistogramObserver::PowerSumDiag( i );
    }
  virtual Vector CDF( Number p ) const
    {
      return HistogramObserver::CDF( p );
    }
  virtual Vector InverseCDF( Number p ) const
    {
      return HistogramObserver::InverseCDF( p );
    }

 private:
  bool mUseHistogram;
};

} // namespace StatisticalObserver

#endif // COMBINED_OBSERVER_H
