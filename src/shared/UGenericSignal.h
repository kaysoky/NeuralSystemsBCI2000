//---------------------------------------------------------------------------

#ifndef UGenericSignalH
#define UGenericSignalH

class GenericIntSignal
{
private:
       int  *Elements;
public:
       GenericIntSignal(unsigned short NewChannels, int NewMaxElements);
       // GenericIntSignal(GenericSignal *sig);
       ~GenericIntSignal();
       short **Value;
       int   Channels;
       int   MaxElements;
       bool  SetElements(unsigned short Channel, int NewElements);
       int   GetElements(unsigned short Channel);
       bool  SetChannel(short *source, int channel);
       short *GetChannel(int channel);
       short GetValue(int Channel, int Element);
       bool  SetValue(int Channel, int Element, short Value);
};


class GenericSignal
{
private:
       int  *Elements;
public:
       GenericSignal(unsigned short NewChannels, int NewMaxElements);
       GenericSignal::GenericSignal(GenericIntSignal *intsig);
       ~GenericSignal();
       float **Value;
       int   Channels;
       int   MaxElements;
       bool  SetElements(unsigned short Channel, int NewElements);
       int   GetElements(unsigned short Channel);
       bool  SetChannel(float *source, int channel);
       bool  SetChannel(short *source, int channel);
       float *GetChannel(int channel);
       float GetValue(int Channel, int Element);
       bool  SetValue(int Channel, int Element, float Value);
};


//---------------------------------------------------------------------------
#endif

