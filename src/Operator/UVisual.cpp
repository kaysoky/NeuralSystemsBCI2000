////////////////////////////////////////////////////////////////////////////////
//
// File:    UVisual.cpp
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
#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "UVisual.h"

#include "UVisConfig.h"
#include "UGenericVisualization.h"

//#pragma link "CSPIN"

//---------------------------------------------------------------------------

#pragma package(smart_init)

VISCFGLIST::VISCFGLIST()
: vis_list( new TList ),
  critsec( new TCriticalSection )
{
}

VISCFGLIST::~VISCFGLIST()
{
 DeleteAllVisuals();

 delete vis_list;
 delete critsec;
}

void VISCFGLIST::DeleteAllVisuals()
{
int     i;
VISUAL  *cur_vis;

 critsec->Acquire();

  // Clean up – must free memory for the items as well as the list
  for (i=0; i<vis_list->Count; i++)
   {
   cur_vis=(VISUAL *)vis_list->Items[i];
   // store defaults in registry
   fVisConfig->SetVisualPrefs(cur_vis->sourceID, cur_vis->vis_type, "Top", cur_vis->form->Top);
   fVisConfig->SetVisualPrefs(cur_vis->sourceID, cur_vis->vis_type, "Left", cur_vis->form->Left);
   fVisConfig->SetVisualPrefs(cur_vis->sourceID, cur_vis->vis_type, "Width", cur_vis->form->Width);
   fVisConfig->SetVisualPrefs(cur_vis->sourceID, cur_vis->vis_type, "Height", cur_vis->form->Height);
   delete cur_vis;
   }

  vis_list->Clear();

 critsec->Release();
}


// **************************************************************************
// Function:   s
// Purpose:    s
// Parameters: s
// Returns:    s
// **************************************************************************
VISUAL *VISCFGLIST::GetVisCfgPtr(BYTE my_sourceID)
{
VISUAL  *ptr, *cur_ptr;
int     i;

 critsec->Acquire();

 ptr=NULL;
 for (i=0; i<vis_list->Count; i++)
  {
  cur_ptr=(VISUAL *)vis_list->Items[i];
  if (cur_ptr->sourceID == my_sourceID)
     {
     ptr=cur_ptr;
     break;
     }
  }

 critsec->Release();
 return(ptr);
}



// **************************************************************************
// Function:   s
// Purpose:    s
// Parameters: s
// Returns:    s
// **************************************************************************
void VISCFGLIST::Add(VISUAL *new_visual)
{
 critsec->Acquire();
 vis_list->Add(new_visual);
 critsec->Release();
}



VISUAL::VISUAL(BYTE my_sourceID, BYTE my_vis_type)
#ifdef NEW_DOUBLEBUF_SCHEME
: invalidRgn( ::CreateRectRgn( 0, 0, 0, 0 ) ),
  redrawRgn( ::CreateRectRgn( 0, 0, 0, 0 ) )
