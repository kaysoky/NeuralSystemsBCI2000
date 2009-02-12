/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <stdio.h>

#include "CalibGenHelp.h"
#include "CalibGenMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "CSPIN"
#pragma resource "*.dfm"
TfMain *fMain;

CALIBCONFIG     calibconfig;
SOURCECONFIG    sourceconfig;

//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TfMain::bSelectInputClick(TObject *Sender)
{
 OpenDialog->Options >> ofAllowMultiSelect
                     << ofFileMustExist;
 if (OpenDialog->Execute())
    {
    sourceconfig.filename=OpenDialog->FileName;
    tFilename->Caption=sourceconfig.filename;
    ReadHeader(&sourceconfig);
    if (sourceconfig.valid)
       {
       tNumChannelsInput->Caption=AnsiString(sourceconfig.channels);
       bSelectCalib->Enabled=true;
       bSelectInput->Enabled=false;
       sourceconfig.targetvalue=atof(eTargetVal->Text.c_str());
       }
    else
       ResetGUI();
    }
 else
    ResetGUI();
}
//---------------------------------------------------------------------------


void TfMain::ResetGUI()
{
 bSelectInput->Enabled=true;
 bSelectCalib->Enabled=false;
 bGo->Enabled=false;

 tFilename->Caption="N/A";
 tNumChannelsInput->Caption="N/A";
 tCalibFile->Caption="N/A";

 bGo->Visible=true;
}


long TfMain::filesize(FILE *stream)
{
   long curpos, length;

   curpos = ftell(stream);
   fseek(stream, 0L, SEEK_END);
   length = ftell(stream);
   fseek(stream, curpos, SEEK_SET);
   return length;

}


void TfMain::ReadHeader(SOURCECONFIG *sourceconfig)
{
FILE    *fp;
char    buf[255];
short   num_channels, dummy, sample_freq;

 sourceconfig->valid=false;
 fp=fopen(sourceconfig->filename.c_str(), "rb");
 if (!fp)
  {
  Application->MessageBox("Could not open source file", "Error", MB_OK);
  return;
  }

 sourceconfig->filesize=filesize(fp);

 fseek(fp, 0, SEEK_SET);
 fread(buf, 9, 1, fp);
 buf[9]=0;
 // is it BCI2000 file format ?
 if (stricmp(buf, "HeaderLen") == 0)
    {
    sourceconfig->fileformat=FILEFORMAT_BCI2000;
    ReadBCI2000Header(sourceconfig);
    }
 else
    {
    sourceconfig->fileformat=FILEFORMAT_ALBANYOLD;
    fseek(fp, 0, SEEK_SET);
    fread(&num_channels, 2, 1, fp);
    fread(&dummy, 2, 1, fp);
    fread(&sample_freq, 2, 1, fp);
    sourceconfig->channels=num_channels;
    sourceconfig->sample_freq=sample_freq;
    sourceconfig->headerlength=6;
    sourceconfig->statevectorlength=2;
    }

 sourceconfig->valid=true;
 fclose(fp);
}


void TfMain::ReadBCI2000Header(SOURCECONFIG *sourceconfig)
{
FILE    *fp;
char    buf[50000], element[255];
int     idx;
AnsiString      header;

 fp=fopen(sourceconfig->filename.c_str(), "rb");
 if (!fp) return;

 fgets(buf, 255, fp);
 idx=0;
 get_next_string(buf, &idx, element);
 get_next_string(buf, &idx, element);           // headerlength
 sourceconfig->headerlength=atoi(element);
 get_next_string(buf, &idx, element);
 get_next_string(buf, &idx, element);
 get_next_string(buf, &idx, element);
 get_next_string(buf, &idx, element);           // state vector length
 sourceconfig->statevectorlength=atoi(element);

 fseek(fp, 0, SEEK_SET);
 fread(buf, sourceconfig->headerlength, 1, fp);
 header=buf;
 idx=header.AnsiPos("SamplingRate");
 if (idx == 0)
    sourceconfig->sample_freq=128;
 else
    {
    idx--;
    get_next_string(buf, &idx, element);
    get_next_string(buf, &idx, element);           // sample frequency
    sourceconfig->sample_freq=atoi(element);
    }

 idx=header.AnsiPos("SourceCh");
 if (idx == 0)
    sourceconfig->channels=64;
 else
    {
    idx--;
    get_next_string(buf, &idx, element);
    get_next_string(buf, &idx, element);           // sample frequency
    sourceconfig->channels=atoi(element);
    }

 fclose(fp);
}


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
void __fastcall TfMain::bSelectCalibClick(TObject *Sender)
{
char buf[255];

 if (SaveDialog->Execute())
    {
    tCalibFile->Caption=SaveDialog->FileName;
    calibconfig.filename=SaveDialog->FileName;
    calibconfig.target=(float)atof(eTargetVal->Text.c_str());
    bGo->Enabled=true;
    bSelectCalib->Enabled=false;
    }
}
//---------------------------------------------------------------------------


