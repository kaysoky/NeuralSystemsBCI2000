#include "ElectrodeRenderer.h"


#include <stdlib.h>
#include <stdio.h>

#include "ElectrodeRenderer.h"

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

#pragma package(smart_init)


///////////////////////////////////////////////////////////////////////////////
/// Makes a new instance of the class and stores the paramters.
/// @param #powner Defines the owner of the Form that provides the canvas.
///////////////////////////////////////////////////////////////////////////////
CElectrodeRenderer::CElectrodeRenderer(TForm *powner)
{
  this->powner = powner;
}

///////////////////////////////////////////////////////////////////////////////
/// Destroys the instance of this class.
///////////////////////////////////////////////////////////////////////////////
CElectrodeRenderer::~CElectrodeRenderer()
{
}


///////////////////////////////////////////////////////////////////////////////
/// Add an instance of #CElectrodeCollection to the renderer.
/// @param pelectrodecollection Instance of #CElectrodeCollection to be added.
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
void CElectrodeRenderer::AddCollection (CElectrodeCollection  *pelectrodecollection,
                                        TRect                  boundingboxelectrodes,
                                        TRect                  boundingboxdevice,
                                        float                  minvalueelectrodes,
                                        float                  maxvalueelectrodes,
                                        int                    minvaluedevice,
                                        int                    maxvaluedevice)
{
  TRENDERDESCRIPTION renderdescription;

  renderdescription.pelectrodecollection  = pelectrodecollection;
  renderdescription.boundingboxelectrodes = boundingboxelectrodes;
  renderdescription.boundingboxdevice     = boundingboxdevice;
  renderdescription.minvalueelectrodes    = minvalueelectrodes;
  renderdescription.maxvalueelectrodes    = maxvalueelectrodes;
  renderdescription.minvaluedevice        = minvaluedevice;
  renderdescription.maxvaluedevice        = maxvaluedevice;
  renderdescription.brender               = true;

  this->vcollection.push_back(renderdescription);

  pelectrodecollection->RegisterRenderer(this);

}

///////////////////////////////////////////////////////////////////////////////
/// Initiates the rendering of the instance. Stores all parameters, initate
/// rendering of all instances of #CElectrodeCollection added with
/// #AddCollection.
/// @param originx offset in x-axis for all collections.
/// @param originy offset in y-axis for all collections.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeRenderer::RenderInit(int originx, int originy)
{
  for (unsigned int index=0; index < vcollection.size(); index++) {

    TRect boundingboxdevice = TRect(vcollection[index].boundingboxdevice.left   + originx,
                                    vcollection[index].boundingboxdevice.top    + originy,
                                    vcollection[index].boundingboxdevice.right  + originx,
                                    vcollection[index].boundingboxdevice.bottom + originy);

    vcollection[index].pelectrodecollection->RenderInit
          (  powner,
             vcollection[index].boundingboxelectrodes,
                                boundingboxdevice,
             vcollection[index].minvalueelectrodes,
             vcollection[index].maxvalueelectrodes,
             vcollection[index].minvaluedevice,
             vcollection[index].maxvaluedevice);
  }

}

