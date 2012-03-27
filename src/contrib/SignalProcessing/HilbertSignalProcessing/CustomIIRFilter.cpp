////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: Jeremy Hill <jezhill@gmail.com>
// Description: 
//   
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
#include "CustomIIRFilter.h"

using namespace std;

RegisterFilter( CustomIIRFilter, 2.C1 );

CustomIIRFilter::CustomIIRFilter()
{
	BEGIN_PARAMETER_DEFINITIONS
		"Filtering:IIR%20Filter float HighPassCorner=  0      0   0 % // ",
		"Filtering:IIR%20Filter int   HighPassOrder=   2      2   0 % // ",
		"Filtering:IIR%20Filter float LowPassCorner=   0      0   0 % // ",
		"Filtering:IIR%20Filter int   LowPassOrder=    4      4   0 % // ",
		"Filtering:IIR%20Filter float NotchCenter=     0     60Hz 0 % // ",
		"Filtering:IIR%20Filter int   NotchOrder=      4      4   0 % // ",
		"Filtering:IIR%20Filter float FilterGain=      1.0    1.0 0 % // ",
	END_PARAMETER_DEFINITIONS
}

CustomIIRFilter::~CustomIIRFilter()
{
	Halt();
}

void
CustomIIRFilter::Halt()
{
	
}

void
CustomIIRFilter::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
	double hp = Parameter( "HighPassCorner" ).InHertz() / Input.SamplingRate();
	double lp = Parameter( "LowPassCorner" ).InHertz() / Input.SamplingRate();
	double notch = Parameter( "NotchCenter" ).InHertz() / Input.SamplingRate();
	double gain = Parameter( "FilterGain" );
	unsigned int hporder = Parameter( "HighPassOrder" );
	unsigned int lporder = Parameter( "LowPassOrder" );
	unsigned int notchorder = Parameter( "NotchOrder" );
	Output = Input;
}

void
CustomIIRFilter::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
	double hp = Parameter( "HighPassCorner" ).InHertz() / Input.SamplingRate();
	double lp = Parameter( "LowPassCorner" ).InHertz() / Input.SamplingRate();
	double notch = Parameter( "NotchCenter" ).InHertz() / Input.SamplingRate();
	unsigned int hporder = Parameter( "HighPassOrder" );
	unsigned int lporder = Parameter( "LowPassOrder" );
	unsigned int notchorder = Parameter( "NotchOrder" );
	mGain = Parameter( "FilterGain" );
	mFilter.SetHP( hp, hporder );
	mFilter.SetLP( lp, lporder );
	mFilter.SetNotch( notch, notchorder );
	mEnabled = ( ( hp > 0.0 && hporder > 0 ) || ( lp > 0.0 && lporder > 0 ) || ( notch > 0.0 && notchorder > 0 ) );
}


void
CustomIIRFilter::StartRun()
{
	mFilter.Reset();
}

void
CustomIIRFilter::Process( const GenericSignal& Input, GenericSignal& Output )
{
	//bciout << Input.Channels() << "x" << Input.Elements() << " -> " << Output.Channels() << "x" << Output.Elements() << endl;
	if( mEnabled )
	{
		mFilter.Process( Input, Output );
		for(int ch = 0; ch < Output.Channels(); ch++)
			for(int el = 0; el < Output.Elements(); el++)
				Output( ch, el ) *= mGain;
	}
	else Output = Input;
}

