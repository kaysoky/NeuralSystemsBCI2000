#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ElectrodeCircle.h"

#define PENSIZE_MARKER 3.0
#define PENSIZE_CIRCLE 1.0

///////////////////////////////////////////////////////////////////////////////
/// Makes a new instance of the class, stores the paramters, calls the
/// constructor of the #CElectrode base class and creates the Win32 API brushed
/// and pens.
/// @param #xpos x-position.
/// @param #ypos y-position.
/// @param #zpos z-position.
/// @param #label label that describes the electrode.
/// @param #color color in RGB of the electrode.
/// @param #marked <B>true</B> if the electrode should be marked else <B>false</B>.
/// @param #colormarker color in RGB of the marker.
/// @param #orderlowpass order of the FIR lowpass filter.
/// @param #orderhighpass order of the FIR highpass filter.
/// @param #runningaverage defines filter to be a running average filter,
///                        overwrite all other filter settings.
///////////////////////////////////////////////////////////////////////////////
CElectrodeCircle::CElectrodeCircle(float xpos, float ypos, float zpos, string label, TColor color, bool marked, TColor colormarker, float learningratehist, float learningrateaverage, float baselineresolution, bool runningaverage, int statisticdisplaytype) :
                        CElectrode(      xpos,       ypos,       zpos,        label,        color,      marked,        colormarker,       learningratehist,       learningrateaverage,       baselineresolution,      runningaverage,     statisticdisplaytype)
{

  this->color           = color;
  this->colormarker     = colormarker;

  hbrushcircle          = CreateSolidBrush(color);
  hbrushcirclecenter    = CreateSolidBrush(clBlack);
  hbrushbackground      = CreateSolidBrush(clWhite);

  hpencircle            = CreatePen(PS_SOLID, PENSIZE_CIRCLE, clBlack);
  hpencirclemarker      = CreatePen(PS_SOLID, PENSIZE_MARKER, colormarker);
  hpenbackground        = CreatePen(PS_SOLID, PENSIZE_CIRCLE, clWhite);

  scalefactordevice     = 1;

}

///////////////////////////////////////////////////////////////////////////////
/// Destroys the instance of this class.
/// Deallocates all Win32 brushes and pens.
///////////////////////////////////////////////////////////////////////////////
CElectrodeCircle::~CElectrodeCircle()
{
  DeleteObject(hbrushcircle);
  DeleteObject(hbrushcirclecenter);
  DeleteObject(hbrushbackground);

  DeleteObject(hpencircle);
  DeleteObject(hpencirclemarker);
  DeleteObject(hpenbackground);
}


///////////////////////////////////////////////////////////////////////////////
/// Determines if a given device coordinate (x,y) is within the cirlce shape
/// defined by the device coordinates (#posxdevice,#posydevice) and the
/// device radius #maxvaluedevice.
/// @returns <B>true</B> if within cirlce shape else <B>false</B>.
///////////////////////////////////////////////////////////////////////////////
bool CElectrodeCircle::IsPosition(int x, int y)
{
  return (sqrt(pow(posxdevice-x,2) + pow(posydevice-y,2)) < (maxvaluedevice*scalefactordevice+PENSIZE_MARKER));
}


