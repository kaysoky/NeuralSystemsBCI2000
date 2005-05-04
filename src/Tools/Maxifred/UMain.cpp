/******************************************************************************
 * Program:   MAXIFRED.EXE                                                    *
 * Module:    UMAIN.CPP                                                       *
 * Comment:   Displays BCI .raw files on the screen                           *
 * Version:   3.3                                                             *
 * Author:    Gerwin Schalk                                                   *
 * Copyright: (C) Wadsworth Center, NYSDOH                                    *
 ******************************************************************************
 * Version History:                                                           *
 *                                                                            *
 * V0.1  - 07/07/1999 - First start                                           *
 * V0.2  - 07/12/1999 - Included buffering mechanism                          *
 *                      show time as x-axis, etc.                             *
 * V0.3  - 07/17/1999 - User can define # of channels and samples to display  *
 * V0.4  - 07/19/1999 - Allows to read in .ch files                           *
 * V1.0  - 08/04/1999 - Export to clip board and printing feature added       *
 * V1.1  - 08/04/1999 - Range marking feature added                           *
 * V1.2  - 09/23/1999 - added boolean operations to the range marking         *
 * V1.3  - 11/01/1999 - little bug in display removed                         *
 * V2.0  - 07/21/2000 - Updated to Borland C++ Builder 5, downgraded from     *
 *                      TeeChart pro; included support for BCI2000 file format*
 * V2.1  - 08/31/2000 - Included a function to save to disk                   *
 * V2.1b - 04/05/2001 - removed small bug w/uneven length of statevector      *
 * V3.0  - 04/17/2001 - using BCI2000DATA class                               *
 * V3.1  - 07/18/2001 - increased sample limit to 25000                       *
 *                      fixed time scale on x axis                            *
 * V3.3  - 01/28/2002 - added scaling factor for y-axis                       *
 ******************************************************************************/

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <stdlib.h>

#include "defines.h"
#include "UAbout.h"
#include "UEditChannellist.h"
#include "UStateDialog.h"
#include "USave2Disk.h"
#include "UMain.h"
#include "Series.hpp"
//---------------------------------------------------------------------------
#pragma link "Chart"
#pragma link "TeEngine"
#pragma link "TeeProcs"
#pragma link "Series"

#pragma link "CSPIN"
#pragma link "CGAUGES"
#pragma resource "*.dfm"
#pragma package(smart_init)

EEGFILEINFO     EEGfileinfo;
TfMain *fMain;
FILE    *fp;
__int16 num_channels, sample_freq, dummy, headerlength;
int     fileformat;
int     start_channel=0;
long    start_sample=0;
extern AnsiString channelnames[MAX_CHANNELS];
extern void init_channelnames();
bool    initialized=false;
int     display_channels, display_samples;
bool    selectrange;
int     selectrange_xcstart, selectrange_xcend;
long    selectrange_samplestart, selectrange_sampleend;

//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner)
: TForm(Owner),
  GetSampleValue( NULL )
{
 bci2000data=NULL;
 
 for( int i = 0; i < ComponentCount; ++i )
 {
   TCSpinEdit* cSpComponent = dynamic_cast<TCSpinEdit*>( Components[ i ] );
   if( cSpComponent != NULL )
     cSpComponent->OnChange = CSpEditOnChange;
 }
 eScaling->OnExit = bGoButton->OnClick;
 ReadCalibrationCheckBox();
}

_fastcall TfMain::~TfMain()
{
 if (bci2000data) delete bci2000data;
 bci2000data=NULL;
}

// Functions to switch between calibrated/uncalibrated behavior via assignment
// to the GetSampleValue pointer.
void TfMain::ReadCalibrationCheckBox()
{
 if( cCalibrationCheckBox->Checked )
   GetSampleValue = GetSampleValueCalibrated;
 else
   GetSampleValue = GetSampleValueUncalibrated;
}

float TfMain::GetSampleValueCalibrated( BCI2000DATA* data, int channel, unsigned long sample )
{
  return data->Value( channel, sample ) / 0.003;
}

float TfMain::GetSampleValueUncalibrated( BCI2000DATA* data, int channel, unsigned long sample )
{
  return data->ReadValue( channel, sample );
}

