//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <Series.hpp>
#include <stdio.h>
#include <math.h>

#include "..\shared\defines.h"
#include "UGenericVisualization.h"
#include "UVisConfig.h"
#include "UCoreMessage.h"

#pragma link "CSPIN"

//---------------------------------------------------------------------------

#pragma package(smart_init)


GenericVisualization::GenericVisualization()
: intsignal( NULL ),
  signal( NULL ),
  corecomm( NULL ),
  new_samples( -1 )
{
}


GenericVisualization::GenericVisualization(PARAMLIST *paramlist, CORECOMM *new_corecomm)
: intsignal( NULL ),
  signal( NULL ),
  corecomm( NULL ),
  new_samples( -1 )
{
 paramlist->AddParameter2List( "Source intlist VisChList= 5 11 23 1 2 3 11 1 64  // list of channels to visualize" );
}


GenericVisualization::~GenericVisualization()
{
 delete intsignal;
 delete signal;
}


void GenericVisualization::SetSourceID(BYTE my_sourceID)
{
 sourceID=my_sourceID;
}


BYTE GenericVisualization::GetSourceID()
{
 return(sourceID);
}


void GenericVisualization::SetDataType(BYTE my_datatype)
{
 datatype=my_datatype;
}


BYTE GenericVisualization::GetDataType()
{
 return(datatype);
}


void GenericVisualization::SetVisualizationType(BYTE my_vistype)
{
 vis_type=my_vistype;
}


const GenericIntSignal *GenericVisualization::GetIntSignal() const
{
 return(intsignal);
}


const GenericSignal *GenericVisualization::GetSignal() const
{
 return(signal);
}


const char *GenericVisualization::GetMemoText() const
{
 return(memotext);
}


bool GenericVisualization::SendMemo2Operator(const char *string)
{
TWinSocketStream        *pStream;
COREMESSAGE     *coremessage;
short   *valueptr;
BYTE    *dataptr;
int     s, t;

 // error and consistency checking
 if (!corecomm)              return(false);       // core communicatino not defined
 if (!corecomm->Connected()) return(false);       // no connection to the core module
 if (strlen(string)+1+4 > COREMESSAGE_MAXBUFFER) return(false);     // data too big for a coremessage

 pStream=new TWinSocketStream(corecomm->GetSocket(), 5000);

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_DATA);
 coremessage->SetSuppDescriptor(VISTYPE_MEMO);
 coremessage->SetLength(strlen(string)+1+4);         // set the length of the coremessage (strlen(..)+1 to account for delimiting 0 byte)

 dataptr=(BYTE *)coremessage->GetBufPtr();
 // construct the header of the core message
 dataptr[0]=sourceID;                        // write the source ID into the coremessage
 strcpy(&dataptr[1], string);                // copy the string into the coremessage

 coremessage->SendCoreMessage(pStream);     // and send it out
 delete coremessage;
 delete pStream;

 return(true);
}


// send signal to operator with decimation
bool GenericVisualization::Send2Operator(const GenericIntSignal *my_signal, int decimation)
{
unsigned short    new_channels;
GenericIntSignal  *new_signal;
bool    ret;
int     ch, samp, count;

 // determine how many samples the decimated signal has
 // there might be a better way of doing this :-(
 // only do this in the beginning or if decimation or signal size changes
 if ((new_samples == -1) || (my_signal->MaxElements() != stored_maxelements) || (decimation != stored_decimation))
    {
    new_samples=0;
    for (samp=0; samp<my_signal->MaxElements(); samp+=decimation)
     new_samples++;
    stored_maxelements=my_signal->MaxElements();
    stored_decimation=decimation;
    }

 new_channels=my_signal->Channels();
 // new_samples=my_signal->MaxElements/decimation;
 // create the new signal
 new_signal=new GenericIntSignal((unsigned short)new_channels, new_samples);
 // copy the content with decimation
 for (ch=0; ch<new_channels; ch++)
  {
  count=0;
  for (samp=0; samp<my_signal->MaxElements(); samp+=decimation)
   {
   new_signal->SetValue(ch, count, my_signal->GetValue(ch, samp));
   count++;
   }
  }

 ret=Send2Operator(new_signal);
 delete new_signal;

 return(ret);
}