void __fastcall TfMain::bGoClick(TObject *Sender)
{
FILE    *fpin, *fpout;
char    buf[50000];
short   cur_val[MAX_CHANNELS], old_val[MAX_CHANNELS];
int     cur_long_val, ch;
long    sample;
float   min_val[MAX_CHANNELS], max_val[MAX_CHANNELS];
int     num_minval[MAX_CHANNELS], num_maxval[MAX_CHANNELS];
bool    pos_slope[MAX_CHANNELS], neg_slope[MAX_CHANNELS];

 fpin=fopen(sourceconfig.filename.c_str(), "rb");
 fpout=fopen(calibconfig.filename.c_str(), "wb");
 if ((!fpin) || (!fpout))
  {
  Application->MessageBox("Could not open source or destination file", "Error", MB_OK);
  if (fpin)  fclose(fpin);
  if (fpout) fclose(fpout);
  ResetGUI();
  return;
  }

 // read and copy header
 if (sourceconfig.headerlength > 50000)
    Application->MessageBox("Header in raw file > 50000", "Error", MB_OK);
 fread(buf, sourceconfig.headerlength, 1, fpin);

 // read first sample and initialize variables
 for (ch=0; ch<sourceconfig.channels; ch++)
  {
  fread(&old_val[ch], 2, 1, fpin);
  min_val[ch]=0;
  max_val[ch]=0;
  num_minval[ch]=0;
  num_maxval[ch]=0;
  pos_slope[ch]=false;
  neg_slope[ch]=false;
  }
 fread(buf, sourceconfig.statevectorlength, 1, fpin);

 // read first 5000 samples and extract min and max values
 // min and max are local minimum and maximum, respectively
 sample=0;
 while ((!feof(fpin)) && (sample < 5000))
  {
  // read all the channels, scale them, and write them
  for (ch=0; ch<sourceconfig.channels; ch++)
   {
   fread(&cur_val[ch], 2, 1, fpin);
   // positive slope ?
   if (cur_val[ch] > old_val[ch])
      {
      if (neg_slope[ch]) // did we have a negative slope before ? -> must be local minimum
         {
         min_val[ch] += (float)old_val[ch];
         num_minval[ch]++;
         neg_slope[ch]=false;
         }
      pos_slope[ch]=true;
      }
   // negative slope ?
   if (cur_val[ch] < old_val[ch])
      {
      if (pos_slope[ch]) // did we have a positive slope before ? -> must be local maximum
         {
         max_val[ch] += (float)old_val[ch];
         num_maxval[ch]++;
         pos_slope[ch]=false;
         }
      neg_slope[ch]=true;
      }
   old_val[ch]=cur_val[ch];
   }
  fread(buf, sourceconfig.statevectorlength, 1, fpin);
  sample++;
  }

 // calculate offset and gain for each channel
 for (ch=0; ch<sourceconfig.channels; ch++)
  {
  calibconfig.offset[ch]=(short)(((int)(min_val[ch]/(float)num_minval[ch])+(int)(max_val[ch]/(float)num_maxval[ch]))/2); // in the calibration signal, the min should equal -max
  calibconfig.gain[ch]=((float)calibconfig.target/2)/(max_val[ch]/(float)num_maxval[ch]-calibconfig.offset[ch]);
  // fprintf(fpout, "%d %.5f\r\n", calibconfig.offset[ch], calibconfig.gain[ch]);
  }

 // write parameter file
 // first the offset
 fprintf(fpout, "Filtering floatlist SourceChOffset= %d ", sourceconfig.channels);
 for (ch=0; ch<sourceconfig.channels; ch++)
   fprintf(fpout, "%d ", calibconfig.offset[ch]);
 fprintf(fpout, "0 -500 500 // offset for channels in A/D units\r\n");

 // and then the gain
 fprintf(fpout, "Filtering floatlist SourceChGain= %d ", sourceconfig.channels);
//jm num_ch=0;
 for (ch=0; ch<sourceconfig.channels; ch++)
   fprintf(fpout, "%.5f ", calibconfig.gain[ch]);
 fprintf(fpout, "0.033 -500 500 // gain for each channel (A/D units -> muV)\r\n");

 // Filtering floatlist SourceChOffset= 16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -500 500 // offset for channels in A/D units
 // Filtering floatlist SourceChGain= 16 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 0.033 -500 500 // gain for each channel (A/D units -> muV)

 if (fpin)  fclose(fpin);
 if (fpout) fclose(fpout);
 ResetGUI();
}
//---------------------------------------------------------------------------


void __fastcall TfMain::bHelpClick(TObject *Sender)
{
 fHelp->Show();
}
//---------------------------------------------------------------------------