// Fix erroneous behaviour of TCSpinEdit component, jm
void __fastcall TfMain::CSpEditOnChange( TObject* Sender )
{
  TEdit* editField = static_cast<TEdit*>( Sender );
  editField->OnChange = NULL;
  AnsiString text = editField->Text;
  int selStart = editField->SelStart;
  bool leaveEditor = text.Pos( "\n" ) || text.Pos( "\r" );
  int i = 1;
  while( i <= text.Length() )
  {
    if( !isdigit( text[ i ] ) )
      text.Delete( i, 1 );
    else
      ++i;
  }
  if( text == "" )
    text = "0";
  editField->Text = text;
  if( leaveEditor )
    editField->SelectAll();
  else
  {
    if( selStart > text.Length() )
      selStart = text.Length();
    editField->SelLength = 0;
    editField->SelStart = selStart;
  }
  editField->OnChange = CSpEditOnChange;

#if 0
  // Take some action.
  if( obj == cDisplaySamples || obj == cNumChannels )
    if( bGoButton->OnClick != NULL )
      bGoButton->OnClick( Sender );
#endif
}

// **************************************************************************
// Function:   UpdateStatusBar
// Purpose:    Sets the text of the status bar to the text given
// **************************************************************************
void TfMain::UpdateStatusBar(AnsiString text)
{
 tMainStatusBar->SimpleText=text;
}

// **************************************************************************
// Function:   InitializeGraph
// Purpose:    initializes the main display chart
// **************************************************************************
//---------------------------------------------------------------------------
void TfMain::InitializeGraph()
{
int     i, t;

 TeeDefaultCapacity=display_samples;

 if (MainChart->SeriesList->Count != 0)
    for (i=0; i<MainChart->SeriesList->Count; i++)
     delete MainChart->Series[i];
 MainChart->RemoveAllSeries();

 // for (i=0; i<MainChart->SeriesCount()-1; i++)
 // MainChart->Series[i]->Clear();

 for (i=0; i<display_channels; i++)
  {
  MainChart->AddSeries(new TLineSeries(this));
  MainChart->Series[i]->Active=false;
  MainChart->Series[i]->Title="title";
  MainChart->Series[i]->SeriesColor=clBlack;
  for (t=0; t<display_samples; t++)
   MainChart->Series[i]->AddXY(t, 0, "", clTeeColor);
  }

 initialized=true;
}


