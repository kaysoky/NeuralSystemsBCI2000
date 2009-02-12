////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A virtual base class for stimuli that are also graphic objects.
//
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "VisualStimulus.h"
#include "GraphObject.h"
#include "Target.h"

using namespace std;
using namespace GUI;

VisualStimulus::VisualStimulus()
: mPresentationMode( ShowHide ),
  mDimFactor( 0.5 ),
  mBeingPresented( false )
{
}

VisualStimulus::~VisualStimulus()
{
}

VisualStimulus&
VisualStimulus::SetPresentationMode( Mode m )
{
  mPresentationMode = m;
  GraphObject* pGraphObject = dynamic_cast<GraphObject*>( this );
  if( pGraphObject )
    pGraphObject->Change();
  return *this;
}

VisualStimulus::Mode
VisualStimulus::PresentationMode() const
{
  return mPresentationMode;
}

VisualStimulus&
VisualStimulus::SetDimFactor( float f )
{
  mDimFactor = f;
  GraphObject* pGraphObject = dynamic_cast<GraphObject*>( this );
  if( pGraphObject )
    pGraphObject->Change();
  return *this;
}

float
VisualStimulus::DimFactor() const
{
  return mDimFactor;
}

void
VisualStimulus::OnPresent()
{
  mBeingPresented = true;
  GraphObject* pGraphObject = dynamic_cast<GraphObject*>( this );
  if( pGraphObject )
    pGraphObject->Invalidate();
}

void
VisualStimulus::OnConceal()
{
  mBeingPresented = false;
  GraphObject* pGraphObject = dynamic_cast<GraphObject*>( this );
  if( pGraphObject )
    pGraphObject->Invalidate();
}

bool
VisualStimulus::BeingPresented() const
{
  return mBeingPresented;
}