///////////////////////////////////////////////////////////////////////////////
/// Initiates the rendering by defining the mapping between electrodes
/// coordinates and device coordinates. Stores the device context.
/// @param powner Defines the canvas of the owner form.
/// @param hdc Defines the device context for the Win32 API functions.
/// @param boundingboxelectrodes Defines area of electrodes that are mapped
///                              to the device area defined by
///                              #boundingboxdevice.
/// @param boundingboxdevice Defines the area that the electrodes area defined
///                          by #boundingboxelectrodes is mapped to.
/// @param minvalueelectrodes Defines the minimum value that is mapped to the
///                           minimum circle radius #minvaluedevice.
/// @param maxvalueelectrodes Defines the maximum value that is mapped to the
///                           maximum circle radius #maxvaluedevice.
/// @param minvaluedevice Defines the minimum circle radius on the device.
/// @param maxvaluedevice Defines the minimum circle radius on the device.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCircle::RenderInit( TForm *powner,
                                   HDC    hdc,
                                   TRect  boundingboxelectrodes,
                                   TRect  boundingboxdevice,
                                   float  minvalueelectrodes,
                                   float  maxvalueelectrodes,
                                   int    minvaluedevice,
                                   int    maxvaluedevice,
                                   float  scalefactordevice)
{
  // store the parameters
  this->powner                  = powner;
  this->hdc                     = hdc;

  this->scalefactordevice       = scalefactordevice;

  this->boundingboxelectrodes   = boundingboxelectrodes;
  this->boundingboxdevice       = boundingboxdevice;
  this->minvalueelectrodes      = minvalueelectrodes;
  this->maxvalueelectrodes      = maxvalueelectrodes;
  this->minvaluedevice          = (float)minvaluedevice/2.0 + (float)minvaluedevice/2.0 * scalefactordevice;
  this->maxvaluedevice          = (float)minvaluedevice/2.0 + (float)maxvaluedevice/2.0 * scalefactordevice;

  // calculate the device coordinates
  this->posxdevice              = (float)(GetXpos() - boundingboxelectrodes.left) / (float)boundingboxelectrodes.Width()  * (float)boundingboxdevice.Width()  + (float)boundingboxdevice.left;
  this->posydevice              = (float)(GetYpos() - boundingboxelectrodes.top ) / (float)boundingboxelectrodes.Height() * (float)boundingboxdevice.Height() + (float)boundingboxdevice.top;

  // delete old graphic objects
  DeleteObject(hbrushcircle);
  DeleteObject(hbrushcirclecenter);
  DeleteObject(hbrushbackground);

  DeleteObject(hpencircle);
  DeleteObject(hpencirclemarker);
  DeleteObject(hpenbackground);

  // create new graphic objects
  hbrushcircle          = CreateSolidBrush(color);
  hbrushcirclecenter    = CreateSolidBrush(clBlack);
  hbrushbackground      = CreateSolidBrush(clWhite);

  hpencircle            = CreatePen(PS_SOLID, PENSIZE_CIRCLE, clBlack);
  hpencirclemarker      = CreatePen(PS_SOLID, PENSIZE_MARKER/2.0 + PENSIZE_MARKER/2.0 * scalefactordevice, colormarker);
  hpenbackground        = CreatePen(PS_SOLID, PENSIZE_CIRCLE, clWhite);

}

///////////////////////////////////////////////////////////////////////////////
/// Updates the mapping of (#minvalueelectrodes,#maxvalueelectrodes) to
/// (#minvaluedevice,#maxvaluedevice) by redefining the
/// (#minvalueelectrodes,#maxvalueelectrodes).
/// @param minvalueelectrodes Defines the minimum value that is mapped to the
///                           minimum circle radius #minvaluedevice.
/// @param maxvalueelectrodes Defines the maximum value that is mapped to the
///                           maximum circle radius #maxvaluedevice.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCircle::RenderUpdate( float  minvalueelectrodes,
                                     float  maxvalueelectrodes)
{
  this->minvalueelectrodes      = minvalueelectrodes;
  this->maxvalueelectrodes      = maxvalueelectrodes;
}


