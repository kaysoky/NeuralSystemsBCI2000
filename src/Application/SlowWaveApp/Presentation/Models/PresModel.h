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
//////////////////////////////////////////////////////////////////////////////

#ifndef PRESMODELH
#define PRESMODELH

#include "PresErrors.h"
#include "StateAccessor.h"

class PARAMLIST;
class TPresBroadcaster;
class TPresView;

class TPresModel
{
  public:
                        TPresModel( PARAMLIST       *inParamList );
    virtual             ~TPresModel();
            TPresError  Initialize( PARAMLIST         *inParamList,
                                    TPresBroadcaster  *inBroadcaster );

            void        ProcessTaskManager( bool inDoReset );
    virtual void        Reset() = 0;
    virtual void        NextTarget() = 0;

  protected:
    virtual void        DoCleanup() = 0;
    virtual TPresError  DoInitialize(   PARAMLIST        *inParamList,
                                        TPresBroadcaster *inBroadcaster ) = 0;

            PARAMLIST              *curParamList;
            std::list< TPresView* > views;

#ifndef BCI2000
  private:
            TStateAccessor          beginOfTrial;
#endif // BCI2000
};
#endif // PRESMODELH