// **************************************************************************
// Function:   bGoButtonClick
// Purpose:    Uses the given input file and displays it, starting with
//             sample 0 and channel 0
// **************************************************************************
//---------------------------------------------------------------------------
void __fastcall TfMain::bGoButtonClick(TObject *Sender)
{
char    filename[255], buf[255];
int     state, ret;

  // is this the same filename as before ?
  if (inputfile != eFilename->Text.c_str())
     {
     inputfile=eFilename->Text.c_str();
     strcpy(filename, inputfile.c_str());
     readonly=false;
     if (bci2000data) delete bci2000data;
     bci2000data=new BCI2000DATA;
     ret=bci2000data->Initialize(filename, 50000);
     if (ret == BCI2000ERR_NOERR)
        {
        // write the state names in the listbox
        cStateListBox->Clear();
        for (state=0; state < bci2000data->GetStateListPtr()->GetNumStates(); state++)
         cStateListBox->Items->Add(bci2000data->GetStateListPtr()->GetStatePtr(state)->GetName());
        }
     }
  else
     strcpy(filename, eFilename->Text.c_str());

  if (fp)
     {
     fclose(fp);
     fp=NULL;
     }
  fp=fopen(filename, "r+b");    // does it open read-write ?
  if (!fp)
     {
     fp=fopen(filename, "rb");  // maybe it opens read-only ?
     if (fp)
        {
        if (readonly == false)
           {
           if (Application->MessageBox("This file seems to be read-only. You won't be able to mark states. Do you want to open it ?", "Question", MB_YESNO) == ID_YES)
              readonly=true;
           else
              {
              fclose(fp);
              fp=NULL;
              return;
              }
           }
        }
     else
        {
        Application->MessageBox("Could not open input file", "Error :-(", MB_OK);
        fp=NULL;
        readonly=false;
        return;
        }
     }

  // bGoButton->Enabled=false;
  bJump->Enabled=true;
  bSave2Disk->Enabled=true;
  bCopy2Clipboard->Enabled=true;
  bEditChannellist->Enabled=true;

  display_channels=cNumChannels->Value;
  display_samples=cDisplaySamples->Value;

  initialized=false;
  num_channels=bci2000data->GetNumChannels();
  sample_freq=bci2000data->GetSampleFrequency();

  UpdateStatusBar("Loading samples from file ...");
  InitializeGraph();
  UpdateStatusBar("Updating graph display ...");
  UpdateMainChart();
  UpdateStatusBar("< idle >");
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   FetchChannelNumber
// Purpose:    Returns, for a given channel number on the screen,
//             the actual physical channel number
// Parameters: displaynumber - channel number in order of display on the screen
// Returns:    physical channel number of this channel
// **************************************************************************
int TfMain::FetchChannelNumber(int displaynumber)
{
int     i, channelnum;

 // per default (or if the corresponding channel is not found), return
 // the same channel number (-> physical channel number = channel number on the screen)
 channelnum=displaynumber;

 for (i=0; i<num_channels && i < MAX_CHANNELS; i++)
  if (channellist[i].displayposition == displaynumber)
     channelnum=i;

 return(channelnum);
}


// **************************************************************************
// Function:   UpdateMainChart
// Purpose:    Given a starting channel and sample (start_channel, start_sample)
//             updates the graph display on the screen
// Parameters: none
// Returns:    N/A
// **************************************************************************
void TfMain::UpdateMainChart()
{
int     t, i, cur_channel;
long    msec, sec, min, hour;
float   car, reference, scaling;

 for (i=0; i<display_channels; i++)
  MainChart->Series[i]->Active=false;

 scaling=atof(eScaling->Text.c_str());

 MainChart->BottomAxis->DateTimeFormat = "hh:mm:ss";
 for (i=0; i<display_channels; i++)
  MainChart->Series[i]->XValues->DateTime=true;

 for (t=0; t<display_samples; t++)
  {
  car=Get_CAR_Value(t+start_sample);
  for (i=0; i<display_channels; i++)
   {
   cur_channel=FetchChannelNumber(i+start_channel);
   reference=0;
   if (channellist[cur_channel].chanreftype == REFTYPE_CAR)
      reference=car;
   if (i+start_channel < num_channels)
      MainChart->Series[i]->YValues->Value[t]=(GetSampleValue( bci2000data, cur_channel, t+start_sample)-reference)/12288*scaling+i;
   else
      MainChart->Series[i]->YValues->Value[t]=i;
   // MainChart->Series[i]->XValues->Value[t]=t+start_sample;
   msec=(long)((((float)t+(float)start_sample)/(float)sample_freq)*1000);
   hour=msec/3600000;
   min=(msec-hour*3600000)/60000;
   sec=(msec-hour*3600000-min*60000)/1000;
   msec=msec-hour*3600000-min*60000-sec*1000;
   MainChart->Series[i]->XValues->Value[t]=EncodeTime((WORD)hour, (WORD)min, (WORD)sec, (WORD)msec);
   }
   cMainGauge->Progress = ( 100 * ( t + 1 ) ) / display_samples;
  }

 for (i=0; i<display_channels; i++)
  MainChart->Series[i]->Active=true;

 MainChart->Refresh();
 cMainGauge->Progress=0;
}


// **************************************************************************
// Function:   Get_CAR_Value
// Purpose:    Returns the Common Average Reference value in the .raw file
//             for a given sample (basically the average of all channels)
// Parameters: sample - sample number
// Returns:    value requested
// **************************************************************************
float TfMain::Get_CAR_Value(ULONG sample)
{
int     i;
float   sum;

 sum=0;
 for (i=0; i<num_channels; i++)
  sum += GetSampleValue( bci2000data, i, sample);

 return(sum/num_channels);
}



// **************************************************************************
// Function:   Set_State
// Purpose:    Sets the state value in the .raw file for a given range of samples
// Parameters: samplestart - start sample of range to change
//             sampleend   - end sample of range to change
//             state - the desired destination state
//             This procedure does not update the main chart, so the calling
//             procedure has to take care of it
// Returns:    no return values
// **************************************************************************
void TfMain::Set_State(long samplestart, long sampleend, __int16 state, int setstatetype)
{
long    file_ptr, sampledummy, i;
__int16 statetoset;
STATEDESC       cur_state;


 return;

 if (sampleend < samplestart)
    {
    sampledummy=sampleend;
    sampleend=samplestart;
    samplestart=sampledummy;
    }

 for (i=samplestart; i<=sampleend; i++)
  {
  // Get_State(i, &cur_state);
  if (setstatetype == SETSTATETYPE_BOOLEANOR) statetoset=cur_state.rawstate|state;
  if (setstatetype == SETSTATETYPE_BOOLEANAND) statetoset=cur_state.rawstate&state;
  if (setstatetype == SETSTATETYPE_JUSTSET) statetoset=state;
  file_ptr=HEADER_SIZE+(ULONG)num_channels*2+i*2*((ULONG)num_channels+1);
  fseek(fp, (long)file_ptr, SEEK_SET);
  fwrite((const void *)&statetoset, 2, 1, fp);
  }

 return;
}


// **************************************************************************
// Function:   FormClose
// Purpose:    Closes the form on end, frees the buffers and closes the input file
// Parameters: don't matter
// **************************************************************************
void __fastcall TfMain::FormClose(TObject *Sender, TCloseAction &Action)
{
  if (fp) fclose(fp);
  fp=NULL;
}
//---------------------------------------------------------------------------


void __fastcall TfMain::LeftRightMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
 start_sample += LeftRight->Position*(display_samples*70/100);
 if (start_sample < 0) start_sample=0;
 LeftRight->Position=0;
 UpdateStatusBar("Updating display ...");
 UpdateMainChart();
 UpdateStatusBar("< idle >");
}
//---------------------------------------------------------------------------