///////////////////////////////////////////////////////////////////////////////
/// Renders the circle electrode according to the settings from #RenderInit.
/// Uses Win32 API for drawing. Does not use double buffered drawing, therefore
/// controlling class should provide this.
/// @warning #RenderInit has to be called before calling #Render.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCircle::Render()
{
  HBRUSH hbrushold;
  HPEN   hpenold;
  float  valuedevice;

  // get the value in device coordinates
  valuedevice = GetValueDevice();

  // limit the maximum of the value to maxvaluedevice
  valuedevice = valuedevice > maxvaluedevice ? maxvaluedevice : valuedevice;

  // sanity check of the maxvaluedevice
  maxvaluedevice = maxvaluedevice < 1 ? 1 : maxvaluedevice;

  // calculate the color value for this value 
  float colorvalue = (float)valuedevice / (float)maxvaluedevice * 255.0;

  // create new brush using this color for the graded colored circle 
  hbrushcircledynamic = CreateSolidBrush(RGB(colorvalue,0,0));

  // draw marker if electrode is marked
  if (GetMarker()) {
    hbrushold = SelectObject(hdc, hbrushbackground);
    hpenold   = SelectObject(hdc, hpencirclemarker);


    Ellipse(hdc,
            posxdevice-(maxvaluedevice+PENSIZE_MARKER),
            posydevice-(maxvaluedevice+PENSIZE_MARKER),
            posxdevice+(maxvaluedevice+PENSIZE_MARKER+1),
            posydevice+(maxvaluedevice+PENSIZE_MARKER+1));


    SelectObject(hdc, hbrushcircledynamic);
    SelectObject(hdc, hpencircle);
  } else {

    // draw the circle if the electrode is not marked
    hbrushold = SelectObject(hdc, hbrushcircledynamic);
    hpenold   = SelectObject(hdc, hpencircle);
  }

  // draw electrode center if value is bellow minvaluedevice
  if (valuedevice > (float)minvaluedevice * scalefactordevice) {

    Ellipse(hdc,
            posxdevice-valuedevice-PENSIZE_CIRCLE,
            posydevice-valuedevice-PENSIZE_CIRCLE,
            posxdevice+valuedevice+PENSIZE_CIRCLE+1,
            posydevice+valuedevice+PENSIZE_CIRCLE+1);


  } else {
  // draw electrode circle

    SelectObject(hdc, hbrushcirclecenter);
    Ellipse(hdc,
            posxdevice-minvaluedevice-PENSIZE_CIRCLE,
            posydevice-minvaluedevice-PENSIZE_CIRCLE,
            posxdevice+minvaluedevice+PENSIZE_CIRCLE+1,
            posydevice+minvaluedevice+PENSIZE_CIRCLE+1);

  }

  // select old brushes
  SelectObject(hdc, hbrushold);
  SelectObject(hdc, hpenold);

  // delete the dynamic brush for the graded color
  DeleteObject(hbrushcircledynamic);

}

///////////////////////////////////////////////////////////////////////////////
/// Toggles the color of the marker. Therefore the old Win32 brush is deleted
/// and a new one is created.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCircle::ToggleMarkerColor()
{
  CElectrode::ToggleMarkerColor();

  DeleteObject(hpencirclemarker);
  colormarker           = GetColorMarker();
  hpencirclemarker      = CreatePen(PS_SOLID, PENSIZE_MARKER, colormarker);

}

///////////////////////////////////////////////////////////////////////////////
/// Returns the #minvalueelectrodes.
///////////////////////////////////////////////////////////////////////////////
float CElectrodeCircle::GetMinValueElectrodes()
{
  return this->minvalueelectrodes;
}

///////////////////////////////////////////////////////////////////////////////
/// Returns the #maxvalueelectrodes.
///////////////////////////////////////////////////////////////////////////////
float CElectrodeCircle::GetMaxValueElectrodes()
{
  return this->maxvalueelectrodes;
}

///////////////////////////////////////////////////////////////////////////////
/// Returns the #minvaluedevice.
///////////////////////////////////////////////////////////////////////////////
int CElectrodeCircle::GetMinValueElectrodesDevice()
{
  return this->minvaluedevice;
}

///////////////////////////////////////////////////////////////////////////////
/// Returns the #maxvaluedevice.
///////////////////////////////////////////////////////////////////////////////
int CElectrodeCircle::GetMaxValueElectrodesDevice()
{
  return this->maxvaluedevice;
}

///////////////////////////////////////////////////////////////////////////////
/// Returns the #posxdevice.
///////////////////////////////////////////////////////////////////////////////
int CElectrodeCircle::GetXposdevice()
{
  return this->posxdevice;
}

///////////////////////////////////////////////////////////////////////////////
/// Returns the #posydevice.
///////////////////////////////////////////////////////////////////////////////
int CElectrodeCircle::GetYposdevice()
{
  return this->posydevice;
}


///////////////////////////////////////////////////////////////////////////////
/// Returns the radius of the cirlce in device coordinates.
///////////////////////////////////////////////////////////////////////////////
float CElectrodeCircle::GetValueDevice()
{

  float  valuedevice;

  // calc the current value of the radius
  if (fabs(maxvalueelectrodes-minvalueelectrodes) >= 1e-16) {
    valuedevice = (float)(GetValue() - minvalueelectrodes)/(float)(maxvalueelectrodes-minvalueelectrodes) * (float)maxvaluedevice + (float)minvaluedevice;
  } else {
    valuedevice = 0;
  }
  
  return valuedevice;

}



