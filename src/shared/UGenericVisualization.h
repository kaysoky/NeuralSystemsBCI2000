//---------------------------------------------------------------------------

#ifndef UGenericVisualizationH
#define UGenericVisualizationH

#include <ScktComp.hpp>
#include <Chart.hpp>

#include <classes.hpp>

#define MAX_XAXISLABELS         1024
#define MAX_YAXISLABELS         256
#define MAX_DISPLAYCHANNELS     256

#include "..\defines.h"
#include "UCoreComm.h"
#include "UGenericSignal.h"
#include "UParameter.h"

class GenericVisualization
{
protected:
        CORECOMM *corecomm;
        int     SendBufBytes(TCustomWinSocket *Socket, char *buf, int length);
        GenericIntSignal *intsignal;
        GenericSignal    *signal;
        char   memotext[512];
        BYTE   sourceID;
        BYTE   datatype;
        BYTE   vis_type;
        int    stored_maxelements, stored_decimation, new_samples;
public:
	GenericVisualization::GenericVisualization();
	GenericVisualization::GenericVisualization(PARAMLIST *paramlist, CORECOMM *);
	GenericVisualization::~GenericVisualization();

        // sends the whole signal to the operator
        bool    Send2Operator(GenericIntSignal *signal);
        // sends only certain channels, as defined in the parameter, to the operator
        int     Send2Operator(GenericIntSignal *signal, PARAM *channellistparam);
        bool    Send2Operator(GenericIntSignal *my_signal, int decimation);
        // the same functions for GenericSignals
        bool    Send2Operator(GenericSignal *signal);
        int     Send2Operator(GenericSignal *signal, PARAM *channellistparam);
        bool    SendMemo2Operator(char *string);
        bool    SendCfg2Operator(BYTE sourceID, BYTE cfgID, char *cfgString);
        void    ParseVisualization(char *buffer, int length);
        void    SetSourceID(BYTE my_sourceID);
        BYTE    GetSourceID();
        void    SetDataType(BYTE my_datatype);
        BYTE    GetDataType();
        GenericIntSignal *GetIntSignal();
        GenericSignal    *GetSignal();
        char    *GetMemoText();
        void    SetVisualizationType(BYTE my_vistype);
        bool    valid;
};


class VISUAL
{
private: 	// User declarations
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
        TCriticalSection        *critsec;               // critical section for screen update
        bool    toggle;
public:		// User declarations
        VISUAL::VISUAL(BYTE my_sourceID, BYTE my_vis_type);
        VISUAL::~VISUAL();
        Graphics::TBitmap *bitmap;
        BYTE   sourceID;
        BYTE   vis_type;
        TForm  *form;
        // TChart *chart;
        TMemo  *memo;
        int    startsample;
        void   RenderData(GenericIntSignal *signal);
        void   RenderData(GenericSignal *signal);
        void   RenderMemo(char *memo);
};


class VISCFGLIST
{
private: 	// User declarations
        TCriticalSection        *critsec;               // critical section for screen update
        TList   *vis_list;
public:		// User declarations
        VISCFGLIST::VISCFGLIST();
        VISCFGLIST::~VISCFGLIST();
        void    Add(VISUAL *new_visual);
        VISUAL  *GetVisCfgPtr(BYTE my_sourceID);
        void    DeleteAllVisuals();
};
//---------------------------------------------------------------------------
#endif

