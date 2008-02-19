#ifndef ElectrodeCircleH
#define ElectrodeCircleH

#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <Graphics.hpp>
#include <vcl>
#include <types.hpp>

#include "Electrode.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
/// CElectrodeCircle class
/// @author <a href="mailto:pbrunner@wadsworth.org">Brunner Peter</a>
/// @version 1.0
/// @brief This class extends the #CElectrode class by an graphic cirlce shape.
/// Therefore the electrode is mapped on an graphic device according to:
/// - source value mapping ( #minvalueelectrodes, #minvalueelectrodes)
/// - destination value mapping (#minvaluedevice, #maxvaluedevice)
/// - source position mapping (#boundingboxelectrodes)
/// - destination device position mapping (#boundingboxdevice)
/// .
/// This results in the properties of the circle:
/// - position (#posxdevice, #posydevice)
/// - radius 
/// .
/// The initiation of the rendering (#RenderInit) defines the mapping to the
/// device. This mapping can be altered in with limitations (#RenderUpdate).
/// In each call of the #Render function the circle is drawn to the device using
/// Win32 API funcitons.
///////////////////////////////////////////////////////////////////////////////
class CElectrodeCircle : public CElectrode
{
  public:

    CElectrodeCircle(float xpos, float ypos, float zpos, string label, TColor color, bool marked=false, TColor colormarker=clBlack, float learningratehist=0, float learningrateaverage=0, float baselineresolution=0, bool runningaverage=false, int statisticdisplaytype=0);
    ~CElectrodeCircle();

    void Render();

    void RenderInit( TForm *powner,
                     HDC    hdc,
                     TRect  boundingboxelectrodes,
                     TRect  boundingboxdevice,
                     float  minvalueelectrodes,
                     float  maxvalueelectrodes,
                     int    minvaluedevice,
                     int    maxvaluedevice,
                     float  scalefactordevice);




    void RenderUpdate( float  minvalueelectrodes,
                       float  maxvalueelectrodes);


    bool IsPosition(int x, int y);

    virtual void ToggleMarkerColor();

    float  GetMinValueElectrodes();
    float  GetMaxValueElectrodes();
    int    GetMinValueElectrodesDevice();
    int    GetMaxValueElectrodesDevice();
    int    GetXposdevice();
    int    GetYposdevice();
    float  GetValueDevice();


  private:

    /// Defines the owner of the Form that provides the canvas.
    /// Is set in #RenderInit.
    TForm *powner;

    /// Defines the device context of the canvas for the Win32 API functions.
    /// Is set in #RenderInit.
    HDC    hdc;

    /// Defines the mapping of device-independet coordinates to device
    /// coordinates defined in #boundingboxdevice.
    /// Is set in #RenderInit.
    TRect  boundingboxelectrodes;

    /// Defines the mapping of device independent coordinates defined in
    /// #boundingboxelectrodes to the device coordinates.
    /// Is set in #RenderInit.
    TRect  boundingboxdevice;

    /// Defines the mapping of values to displayed radius on the device.
    /// Therefore #minvalueelectrodes is mapped to #minvaluedevice.
    /// Is set in #RenderInit.
    float  minvalueelectrodes;

    /// Defines the mapping of values to displayed radius on the device.
    /// Therefore #maxvalueelectrodes is mapped to #maxvaluedevice.
    /// Is set in #RenderInit.
    float  maxvalueelectrodes;

    /// Defines the mapping of values to displayed radius on the device.
    /// Therefore #minvalueelectrodes is mapped to #minvaluedevice.
    /// Is set in #RenderInit.
    int    minvaluedevice;

    /// Defines the mapping of values to displayed radius on the device.
    /// Therefore #maxvalueelectrodes is mapped to #maxvaluedevice.
    /// Is set in #RenderInit.
    int    maxvaluedevice;

    /// This is according to the mapping from #boundingboxelectrodes to
    /// boundingboxdevice calculated x-coordinate of the elctrode on the device.
    /// Is calculated in #RenderInit.
    int    posxdevice;

    /// This is according to the mapping from #boundingboxelectrodes to
    /// boundingboxdevice calculated y-coordinate of the elctrode on the device.
    /// Is calculated in #RenderInit.
    int    posydevice;

    /// Preconstructed Win32 API brush. Used for drawing the circle.
    /// Allocated in Constructor. Deallocated in Destructor.
    HBRUSH hbrushcircle;

    /// Preconstructed Win32 API brush. Used for drawing the circle in dynamic color.
    /// Allocated on demand. Deallocated on demand.
    HBRUSH hbrushcircledynamic;

    /// Preconstructed Win32 API brush. Used for drawing the circle marker.
    /// Allocated in Constructor. Deallocated in Destructor.
    HBRUSH hbrushcirclemarker;

    /// Preconstructed Win32 API brush. Used for drawing the circle center.
    /// Allocated in Constructor. Deallocated in Destructor.
    HBRUSH hbrushcirclecenter;

    /// Preconstructed Win32 API brush. Used for drawing the background.
    /// Allocated in Constructor. Deallocated in Destructor.
    HBRUSH hbrushbackground;

    /// Preconstructed Win32 API pen. Used for drawing the circle.
    /// Allocated in Constructor. Deallocated in Destructor.
    HPEN   hpencircle;

    /// Preconstructed Win32 API pen. Used for drawing the circle marker.
    /// Allocated in Constructor. Deallocated in Destructor.
    HPEN   hpencirclemarker;

    /// Preconstructed Win32 API pen. Used for drawing the background.
    /// Allocated in Constructor. Deallocated in Destructor.
    HPEN   hpenbackground;

    /// Defines the scalefactor for all values in device coordinates.
    /// Set in #RenderInit.
    float  scalefactordevice;

    /// Defines the color of the circle.
    /// Set in Constructor.
    TColor color;

    /// Defines the color of the cirlce marker.
    /// Set in Constructor.
    TColor colormarker;

};

#endif
