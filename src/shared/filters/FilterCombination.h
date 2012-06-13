////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Filter templates that represent parallel or serial combinations
//   of two existing filters. To use it with two filters named Filter1 and
//   Filter2, enter the following into your PipeDefinition.h:
//
//   #include "FilterCombinations.h"
//   #include "Filter1.h"
//   #include "Filter2.h"
//   struct MyCombinedFilter : ParallelCombination<Filter1, Filter2> {};
//   Filter( MyCombinedFilter, 2.D );
//
//   Using nested constructs from ParallelCombination<> and SerialCombination<>,
//   one may build arbitrary networks of filters.
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
#ifndef FILTER_COMBINATION_H
#define FILTER_COMBINATION_H

#include "GenericFilter.h"
#include <algorithm>

template<class F1, class F2>
class FilterCombination : public GenericFilter
{
 private:
   virtual void Publish();
   virtual void Preflight( const SignalProperties&, SignalProperties& ) const = 0;
   virtual void Initialize( const SignalProperties&, const SignalProperties& ) = 0;
   virtual void Process( const GenericSignal&, GenericSignal& ) = 0;
   virtual void StartRun();
   virtual void StopRun();
   virtual void Resting();
   virtual void Halt();

 protected:
   F1 mFilter1;
   F2 mFilter2;
   mutable SignalProperties mProperties1,
                            mProperties2;
   GenericSignal mOutput1,
                 mOutput2;
};

template<class F1, class F2>
class ParallelCombination : public FilterCombination<F1, F2>
{
 private:
   virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
   virtual void Initialize( const SignalProperties&, const SignalProperties& );
   virtual void Process( const GenericSignal&, GenericSignal& );

   typedef FilterCombination<F1, F2> Parent;
   using Parent::mFilter1;
   using Parent::mFilter2;
   using Parent::mProperties1;
   using Parent::mProperties2;
   using Parent::mOutput1;
   using Parent::mOutput2;
};

template<class F1, class F2>
class LinearCombination : public FilterCombination<F1, F2>
{
 private:
   virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
   virtual void Initialize( const SignalProperties&, const SignalProperties& );
   virtual void Process( const GenericSignal&, GenericSignal& );

   typedef FilterCombination<F1, F2> Parent;
   using Parent::mFilter1;
   using Parent::mFilter2;
   using Parent::mProperties1;
   using Parent::mOutput1;
};

// FilterCombination implementation

template<class F1, class F2>
void
FilterCombination<F1, F2>::Publish()
{
  // reverse order of publication matters
  mFilter2.CallPublish();
  mFilter1.CallPublish();
}

template<class F1, class F2>
void
FilterCombination<F1, F2>::StartRun()
{
  mFilter1.CallStartRun();
  mFilter2.CallStartRun();
}

template<class F1, class F2>
void
FilterCombination<F1, F2>::StopRun()
{
  mFilter1.CallStopRun();
  mFilter2.CallStopRun();
}

template<class F1, class F2>
void
FilterCombination<F1, F2>::Resting()
{
  mFilter1.CallResting();
  mFilter2.CallResting();
}

template<class F1, class F2>
void
FilterCombination<F1, F2>::Halt()
{
  mFilter1.CallHalt();
  mFilter2.CallHalt();
}

// ParallelCombination implementation

template<class F1, class F2>
void
ParallelCombination<F1, F2>::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  mFilter1.CallPreflight( Input, mProperties1 );
  mFilter2.CallPreflight( Input, mProperties2 );

  Output = Input;
  Output.SetChannels( mProperties1.Channels() + mProperties2.Channels() );
  Output.SetElements( std::max( mProperties1.Elements(), mProperties2.Elements() ) );

  Output.ChannelUnit() = PhysicalUnit();
  Output.ElementUnit() = PhysicalUnit().SetRawMax( Output.Elements() - 1 );
  Output.ValueUnit() = PhysicalUnit();
  double min = std::min( mProperties1.ValueUnit().RawMin(), mProperties2.ValueUnit().RawMin() ),
         max = std::max( mProperties1.ValueUnit().RawMax(), mProperties2.ValueUnit().RawMax() );
  Output.ValueUnit().SetRawMin( min ).SetRawMax( max );

  if( !mProperties1.ChannelLabels().IsTrivial() )
    for( int ch = 0; ch < mProperties1.Channels(); ++ch )
      Output.ChannelLabels()[ch] = mProperties1.ChannelLabels()[ch];
  if( !mProperties2.ChannelLabels().IsTrivial() )
    for( int ch = 0; ch < mProperties2.Channels(); ++ch )
      Output.ChannelLabels()[ch + mProperties1.Channels()] = mProperties2.ChannelLabels()[ch];
}

template<class F1, class F2>
void
ParallelCombination<F1, F2>::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
  mFilter1.CallInitialize( Input, mProperties1 );
  mFilter2.CallInitialize( Input, mProperties2 );
  mOutput1 = GenericSignal( mProperties1 );
  mOutput2 = GenericSignal( mProperties2 );
}

template<class F1, class F2>
void
ParallelCombination<F1, F2>::Process( const GenericSignal& Input, GenericSignal& Output )
{
  mFilter1.CallProcess( Input, mOutput1 );
  mFilter2.CallProcess( Input, mOutput2 );

  for( int ch = 0; ch < mOutput1.Channels(); ++ch )
    for( int el = 0; el < mOutput1.Elements(); ++el )
      Output( ch, el ) = mOutput1( ch, el );

  for( int ch = 0; ch < mOutput2.Channels(); ++ch )
    for( int el = 0; el < mOutput2.Elements(); ++el )
      Output( ch + mOutput1.Channels(), el ) = mOutput2( ch, el );
}

// LinearCombination implementation

template<class F1, class F2>
void
LinearCombination<F1, F2>::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  mFilter1.CallPreflight( Input, mProperties1 );
  mFilter2.CallPreflight( mProperties1, Output );
}

template<class F1, class F2>
void
LinearCombination<F1, F2>::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
  mFilter1.CallInitialize( Input, mProperties1 );
  mFilter2.CallInitialize( mProperties1, Output );
  mOutput1 = GenericSignal( mProperties1 );
}

template<class F1, class F2>
void
LinearCombination<F1, F2>::Process( const GenericSignal& Input, GenericSignal& Output )
{
  mFilter1.CallProcess( Input, mOutput1 );
  mFilter2.CallProcess( mOutput1, Output );
}

#endif // FILTER_COMBINATION_H