void __fastcall TfMain::UpDownMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
 if (display_channels >= (int)(1.5*DISPLAYDELTA_CHANNELS))
    start_channel += UpDown->Position*DISPLAYDELTA_CHANNELS;
 else
    start_channel += UpDown->Position*1;
 if (start_channel < 0) start_channel=0;
 UpDown->Position=0;
 UpdateStatusBar("Updating display ...");
 UpdateMainChart();
 UpdateStatusBar("< idle >");
}
//---------------------------------------------------------------------------


void __fastcall TfMain::FormKeyUp(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
//MainChart->SetFocus();

 if (Key == 38)
    {
    if (display_channels >= (int)(1.5*DISPLAYDELTA_CHANNELS))
       start_channel += DISPLAYDELTA_CHANNELS;
    else
       start_channel ++;
    if (start_channel >= num_channels-1) start_channel=num_channels-1;
    UpdateStatusBar("Detected key up ...");
    UpdateMainChart();
    UpdateStatusBar("< idle >");
    }
 if (Key == 40)
    {
    if (display_channels >= (int)(1.5*DISPLAYDELTA_CHANNELS))
       start_channel -= DISPLAYDELTA_CHANNELS;
    else
       start_channel --;
    if (start_channel < 0) start_channel=0;
    UpdateStatusBar("Detected key down ...");
    UpdateMainChart();
    UpdateStatusBar("< idle >");
    }
 if (Key == 37)
    {
    start_sample -= (display_samples*70/100);
    if (start_sample < 0) start_sample=0;
    UpdateStatusBar("Detected key left ...");
    UpdateMainChart();
    UpdateStatusBar("< idle >");
    }
 if (Key == 39)
    {
    start_sample += (display_samples*70/100);
    UpdateStatusBar("Detected key right ...");
    UpdateMainChart();
    UpdateStatusBar("< idle >");
    }
}
//---------------------------------------------------------------------------


