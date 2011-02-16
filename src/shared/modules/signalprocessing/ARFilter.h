////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: mcfarlan@wadsworth.org, juergen.mellinger@uni-tuebingen.de,
//          Adam Wilson
// Description: The ARFilter fits a Maximum Entropy AR model to a window
//   of past input data.
//   Its output can be configured to be
//   - raw AR coefficients,
//   - the model's amplitude spectrum,
//   - the model's intensity spectrum.
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
#ifndef AR_FILTER_H
#define AR_FILTER_H

#include "ARGroup.h"
#include "GenericFilter.h"

#include <vector>

class ARFilter : public GenericFilter
{
  enum OutputTypes
  {
    SpectralAmplitude = 0,
    SpectralPower = 1,
    ARCoefficients = 2,
  };

  enum DetrendOptions
  {
    none = 0,
    mean = 1,
    linear = 2,
  };

 public:
  ARFilter();
  virtual ~ARFilter();
  virtual void Preflight( const SignalProperties&, SignalProperties& ) const;
  virtual void Initialize( const SignalProperties&, const SignalProperties& );
  virtual void Process( const GenericSignal&, GenericSignal& );
  virtual void Halt();

private:
  int     mOutputType,
          mDetrend;
  ARGroup *mpAR;
};

#endif // AR_FILTER_H