bool GenericVisualization::Send2Operator(const GenericIntSignal *my_signal)
{
TWinSocketStream        *pStream;
unsigned short  *short_dataptr;
COREMESSAGE     *coremessage;
short   *valueptr;
BYTE    *dataptr;
int     s, t;

 // error and consistency checking
 if (my_signal->Channels() > 255)      return(false);       // Channels > 255
 if (my_signal->MaxElements() > 65535) return(false);       // samples per channel > 65535
 if (!corecomm)                      return(false);       // core communication not defined
 if (!corecomm->Connected())         return(false);       // no connection to the core module
 if ((long)my_signal->Channels()*(long)my_signal->MaxElements()+9 > COREMESSAGE_MAXBUFFER) return(false);     // data too big for a coremessage

 pStream=new TWinSocketStream(corecomm->GetSocket(), 5000);

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_DATA);
 coremessage->SetSuppDescriptor(VISTYPE_GRAPH);
 coremessage->SetLength(sizeof(unsigned short)*(unsigned short)my_signal->Channels()*(unsigned short)my_signal->MaxElements()+5);         // set the length of the coremessage

 dataptr=(BYTE *)coremessage->GetBufPtr();
 // construct the header of the core message
 dataptr[0]=sourceID;                   // write the source ID into the coremessage
 dataptr[1]=DATATYPE_INTEGER;           // write the datatype into the coremessage
 dataptr[2]=(BYTE)my_signal->Channels();// write the # of channels into the coremessage
 short_dataptr=(unsigned short *)&dataptr[3];
 *short_dataptr=(unsigned short)my_signal->MaxElements(); // write the # of samples into the coremessage
 // write the actual data into the coremessage
 for (t=0; t<my_signal->Channels(); t++)
  for (s=0; s<my_signal->MaxElements(); s++)
   {
   valueptr=(short *)&dataptr[5];
   valueptr[t*my_signal->MaxElements()+s]=my_signal->GetValue(t, s);
   }

 coremessage->SendCoreMessage(pStream);     // and send it out
 delete coremessage;
 delete pStream;

 return(true);
}


bool GenericVisualization::Send2Operator(const GenericSignal *my_signal)
{
TWinSocketStream        *pStream;
unsigned short  *short_dataptr;
COREMESSAGE     *coremessage;
BYTE    *dataptr, *dataptr2;
int     s, t;
signed char exponent;
float   value, value2;

 // error and consistency checking
 if (my_signal->Channels() > 255)      return(false);       // Channels > 255
 if (my_signal->MaxElements() > 65535) return(false);       // samples per channel > 65535
 if (!corecomm)                      return(false);       // core communication not defined
 if (!corecomm->Connected())         return(false);       // no connection to the core module
 if ((long)my_signal->Channels()*(long)my_signal->MaxElements()+8 > COREMESSAGE_MAXBUFFER) return(false);     // data too big for a coremessage

 pStream=new TWinSocketStream(corecomm->GetSocket(), 5000);

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_DATA);
 coremessage->SetSuppDescriptor(VISTYPE_GRAPH);
 coremessage->SetLength(3*(unsigned short)my_signal->Channels()*(unsigned short)my_signal->MaxElements()+5);         // set the length of the coremessage

 dataptr=(BYTE *)coremessage->GetBufPtr();
 // construct the header of the core message
 dataptr[0]=sourceID;                   // write the source ID into the coremessage
 dataptr[1]=DATATYPE_FLOAT;             // write the datatype into the coremessage
 dataptr[2]=(BYTE)my_signal->Channels();  // write the # of channels into the coremessage
 short_dataptr=(unsigned short *)&dataptr[3];
 *short_dataptr=(unsigned short)my_signal->MaxElements(); // write the # of samples into the coremessage
 // write the actual data into the coremessage
 for (t=0; t<my_signal->Channels(); t++)
  for (s=0; s<my_signal->MaxElements(); s++)
   {
   value=my_signal->GetValue(t, s);
   if (value != 0)
      {
      exponent=(int)(ceil(log10(fabs(value))));
      value2=value/pow10((int)exponent);
      exponent-=4;
      value2*=10000;
      }
   else
      {
      value2=0;
      exponent=1;
      }
   dataptr2=&dataptr[5+3*t*my_signal->MaxElements()+3*s];
   *((short *)&dataptr2[0])=(short)value2;
   *((signed char *)&dataptr2[2])=exponent;
   }

 coremessage->SendCoreMessage(pStream);     // and send it out
 delete coremessage;
 delete pStream;

 return(true);
}


bool GenericVisualization::SendCfg2Operator(BYTE sourceID, BYTE cfgID, const char *cfgString)
{
TWinSocketStream        *pStream;
COREMESSAGE     *coremessage;
BYTE    *dataptr, *dataptr2;
int     s, t;
signed char exponent;
float   value, value2;

 if (!corecomm)                    return(false);       // socket not defined
 if (!corecomm->Connected())       return(false);       // no connection to the core module

 // error and consistency checking
 if (strlen(cfgString) > 255) return(false);

 pStream=new TWinSocketStream(corecomm->GetSocket(), 5000);

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_DATA);
 coremessage->SetSuppDescriptor(VISTYPE_VISCFG);
 coremessage->SetLength(2+strlen(cfgString)+4);         // set the length of the coremessage

 dataptr=(BYTE *)coremessage->GetBufPtr();
 // construct the header of the core message
 dataptr[0]=sourceID;                      // write the source ID into the coremessage
 dataptr[1]=cfgID;                         // write the config ID into the coremessage
 strcpy((char *)&(dataptr[2]), cfgString); // copy the config string into the coremessage

 coremessage->SendCoreMessage(pStream);     // and send it out
 delete coremessage;
 delete pStream;

 return(true);
}


