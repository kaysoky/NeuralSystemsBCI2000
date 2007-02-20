/////////////////////////////////////////////////////////////////////////////
//
// File: GUITargetView.h
//
// Date: Nov 9, 2001
//
// Author: Juergen Mellinger
//
// Description: The TGUITargetView class implements the GUI specific details
//              of the TTargetView class.
//
// Changes:
//
// (C) 2000-2007, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef GUITARGETVIEWH
#define GUITARGETVIEWH

#include "OSIncludes.h"

#include "GUI.h"
#include "GUIView.h"
#include "TextProperties.h"
#include "PresErrors.h"

class TGUITargetView : protected TGUIView, protected TTextProperties
{
  protected:
    // Some constants.
    static const int    numBlinks = 2;              // Number of goal blinks
    static const int    blinkingPeriod = 200;       // Time for one blink (i.e. on and off) in ms

        // Only derived classes may instantiate this class.
                    TGUITargetView( TViewZ );
                    ~TGUITargetView();

  public:
    virtual void    Paint();
    virtual void    Resized();

  protected:
        TPresError  InitGoal();
        void        NormalGoal();
        void        LightedGoal();
        void        BlinkingGoal();
        void        HideGoal();
        void        ShowGoal();
        void        SetLabel( const char* inLabel );

  private:
// OS/library specific members go here.
#ifdef VCL
            enum    TGoalState{ normal, lighted, blinkActive };

            void    StopBlinking();
            void    DrawNormalGoal();
            void    DrawLightedGoal();
            void    DrawBlinkingGoal();
            void    DrawLabel( TGUIElement inElement );
    static  void    CALLBACK BlinkingCallback(  UINT    inTimerID,
                                                UINT,
                                                DWORD   inInstance,
                                                DWORD,
                                                DWORD );

        TGoalState  goalState;
        bool        goalBlinking,
                    goalHidden;
        int         blinkCounter,
                    lastBlinkCounter;
        UINT        timerID;
        Graphics::TBitmap   *goalBuffer;
        AnsiString  label;
#endif // VCL
};

#endif // GUITARGETVIEWH