void __fastcall TfMain::bGetFileClick(TObject *Sender)
{
 if (OpenDialog->Execute())
    eFilename->Text=OpenDialog->FileName;
 else
    return;
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   MainChartGetAxisLabel
// Purpose:    draws the axis labels for the left (=Y) axis
// Parameters: don't matter
// **************************************************************************
void __fastcall TfMain::MainChartGetAxisLabel(TChartAxis *Sender,
      TChartSeries *Series, int ValueIndex, AnsiString &LabelText)
{
char    buf[15];
int     channelnum, cur_channel;

 if (Sender == MainChart->LeftAxis)
    {
    channelnum=atoi(LabelText.c_str());
    sprintf(buf, "%d", channelnum);
    if (LabelText == (AnsiString)buf)
       {
       cur_channel=FetchChannelNumber(channelnum+start_channel);
       if ((channelnum+start_channel < num_channels) && (channelnum+start_channel >= 0))
          sprintf(buf, "%d-%s", cur_channel+1, channellist[cur_channel].name);
       else
          strcpy(buf, "N/A");
       LabelText=buf;
       }
    }
}


//---------------------------------------------------------------------------
void __fastcall TfMain::FormCreate(TObject *Sender)
{
int     i;

 init_channelnames();

 for (i=0; i<MAX_CHANNELS; i++)
  {
  channellist[i].chanreftype=REFTYPE_NORMAL;
  channellist[i].name=channelnames[i];
  channellist[i].displayposition=i;
  }

 fMain->Width=722;
 fMain->Height=480;
 selectrange=false;

 inputfile="";
 readonly=false;
}
//---------------------------------------------------------------------------


AnsiString TfMain::GetStateText(int state)
{
if (state == STATE_IRI)         return("IRI");
if (state == STATE_GETTARGET)   return("Get");
if (state == STATE_HIT)         return("Hit");
if (state == STATE_MISS)        return("Miss");
if (state == STATE_ITI)         return("ITI");
if (state == STATE_BASELINE)    return("Baseline");

if (state == 2)  return("UL DR");
if (state == 3)  return("UM DR");
if (state == 4)  return("UR DR");
if (state == 5)  return("MR DR");
if (state == 6)  return("LR DR");
if (state == 7)  return("LM DR");
if (state == 8)  return("LL DR");
if (state == 9)  return("ML DR");
if (state == 10) return("UL USE");
if (state == 11) return("UM USE");
if (state == 12) return("UR USE");
if (state == 13) return("MR USE");
if (state == 14) return("LR USE");
if (state == 15) return("LM USE");
if (state == 16) return("LL USE");
if (state == 17) return("ML USE");

return(AnsiString(state));
}


// **************************************************************************
// Function:   MainChartAfterDraw
// Purpose:    after redrawing the main chart, draw the state change
//             indicators on top of that
// Parameters: don't matter
// **************************************************************************
void __fastcall TfMain::MainChartAfterDraw(TObject *Sender)
{
AnsiString statetext;
int     xc, t;
float   factor;
int     cur_state, old_state, state;
char    cur_statename[256];

 if (!bci2000data) return;
 if (bci2000data->Initialized())
    {
    // now draw all the lines
    for (state=0; state < bci2000data->GetStateListPtr()->GetNumStates(); state++)
     {
     if (cStateListBox->Checked[state])
        {
        old_state=0;
        strcpy(cur_statename, bci2000data->GetStateListPtr()->GetStatePtr(state)->GetName());
        for (t=0; t<display_samples; t++)
         {
         bci2000data->ReadStateVector(t+start_sample);
         cur_state=bci2000data->GetStateVectorPtr()->GetStateValue(cur_statename);
         if ((old_state != cur_state) && (t != 0))
            {
            MainChart->Canvas->Pen->Color=clAqua;
            // MainChart->Canvas->Pen->Style=psSolid;
            // MainChart->Canvas->Pen->Mode=pmCopy;
            factor=(float)(MainChart->ChartRect.Right-MainChart->ChartRect.Left)/(float)(display_samples);
            xc=(int)((float)t*factor+(float)MainChart->ChartRect.Left);
            MainChart->Canvas->MoveTo(xc, MainChart->ChartRect.Top);
            MainChart->Canvas->LineTo(xc, MainChart->ChartRect.Bottom);
            statetext=AnsiString(cur_statename)+": "+AnsiString(cur_state);
            MainChart->Canvas->TextOut(xc, MainChart->ChartRect.Top-MainChart->Canvas->TextHeight(statetext), statetext);
            }
         old_state=cur_state;
         }
        }
       }
     }
}
//---------------------------------------------------------------------------


void __fastcall TfMain::bJumpClick(TObject *Sender)
{
int     cur_state, old_state;
int     cur_run, target_run;
int     cur_trial, target_trial;
long    sample;
char    buf[255];
STATEDESC       my_state;

 target_run=eRun->Value;
 bci2000data->SetRun(target_run);

 start_sample=0;
 UpdateStatusBar("Updating display ...");
 UpdateMainChart();
 UpdateStatusBar("< idle >");

 return;

 if (fp)
    {
    // seeking for given run
    sample=0;
    cur_run=1;
    target_run=eRun->Value;
    old_state=-1;
    sprintf(buf, "Seeking for given run; current run=%d ...", cur_run);
    UpdateStatusBar(buf);
    while ((!feof(fp)) && (target_run != cur_run))
     {
     // Get_State(sample, &my_state);
     cur_state=my_state.state;
     if (old_state != cur_state)
        {
        if (cur_state == STATE_IRI)
           {
           cur_run++;
           sprintf(buf, "Seeking for given run; current run=%d ...", cur_run);
           UpdateStatusBar(buf);
           Application->ProcessMessages();
           }
        }
     old_state=cur_state;
     sample++;
     }

    // seeking for given trial in this run
    cur_trial=1;
    target_trial=eTrial->Value;
    old_state=-1;
    sprintf(buf, "Seeking for given trial; current run=%d ...", cur_trial);
    UpdateStatusBar(buf);
    while ((!feof(fp)) && (target_trial != cur_trial))
     {
     // Get_State(sample, &my_state);
     cur_state=my_state.state;
     if (old_state != cur_state)
        {
        if (cur_state == STATE_ITI)
           {
           cur_trial++;
           sprintf(buf, "Seeking for given trial; current run=%d ...", cur_trial);
           UpdateStatusBar(buf);
           Application->ProcessMessages();
           }
        }
     old_state=cur_state;
     sample++;
     }

    start_sample=sample;
    UpdateStatusBar("Updating display ...");
    UpdateMainChart();
    UpdateStatusBar("< idle >");
    }
}
//---------------------------------------------------------------------------



// **************************************************************************
// Function:   MainChartMouseUp
// Purpose:    detects mouse click releases on the main chart
//             if there is a left click within the chart area, ask whether
//             user wants to insert a new state
// **************************************************************************
//---------------------------------------------------------------------------
void __fastcall TfMain::MainChartMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
long    xc;
float   factor;

 if ((initialized) && (Button == mbLeft))
    {
    // check if within chart area
    if ((X > MainChart->ChartRect.Left) && (X < MainChart->ChartRect.Right))
       if ((Y > MainChart->ChartRect.Top) && (Y < MainChart->ChartRect.Bottom))
          if (selectrange == true)
             {
             xc=X-MainChart->ChartRect.Left;
             factor=(float)xc/(float)(MainChart->ChartRect.Right-MainChart->ChartRect.Left);
             // are we working on a read-only file ?
             if (readonly == true)
                {
                Application->MessageBox("You opened the file read-only. You will not be able to mark states", "Error :-(", MB_OK);
                TCanvas3DRectangle(selectrange_xcstart, selectrange_xcend);
                }
             else
                {
                OKBottomDlg->ShowModal();
                if (UserStateEntered)
                   {
                   selectrange_sampleend=(int)((float)start_sample+factor*(float)display_samples);
                   Set_State(selectrange_samplestart, selectrange_sampleend, UserStateVal, SetStateType);
                   UpdateMainChart();
                   }
                else
                   TCanvas3DRectangle(selectrange_xcstart, selectrange_xcend);
                }
             }
    }

 selectrange=false;
}
//---------------------------------------------------------------------------

