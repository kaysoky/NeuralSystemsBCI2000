#ifndef ElectrodeCollectionH
#define ElectrodeCollectionH

#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <graphics.hpp>
#include <controls.hpp>
#include <comctrls.hpp>

#include <gl\gl.h>
#include <gl\glu.h>

#include "Electrode.h"
#include "ElectrodeCircle.h"

using namespace std;

#define CM_STATCLICK (WM_APP + 400)

typedef void CALLBACK CALLBACKFUNCTION(int position, int min, int max);

class CElectrodeRenderer;

///////////////////////////////////////////////////////////////////////////////
/// CElectrodeCollection class
/// @author <a href="mailto:pbrunner@wadsworth.org">Brunner Peter</a>
/// @version 1.0
/// @brief This class manages a number of electrodes with double
/// buffered rendering as well as the rendering of the framework. Also the
/// eventhandling is done for mouse events is done by this class. 
/// Electrodes are added using the #AddElectrodeCircle function.
/// For all electrodes the same parameters are used.
///////////////////////////////////////////////////////////////////////////////
class CElectrodeCollection
{
  public:
    CElectrodeCollection(string label, string scalex="", string scaley="",float scalexvalue=0,float scaleyvalue=0,float learningratehist=0, float learningrateaverage=0, float baselineresolution=0, bool runningaverage=false, bool bcommonslider=false, int statisticdisplaytype=0, vector<int> vautoscalechannellist=vector<int>(), float LearningRateAverage=0);
    ~CElectrodeCollection();

    void                AddElectrodeCircle          (float xpos, float ypos, float zpos, string label,
                                                     TColor color, bool marked=false, TColor colormarker=clBlack);

    void                RegisterRenderer            (CElectrodeRenderer *prenderer);
    void                Process                     (bool bsilent=false);
    void                Render                      ();
    void                RenderInit                  (TForm *powner,
                                                     TRect  boundingboxelectrodes,
                                                     TRect  boundingboxdevice,
                                                     float  minvalueelectrodes,
                                                     float  maxvalueelectrodes,
                                                     int    minvaluedevice,
                                                     int    maxvaluedevice);
    void                Hide                        ();
    void                Show                        ();


    void                SetTrackBarMinChange        (int position, int min, int max);
    void                SetTrackBarMaxChange        (int position, int min, int max);
    void                SetCursorPos                (int xpos, int ypos);

    void                ToggleMarker                (int x, int y);
    void                ToggleMarkerColor           (int x, int y);
    void                Highlight                   (bool bhighlight);

    string              GetTitle                    ();
    int                 GetSize                     ();
    CElectrodeCircle   *GetElectrodeCircle          (int index);
    TRect               GetBoundingBoxElectrodes    ();
    float               GetMinValueElectrodes       ();
    float               GetMaxValueElectrodes       ();

    void                EstimateElectrodeMinMax     ();
    void                EstimateElectrodeQuantiles  (int quantile_min, int quantile_max);

    bool                IsCommonSlider              ();

  private:
    bool                Within                      (int x, int y, int x1, int y1, int x2, int y2);
    TRect               GetBoundingBoxInnerDevice   ();
    void                RenderFramework             ();
    int                 GetElectrode                (int x, int y);
    void                RenderDetailsInit           ();
    void                RenderDetails               ();
    void                RenderScale                 ();
    void                RenderTrackBarInit          ();
    void                RenderTrackBar              ();