#endif // NEW_DOUBLEBUF_SCHEME
{
int        ch, i, value;
AnsiString windowtitle;
char       buf[256];

 sourceID=my_sourceID;
 vis_type=my_vis_type;

 form=NULL;
 // chart=NULL;
 memo=NULL;
 bitmap=NULL;

 startsample=0;
 displaychannels=-1;
 total_displaychannels=-1;
 startchannel=0;
 critsec=new TCriticalSection();
 critsec->Acquire();

 // set the buffers for the charting data points to NULL
 for (ch=0; ch<MAX_DISPLAYCHANNELS; ch++)
  points[ch]=NULL;

 if (vis_type == VISTYPE_GRAPH)
    {
#ifndef NEW_DOUBLEBUF_SCHEME
    form=new TForm(Application);
#else
    form = new TVisForm;
#endif // NEW_DOUBLEBUF_SCHEME

    // create the form
    if (fVisConfig->GetVisualPrefs(sourceID, vis_type, "Top", value))
       form->Top=value;
    else
       form->Top=100;
    if (fVisConfig->GetVisualPrefs(sourceID, vis_type, "Left", value))
       form->Left=value;
    else
       form->Left=100;
    if (fVisConfig->GetVisualPrefs(sourceID, vis_type, "Width", value))
       form->Width=value;
    else
       form->Width=100;
    if (fVisConfig->GetVisualPrefs(sourceID, vis_type, "Height", value))
       form->Height=value;
    else
       form->Height=100;
    // form->BorderIcons >> biSystemMenu;
    form->BorderStyle=bsSizeable;
    form->Show();

    // get the default values from the registry
    if (!fVisConfig->GetVisualPrefs(sourceID, vis_type, "WindowTitle", windowtitle))
       form->Caption=AnsiString((int)my_sourceID);
    else
       form->Caption=windowtitle+" ("+AnsiString((int)my_sourceID)+")";
    if (!fVisConfig->GetVisualPrefs(sourceID, vis_type, "NumSamples", displaysamples))
       displaysamples=128;
    if (displaysamples == 0)
       displaysamples=128;
    if (!fVisConfig->GetVisualPrefs(sourceID, vis_type, "MinValue", minvalue))
       minvalue=-32768;
    if (!fVisConfig->GetVisualPrefs(sourceID, vis_type, "MaxValue", maxvalue))
       maxvalue=+32768;
    if (minvalue >= maxvalue)
       {
       maxvalue=32768;
       minvalue=-32768;
       }

    // create buffer for the chart within the form
    bitmap=new Graphics::TBitmap();
    bitmap->Width=form->ClientWidth;
    bitmap->Height=form->ClientHeight;

    form->OnResize=FormResize;
    form->OnKeyUp=FormKeyUp;
    form->OnPaint=FormPaint;

    // create the chart within the form
    /* chart=new TChart(form);
    chart->Visible=false;
    chart->Parent=form;
    chart->Left=0;
    chart->Top=0;
    chart->Width=form->ClientWidth;
    chart->Height=form->ClientHeight;
    chart->Title->Visible=false;
    chart->Legend->Visible=false;
    chart->AllowPanning=pmVertical;
    chart->AllowZoom=false;
    chart->View3D=false;
    chart->Anchors << akLeft << akTop << akRight << akBottom;
    // chart->BottomAxis->LabelStyle=talText;
    chart->Visible=true; */
    }

 if (vis_type == VISTYPE_MEMO)
    {
#ifdef NEW_DOUBLEBUF_SCHEME
    form = new TForm( ( TComponent* )NULL );
#else // NEW_DOUBLEBUF_SCHEME
    form=new TForm(Application);
#endif // NEW_DOUBLEBUF_SCHEME

    // create the form
    if (fVisConfig->GetVisualPrefs(sourceID, vis_type, "Top", value))
       form->Top=value;
    else
       form->Top=100;
    if (fVisConfig->GetVisualPrefs(sourceID, vis_type, "Left", value))
       form->Left=value;
    else
       form->Left=100;
    if (fVisConfig->GetVisualPrefs(sourceID, vis_type, "Width", value))
       form->Width=value;
    else
       form->Width=100;
    if (fVisConfig->GetVisualPrefs(sourceID, vis_type, "Height", value))
       form->Height=value;
    else
       form->Height=100;
    // form->BorderIcons >> biSystemMenu;
    form->BorderStyle=bsSizeable;
    form->Show();

    // get the default values from the registry
    if (!fVisConfig->GetVisualPrefs(sourceID, vis_type, "WindowTitle", windowtitle))
       form->Caption=AnsiString((int)my_sourceID);
    else
       form->Caption=windowtitle+" ("+AnsiString((int)my_sourceID)+")";

    // critsec=new TCriticalSection();

    // create the memo within the form
    memo=new TMemo(form);
    memo->Visible=false;
    memo->Parent=form;
    memo->Left=0;
    memo->Top=0;
    memo->Width=form->ClientWidth;
    memo->Height=form->ClientHeight;
    memo->Anchors << akLeft << akTop << akRight << akBottom;
    memo->ScrollBars=ssVertical;
    memo->Visible=true;
    }

 critsec->Release();
}


