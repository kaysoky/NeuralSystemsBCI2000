////////////////////////////////////////////////////////////////////////////////
//
// File: UGenericSignal.cpp
//
// Description: This file declares a SignalProperties base class and a BasicSignal
//   class template deriving from it with the signal's numerical type as the
//   template argument.
//   Two classes, GenericSignal and GenericIntSignal, are derived from a float
//   and int instantiation of this template. With a compatibility flag set
//   (SIGNAL_BACK_COMPAT) existing code should compile without changes.
//   For the future, the following name transitions might be considered:
//     BasicSignal --> GenericSignal
//     GenericSignal --> FloatSignal
//     GenericIntSignal --> IntSignal
//   as the latter two don't have anything generic about them any more.
//
// Changes: June 28, 2002, juergen.mellinger@uni-tuebingen.de
//          - Rewrote classes from scratch but kept old class interface.
//
////////////////////////////////////////////////////////////////////////////////
#pragma hdrstop

#include "UGenericSignal.h"

bool
SignalProperties::SetNumElements( size_t inChannel, size_t inElements )
{
  bool elementsTooBig = ( inElements > maxElements );
  if( elementsTooBig )
    elements.at( inChannel ) = maxElements;
  else
    elements.at( inChannel ) = inElements;
  return elementsTooBig;
}

bool
SignalProperties::operator>=( const SignalProperties& sp ) const
{
  if( elements.size() < sp.elements.size() )
    return false;
  for( size_t i = 0; i < sp.elements.size(); ++i )
    if( elements[ i ] < sp.elements[ i ] )
      return false;
  return true;
}

bool
SignalProperties::operator<=( const SignalProperties& sp ) const
{
  if( sp.elements.size() < elements.size() )
    return false;
  for( size_t i = 0; i < elements.size(); ++i )
    if( sp.elements[ i ] < elements[ i ] )
      return false;
  return true;
}

void
GenericSignal::SetChannel( const short *inSource, size_t inChannel )
{
  for( size_t i = 0; i < elements.at( inChannel ); ++i )
    Value[ inChannel ][ i ] = ( float )inSource[ i ];
}

const GenericSignal&
GenericSignal::operator=( const GenericIntSignal& inRHS )
{
  SetProperties( inRHS );
  for( size_t i = 0; i < inRHS.Channels(); ++i )
    for( size_t j = 0; j < inRHS.GetNumElements( i ); ++j )
      SetValue( i, j, ( float )inRHS.GetValue( i, j ) );
  return *this;
}

#if 0
// **************************************************************************
// Function:   GenericSignal
// Purpose:    This is the constructor for the GenericSignal class
//             It allocates memory for the two dimensional array Value
// Parameters: NewChannels - number of channels in the signal
//             NewMaxElements - maximum number of elements per channel
// Returns:    N/A
// **************************************************************************
GenericSignal::GenericSignal(unsigned short NewChannels, int NewMaxElements)
{
int     i;

 Channels=NewChannels;
 MaxElements=NewMaxElements;

 if (Channels <= 0) return;

 // allocate for the Elements array
 Elements=new int[NewChannels];

 // allocate memory for the two dimensional array holding the actual data
 Value=new float *[NewChannels];
 for (i=0; i<NewChannels; i++)
  {
  Value[i]=new float[NewMaxElements];
  Elements[i]=NewMaxElements;
  }
}

// **************************************************************************
// Function:   GenericSignal
// Purpose:    This is the constructor for the GenericSignal class
//             It allocates memory for the two dimensional array Value
//             and makes an exact copy of the provided GenericIntSignal
//             (which stores values in integers, rather than in floats
// Parameters: pointer to GenericIntSignal that will be copied
// Returns:    N/A
// **************************************************************************
GenericSignal::GenericSignal(GenericIntSignal *intsig)
{
int     x1, x2;
int     i;

 Channels=intsig->Channels;
 MaxElements=intsig->MaxElements;

 if (Channels <= 0) return;

 // allocate for the Elements array
 Elements=new int[Channels];

 // allocate memory for the two dimensional array holding the actual data
 Value=new float *[Channels];
 for (i=0; i<Channels; i++)
  {
  Value[i]=new float[intsig->GetElements(i)];
  Elements[i]=intsig->GetElements(i);
  }

 for (x1=0; x1<Channels; x1++)
  for (x2=0; x2<intsig->GetElements(x1); x2++)
   SetValue(x1, x2, (float)intsig->GetValue(x1, x2));
}


// **************************************************************************
// Function:   ~GenericSignal
// Purpose:    This is the destructor for the GenericSignal class
//             It frees the memory that held the signal
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
GenericSignal::~GenericSignal()
{
int     i;

 if (Channels > 0)
    {
    for (i=0; i<Channels; i++)
     if (Value[i]) delete [] Value[i];
    if (Value) delete [] Value;
    if (Elements) delete [] Elements;
    Value=NULL;
    Elements=NULL;
    }

 Channels=0;
}