///////////////////////////////////////////////////////////////////////////////
/// Rendering of all instances of #CElectrodeCollection added with
/// #AddCollection.
/// @warning #RenderInit has to be called before calling #Render.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeRenderer::Render()
{
  for (unsigned int index=0; index < vcollection.size(); index++) {
    if (vcollection[index].brender) {
      vcollection[index].pelectrodecollection->Render();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Callback function for the owner's OnMouseMove function to update the
/// current #cursorposx and #cursorposy in each electrode collection.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeRenderer::SetCursorPos(int cursorposx, int cursorposy)
{
  for (unsigned int index=0; index < vcollection.size(); index++) {
    vcollection[index].pelectrodecollection->SetCursorPos(cursorposx,cursorposy);
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Event handler for the TForm OnMouseMove event.
/// Updates the mouse position for each connected electrode collection.
///////////////////////////////////////////////////////////////////////////////
void  __fastcall CElectrodeRenderer::OnMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{
  SetCursorPos(X,Y);
}

///////////////////////////////////////////////////////////////////////////////
/// Callback function that updates the trackbar position in case of a common
/// trackbar setting for the max trackbar.
/// @param position Current position of the common track bar.
/// @param min Minimal position of the common track bar.
/// @param max Maximal position of the common track bar.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeRenderer::CallbackTrackBarMax(int position, int min, int max)
{
  for (unsigned int index=0; index < vcollection.size(); index++) {
    if (vcollection[index].pelectrodecollection->IsCommonSlider()) {
      vcollection[index].pelectrodecollection->SetTrackBarMaxChange(position,min,max);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Callback function that updates the trackbar position in case of a common
/// trackbar setting for the min trackbar.
/// @param position Current position of the common track bar.
/// @param min Minimal position of the common track bar.
/// @param max Maximal position of the common track bar.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeRenderer::CallbackTrackBarMin(int position, int min, int max)
{
  for (unsigned int index=0; index < vcollection.size(); index++) {
    if (vcollection[index].pelectrodecollection->IsCommonSlider()) {
      vcollection[index].pelectrodecollection->SetTrackBarMinChange(position,min,max);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Callthrough function to highlight the title of one of the connected
/// electrode collections.
/// @param indexhighlight Index of the collection for which the title should be
///                       highlighted.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeRenderer::Highlight(int indexhighlight)
{
  if (indexhighlight >= 0 && vcollection.size()) {
    vcollection[indexhighlight].pelectrodecollection->Highlight(true);
  }

/*
  for (int index=0; index < vcollection.size(); index++) {
    if (index == indexhighlight) {
      vcollection[index].pelectrodecollection->Highlight(true);
    } else {
      vcollection[index].pelectrodecollection->Highlight(false);
    }
  }
  */
}

///////////////////////////////////////////////////////////////////////////////
/// Callthrough function to turn off the highlight for the title of one of the
/// connected electrode collections.
/// @param indexhighlight Index of the collection for which the title should be
///                       highlighted.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeRenderer::HighlightOff(int indexhighlight)
{
  /*
  for (int index=0; index < vcollection.size(); index++) {
    if (index == indexhighlight) {
      vcollection[index].pelectrodecollection->Highlight(false);
    }
  }
  */
  if (indexhighlight >= 0 && vcollection.size()) {
    vcollection[indexhighlight].pelectrodecollection->Highlight(false);
  }
}



///////////////////////////////////////////////////////////////////////////////
/// Updates the render parameters of a connected electrode collection.
/// @param index Index of the connected electrode collection.
/// @param boundingboxdevice Bounding box to map the electrode collection to.
/// @param minvaluedevice Min radius of the circles.
/// @param maxvaluedevice Max radius of the circles.
/// @param brender Marker if it is currently rendered.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeRenderer::UpdateCollectionDevice(int   index,
                                                TRect boundingboxdevice,
                                                int   minvaluedevice,
                                                int   maxvaluedevice,
                                                bool  brender)
{
  vcollection[index].boundingboxdevice     = boundingboxdevice;
  vcollection[index].minvaluedevice        = minvaluedevice;
  vcollection[index].maxvaluedevice        = maxvaluedevice;
  vcollection[index].brender               = brender;

  if (brender) {
    vcollection[index].pelectrodecollection->Show();
  } else {
    vcollection[index].pelectrodecollection->Hide();
  }

}

///////////////////////////////////////////////////////////////////////////////
/// Finishes iteration of all instances of #CElectrodeCollection added with
/// #AddCollection.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeRenderer::Process(bool bsilient)
{
  for (unsigned int index=0; index < vcollection.size(); index++) {
    if (vcollection[index].brender) {
      vcollection[index].pelectrodecollection->Process(bsilient);
    }
  }
}
