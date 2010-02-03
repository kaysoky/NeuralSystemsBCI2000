/////////////////////////////////////////////////////////////////////////////
//
// File: GUIStimulusView.h
//
// Date: Nov 6, 2001
//
// Author: Juergen Mellinger
//
// Description: The TGUIStimulusView class implements the GUI specific details
//              of the TStimulusView class.
//
// Changes:
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef GUI_STIMULUS_VIEW_H
#define GUI_STIMULUS_VIEW_H

#include "OSIncludes.h"

#include "GUI.h"
#include "GUIView.h"
#include "PresErrors.h"

class TGUIStimulusView : protected TGUIView
{
  protected:
        // Only derived classes may instantiate this class.
                    TGUIStimulusView();
                    ~TGUIStimulusView();

  public:
    static  const int   stimulusTimeResolution = 10; // ms

    virtual void        Paint();
    virtual void        Resized();

  protected:
            void        AttachVisualStimulus( const char *inVisualStimulusFile );
            void        PresentVisualStimulus( int inDuration /* in ms */ );

  private:
// OS/library specific members go here.
#ifdef VCL
    static  void        CALLBACK HideVisualStimulus(    UINT inTimerID,
                                                        UINT,
                                                        DWORD inInstance,
                                                        DWORD,
                                                        DWORD );
            void     __fastcall MediaPlayerNotifyHandler( TObject* );
            
            int                 leftX,
                                topY;
            bool                visible,
                                useMediaPlayer;
            Graphics::TBitmap   *bitmap;
            class TMediaPlayer  *mediaPlayer;
            TRect               bitmapTRect;
            UINT                timerID;
#endif // VCL
};

#endif // GUI_STIMULUS_VIEW_H