VISUAL::~VISUAL()
{
int ch;

 // if (chart) delete chart;
 // delete the buffer for the charting data points
 for (ch=0; ch<MAX_DISPLAYCHANNELS; ch++)
  {
  if (points[ch]) delete points[ch];
  points[ch]=NULL;
  }

 if (bitmap) delete bitmap;
 if (critsec) delete critsec;

 if (memo)  delete memo;
#ifndef NEW_DOUBLEBUF_SCHEME
 if (form)  form->Close();
 form=NULL;
#else // NEW_DOUBLEBUF_SCHEME
 delete form;
 ::DeleteObject( invalidRgn );
 ::DeleteObject( redrawRgn );
#endif // NEW_DOUBLEBUF_SCHEME
 // chart=NULL;
 memo=NULL;
 bitmap=NULL;
 critsec=NULL;
}


void VISUAL::RenderMemo(const char *memotext)
{
 if (!memotext) return;

 memo->Lines->Add(AnsiString(memotext));
}


void __fastcall VISUAL::FormResize(TObject *Sender)
{
 // only do this, if we have a graph
 if (vis_type == VISTYPE_GRAPH)
    {
    critsec->Acquire();
    // if we already had buffers for the bitmap, delete them and create new ones
    // the size of the re-sized window
#ifdef NEW_DOUBLEBUF_SCHEME
    bitmap->Canvas->Lock();
#else
    if (bitmap) delete bitmap;
    bitmap=new Graphics::TBitmap();
#endif // NEW_DOUBLEBUF_SCHEME

    bitmap->Width=form->ClientWidth;
    bitmap->Height=form->ClientHeight;
#ifdef NEW_DOUBLEBUF_SCHEME
    bitmap->Canvas->Unlock();
    form->Invalidate();
#endif // NEW_DOUBLEBUF_SCHEME

    // update the display
    RenderGraph(0, displaychannels-1, 0, displaysamples-1);
    critsec->Release();
#ifndef NEW_DOUBLEBUF_SCHEME
    Application->ProcessMessages();
#endif // NEW_DOUBLEBUF_SCHEME
    }
}
//---------------------------------------------------------------------------

#ifndef NEW_DOUBLEBUF_SCHEME
void __fastcall VISUAL::FormPaint(TObject *Sender)
{
 // only do this, if we have a graph
 if (vis_type == VISTYPE_GRAPH)
    {
    critsec->Acquire();
    RenderGraph(0, displaychannels-1, 0, displaysamples-1);
    critsec->Release();
    }
}
#else // NEW_DOUBLEBUF_SCHEME
void __fastcall VISUAL::FormPaint( TObject* Sender )
{
  PaintGraph( 0, displaychannels - 1, 0, displaysamples - 1 );
  TForm* Form = static_cast<TForm*>( Sender );
  bitmap->Canvas->Lock();
  Form->Canvas->Draw( 0, 0, bitmap );
  bitmap->Canvas->Unlock();
}
#endif // NEW_DOUBLEBUF_SCHEME
//---------------------------------------------------------------------------


void __fastcall VISUAL::FormKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
 // only do this, if we have a graph
 if (vis_type == VISTYPE_GRAPH)
    {
    // 1.) the number of channels on the screen has to be smaller than the total number of channels (otherwise, scrolling makes no sense)
    // 2.) the "right" (i.e., up or down arrow) key has to be pressed
    // 3.) we don't want to be able to scroll beyond the total number of channels
    if ((displaychannels < total_displaychannels) && (Key == 38) && (startchannel+displaychannels <= total_displaychannels))
       {
       critsec->Acquire();
       startchannel+=displaychannels/2;
       RenderGraph(0, displaychannels-1, 0, displaysamples-1);
       critsec->Release();
       }
    if ((displaychannels < total_displaychannels) && (Key == 40) && (startchannel >= 0))
       {
       critsec->Acquire();
       startchannel-=displaychannels/2;
       RenderGraph(0, displaychannels-1, 0, displaysamples-1);
       critsec->Release();
       }
    }
}


