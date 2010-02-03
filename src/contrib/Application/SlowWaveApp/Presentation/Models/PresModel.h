/////////////////////////////////////////////////////////////////////////////
//
// File: PresModel.h
//
// Date: Oct 15, 2001
//
// Author: Juergen Mellinger
//
// Description:
//
// Changes:
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef PRESMODELH
#define PRESMODELH

#include "PresErrors.h"
#include "StateAccessor.h"

class ParamList;
class TPresBroadcaster;
class TPresView;

class TPresModel
{
  public:
                        TPresModel( ParamList       *inParamList );
    virtual             ~TPresModel();
            TPresError  Initialize( ParamList         *inParamList,
                                    TPresBroadcaster  *inBroadcaster );

            void        ProcessTaskManager( bool inDoReset );
    virtual void        Reset() = 0;
    virtual void        NextTarget() = 0;

  protected:
    virtual void        DoCleanup() = 0;
    virtual TPresError  DoInitialize(   ParamList        *inParamList,
                                        TPresBroadcaster *inBroadcaster ) = 0;

            ParamList              *curParamList;
            std::list< TPresView* > views;

#ifndef BCI2000
  private:
            TStateAccessor          beginOfTrial;
#endif // BCI2000
};
#endif // PRESMODELH
