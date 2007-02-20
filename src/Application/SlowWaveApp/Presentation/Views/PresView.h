/////////////////////////////////////////////////////////////////////////////
//
// File: PresView.h
//
// Date: Oct 15, 2001
//
// Author: Juergen Mellinger
//
// Description:
//
// Changes:
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef PRES_VIEW_H
#define PRES_VIEW_H

#include "PresErrors.h"
#include "PresBroadcasting.h"
#include "GUI.h"

class PARAMLIST;

class TPresView : public TPresListener
{
  public:
                        TPresView( PARAMLIST *inParamList )
                        : curParamList( inParamList ) {}
    virtual             ~TPresView() {}
    virtual TPresError  Initialize(       PARAMLIST *inParamList,
                                    const TGUIRect  &inRect ) = 0;

  protected:
            PARAMLIST   *curParamList;
};

#endif // PRES_VIEW_H
