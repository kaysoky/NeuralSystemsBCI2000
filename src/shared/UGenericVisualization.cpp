////////////////////////////////////////////////////////////////////////////////
//
// File:    UGenericVisualization.cpp
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
//          Separated VISUAL and VISCFGLIST into a file belonging to
//          the operator module.
//
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "UGenericVisualization.h"

#include "UCoreMessage.h"
#include "UCoreComm.h"
#include "UEnvironment.h"
#include "UVisConfig.h"
#include "UGenericSignal.h"

#include <math.h>
#include <stdio.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

GenericVisualization::GenericVisualization()
: signal( NULL ),
  new_samples( -1 ),
  sourceID( -1 ),
  vis_type( VISTYPE_GRAPH ),
  datatype( DATATYPE_FLOAT )
{
 // paramlist->AddParameter2List( "Source intlist VisChList= 5 11 23 1 2 3 11 1 64  // list of channels to visualize" );
}

GenericVisualization::GenericVisualization( BYTE inSourceID, BYTE inVisType )
: signal( NULL ),
  new_samples( -1 ),
  sourceID( inSourceID ),
  vis_type( inVisType ),
  datatype( DATATYPE_FLOAT )
{
}

GenericVisualization::~GenericVisualization()
{
 delete signal;
}


void GenericVisualization::SetSourceID(BYTE my_sourceID)
{
 sourceID=my_sourceID;
}


BYTE GenericVisualization::GetSourceID() const
{
 return(sourceID);
}


void GenericVisualization::SetDataType(BYTE my_datatype)
{
 datatype=my_datatype;
}


BYTE GenericVisualization::GetDataType() const
{
 return(datatype);
}


void GenericVisualization::SetVisualizationType(BYTE my_vistype)
{
 vis_type=my_vistype;
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
CORECOMM* corecomm = Environment::Corecomm;
// ... temporary glue code

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
int     ch, count;
size_t  samp;
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
CORECOMM* corecomm = Environment::Corecomm;
// ... temporary glue code

TWinSocketStream        *pStream;
unsigned short  *short_dataptr;
COREMESSAGE     *coremessage;
short   *valueptr;
BYTE    *dataptr;

 // error and consistency checking
 if (my_signal->Channels() > 255)      return(false);       // Channels > 255
 if (my_signal->MaxElements() > 65535) return(false);       // samples per channel > 65535
 if (!corecomm)                      return(false);       // core communication not defined
 if (!corecomm->Connected())         return(false);       // no connection to the core module
 if ((long)my_signal->Channels()*(long)my_signal->MaxElements()+9 > COREMESSAGE_MAXBUFFER) return(false);     // data too big for a coremessage

 pStream=new TWinSocketStream(corecomm->GetSocket(), 5000);

 coremessage=new COREMESSAGE;
 coremessage->SetDescriptor(COREMSG_DATA);
 coremessage->SetSuppDescriptor( vis_type );
 coremessage->SetLength(sizeof(unsigned short)*(unsigned short)my_signal->Channels()*(unsigned short)my_signal->MaxElements()+5);         // set the length of the coremessage

 dataptr=(BYTE *)coremessage->GetBufPtr();
 // construct the header of the core message
 dataptr[0]=sourceID;                   // write the source ID into the coremessage
 dataptr[1]=DATATYPE_INTEGER;           // write the datatype into the coremessage
 dataptr[2]=(BYTE)my_signal->Channels();// write the # of channels into the coremessage
 short_dataptr=(unsigned short *)&dataptr[3];
 *short_dataptr=(unsigned short)my_signal->MaxElements(); // write the # of samples into the coremessage
 // write the actual data into the coremessage
 for (size_t t=0; t<my_signal->Channels(); t++)
  for (size_t s=0; s<my_signal->MaxElements(); s++)
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
CORECOMM* corecomm = Environment::Corecomm;
// ... temporary glue code

TWinSocketStream        *pStream;
unsigned short  *short_dataptr;
COREMESSAGE     *coremessage;
BYTE    *dataptr, *dataptr2;
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
 coremessage->SetSuppDescriptor( vis_type );
 coremessage->SetLength(3*(unsigned short)my_signal->Channels()*(unsigned short)my_signal->MaxElements()+5);         // set the length of the coremessage

 dataptr=(BYTE *)coremessage->GetBufPtr();
 // construct the header of the core message
 dataptr[0]=sourceID;                   // write the source ID into the coremessage
 dataptr[1]=DATATYPE_FLOAT;             // write the datatype into the coremessage
 dataptr[2]=(BYTE)my_signal->Channels();  // write the # of channels into the coremessage
 short_dataptr=(unsigned short *)&dataptr[3];
 *short_dataptr=(unsigned short)my_signal->MaxElements(); // write the # of samples into the coremessage
 // write the actual data into the coremessage
 for (size_t t=0; t<my_signal->Channels(); t++)
  for (size_t s=0; s<my_signal->MaxElements(); s++)
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

bool GenericVisualization::SendCfg2Operator( BYTE sourceID, BYTE cfgID, int cfgValue )
{
  return SendCfg2Operator( sourceID, cfgID, AnsiString( cfgValue ).c_str() );
}

bool GenericVisualization::SendCfg2Operator(BYTE sourceID, BYTE cfgID, const char *cfgString)
{
CORECOMM* corecomm = Environment::Corecomm;
// ... temporary glue code

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

#if 0 // Went into operator code (class VISUAL).
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
       delete signal;
       signal=new GenericSignal((unsigned short)channels, (int)samples);
       unsigned short* data = ( unsigned short* )&buffer[ 5 ];
       for (i=0; i<channels; i++)
        for( int j = 0; j < samples; ++j )
          signal->SetValue( i, j, data[ j * channels + i ] );   // set the values in the signal to the data
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
#endif
}