// **************************************************************************
// Function:   SetElements
// Purpose:    This routine sets the number of elements in one particular
//             channel. It does not change this number, if the channel number
//             is out of range, or NewElements is bigger than MaxElements
// Parameters: Channel - channel number for the new number of elements
//             NewElements - new number of elements for this particular channel
// Returns:    true - if successful
//             false - either Channel is bigger than the total number of channels
//                     or NewElements is bigger than the max. number of elements
// **************************************************************************
bool GenericSignal::SetElements(unsigned short Channel, int NewElements)
{
 if (Channel < Channels)
    if (NewElements > MaxElements)
       {
       Elements[Channel]=NewElements;
       return(true);
       }

return(false);
}


// **************************************************************************
// Function:   GetElements
// Purpose:    This routine returns the number of elements in this channel
// Parameters: Channel - channel number for the requested number of elements
// Returns:    the number of elements in this channel, or
//             0, if the specified channel number is out of range
// **************************************************************************
int GenericSignal::GetElements(unsigned short Channel)
{
 if (Channel < Channels)
    return(Elements[Channel]);
 else
    return(0);
}


// **************************************************************************
// Function:   SetChannel
// Purpose:    This routine sets the actual values for a specified channel
//             to the values pointed to by source
//             Elements[channel] determines the number of elements copied
// Parameters: channel - destination channel number
//             source - pointer to the source data
// Returns:    true - if successful
//             false - channel number out of range
// **************************************************************************
bool  GenericSignal::SetChannel(float *source, int channel)
{
int     i;
float   *samples;

 if ((channel >= 0) && (channel < Channels))
    {
    samples=GetChannel(channel);
    for (i=0; i<Elements[channel]; i++)
     samples[i]=source[i];
    return(true);
    }
 else
    return(false);
}

// **************************************************************************
// Function:   SetChannel
// Purpose:    This routine sets the actual values for a specified channel
//             to the values pointed to by source
//             Elements[channel] determines the number of elements copied
//             This actually copies short values into the float array
// Parameters: channel - destination channel number
//             source - pointer to the source data
// Returns:    true - if successful
//             false - channel number out of range
// **************************************************************************
void
GenericSignal::SetChannel( const short *inSource, size_t inChannel )
{
  for( size_t i = 0; i < elements.at( inChannel ); ++i )
    Value[ inChannel ][ i ] = ( float )inSource[ i ];
}

const GenericSignal&
GenericSignal::operator=( const GenericIntSignal& inRHS )
{
  SetProperties( inRHS );
  for( size_t i = 0; i < inRHS.Channels(); ++i )
    for( size_t j = 0; j < inRHS.GetNumElements( i ); ++j )
      SetValue( i, j, ( float )inRHS.GetValue( i, j ) );
  return *this;
}

// **************************************************************************
// Function:   GetChannel
// Purpose:    This routine returns a pointer to the values for a specified channel
// Parameters: channel - requested channel number
// Returns:    pointer to the values for this channel - if successful
//             NULL, if channel number is out of range
// **************************************************************************
float *GenericSignal::GetChannel(int channel)
{
 if ((channel >= 0) && (channel < Channels))
    return(Value[channel]);
 else
    return(NULL);
}


// **************************************************************************
// Function:   GetValue
// Purpose:    Retrieves a single value for a given channel and element
// Parameters: Channel - channel for requested element
//             Element - number of the element
// Returns:    value for this specific Channel/Element combination, or
//             0, if either channel number or number of elements is out of range
// **************************************************************************
float GenericSignal::GetValue(int Channel, int Element)
{
 if ((Channel >= 0) && (Channel < Channels))
    if ((Element >= 0) && (Element < Elements[Channel]))
       return(Value[Channel][Element]);

 return(0);
}


// **************************************************************************
// Function:   SetValue
// Purpose:    Sets a single value for a given channel and element
// Parameters: Channel - channel for requested element
//             Element - number of the element
//             NewValue - new value for this Channel/Element combination
// Returns:    true - value changed successfully
//             false - either channel number or Element number is out of range
// **************************************************************************
bool GenericSignal::SetValue(int Channel, int Element, float NewValue)
{
 if ((Channel >= 0) && (Channel < Channels))
    if ((Element >= 0) && (Element < Elements[Channel]))
       {
       Value[Channel][Element]=NewValue;
       return(true);
       }

 return(false);
}


// **************************************************************************
// Function:   GenericIntSignal
// Purpose:    This is the constructor for the GenericIntSignal class
//             It allocates memory for the two dimensional array Value
// Parameters: NewChannels - number of channels in the signal
//             NewMaxElements - maximum number of elements per channel
// Returns:    N/A
// **************************************************************************
GenericIntSignal::GenericIntSignal(unsigned short NewChannels, int NewMaxElements)
{
int     i;

 Channels=NewChannels;
 MaxElements=NewMaxElements;

 if (Channels <= 0) return;

 // allocate for the Elements array
 Elements=new int[NewChannels];

 // allocate memory for the two dimensional array holding the actual data
 Value=new short *[NewChannels];
 for (i=0; i<NewChannels; i++)
  {
  Value[i]=new short[NewMaxElements];
  Elements[i]=NewMaxElements;
  }
}


