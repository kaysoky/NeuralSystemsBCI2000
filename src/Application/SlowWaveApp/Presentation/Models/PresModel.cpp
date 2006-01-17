/////////////////////////////////////////////////////////////////////////////
//
// File: PresModel.cpp
//
// Date: Oct 15, 2001
//
// Author: Juergen Mellinger
//
// Description:
//
// Changes: Feb 5, 2004, jm: Created file PresModel.cpp, introduced
//          factorization of ProcessTaskManager() into public Reset() and
//          NextTarget() functions.
//
//////////////////////////////////////////////////////////////////////////////
#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "PresModel.h"

#include "StateAccessor.h"
#include "Views/PresView.h"
#include <cassert>

TPresModel::TPresModel( PARAMLIST   *inParamList )
: curParamList( inParamList )
{
    assert( curParamList != NULL );
#ifndef BCI2000
    beginOfTrial.AttachState( "BeginOfTrial" );
#endif // BCI2000
}

TPresModel::~TPresModel()
{
    for( std::list< TPresView* >::iterator i = views.begin();
                i != views.end(); ++i )
        delete *i;
}

TPresError
TPresModel::Initialize( PARAMLIST   *inParamList, TPresBroadcaster *inBroadcaster )
{
    DoCleanup();
    for( std::list< TPresView* >::iterator i = views.begin();
                i != views.end(); ++i )
        delete *i;
    return DoInitialize( inParamList, inBroadcaster );
}

#ifndef BCI2000 // Make sure this is not used in the BCI2000 context.
void
TPresModel::ProcessTaskManager( bool inDoReset )
{
  if( inDoReset )
    Reset();
  if( beginOfTrial.GetStateValue() == 1 )
    NextTarget();
}
#endif // BCI2000