// update the actual display of the graph
// at the moment, it will always update the WHOLE GRAPH, i.e., all the channels and samples
// it uses the graph's point buffer (prepared by RenderData), scales the values and
// renders the graph into a double buffer
// a windows message subsequently (in fMain) then invokes a message handler
// (in the main VCL thread and thereby avoiding threading problems), which blits the double buffer
void VISUAL::RenderGraph(int startch, int endch, int startsamp, int endsamp)
{
#ifdef NEW_DOUBLEBUF_SCHEME
  // Restrict drawing to the part of the window that has actually changed.
  // This can dramatically improve performance for big windows and high
  // color depths.
  //
  // This piece of code uses a slightly more conventional way to compute pixel
  // coordinates from sample indices than PaintGraph().
  // To account for the resulting differences, we enlarge the computed rectangle
  // by (compatibilityBelt) pixels on both sides.
  const compatibilityBelt = 5;

  // We always redraw the previously updated region, too,
  // because of different interpolations and the progress line.
  ::CombineRgn( invalidRgn, redrawRgn, NULL, RGN_COPY );

  // Which parts of the window will change? Assemble a region and invalidate it.
  int winxmin = 20,                            // These must be consistent with
      winxmax = form->ClientWidth,             // the values in PaintGraph().
      deltaWinX = winxmax - winxmin,           //
      deltaDatX = displaysamples,              //
      firstSample = startsample - cur_samples,
      afterLastSample = startsample,
      leftX = winxmin + ( firstSample * deltaWinX ) / deltaDatX - compatibilityBelt,
      rightX = winxmin + ( afterLastSample * deltaWinX ) / deltaDatX + compatibilityBelt;

  ::SetRectRgn( invalidRgn, leftX, 0, rightX, form->ClientHeight );
  if( leftX + compatibilityBelt < winxmin )
  {
    HRGN rectRgn = ::CreateRectRgn( leftX + deltaWinX, 0, rightX + deltaWinX, form->ClientHeight );
    ::CombineRgn( redrawRgn, redrawRgn, rectRgn, RGN_OR );
    ::DeleteObject( rectRgn );
  }
  else if( rightX - compatibilityBelt >= winxmax )
  {
    HRGN rectRgn = ::CreateRectRgn( leftX - deltaWinX, 0, rightX - deltaWinX, form->ClientHeight );
    ::CombineRgn( redrawRgn, redrawRgn, rectRgn, RGN_OR );
    ::DeleteObject( rectRgn );
  }
  ::CombineRgn( invalidRgn, invalidRgn, redrawRgn, RGN_OR );

  // Apply the invalid region.
  ::InvalidateRgn( form->Handle, invalidRgn, false );
  bitmap->Canvas->Lock();
  ::SelectClipRgn( bitmap->Canvas->Handle, invalidRgn );
  bitmap->Canvas->Unlock();
}