void  GenericVisualization::ParseVisualization(const char *buffer, int length)
{
int     i, t;
BYTE    channels, cfgID;
unsigned short samples;
signed char exponent;
short   *dataptr, value;
char    buf[256];

 valid=false;
 if (length < 2) return;

 sourceID=buffer[0];

 // visualization type == 1, i.e., type graph
 if (vis_type == VISTYPE_GRAPH)
    {
    datatype=buffer[1];
    if (length < 5) return;
    channels=buffer[2];
    samples=*((unsigned short *)&buffer[3]);
    if (datatype == DATATYPE_INTEGER)
       {
       if (length != (int)channels*(int)samples*2+5) return;    // consistency checks
       if (intsignal) delete intsignal;
       intsignal=new GenericIntSignal((unsigned short)channels, (int)samples);
       for (i=0; i<channels; i++)
        {
        dataptr=(short *)(buffer+5+i*sizeof(short)*(int)samples);
        intsignal->SetChannel(dataptr, i);                      // set the values in the intsignal to the data
        }
       valid=true;
       }
    if (datatype == DATATYPE_FLOAT)
       {
       if (length != (int)channels*(int)samples*3+5) return;    // consistency checks
       if (signal) delete signal;
       signal=new GenericSignal((unsigned short)channels, (int)samples);
       for (i=0; i<channels; i++)
        {
        for (t=0; t<samples; t++)
         {
         dataptr=(short *)(buffer+5+i*3*(int)samples+t*3);
         value=dataptr[0];
         exponent=((signed char *)dataptr)[2];
         signal->SetValue(i, t, value*(float)pow(10, (double)exponent));        // set the values in the signal to the data
         }
        }
       valid=true;
       }
    }

 // visualization type == 2, i.e., type memo field
 if (vis_type == VISTYPE_MEMO)
    strcpy(memotext, &buffer[1]);

 // visualization type == 255, i.e., type visualization config
 if (vis_type == VISTYPE_VISCFG)
    {
    cfgID=buffer[1];
    if (cfgID == CFGID_MINVALUE)
       fVisConfig->SetVisualPrefs(sourceID, 0, "MinValue", atof(&buffer[2]));
    if (cfgID == CFGID_MAXVALUE)
       fVisConfig->SetVisualPrefs(sourceID, 0, "MaxValue", atof(&buffer[2]));
    if (cfgID == CFGID_NUMSAMPLES)
       fVisConfig->SetVisualPrefs(sourceID, 0, "NumSamples", (int)atoi(&buffer[2]));
    if (cfgID == CFGID_WINDOWTITLE)
       fVisConfig->SetVisualPrefs(sourceID, 0, "WindowTitle", &buffer[2]);
    if (cfgID == CFGID_XAXISLABEL)
       {
       sprintf(buf, "XAxisLabel%d", atoi(&buffer[2]));
       fVisConfig->SetVisualPrefs(sourceID, 0, buf, &buffer[6]);
       }
    }

}


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
    form=new TForm(Application);

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
    form=new TForm(Application);

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
 if (form)  form->Close();
 form=NULL;
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
    if (bitmap) delete bitmap;

    bitmap=new Graphics::TBitmap();
    bitmap->Width=form->ClientWidth;
    bitmap->Height=form->ClientHeight;

    // update the display
    RenderGraph(0, displaychannels-1, 0, displaysamples-1);
    critsec->Release();

    Application->ProcessMessages();
    }
}
//---------------------------------------------------------------------------


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
AnsiString label;
TCanvas *canvas;
int     ch, samp;
float	dataymin, dataymax;
float	dataxmin, dataxmax;
float	winymin, winymax;
float	winxmin, winxmax;
float	scalex, scaley;
TPoint  *scaledpoints;
TRect   allwindow;

// canvas=form->Canvas;
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
 } catch(...) {;}

 startsample=(samples+startsample)%displaysamples;

 // actually render the graph in the double buffer
 // that is, ALL channels and ALL samples
 // a windows message subsequently (in fMain) then invokes a message handler
 // (in the main VCL thread and thereby avoiding threading problems), which blits the double buffer
 RenderGraph(0, displaychannels-1, 0, displaysamples-1);
 critsec->Release();
}


