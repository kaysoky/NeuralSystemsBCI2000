////////////////////////////////////////////////////////////////////////////////
//
// File:    UVisual.h
//
// Authors: Gerwin Schalk, Juergen Mellinger
//
// Changes: Apr 15, 2003, juergen.mellinger@uni-tuebingen.de:
//          Reworked graph display double buffering scheme.
//          Untangled window painting from content changes.
//          Introduced clipping to reduce the amount of time spent blitting
//          graphics data.
//
//          To get the previous code, remove NEW_DOUBLEBUF_SCHEME
//          from the "Conditional defines" in the project options.
//
//          May 27, 2003, jm:
//          Created Operator/UVisual to maintain VISUAL and VISCFGLIST
//          as part of the operator module.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UVisualH
#define UVisualH

#include <vcl.h>
#include <syncobjs.hpp>
#include "UGenericSignal.h"

class VISUAL
{
private:    // User declarations
        enum
        {
          MAX_XAXISLABELS = 1024,
          MAX_YAXISLABELS = 256,
          MAX_DISPLAYCHANNELS = 256,
        };
        double  minvalue;
        double  maxvalue;
        int     displaysamples, displaychannels;        // current number of samples and channels in the graph
        int     cur_samples;                            // current number of samples in the signal
        int     total_displaychannels;                  // total number of channels in the graph (e.g., with total of 64, only 16 might be displayed)
        int     startchannel;                           // first channel on the graph
        TPoint  *points[MAX_DISPLAYCHANNELS];
        void    RenderGraph(int startch, int endch, int startsample, int endsample);
        AnsiString      XAxisLabel[MAX_XAXISLABELS];
        AnsiString      YAxisLabel[MAX_YAXISLABELS];
        void __fastcall FormResize(TObject *Sender);    // called when the user resizes the window
        void __fastcall FormKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);       // called, when user presses a key
        void __fastcall FormPaint(TObject *Sender);     // when the graph needs to be repainted
#ifdef NEW_DOUBLEBUF_SCHEME
        // This is a do-nothing replacement class.
        // Of course, this dirty trick will go away ASAP ...
        struct TCriticalSection
        {
          static void Acquire() {}
          static void Release() {}
        } *critsec;
#else // NEW_DOUBLEBUF_SCHEME
        TCriticalSection        *critsec;               // critical section for screen update
#endif // NEW_DOUBLEBUF_SCHEME
        bool    toggle;
public:     // User declarations
        VISUAL::VISUAL(BYTE my_sourceID, BYTE my_vis_type);
        VISUAL::~VISUAL();
        Graphics::TBitmap *bitmap;
        BYTE   sourceID;
        BYTE   vis_type;
        TForm  *form;
#ifdef NEW_DOUBLEBUF_SCHEME
private:
        HRGN invalidRgn, redrawRgn;
        void PaintGraph(int startch, int endch, int startsample, int endsample);
        void SetStartChannel( int );
        class TVisForm : public TForm
        {
         public:
          TVisForm() : TForm( ( TComponent* )NULL, 1 ) {}
          // To avoid flicker and save memory bandwidth, use a WM_ERASEBKGND
          // handler that does not do anything.
          void __fastcall WMEraseBkgnd( TWMEraseBkgnd& ) {}
          BEGIN_MESSAGE_MAP
            VCL_MESSAGE_HANDLER( WM_ERASEBKGND, TWMEraseBkgnd, WMEraseBkgnd )
          END_MESSAGE_MAP( TForm )
        };
public:
#endif // NEW_DOUBLEBUF_SCHEME
        TMemo  *memo;
        int    startsample;
        void   RenderData(const GenericIntSignal *signal);
        void   RenderData(const GenericSignal *signal);
        void   RenderMemo(const char *memo);
};


class VISCFGLIST
{
private:    // User declarations
        TCriticalSection        *critsec;               // critical section for screen update
        TList   *vis_list;
public:     // User declarations
        VISCFGLIST::VISCFGLIST();
        VISCFGLIST::~VISCFGLIST();
        void    Add(VISUAL *new_visual);
        VISUAL  *GetVisCfgPtr(BYTE my_sourceID);
        void    DeleteAllVisuals();
};
//---------------------------------------------------------------------------
#endif // UVisualH