void __fastcall TfMain::FormShow(TObject *Sender)
{
 fAbout->ShowModal();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::bEditChannellistClick(TObject *Sender)
{
 fEditChannellist->ShowModal();
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   get_next_string
// Purpose:    gets the next delimited token in a string
// **************************************************************************
void TfMain::get_next_string(char *buf, int *start_idx, char *dest)
{
int     idx;

idx=*start_idx;
while ((buf[idx] != ',') && (buf[idx] != ' ') && (buf[idx] != 0x0D) && (buf[idx] != 0x0A) && (buf[idx] != 0x00) && (idx-*start_idx < 255))
 idx++;

strncpy(dest, (const char *)&buf[*start_idx], idx-*start_idx);
dest[idx-*start_idx]=0x00;

while (((buf[idx] == ',') || (buf[idx] == ' ')) && (idx-*start_idx < 2048))
 idx++;

*start_idx=idx;
}


// **************************************************************************
// Function:   bGetCHDescFileClick
// Purpose:    reads a channel definition file (*.ch)
// **************************************************************************
void __fastcall TfMain::bGetCHDescFileClick(TObject *Sender)
{
AnsiString      filename, line, ansielement;
FILE            *fp;
char            buf[255], element[255];
int             idx, channelnum;

 if (OpenDialog1->Execute())
    filename=OpenDialog1->FileName;
 else
    return;

 fp=fopen(filename.c_str(), "rb");
 if (!fp)
    {
    Application->MessageBox("Could not open channel definition file", "Error ...", MB_OK);
    return;
    }

 fgets(buf, 255, fp);
 line=(AnsiString)buf;
 if (line.SetLength(10) != "[channels]")
    {
    Application->MessageBox("Wrong format. The file does not contain [channels] as its first line", "Error ...", MB_OK);
    fclose(fp);
    return;
    }

 while (!feof(fp))
  {
  fgets(buf, 255, fp);
  idx=0;
  get_next_string(buf, &idx, element);
  ansielement=(AnsiString)element;
  if (((AnsiString)element).SetLength(2) != "//")         // it is not a remark line
     {
     try    // we might have an exception in ToInt()
      {
      channelnum=ansielement.ToInt()-1;
      if ((channelnum >= 0) && (channelnum < MAX_CHANNELS))
         {
         get_next_string(buf, &idx, element);
         channellist[channelnum].name=element;
         get_next_string(buf, &idx, element);
         ansielement=(AnsiString)element;
         channellist[channelnum].displayposition=ansielement.ToInt()-1;
         get_next_string(buf, &idx, element);
         ansielement=(AnsiString)element;
         channellist[channelnum].chanreftype=REFTYPE_NORMAL;            // per default, we have normal
         if (ansielement.LowerCase() == "lglap")
            channellist[channelnum].chanreftype=REFTYPE_LGLAP;
         if (ansielement.LowerCase() == "car")
            channellist[channelnum].chanreftype=REFTYPE_CAR;
         }
      }
     catch ( ... )      // we just don't do anything on a 'corrupted' line
      {
      }
     }
  }

 if (fp) fclose(fp);
 UpdateMainChart();
}
//---------------------------------------------------------------------------


void __fastcall TfMain::bPrintClick(TObject *Sender)
{
// int size;

 // size=MainChart->LeftAxis->LabelsFont->Size;
 // size=GetDeviceCaps(Printer()->Handle, LOGPIXELSX);
 // dpi_x=GetDeviceCaps(my_printer->Handle, LOGPIXELSX);
 // dpi_y=GetDeviceCaps(my_printer->Handle, LOGPIXELSY);
 // dpiscalex=dpi_x/300;
 // dpiscaley=dpi_y/300;

 // MainChartPreviewer->Execute();
}
//---------------------------------------------------------------------------

void __fastcall TfMain::bCopy2ClipboardClick(TObject *Sender)
{
 MainChart->Color=clWhite;
 // MainChart->SaveToMetafile("c:\\temp\\test.wmf");
 // MainChart->SaveToMetafileEnh("c:\\temp\\test2.emf");
 MainChart->CopyToClipboardMetafile(false);
 MainChart->Color=clBtnFace;
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   MainChartMouseDown
// Purpose:    This procedure is automatically being invoked when the
//             User presses the mouse button over the main chart
// **************************************************************************
void __fastcall TfMain::MainChartMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
long    cur_sample, xc;
float   factor;

 if ((initialized) && (Button == mbLeft))
    {
    // check if within chart area
    if ((X > MainChart->ChartRect.Left) && (X < MainChart->ChartRect.Right))
       if ((Y > MainChart->ChartRect.Top) && (Y < MainChart->ChartRect.Bottom))
          {
          xc=X-MainChart->ChartRect.Left;
          factor=(float)xc/(float)(MainChart->ChartRect.Right-MainChart->ChartRect.Left);
          cur_sample=(int)((float)start_sample+factor*(float)display_samples);
          selectrange=true;
          selectrange_samplestart=cur_sample;
          selectrange_xcstart=X;
          selectrange_xcend=X;
          }
    }
}
//---------------------------------------------------------------------------


// **************************************************************************
// Function:   TCanvas3DRectangle
// Purpose:    Alright, I know this looks ridiculous
//             This procedure draws a rectangle on the main chart, inverting
//             the background. This is used when the user selects a range that
//             he wants to change the state of.
//             Sadly, the TeeChart's TCanvas3D object does not contain the
//             CopyMode property and therefore I can't do a FillRect and
//             have to use multiple MoveTos and LineTos instead.
// **************************************************************************
void TfMain::TCanvas3DRectangle(int x1, int x2)
{
int     x, i;

 if (x2 < x1)
    {
    x=x2;
    x2=x1;
    x1=x;
    }

 MainChart->Canvas->Pen->Mode=pmNot;
 for (i=x1; i<=x2; i++)
  {
  MainChart->Canvas->MoveTo(i, MainChart->ChartRect.Top+1);
  MainChart->Canvas->LineTo(i, MainChart->ChartRect.Bottom-1);
  }
}



// **************************************************************************
// Function:   MainChartMouseDown
// Purpose:    This procedure is automatically being invoked when the
//             User moves the mouse over the main chart
// **************************************************************************
void __fastcall TfMain::MainChartMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
long    xc;

 if (initialized)
    {
    // check if within chart area
    if ((X > MainChart->ChartRect.Left) && (X < MainChart->ChartRect.Right))
       if ((Y > MainChart->ChartRect.Top) && (Y < MainChart->ChartRect.Bottom))
          if (selectrange == true)
             {
             xc=X-MainChart->ChartRect.Left;
             if (selectrange_xcend != selectrange_xcstart)
                TCanvas3DRectangle(selectrange_xcstart, selectrange_xcend);
             TCanvas3DRectangle(selectrange_xcstart, X);
             selectrange_xcend=X;
             }
    }
}
//---------------------------------------------------------------------------



void __fastcall TfMain::bSave2DiskClick(TObject *Sender)
{
TDateTime       cur_time;
int     t, i, cur_channel;
long    msec, sec, min, hour;
float   car, reference;
FILE    *fp;

 fSave2Disk->ShowModal();
 if (!fSave2Disk->save) return;
 fp=fopen(fSave2Disk->filename.c_str(), "wb");
 if (!fp)
    {
    Application->MessageBox("Could not open output file", "Error", MB_OK);
    return;
    }

 // the first row is the channel names
 fprintf(fp, "- ");
 for (i=0; i<display_channels; i++)
  {
  cur_channel=FetchChannelNumber(i+start_channel);
  fprintf(fp, "%s ", channellist[cur_channel].name.c_str());
  }
 fprintf(fp, "\r\n");

 // now output all the columns, i.e., all the channels
 for (t=0; t<display_samples; t++)
  {
  // the first 'sample' is the time
  msec=(long)((((float)t+(float)start_sample)/(float)sample_freq)*1000);
  hour=msec/3600000;
  min=(msec-hour*3600000)/60000;
  sec=(msec-hour*3600000-min*60000)/1000;
  fprintf(fp, "%02d:%02d:%02d ", hour, min, sec);
  for (i=0; i<display_channels; i++)
   {
   cur_channel=FetchChannelNumber(i+start_channel);
   if (i+start_channel < num_channels)
      fprintf(fp, "%d ", bci2000data->ReadValue(cur_channel, t+start_sample));
   else
      fprintf(fp, "%d ", i);
   }
  fprintf(fp, "\r\n");
  cMainGauge->Progress = ( 100 * ( t + 1 ) ) / display_samples;
  }

 cMainGauge->Progress=0;
 fclose(fp);
}
//---------------------------------------------------------------------------


void __fastcall TfMain::cStateListBoxClickCheck(TObject *Sender)
{
 // cStateListBox->Items->Strings[cStateListBox->ItemIndex]="papa";
}
//---------------------------------------------------------------------------


void __fastcall TfMain::cCalibrationCheckBoxClick(TObject *Sender)
{
  ReadCalibrationCheckBox();
  bGoButton->Click();    
}
//---------------------------------------------------------------------------

