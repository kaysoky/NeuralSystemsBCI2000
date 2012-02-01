////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A filter that watches a given state for changes, and simulates
//         a key press for the respective number key.
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
#ifndef KEYSTROKE_FILTER_H
#define KEYSTROKE_FILTER_H

#include "GenericFilter.h"
#include <string>

class KeystrokeFilter : public GenericFilter
{
 public:
          KeystrokeFilter();
  virtual ~KeystrokeFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual bool AllowsVisualization() const { return false; }

 private:
  void SendKeystroke( State::ValueType );

  std::string mKeystrokeStateName;
  State::ValueType mPreviousStateValue;
};

#endif // KEYSTROKE_FILTER_H