/*
// **************************************************************************
// Function:   GenericIntSignal
// Purpose:    This is the constructor for the GenericIntSignal class
//             It allocates memory for the two dimensional array Value
//             and makes an exact copy of the provided GenericSignal
//             (which stores values in floats, rather than in shorts
// Parameters: pointer to GenericSignal that will be copied
// Returns:    N/A
// **************************************************************************
GenericIntSignal::GenericIntSignal(GenericSignal *sig)
{
int     x1, x2;
int     i;

 Channels=intsig->Channels;
 MaxElements=intsig->MaxElements;

 if (Channels <= 0) return;

 // allocate for the Elements array
 Elements=new int[Channels];

 // allocate memory for the two dimensional array holding the actual data
 Value=new float *[Channels];
 for (i=0; i<Channels; i++)
  {
  Value[i]=new float[intsig->GetElements(i)];
  Elements[i]=sig->GetElements(i);
  }

 for (x1=0; x1<Channels; x1++)
  for (x2=0; x2<intsig->GetElements(x1); x2++)
   SetValue(x1, x2, (short)sig->GetValue(x1, x2));
}
*/

// **************************************************************************
// Function:   ~GenericIntSignal
// Purpose:    This is the destructor for the GenericIntSignal class
//             It frees the memory that held the signal
// Parameters: N/A
// Returns:    N/A
// **************************************************************************
GenericIntSignal::~GenericIntSignal()
{
int     i;

 if (Channels > 0)
    {
    for (i=0; i<Channels; i++)
     if (Value[i]) delete [] Value[i];
    if (Value) delete [] Value;
    if (Elements) delete [] Elements;
    Value=NULL;
    Elements=NULL;
    }

 Channels=0;
}


// **************************************************************************
// Function:   SetElements
// Purpose:    This routine sets the number of elements in one particular
//             channel. It does not change this number, if the channel number
//             is out of range, or NewElements is bigger than MaxElements
// Parameters: Channel - channel number for the new number of elements
//             NewElements - new number of elements for this particular channel
// Returns:    true - if successful
//             false - either Channel is bigger than the total number of channels
//                     or NewElements is bigger than the max. number of elements
// **************************************************************************
bool GenericIntSignal::SetElements(unsigned short Channel, int NewElements)
{
 if (Channel < Channels)
    if (NewElements > MaxElements)
       {
       Elements[Channel]=NewElements;
       return(true);
       }

return(false);
}


// **************************************************************************
// Function:   GetElements
// Purpose:    This routine returns the number of elements in this channel
// Parameters: Channel - channel number for the requested number of elements
// Returns:    the number of elements in this channel, or
//             0, if the specified channel number is out of range
// **************************************************************************
int GenericIntSignal::GetElements(unsigned short Channel)
{
 if (Channel < Channels)
    return(Elements[Channel]);
 else
    return(0);
}


// **************************************************************************
// Function:   SetChannel
// Purpose:    This routine sets the actual values for a specified channel
//             to the values pointed to by source
//             Elements[channel] determines the number of elements copied
// Parameters: channel - destination channel number
//             source - pointer to the source data
// Returns:    true - if successful
//             false - channel number out of range
// **************************************************************************
bool  GenericIntSignal::SetChannel(short *source, int channel)
{
int     i;
short   *samples;

 if ((channel >= 0) && (channel < Channels))
    {
    samples=GetChannel(channel);
    for (i=0; i<Elements[channel]; i++)
     samples[i]=source[i];
    return(true);
    }
 else
    return(false);
}


// **************************************************************************
// Function:   GetChannel
// Purpose:    This routine returns a pointer to the values for a specified channel
// Parameters: channel - requested channel number
// Returns:    pointer to the values for this channel - if successful
//             NULL, if channel number is out of range
// **************************************************************************
short *GenericIntSignal::GetChannel(int channel)
{
 if ((channel >= 0) && (channel < Channels))
    return(Value[channel]);
 else
    return(NULL);
}


// **************************************************************************
// Function:   GetValue
// Purpose:    Retrieves a single value for a given channel and element
// Parameters: Channel - channel for requested element
//             Element - number of the element
// Returns:    value for this specific Channel/Element combination, or
//             0, if either channel number or number of elements is out of range
// **************************************************************************
short GenericIntSignal::GetValue(int Channel, int Element)
{
 if ((Channel >= 0) && (Channel < Channels))
    if ((Element >= 0) && (Element < Elements[Channel]))
       return(Value[Channel][Element]);

 return(0);
}


// **************************************************************************
// Function:   SetValue
// Purpose:    Sets a single value for a given channel and element
// Parameters: Channel - channel for requested element
//             Element - number of the element
//             NewValue - new value for this Channel/Element combination
// Returns:    true - value changed successfully
//             false - either channel number or Element number is out of range
// **************************************************************************
bool GenericIntSignal::SetValue(int Channel, int Element, short NewValue)
{
 if ((Channel >= 0) && (Channel < Channels))
    if ((Element >= 0) && (Element < Elements[Channel]))
       {
       Value[Channel][Element]=NewValue;
       return(true);
       }

 return(false);
}
#endif

