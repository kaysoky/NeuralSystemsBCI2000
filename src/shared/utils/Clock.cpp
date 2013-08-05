//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: An object that triggers an event at regular time intervals,
//   using the most accurate timing source available on the machine.
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
///////////////////////////////////////////////////////////////////////
#include "Clock.h"

#include "ThreadUtils.h"
#include "PrecisionTime.h"

using namespace bci;

Clock::Clock()
: mInterval( 0 )
{
}

Clock::~Clock()
{
  Stop();
}

Clock&
Clock::SetInterval( double ms )
{
  if( OSThread::InOwnThread() )
    mInterval = ms;
  else
  {
    bool running = !OSThread::IsTerminated();
    if( running )
      OSThread::TerminateWait();
    mInterval = ms;
    if( running )
      OSThread::Start();
  }
  return *this;
}

double
Clock::Interval() const
{
  return mInterval;
}


bool
Clock::Start()
{
  OSThread::Start();
  return !OSThread::IsTerminated();
}

bool
Clock::Stop()
{
  if( OSThread::InOwnThread() )
    OSThread::Terminate();
  else
    OSThread::TerminateWait();
  return OSThread::IsTerminated();
}

bool
Clock::Wait( double timeout )
{
  return mEvent.Wait( static_cast<int>( timeout ) );
}

bool
Clock::Wait()
{
  return Wait( 2 * mInterval );
}

bool
Clock::Reset()
{
  return mEvent.Reset();
}

int
Clock::OnExecute()
{
  PrecisionTime::NumType interval = static_cast<PrecisionTime::NumType>( mInterval ),
                         nextTime = PrecisionTime::Now() + interval;
  while( !OSThread::IsTerminating() )
  {
    ThreadUtils::SleepUntil( nextTime );
    mEvent.Set();
    nextTime += interval;
  }
  return 0;
}