void VISUAL::PaintGraph(int startch, int endch, int startsamp, int endsamp)
{
#endif // NEW_DOUBLEBUF_SCHEME
AnsiString label;
TCanvas *canvas;
int     ch, samp;
float   dataymin, dataymax;
float   dataxmin, dataxmax;
float   winymin, winymax;
float   winxmin, winxmax;
float   scalex, scaley;
TPoint  *scaledpoints;
TRect   allwindow;

// canvas = form->Canvas;

canvas=bitmap->Canvas;
canvas->Lock();

// limits of the drawing area within the window
winymin=0;
winymax=(float)form->ClientHeight-20;
winxmin=20;
winxmax=(float)form->ClientWidth;

// limits of the data in both directions
dataxmin=0;
dataxmax=displaysamples;
dataymin=0;
dataymax=65536*displaychannels;                 // limits for each channel 0..65535

// the total size of the whole chart within the window
allwindow=Rect(0, 0, form->ClientWidth, form->ClientHeight);

// if we do a complete update, clear the background first
// if (update_type == UPDATETYPE_ALL)
//    {
   canvas->Brush->Color=clBlack;
   // canvas->FillRect(Rect(winxmin, winymin, winxmax, winymax));
   // fill everything including the chart axes
   canvas->FillRect(allwindow);
   canvas->Pen->Color=clAqua;
   canvas->Pen->Width=1;
   // draw the x and y axis
   canvas->MoveTo(winxmin-1, winymin+1);
   canvas->LineTo(winxmin-1, winymax+1);
   canvas->MoveTo(winxmin-1, winymax+1);
   canvas->LineTo(winxmax, winymax+1);
   canvas->Pen->Color=clWhite;
//    }

 scalex=(winxmax-winxmin)/(dataxmax-dataxmin-1);
 scaley=(winymax-winymin)/(dataymax-dataymin);

 //
 // draw the graph
 //
 scaledpoints=new TPoint[displaysamples];
 // re-scale the data points to fit the window and draw the lines
 // this (acting on a "generic" data source and re-scaling it has a little overhead, but is much more flexible
 for (ch=0; ch<displaychannels; ch++)
  {
  for (samp=0; samp<displaysamples; samp++)
   {
   scaledpoints[samp].x=(int)(winxmin+((float)samp-(float)startsamp)*scalex);
   if ((ch+startchannel >= 0) && (ch+startchannel < total_displaychannels))
      scaledpoints[samp].y=(int)(winymax-((float)points[ch+startchannel][samp].y-dataymin+(float)ch*65536)*scaley);
   else
      scaledpoints[samp].y=(int)(winymax-((float)ch*65536)*scaley);
   }
  // actually draw the lines
  // if (ch%2 == 0)
  //    canvas->Pen->Color=clWhite;
  // else
  //    canvas->Pen->Color=clYellow;
  canvas->Polyline((TPoint *)scaledpoints, displaysamples-1);
  }
 delete scaledpoints;

 // draw the progress line (if the number of samples in the signal is smaller than the total number in the display)
 if (cur_samples < displaysamples)
    {
    canvas->Pen->Color=clYellow;
    canvas->MoveTo((int)(winxmin+((float)startsample)*scalex), (int)winymin);
    canvas->LineTo((int)(winxmin+((float)startsample)*scalex), (int)winymax);
    }

 //
 // draw the axis labels
 //
 canvas->Font->Color=clAqua;
 canvas->Pen->Color=clAqua;
 // labels on y axis
 for (ch=0; ch<displaychannels; ch++)
  {
  if ((ch+startchannel >= 0) && (ch+startchannel < total_displaychannels))
     canvas->TextOut(2, (int)(winymax-(32768-dataymin+(float)ch*65536)*scaley)-abs(canvas->Font->Height)/2, AnsiString(ch+1+startchannel));
  else
     canvas->TextOut(2, (int)(winymax-(32768-dataymin+(float)ch*65536)*scaley)-abs(canvas->Font->Height)/2, "NA");
  }
 // labels on x axis
 for (samp=0; samp<displaysamples; samp+=50)
  {
  label=AnsiString(samp+1);
  canvas->TextOut((int)(winxmin+(float)samp*scalex)-canvas->TextWidth(label)/2, winymax+5, label);
  }

 //
 // draw the ticks
 //
 // ticks on y axis
 for (ch=0; ch<displaychannels; ch++)
  {
  canvas->MoveTo(winxmin-3, (int)(winymax-(32768-dataymin+(float)ch*65536)*scaley));
  canvas->LineTo(winxmin, (int)(winymax-(32768-dataymin+(float)ch*65536)*scaley));
  }

 // ticks on x axis
 for (samp=0; samp<displaysamples; samp+=50)
  {
  canvas->MoveTo((int)(winxmin+(float)samp*scalex), winymax+1);
  canvas->LineTo((int)(winxmin+(float)samp*scalex), winymax+4);
  }

 // double buffering; copy the whole image into the actual form at once
 canvas->Unlock();

 // lock the destination (i.e., the form's) canvas before copying to prevent
 // the main thread from interfering
 // DO NOT ASK ME WHY THIS DIDN'T WORK (at least didn't work all the time)
 // in fMain, there is a Windows message that invokes a message handler that blits the double buffer
}

