/////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Utilities for handling measurement units and conversions.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "MeasurementUnits.h"
#include "Expression.h"

#include "BCIError.h"
#include <cmath>

using namespace std;

PhysicalUnit MeasurementUnits::sTimeUnit;
PhysicalUnit MeasurementUnits::sFreqUnit;
PhysicalUnit MeasurementUnits::sVoltageUnit;

