#ifndef ElectrodeRendererH
#define ElectrodeRendererH

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <vcl>
#include <types.hpp>

#include "ElectrodeCollection.h"

using namespace std;

typedef struct RENDERDESCRIPTION {
  CElectrodeCollection *pelectrodecollection;
  TRect                 boundingboxelectrodes;
  TRect                 boundingboxdevice;
  float                 minvalueelectrodes;
  float                 maxvalueelectrodes;
  int                   minvaluedevice;
  int                   maxvaluedevice;
  bool                  brender;
} TRENDERDESCRIPTION;


///////////////////////////////////////////////////////////////////////////////
/// CElectrodeRenderer class
/// @author <a href="mailto:pbrunner@wadsworth.org">Brunner Peter</a>
/// @version 1.0
/// @brief This class manages a number of instances of #CElectrodeCollection
/// and renders them to the device context passed with the constructor
/// with an given offset.
///////////////////////////////////////////////////////////////////////////////
class CElectrodeRenderer
{
  public:
    CElectrodeRenderer(TForm *powner);
    ~CElectrodeRenderer();

    void AddCollection (CElectrodeCollection  *pelectrodecollection,
                        TRect                  boundingboxelectrodes,
                        TRect                  boundingboxdevice,
                        float                  minvalueelectrodes,
                        float                  maxvalueelectrodes,
                        int                    minvaluedevice,
                        int                    maxvaluedevice);

    void UpdateCollectionDevice(int            index,
                                TRect          boundingboxdevice,
                                int            minvaluedevice,
                                int            maxvaluedevice,
                                bool           brender);



    void RenderInit    (int                    originx,
                        int                    originy);

    void Render();
    void Process(bool bsilient=false);

    void SetCursorPos(int cursorposx, int cursorposy);

    void RenderDetails(CElectrodeCollection *pelectrodes, int x, int y);

    void CallbackTrackBarMin(int position, int min, int max);

    void CallbackTrackBarMax(int position, int min, int max);

    void Highlight(int indexhighlight);
    void HighlightOff(int indexhighlight);

    void  __fastcall OnMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);

  private:

    /// Defines the owner of the Form that provides the canvas.
    /// Is set in constructor.
    TForm                      *powner;

    /// Stores the instances of #CElectrodeCollection that are added using
    /// #AddCollection.
    vector<TRENDERDESCRIPTION>  vcollection;


};




#endif
