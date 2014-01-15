////////////////////////////////////////////////////////////////////////////////
// $Id: PipeDefinition.cpp 3798 2012-02-01 18:07:06Z mellinger $
// Description: This file defines which filters will be used, and the sequence
//   in which they are applied.
//   Each Filter() entry consists of the name of the filter and a string which,
//   by lexical comparison, defines the relative position of the filter in the
//   sequence.
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
#include "PCHIncludes.h"
#pragma hdrstop

#include "WindowingFilter.h"
Filter( WindowingFilter, 2.A );

/**
 * Takes 'n' channels and decomposes each into 'Parameter(MaxIMFs) * n' channels
 *   Each IMF is stored sequentially, so the highest frequency IMFs will be first
 *   Up to 'Parameter(MaxIMFs) - 1' IMFs will be calculated
 * The residue is stored in the channel after the last calculated IMF
 *   There may be empty channels after the residue
 */
#include "EMDFilter.h"
Filter( EMDFilter, 2.B );

/**
 * Combines every 'Parameter(NumChannels)' into two channels
 * The first channel contains the sum of channels with given frequencies 
 *   The frequencies are 'Parameter(ValidFrequencies) \pm Parameter(FrequencyBand)'
 * The second channel contains the sum of all other channels
 */
// #include "InstantFrequencyFilter.h"
// Filter( InstantFrequencyFilter, 2.C );

#include "FFTSpectrum.h"
Filter( FFTSpectrum, 2.C );