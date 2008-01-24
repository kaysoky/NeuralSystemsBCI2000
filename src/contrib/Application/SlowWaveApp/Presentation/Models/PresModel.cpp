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
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////
#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "PresModel.h"

#include "StateAccessor.h"
#include "Views/PresView.h"
#include <cassert>

TPresModel::TPresModel( ParamList   *inParamList )
: curParamList( inParamList )
{
    assert( curParamList != NULL );
}

TPresModel::~TPresModel()
{
    for( std::list< TPresView* >::iterator i = views.begin();
                i != views.end(); ++i )
        delete *i;
}

TPresError
TPresModel::Initialize( ParamList   *inParamList, TPresBroadcaster *inBroadcaster )
{
    DoCleanup();
    for( std::list< TPresView* >::iterator i = views.begin();
                i != views.end(); ++i )
        delete *i;
    return DoInitialize( inParamList, inBroadcaster );
}


