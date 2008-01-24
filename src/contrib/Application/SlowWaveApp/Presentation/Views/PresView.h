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
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef PRES_VIEW_H
#define PRES_VIEW_H

#include "PresErrors.h"
#include "PresBroadcasting.h"
#include "GUI.h"

class ParamList;

class TPresView : public TPresListener
{
  public:
                        TPresView( ParamList *inParamList )
                        : curParamList( inParamList ) {}
    virtual             ~TPresView() {}
    virtual TPresError  Initialize(       ParamList *inParamList,
                                    const TGUIRect  &inRect ) = 0;

  protected:
            ParamList   *curParamList;
};

#endif // PRES_VIEW_H
