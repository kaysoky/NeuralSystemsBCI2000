////////////////////////////////////////////////////////////////////////////////
//
// File:    UGenericVisualization.h
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
#ifndef UGenericVisualizationH
#define UGenericVisualizationH

#include <ScktComp.hpp>
#include "defines.h"

class GenericIntSignal;
class GenericSignal;
class PARAM;

class GenericVisualization
{
protected:
        int     SendBufBytes(TCustomWinSocket *Socket, const char *buf, int length);
        GenericSignal    *signal;
        char   memotext[512];
        BYTE   sourceID;
        BYTE   datatype;
        BYTE   vis_type;
        int    stored_decimation, new_samples;
        size_t stored_maxelements;
public:
    GenericVisualization();
    GenericVisualization( BYTE sourceID, BYTE visType );
    ~GenericVisualization();

    // Some convenience declarations.
    bool Send( CFGID::CFGID cfgID, const char* cfgString )
             { return SendCfg2Operator( sourceID, cfgID, cfgString ); }
    bool Send( CFGID::CFGID cfgID, int cfgValue )
             { return SendCfg2Operator( sourceID, cfgID, cfgValue ); }
    bool Send( const char* memoString )
             { return SendMemo2Operator( memoString ); }
    bool Send( GenericSignal* signal )
             { return Send2Operator( signal ); }
    bool Send( GenericIntSignal* signal )
             { return Send2Operator( signal ); }

        // sends the whole signal to the operator
        bool    Send2Operator(const GenericIntSignal *signal);
        // sends only certain channels, as defined in the parameter, to the operator
        int     Send2Operator(const GenericIntSignal *signal, const PARAM *channellistparam);
        bool    Send2Operator(const GenericIntSignal *my_signal, int decimation);
        // the same functions for GenericSignals
        bool    Send2Operator(const GenericSignal *signal);
        int     Send2Operator(const GenericSignal *signal, const PARAM *channellistparam);
        bool    SendMemo2Operator(const char *string);
        bool    SendCfg2Operator(BYTE sourceID, BYTE cfgID, const char *cfgString);
        bool    SendCfg2Operator( BYTE sourceID, BYTE cfgID, int cfgValue );
        void    ParseVisualization(const char *buffer, int length);
        void    SetSourceID(BYTE my_sourceID);
        BYTE    GetSourceID() const;
        void    SetDataType(BYTE my_datatype);
        BYTE    GetDataType() const;
        const   GenericSignal    *GetSignal() const;
        const char *GetMemoText() const;
        void    SetVisualizationType(BYTE my_vistype);
        BYTE    GetVisualizationType() const { return vis_type; }
        bool    valid;
};

#endif // UGenericVisualizationH