    void __fastcall     OnMouseMove                 (TObject *Sender, TShiftState Shift, int X, int Y);
    void __fastcall     OnMouseDown                 (TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall     OnTrackBarMinChange         (TObject *Sender);
    void __fastcall     OnTrackBarMaxChange         (TObject *Sender);
    void __fastcall     OnUpDownMinClick            (TObject *Sender, TUDBtnType Button);
    void __fastcall     OnUpDownMaxClick            (TObject *Sender, TUDBtnType Button);
    void __fastcall     OnAutoscaleClick            (TObject *Sender);

    /// Defines the owner of the Form that provides the canvas.
    /// Is set in #RenderInit.
    TForm *powner;

    /// Defines the owner of the Form that provides the canvas.
    /// Is set in #RenderInit.
    HDC       hdc;

    /// Defines the device context of the canvas for the Win32 API functions.
    /// Is set in #RenderInit.
    HDC       hdcmemory;

    /// Defines an Win32 bitmap that is used for double buffered drawing.
    /// Is allocated in #RenderInit and deallocated in the destructor.
    HBITMAP   hbitmapmemory;

    /// Preconstructed Win32 API brush. Used for drawing the background.
    /// Allocated in Constructor. Deallocated in Destructor.
    HBRUSH    hbrushbackground;

    /// Defines the area of the canvas that is defined by #powner and #hdc.
    /// Is set in #RenderInit.
    TRect     boundingboxowner;

    /// Defines the area of electrode coordinates that are mapped to
    /// #boundingboxelectrodesdevice.
    /// Is set in #RenderInit.
    TRect     boundingboxelectrodes;

    /// Defines the inner rendering area of #boundingboxdevice.
    /// Differs from #boundingboxdevice only by subtraction of the
    /// border line width.
    /// Is set in #RenderInit.
    TRect     boundingboxelectrodesdevice;

    /// Defines the inner rendering area of #boundingboxdevice.
    /// Is set in #RenderInit.
    TRect     boundingboxdevice;

    /// Defines the mapping of values to displayed radius on the device.
    /// Therefore #minvalueelectrodes is mapped to #minvaluedevice.
    /// Is set in #RenderInit.
    float     minvalueelectrodes;

    /// Defines the mapping of values to displayed radius on the device.
    /// Therefore #maxvalueelectrodes is mapped to #maxvaluedevice.
    /// Is set in #RenderInit.
    float     maxvalueelectrodes;

    /// Saves the first initial set value of #minvalueelectrodes. Is used to
    /// scale #minvalueelectrodes using the sliders.
    float     minvalueelectrodesinitial;

    /// Saves the first initial set value of #maxvalueelectrodes. Is used to
    /// scale #maxvalueelectrodes using the sliders.
    float     maxvalueelectrodesinitial;

    /// Defines the mapping of values to displayed radius on the device.
    /// Therefore #minvalueelectrodes is mapped to #minvaluedevice.
    /// Is set in #RenderInit.
    int       minvaluedevice;

    /// Defines the mapping of values to displayed radius on the device.
    /// Therefore #maxvalueelectrodes is mapped to #maxvaluedevice.
    /// Is set in #RenderInit.
    int       maxvaluedevice;

    /// Defines the value in electrode coordinates that corresponds to
    /// the string in #plegendx.
    /// Is set in constructor.
    float     scalexvalue;

    /// Defines the value in electrode coordinates that corresponds to
    /// the string in #plegendy.
    /// Is set in constructor.
    float     scaleyvalue;

    /// Defines the value in device coordinates that corresponds to
    /// the string in #plegendx.
    /// The mapping is based on #boundingboxelectrodes and
    /// #boundingboxelectrodesdevice.
    /// Is set in #RenderInit.
    int       scalexvaluedevice;

    /// Defines the value in device coordinates that corresponds to
    /// the string in #plegendx.
    /// The mapping is based on #boundingboxelectrodes and
    /// #boundingboxelectrodesdevice.
    /// Is set in #RenderInit.
    int       scaleyvaluedevice;

    /// Defines the fontsize of the title that is stored in #title and displayed
    /// using the #ptitle control.
    int       fontsize;

    /// Defines the index of the current electrode of the vector #electrodes
    /// for witch the details are currently displeyd using #RenderDetails.
    /// using the #ptitle control.
    int       indexdetails;


    /// Defines the filter to be a running average.
    /// Overwrites all other filters.
    bool      runningaverage;

    /// Defines the title of the displayed collection. Is shown using the
    /// #ptitle control. Is set in constructor.
    string    title;

    /// Shows a bounding rectangle of size #boundingboxdevice. Used of event
    /// handling of mouse events #OnMouseMove and #OnMouseDown.
    /// Is shown in #RenderFramework.
    TShape   *pboundingrectangle;

    /// Shows a line that represents the scale. The length is defined by
    /// #scalexvaluedevice.
    /// Is shown in #RenderScale.
    TShape   *pscalex;

    /// Adds to #pscalex on the left side a bar to better indicate the length.
    /// Is shown in #RenderScale.
    TShape   *pscalexleft;

    /// Adds to #pscalex on the right side a bar to better indicate the length.
    /// Is shown in #RenderScale.
    TShape   *pscalexright;

    /// Shows a line that represents the scale. The length is defined by
    /// #scaleyvaluedevice.
    /// Is shown in #RenderScale.
    TShape   *pscaley;

    /// Adds to #pscaley on the top a bar to better indicate the length.
    /// Is shown in #RenderScale.
    TShape   *pscaleytop;

    /// Adds to #pscaley on the bottom a bar to better indicate the length.
    /// Is shown in #RenderScale.
    TShape   *pscaleybottom;

    /// Shows the title #title on the top of the #boundingboxdevice.
    /// Is shown in #RenderFramework.
    TLabel   *ptitle;

    /// Shows the text at corresonds to #scalexvaluedevice.
    /// Is shown in #RenderScale.
    TLabel   *plegendx;

    /// Shows the text at corresonds to #scaleyvaluedevice.
    /// Is shown in #RenderScale.
    TLabel   *plegendy;

    /// Shows the details information that is triggered by mouse movement.
    /// Is shown in #RenderDetails.
    TLabel   *pdetails;

    /// Shows the current scaling minima in device coordinates.
    /// Is shown in #RenderTrackBar.
    TLabel   *ptextmin;

    /// Shows the current scaling maxima in device coordinates.
    /// Is shown in #RenderTrackBar.
    TLabel   *ptextmax;

    /// Shows a track bar that allows the scaling of the mapping between
    /// electrodes coordinates and device coordinates by
    /// changing #minvalueelectrodes.
    /// Eventhandling is performed for mouse events in performed
    /// in #OnTrackBarMinChange.
    /// Is shown in RenderTrackbar.
    TTrackBar *ptrackbarmax;

    /// Shows a track bar that allows the scaling of the mapping between
    /// electrodes coordinates and device coordinates by
    /// changing #minvalueelectrodes.
    /// Eventhandling is performed for mouse events in performed
    /// in #OnTrackBarMaxChange.
    /// Is shown in RenderTrackbar.
    TTrackBar *ptrackbarmin;

    /// Shows a up/down bar that allows to extend the minimum scaling limits
    /// of the mapping between electrodes coordinates and device coordinates.
    /// Eventhandling is performed for mouse events in performed
    /// in #OnUpDownMinClick.
    /// Is shown in RenderTrackbar.
    TUpDown   *pupdownmin;

    /// Shows a up/down bar that allows to extend the maximum scaling limits
    /// of the mapping between electrodes coordinates and device coordinates.
    /// Eventhandling is performed for mouse events in performed
    /// in #OnUpDownMinClick.
    /// Is shown in RenderTrackbar.
    TUpDown   *pupdownmax;

    /// Shows the checkbox to enable/disable the autoscale feature.
    /// Eventhandling for click events is performed in #OnAutoscaleClick.
    /// Setting is mirrored by marker #bautoscale.
    /// Is shown in RenderTrackbar.
    TCheckBox *pcheckboxautoscale;

    /// Stores the electrodes that are added using the function
    /// #AddElectrodeCircle. The electrodes can be accessed using the function
    /// #GetElectrodeCircle.
    vector<CElectrodeCircle*> electrodes;

    /// Handle to the superior renderer for this electrode collection.
    /// Is initialized in #RegisterRenderer.
    CElectrodeRenderer *prenderer;

    /// Marker to loock OnMouseClick calls to the trackbar while handling
    /// the last click.
    bool btrackbarmaxignore;

    /// Marker to loock OnMouseClick calls to the trackbar while handling
    /// the last click.
    bool btrackbarminignore;

    /// Marker to idicate that the trackbar for this slider is commonly
    /// controlled with the trackbar of one or more other collections.
    /// Initialized in the Constructor. 
    bool bcommonslider;

    /// Stores the type of statistic that is calculated when the #GetValue
    /// function is called.
    /// Possible settings are (z_score=0, difference_of_means=1, r_square=2).
    /// Initialized in the Constructor.
    int statisticdisplaytype;

    /// Defines the color of the collection title #ptitle.
    /// Defined as hardcoded color in Constructor.
    TColor colortitle;

    /// Marker that indicates that the collection title #ptitle is supposed
    /// to be rendered in highlited color.
    /// Initialized in the Constructor.
    /// Toggled in the function #Highlight.
    bool bhighlight;

    /// Filter coefficient of the 1st-order IIR filter that is used to age
    /// the histogram of all values assigned to one electrode.
    /// Disables histogram learning if set to 0.
    float learningratehist;

    /// Filter coefficient of the 1st-order IIR filter that is used to age
    /// the running value of all values assigned to one electrode.
    /// Disables average learning if set to 0.
    float learningrateaverage;

    /// Filter coefficient of the 1st-order IIR filter that is used to age
    /// the running value of all values assigned to one electrode.
    /// Disables autoscale if set to 0.
    float learningrateautoscale;

    /// Defines the resolution of the histogram of all values assigned to
    /// one electrode.
    float baselineresolution;

    /// Marker to indicate that #RenderInit was successfully called.
    /// Initialized in Constructor.
    bool firsttimeinitialized;

    /// Marker to indicate that the #minvalueallelectrodes and
    /// #maxvalueallelectrodes was successfully learned.
    /// Initialized in Constructor.
    bool firsttimelearned;

    /// Stores the learned (IIR-filtered with 0.01) maximum value of
    /// all electrodes in this collection.
    float minvalueallelectrodes;

    /// Stores the learned (IIR-filtered with 0.01) maximum value of
    /// all electrodes in this collection.
    float maxvalueallelectrodes;

    /// Marker that indicates that the autoscale feature is enabled.
    /// Set/Unset in #OnAutoscaleClick.
    bool bautoscale;

    /// Stores the scalefactor for the collection and all surounding controls.
    /// Set in #RenderInit based on the ratio between the width of the
    /// collection and 300 pixels.
    float scalefactordevice;

    /// Stores the list of channels that are used to estimate
    /// #minvalueallelectrodes and #maxvalueallelectrodes in the case
    /// the autoscaling feature is enabled.
    /// Initialized in the Constructor.
    vector<int> vautoscalechannellist;

    /// Stores the current horizontal mouse position.
    /// Set in #SetCursorPos.
    int cursorposx;

    /// Stores the current vertical mouse position.
    /// Set in #SetCursorPos.
    int cursorposy;

};

#endif