// prepare generic int signal and call the actual rendering procedure
// i.e., fill the graphing point buffer with the actual (i.e., raw) values
// a subsequent call to RenderGraph then renders these values into a double buffer
// a windows message subsequently (in fMain) then invokes a message handler
// (in the main VCL thread and thereby avoiding threading problems), which blits the double buffer
void VISUAL::RenderData(const GenericIntSignal *signal)
{
int     channels, samples, ch, samp, i;
char    buf[256];
bool    recreate;

 // if ((!signal) || (!chart)) return;
 if (!signal) return;

 critsec->Acquire();
 channels=signal->Channels();
 samples=cur_samples=signal->MaxElements();
 recreate=false;
 if (samples > displaysamples)
    {
    displaysamples=samples;
    recreate=true;
    }

 // if the current number of channels in the chart does not match the
 // number of channels, delete the old ones and set new ones up
 if ((total_displaychannels != channels) || (recreate))
    {
    total_displaychannels=channels;
    displaychannels=channels;
    if (displaychannels > 16) displaychannels=16;
    // delete the data for all the channels
    for (ch=0; ch<MAX_DISPLAYCHANNELS; ch++)
     {
     if (points[ch]) delete points[ch];
     points[ch]=NULL;
     }
    for (ch=0; ch<channels; ch++)
     {
     points[ch]=new TPoint[displaysamples];
     for (samp=0; samp<displaysamples; samp++)
      {
      points[ch][samp].x=samp;
      points[ch][samp].y=0;
      }
    }
   }

 // now, fill the data point arrays with the new values
 for (ch=0; ch<channels; ch++)
  {
  for (samp=0; samp<samples; samp++)
   points[ch][(samp+startsample)%displaysamples].y=(int)(((double)signal->GetValue(ch, samp)-minvalue)/(maxvalue-minvalue)*65536);
  }

 startsample=(samples+startsample)%displaysamples;

 // actually render the graph in the double buffer
 // that is, ALL channels and ALL samples
 // a windows message subsequently (in fMain) then invokes a message handler
 // (in the main VCL thread and thereby avoiding threading problems), which blits the double buffer
 RenderGraph(0, displaychannels-1, 0, displaysamples-1);
 critsec->Release();
}


// prepare generic signal and call the actual rendering procedure
// i.e., fill the graphing point buffer with the actual (i.e., raw) values
// a subsequent call to RenderGraph then renders these values into a double buffer
// a windows message subsequently (in fMain) then invokes a message handler
// (in the main VCL thread and thereby avoiding threading problems), which blits the double buffer
void VISUAL::RenderData(const GenericSignal *signal)
{
char    buf[256];
int     channels, samples, ch, samp, i;

 // if ((!signal) || (!chart)) return;
 if (!signal) return;

 critsec->Acquire();
 channels=signal->Channels();
 samples=cur_samples=signal->MaxElements();
 if (samples > displaysamples)
    displaysamples=samples;

 // if the current number of channels in the chart does not match the
 // number of channels, delete the old ones and set new ones up
 if (total_displaychannels != channels)
    {
    total_displaychannels=channels;
    displaychannels=channels;
    if (displaychannels > 16) displaychannels=16;
    // delete the data for all the channels
    for (ch=0; ch<MAX_DISPLAYCHANNELS; ch++)
     {
     if (points[ch]) delete points[ch];
     points[ch]=NULL;
     }
    for (ch=0; ch<channels; ch++)
     {
     points[ch]=new TPoint[displaysamples];
     for (samp=0; samp<displaysamples; samp++)
      {
      points[ch][samp].x=samp;
      points[ch][samp].y=0;
      }
    }
   }

 // now, fill the data point arrays with the new values
 try {
 for (ch=0; ch<channels; ch++)
  {
  for (samp=0; samp<samples; samp++)
   points[ch][(samp+startsample)%displaysamples].y=(int)(((double)signal->GetValue(ch, samp)-minvalue)/(maxvalue-minvalue)*65536);
  }
 } catch( TooGeneralCatch& ) {;}

 startsample=(samples+startsample)%displaysamples;

 // actually render the graph in the double buffer
 // that is, ALL channels and ALL samples
 // a windows message subsequently (in fMain) then invokes a message handler
 // (in the main VCL thread and thereby avoiding threading problems), which blits the double buffer
 RenderGraph(0, displaychannels-1, 0, displaysamples-1);
 critsec->Release();
}

