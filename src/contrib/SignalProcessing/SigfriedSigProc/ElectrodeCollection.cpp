
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ElectrodeRenderer.h"
#include "ElectrodeCollection.h"

const UINT TRACK_BAR_MIN_CHANGE =
   ::RegisterWindowMessage(_T("TRACK_BAR_MIN_CHANGE{3515878E-AF97-42ab-9810-14B4B41363AB}CElectrodeCollection"));

const UINT TRACK_BAR_MAX_CHANGE =
   ::RegisterWindowMessage(_T("TRACK_BAR_MAX_CHANGE{D88DC815-582D-4106-895A-8EA2EA2E22FD}CElectrodeCollection"));

///////////////////////////////////////////////////////////////////////////////
/// Makes a new instance of the class, stores the paramters and creates
/// all VCL objects, and initates the event handling for mouse events.
/// @param title Title that is rendered at the top of the collection.
/// @param scalex String that corresponds to scalexvalue.
/// @param scaley String that corresponds to scaleyvalue.
/// @param #scalexvalue Length in electrodes coordinates that correspond to
///                    the string scalex.
/// @param #scaleyvalue Length in electrodes coordinates that correspond to
///                    the string scaley.
/// @param learningratehist Filter coefficient of an 1st-order IIR filter that
///                         is used to age the estimated histogram of all
///                         values that were assigned to one electrode.
/// @param #runningaverage defines filter to be a running average filter,
///                        overwrite all other filter settings.
/// @param baselineresolution Resolution of the histogram that is used to
///                           estimate the distribution of all values assigned
///                           to one electrode.
/// @param runningaverage Use running average as a statistic measure.
/// @param bcommonslider Use sliders that are connected with sliders from
///                      other electrode collections.
/// @param statisticdisplaytype Defines the statistic measure.
/// @param vautoscalechannellist Defines the channels that are used to estimate
///                              the min/max for the autoscale feature.
///////////////////////////////////////////////////////////////////////////////
CElectrodeCollection::CElectrodeCollection(string title, string scalex, string scaley, float scalexvalue, float scaleyvalue, float learningratehist, float learningrateaverage, float baselineresolution, bool runningaverage, bool bcommonslider, int statisticdisplaytype, vector<int> vautoscalechannellist, float learningrateautoscale)
{

  this->firsttimeinitialized      = false;
  this->firsttimelearned          = false;
  this->title                     = title;
  this->colortitle                = clBlack;
  this->bhighlight                = false;

  // create TShapes
  this->pboundingrectangle        = new TShape(static_cast<TComponent*>(NULL));
  this->pscalex                   = new TShape(static_cast<TComponent*>(NULL));
  this->pscalexleft               = new TShape(static_cast<TComponent*>(NULL));
  this->pscalexright              = new TShape(static_cast<TComponent*>(NULL));
  this->pscaley                   = new TShape(static_cast<TComponent*>(NULL));
  this->pscaleytop                = new TShape(static_cast<TComponent*>(NULL));
  this->pscaleybottom             = new TShape(static_cast<TComponent*>(NULL));

  // create TLabels
  this->ptitle                    = new TLabel(static_cast<TComponent*>(NULL));
  this->plegendx                  = new TLabel(static_cast<TComponent*>(NULL));
  this->plegendy                  = new TLabel(static_cast<TComponent*>(NULL));
  this->pdetails                  = new TLabel(static_cast<TComponent*>(NULL));
  this->ptextmin                  = new TLabel(static_cast<TComponent*>(NULL));
  this->ptextmax                  = new TLabel(static_cast<TComponent*>(NULL));

  // create TTrackbars
  this->ptrackbarmax              = new TTrackBar(static_cast<TComponent*>(NULL));
  this->ptrackbarmin              = new TTrackBar(static_cast<TComponent*>(NULL));

  // crate TUpDowns
  this->pupdownmin                = new TUpDown(static_cast<TComponent*>(NULL));
  this->pupdownmax                = new TUpDown(static_cast<TComponent*>(NULL));

  // create TCheckBox
  this->pcheckboxautoscale        = new TCheckBox(static_cast<TComponent*>(NULL));

  if (learningrateautoscale > 0) {
    this->pcheckboxautoscale->Checked = true;
    this->bautoscale                  = true;
  } else {
    this->pcheckboxautoscale->Checked = false;
    this->bautoscale                  = false;
  }

  // Initialize TShapes
  this->pboundingrectangle->Shape = stRectangle;
  this->pscalex->Shape            = stRectangle;
  this->pscaley->Shape            = stRectangle;

  // store parameters of Constructor
  this->scalexvalue               = scalexvalue;
  this->scaleyvalue               = scaleyvalue;
  this->runningaverage            = runningaverage;
  this->learningratehist          = learningratehist;
  this->learningrateaverage       = learningrateaverage;
  this->learningrateautoscale     = learningrateautoscale;
  this->baselineresolution        = baselineresolution;
  this->bcommonslider             = bcommonslider;
  this->statisticdisplaytype      = statisticdisplaytype;
  this->vautoscalechannellist     = vautoscalechannellist;

  // Make sure just created objects are hidden
  this->pboundingrectangle->Visible = false;
  this->pscalex->Visible            = false;
  this->pscaley->Visible            = false;

  // Set captions of TLabel objects
  this->ptitle->Caption           = title.c_str();
  this->plegendx->Caption         = scalex.c_str();
  this->plegendy->Caption         = scaley.c_str();

  // Connect the event handlers
  this->pboundingrectangle->OnMouseMove = OnMouseMove;
  this->pboundingrectangle->OnMouseDown = OnMouseDown;
  this->ptrackbarmin->OnChange          = OnTrackBarMinChange;
  this->ptrackbarmax->OnChange          = OnTrackBarMaxChange;
  this->pupdownmin->OnClick             = OnUpDownMinClick;
  this->pupdownmax->OnClick             = OnUpDownMaxClick;

  // initialize boolean variables
  this->btrackbarminignore = false;
  this->btrackbarmaxignore = false;

  // initialize pointer
  this->prenderer                         = NULL;
  this->hbitmapmemory                     = NULL;
  this->hbrushbackground                  = NULL;
  this->hdcmemory                         = NULL;

  // initialize other variables
  this->scalefactordevice     = 1;
  this->cursorposx            = 0;
  this->cursorposy            = 0;
  this->minvalueallelectrodes =  1e-32;
  this->maxvalueallelectrodes = -1e-32;
  this->indexdetails          = -1;

}

///////////////////////////////////////////////////////////////////////////////
/// Destroys the instance of this class.
/// Deallocates all VCL objects, as well as the objects allocated for the
/// Win32 double buffered drawing.
///////////////////////////////////////////////////////////////////////////////
CElectrodeCollection::~CElectrodeCollection()
{

  // delete all CElectrodeCircle objects
  for (unsigned int index=0; index < electrodes.size(); index++) {
    delete electrodes[index];
  }

  // hide all VCL objects before deleting them
  this->pboundingrectangle->Visible=false;
  this->ptitle->Visible=false;
  this->pscalex->Visible=false;
  this->pscalexleft->Visible=false;
  this->pscalexright->Visible=false;
  this->pscaley->Visible=false;
  this->pscaleytop->Visible=false;
  this->pscaleybottom->Visible=false;
  this->plegendx->Visible=false;
  this->plegendy->Visible=false;
  this->pdetails->Visible=false;
  this->ptrackbarmax->Visible=false;
  this->ptrackbarmin->Visible=false;
  this->pupdownmin->Visible=false;
  this->pupdownmax->Visible=false;
  this->ptextmin->Visible=false;
  this->ptextmax->Visible=false;
  this->pcheckboxautoscale->Visible=false;

  // reset connection to parent before deleting VCL objects
  this->pboundingrectangle->Parent=NULL;
  this->ptitle->Parent=NULL;
  this->pscalex->Parent=NULL;
  this->pscalexleft->Parent=NULL;
  this->pscalexright->Parent=NULL;
  this->pscaley->Parent=NULL;
  this->pscaleytop->Parent=NULL;
  this->pscaleybottom->Parent=NULL;
  this->plegendx->Parent=NULL;
  this->plegendy->Parent=NULL;
  this->pdetails->Parent=NULL;
  this->ptrackbarmax->Parent=NULL;
  this->ptrackbarmin->Parent=NULL;
  this->pupdownmin->Parent=NULL;
  this->pupdownmax->Parent=NULL;
  this->ptextmin->Parent=NULL;
  this->ptextmax->Parent=NULL;
  this->pcheckboxautoscale->Parent=NULL;

  // delete all VCL objects
  delete this->pboundingrectangle;
  delete this->ptitle;
  delete this->pscalex;
  delete this->pscalexleft;
  delete this->pscalexright;
  delete this->pscaley;
  delete this->pscaleytop;
  delete this->pscaleybottom;
  delete this->plegendx;
  delete this->plegendy;
  delete this->pdetails;
  delete this->ptrackbarmax;
  delete this->ptrackbarmin;
  delete this->pupdownmin;
  delete this->pupdownmax;
  delete this->ptextmin;
  delete this->ptextmax;
  delete this->pcheckboxautoscale;

  // delte all GDI objects
  if (hbitmapmemory)    DeleteObject(hbitmapmemory);
  if (hbrushbackground) DeleteObject(hbrushbackground);
  if (hdcmemory)        DeleteDC(hdcmemory);
}

///////////////////////////////////////////////////////////////////////////////
/// Returns #title.
///////////////////////////////////////////////////////////////////////////////
string CElectrodeCollection::GetTitle()
{
  return title;
}

///////////////////////////////////////////////////////////////////////////////
/// Returns the number of electrodes added with #AddElectrodeCircle.
///////////////////////////////////////////////////////////////////////////////
int CElectrodeCollection::GetSize()
{
  return electrodes.size();
}

///////////////////////////////////////////////////////////////////////////////
/// Accesses an electrode that is added with #AddElectrodeCircle.
///////////////////////////////////////////////////////////////////////////////
CElectrodeCircle *CElectrodeCollection::GetElectrodeCircle(int index)
{
  return electrodes[index];
}

///////////////////////////////////////////////////////////////////////////////
/// Addes an electrode to the collection. Creates a new #CElectrodeCircle
/// object with this parameters.
/// @param xpos x-position.
/// @param ypos y-position.
/// @param zpos z-position.
/// @param label label that describes the electrode.
/// @param color color in RGB of the electrode.
/// @param marked <B>true</B> if the electrode should be marked else <B>false</B>.
/// @param colormarker color in RGB of the marker.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::AddElectrodeCircle(float xpos, float ypos, float zpos, string label, TColor color, bool marked, TColor colormarker)
{
  electrodes.push_back(new CElectrodeCircle(xpos,ypos,zpos,label,color,marked,colormarker,learningratehist,learningrateaverage,baselineresolution,runningaverage,statisticdisplaytype));
}

///////////////////////////////////////////////////////////////////////////////
/// Initiates the Rendering of details by finding the corresponding electrode.
/// @param x x-coordinate in device coordinates.
/// @param y y-coordinate in device coordinates.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::RenderDetailsInit()
{
/*
  int x = Mouse->CursorPos.x;
  int y = Mouse->CursorPos.y;

  if  (Within(x,y,this->pboundingrectangle->Left+this->pboundingrectangle->Pen->Width,
                  this->pboundingrectangle->Top+this->pboundingrectangle->Pen->Width,
                  this->pboundingrectangle->Left+this->pboundingrectangle->Width-this->pboundingrectangle->Pen->Width,
                  this->pboundingrectangle->Top +this->pboundingrectangle->Height-this->pboundingrectangle->Pen->Width))

  {
    indexdetails = GetElectrode(x,y);
  } else {
    indexdetails = -1;
  }

  // hack
  indexdetails = -1;
*/
/*
  int formleft = 0;
  int formtop  = 0;

  if (powner != NULL) {

    try {
      formleft = powner->Left+ 5;
      formtop  = powner->Top + 20;
    } catch (...) {
      formleft = 0;
      formtop  = 0;
    }

  }

  int x = Mouse->CursorPos.x - formleft;
  int y = Mouse->CursorPos.y - formtop;
*/
/*
  int x = cursorposx;
  int y = cursorposy;

  if  (Within(x,y,this->pboundingrectangle->Left,
                  this->pboundingrectangle->Top,
                  this->pboundingrectangle->Left+this->pboundingrectangle->Width,
                  this->pboundingrectangle->Top +this->pboundingrectangle->Height))

  {

  } else {
    indexdetails = -1;
  }
*/


  RenderDetails();
//  OnAutoscaleClick(NULL);
}

///////////////////////////////////////////////////////////////////////////////
/// Renders the details for one electrode that is selected by mouse movement.
/// Makes the tracker bars #ptrackbarmin and #ptrackbarmin invisible to show
/// the details text on this position using the control #pdetails.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::RenderDetails()
{
  double         value;
  AnsiString     szdetails;
  AnsiString     szSign;
  AnsiString     szminvalueelectrodes;
  AnsiString     szmaxvalueelectrodes;
  string         szlabel;


  // only show details if mouse is over electrode
  if (indexdetails != -1) {

    szlabel             = electrodes[indexdetails]->GetLabel();
    value               = electrodes[indexdetails]->GetValue();

    EstimateElectrodeMinMax();

    // create signum character
    if (value < 0) {
      szSign = "-";
    } else {
      szSign = "+";
    }

    // use differnt format depending on the scale factor
    if (scalefactordevice > 0.5) {
      szdetails.sprintf("%s: %s%#8.3f",szlabel.c_str(),szSign,fabs(value));
    } else if (scalefactordevice > 0.4) {
      szdetails.sprintf("%s: %s%#7.3f",szlabel.c_str(),szSign,fabs(value));
    } else if (scalefactordevice > 0.3) {
      szdetails.sprintf("%s: %s%#6.2f",szlabel.c_str(),szSign,fabs(value));
    } else if (scalefactordevice > 0.2) {
      szdetails.sprintf("%s: %s%#5.2f",szlabel.c_str(),szSign,fabs(value));
    } else {
      szdetails.sprintf("%s: %s%#4.2f",szlabel.c_str(),szSign,fabs(value));
    }
    
    pdetails->Caption              = szdetails.c_str();

  } else {
    // mouse is not over any valid electrode

    pdetails->Caption              = "";
  }
  
}

///////////////////////////////////////////////////////////////////////////////
/// Finds an ElectrodeCircle thats position is within the coordinates provided.
/// The cutof is defined as the euclidian distance between the center of the
/// electrode beeing more than the maximum size of the electrode.
/// @param x x-coordinate in device coordinates.
/// @param y y-coordinate in device coordinates.
/// @returns index that corresponds to the vector #electrodes if found else -1.
///////////////////////////////////////////////////////////////////////////////
int CElectrodeCollection::GetElectrode(int x, int y)
{

  // initialize variables
  float distthis = 1e32;
  float distmin  = 1e32;
  int   indexmin = -1;

  for (unsigned int index=0; index < electrodes.size(); index++) {

    // euclidian distance between (x,y) and the center of this electrode
    distthis = sqrt(pow(electrodes[index]->GetXposdevice()-x,2) + pow(electrodes[index]->GetYposdevice()-y,2));

    if (distthis < distmin && distthis <= maxvaluedevice) {
      indexmin = index;
      distmin  = distthis;
    }
  }

  // return the electrode with the lowest euclidian distance to its center
  return indexmin;

}

///////////////////////////////////////////////////////////////////////////////
/// Determines the bounding rectangle in device coordinates of all in the
/// vector #electrodes stored electrodes. The bounding rectangle is limited
/// by the given mapping area #boundingboxelectrodesdevice.
/// @returns Bounding rectangle of electrodes in device coordinates.
///////////////////////////////////////////////////////////////////////////////
TRect CElectrodeCollection::GetBoundingBoxInnerDevice()
{

  // intialize variables
  int posxmin = INT_MAX;
  int posxmax = INT_MIN;
  int posymin = INT_MAX;
  int posymax = INT_MIN;
  int xpos;
  int ypos;

  for (unsigned int index=0; index < electrodes.size(); index++) {
    xpos = electrodes[index]->GetXposdevice();
    ypos = electrodes[index]->GetYposdevice();

    // check if xpos and ypos are within the mapping area
    if (xpos >= boundingboxelectrodesdevice.left  &&
        xpos <= boundingboxelectrodesdevice.right &&
        ypos >= boundingboxelectrodesdevice.top   &&
        ypos <= boundingboxelectrodesdevice.bottom) {

        posxmin = posxmin > xpos ? xpos : posxmin;
        posxmax = posxmax < xpos ? xpos : posxmax;
        posymin = posymin > ypos ? ypos : posymin;
        posymax = posymax < ypos ? ypos : posymax;
    }
  }
  return TRect(posxmin,posymin,posxmax,posymax);
}

///////////////////////////////////////////////////////////////////////////////
/// Determines the bounding rectangle in electrodes coordinates of all in the
/// vector #electrodes stored electrodes.
/// @returns Bounding rectangle of electrodes in electrodes coordinates.
///////////////////////////////////////////////////////////////////////////////
TRect CElectrodeCollection::GetBoundingBoxElectrodes()
{

  // initialize variables
  int posxmin = INT_MAX;
  int posxmax = INT_MIN;
  int posymin = INT_MAX;
  int posymax = INT_MIN;
  int xpos;
  int ypos;

  for (unsigned int index=0; index < electrodes.size(); index++) {
    xpos = electrodes[index]->GetXpos();
    ypos = electrodes[index]->GetYpos();

    posxmin = posxmin > xpos ? xpos : posxmin;
    posxmax = posxmax < xpos ? xpos : posxmax;
    posymin = posymin > ypos ? ypos : posymin;
    posymax = posymax < ypos ? ypos : posymax;
  }
  return TRect(posxmin,posymin,posxmax,posymax);
}


///////////////////////////////////////////////////////////////////////////////
/// Event handler for Mouse Move Event.
/// Used for rendering the details of an electrode.
/// Calls #RenderDetailsInit.
///////////////////////////////////////////////////////////////////////////////
void  __fastcall CElectrodeCollection::OnMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)
{
//  RenderDetailsInit(X+this->pboundingrectangle->Left,Y+this->pboundingrectangle->Top);
//  RenderDetailsInit();

//  int x = Mouse->CursorPos.x;
//  int y = Mouse->CursorPos.y;

  // determine if the mouse is over an electrode
  indexdetails = GetElectrode(X+this->pboundingrectangle->Left,Y+this->pboundingrectangle->Top);

  RenderDetailsInit();

}



///////////////////////////////////////////////////////////////////////////////
/// Event handler for Mouse Down Event:
/// -  left button: toggles marker on/off (#ToggleMarker)
/// - right button: toggles between the different colors (#ToggleMarkerColor)
///////////////////////////////////////////////////////////////////////////////
void __fastcall CElectrodeCollection::OnMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{

  if (Button == mbLeft) {
    // left button: toggle on/off
    ToggleMarker(X+this->pboundingrectangle->Left,Y+this->pboundingrectangle->Top);
  } else if (Button == mbRight) {
    // right button: toggle color
    ToggleMarkerColor(X+this->pboundingrectangle->Left,Y+this->pboundingrectangle->Top);
  }

}

///////////////////////////////////////////////////////////////////////////////
/// Initiates the rendering of the instance. Stores all parameters, initiate
/// rendering of all electrodes added with #AddElectrodeCircle and render
/// Framework to screen.
/// @param #powner Defines the canvas of the owner form.
/// @param #boundingboxelectrodes Defines area of electrodes that are mapped
///                              to the device area defined by
///                              #boundingboxdevice.
/// @param #boundingboxdevice Defines the area that the electrodes area defined
///                          by #boundingboxelectrodes is mapped to.
/// @param #minvalueelectrodes Defines the minimum value that is mapped to the
///                           minimum circle radius #minvaluedevice.
/// @param #maxvalueelectrodes Defines the maximum value that is mapped to the
///                           maximum circle radius #maxvaluedevice.
/// @param #minvaluedevice Defines the minimum circle radius on the device.
/// @param #maxvaluedevice Defines the minimum circle radius on the device.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::RenderInit(  TForm *powner,
                                        TRect  boundingboxelectrodes,
                                        TRect  boundingboxdevice,
                                        float  minvalueelectrodes,
                                        float  maxvalueelectrodes,
                                        int    minvaluedevice,
                                        int    maxvaluedevice)
{
  // free previously allocated GDI objects
  if (hbitmapmemory)    DeleteObject(hbitmapmemory);
  if (hbrushbackground) DeleteObject(hbrushbackground);
  if (hdcmemory)        DeleteDC(hdcmemory);

  // save parameters
  this->powner                = powner;
  this->boundingboxelectrodes = boundingboxelectrodes;
  this->boundingboxdevice     = boundingboxdevice;
  this->minvalueelectrodes    = minvalueelectrodes;
  this->maxvalueelectrodes    = maxvalueelectrodes;
  this->minvaluedevice        = minvaluedevice;
  this->maxvaluedevice        = maxvaluedevice;

  // calculate the scale factor based on a 300 pixel standard width of the electrode condition window
  this->scalefactordevice     = (float)(boundingboxdevice.Right - boundingboxdevice.Left) / 300.0;

  // inititate double buffered painting
  GetClientRect(powner->Handle, &boundingboxowner);
  hdc              = GetDC(powner->Handle);
  hdcmemory        = CreateCompatibleDC(hdc);
  hbitmapmemory    = CreateCompatibleBitmap(hdc,
                                         boundingboxowner.right -boundingboxowner.left,
                                         boundingboxowner.bottom-boundingboxowner.top);

  hbrushbackground = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

  // render the framework
  RenderFramework();

  // initiate the electrodes
  for (unsigned int index=0; index < electrodes.size(); index++) {
    electrodes[index]->RenderInit(powner,
                                  hdcmemory,
                                  boundingboxelectrodes,
                                  boundingboxelectrodesdevice,
                                  minvalueelectrodes,
                                  maxvalueelectrodes,
                                  minvaluedevice,
                                  maxvaluedevice,
                                  scalefactordevice);
  }

  this->scalexvaluedevice     = scalexvalue / (boundingboxelectrodes.right  - boundingboxelectrodes.left)  * (float)(boundingboxelectrodesdevice.right  - boundingboxelectrodesdevice.left);
  this->scaleyvaluedevice     = scaleyvalue / (boundingboxelectrodes.bottom - boundingboxelectrodes.top)   * (float)(boundingboxelectrodesdevice.bottom - boundingboxelectrodesdevice.top);

  // initiate scale
//  RenderScale();

  // initiate trackbar
  RenderTrackBarInit();
  RenderFramework();

  this->firsttimeinitialized = true;

}

///////////////////////////////////////////////////////////////////////////////
/// Renders all electrodes added with #AddElectrodeCircle. Uses double buffered
/// Win32 API. Renders also details if mouse has moved above electrode.
/// @warning #RenderInit has to be called before calling #Render.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::Render()
{

  HBITMAP hbitmapold;

  // save old device context
  hbitmapold = SelectObject(hdcmemory, hbitmapmemory);

  // clear device context
  FillRect    (hdcmemory, &boundingboxowner, hbrushbackground);
  SetBkMode   (hdcmemory, TRANSPARENT);
  SetTextColor(hdcmemory, GetSysColor(COLOR_WINDOWTEXT));

  // render all electrodes
  for (unsigned int index=0; index < electrodes.size(); index++) {
    electrodes[index]->Render();
  }

  // copy rendered electrodes to displayed device context
  BitBlt(hdc,
         boundingboxelectrodesdevice.left, boundingboxelectrodesdevice.top,
         boundingboxelectrodesdevice.right-boundingboxelectrodesdevice.left,
         boundingboxelectrodesdevice.bottom-boundingboxelectrodesdevice.top,
         hdcmemory,
         boundingboxelectrodesdevice.left, boundingboxelectrodesdevice.top,
         SRCCOPY);

  // restore old device context         
  SelectObject(hdcmemory, hbitmapold);

  // render detail information
  RenderDetailsInit();
//  RenderDetails();
}

///////////////////////////////////////////////////////////////////////////////
/// Renders the framework of the electrode consisting of bounding rectangle
/// #pboundingrectangle, the title #ptitle, and the details #pdetails.
/// @warning #RenderInit has to be called before calling #Render.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::RenderFramework()
{
  fontsize = 13;

  // title
  do {
    ptitle->Parent                   = this->powner;
    ptitle->Top                      = boundingboxdevice.top - fontsize - 10;
    ptitle->Left                     = boundingboxdevice.left;
    ptitle->Alignment                = taCenter;
    ptitle->Width                    = boundingboxdevice.right - boundingboxdevice.left;
    ptitle->Layout                   = tlBottom;
    ptitle->AutoSize                 = true;
    ptitle->Font->Size               = fontsize--;
    ptitle->Font->Style              = TFontStyles() << fsBold;
    ptitle->Font->Color              = colortitle;
    ptitle->Visible                  = true;
  } while (ptitle->Width > boundingboxdevice.right - boundingboxdevice.left && fontsize > 4);

  ptitle->Width                    = boundingboxdevice.right - boundingboxdevice.left;

  // bounding box
  pboundingrectangle->Parent         = this->powner;
  pboundingrectangle->Pen->Width     = 2.0 + 1.0 * scalefactordevice;
  pboundingrectangle->Width          = boundingboxdevice.right  - boundingboxdevice.left;
  pboundingrectangle->Height         = boundingboxdevice.bottom - boundingboxdevice.top;
  pboundingrectangle->Top            = boundingboxdevice.top;
  pboundingrectangle->Left           = boundingboxdevice.left;
  pboundingrectangle->Pen->Color     = colortitle;
  pboundingrectangle->Brush->Style   = bsClear;
  pboundingrectangle->Visible        = true;

  // estimate perimeters of the bounding box
  boundingboxelectrodesdevice.top    = pboundingrectangle->Top   + pboundingrectangle->Pen->Width;
  boundingboxelectrodesdevice.left   = pboundingrectangle->Left  + pboundingrectangle->Pen->Width;
  boundingboxelectrodesdevice.bottom = boundingboxelectrodesdevice.top  + pboundingrectangle->Height - 2*pboundingrectangle->Pen->Width;
  boundingboxelectrodesdevice.right  = boundingboxelectrodesdevice.left + pboundingrectangle->Width  - 2*pboundingrectangle->Pen->Width;

  // details
  pdetails->Parent                 = this->powner;
  pdetails->Top                    = boundingboxelectrodesdevice.bottom + this->pboundingrectangle->Pen->Width + pupdownmin->Height + 5;
  pdetails->Left                   = boundingboxelectrodesdevice.left   + pupdownmin->Width + pupdownmax->Width + ptrackbarmin->Width + 5.0 * scalefactordevice;
  pdetails->Font->Size             = 7.0 + scalefactordevice * 3.0;
  pdetails->Alignment              = taCenter;
  pdetails->Width                  = 100;
  pdetails->Layout                 = tlBottom;
  pdetails->Font->Name             = "Courier New";
  pdetails->Font->Style            = pdetails->Font->Style << fsBold;

  // initialize the rendering of the details
  RenderDetailsInit();
//  RenderDetails();

}
///////////////////////////////////////////////////////////////////////////////
/// Toggles the marker status of an electrode defined by device coordinates
/// (x,y). The electrode at this position is found using #GetElectrode function.
/// If there is no Electrode at this position no toggling is performed.
/// @param x x-coordinate in device coordinates.
/// @param y y-coordinate in device coordinates.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::ToggleMarker(int x, int y)
{
  // is x/y within the perimeters of a electrode?
  int index = GetElectrode(x,y);

  if (index!=-1) {
    // toggle the marker
    electrodes[index]->ToggleMarker();
    // render the change
    Render();
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Toggles the marker color of an electrode defined by device coordinates
/// (x,y). The electrode at this position is found using #GetElectrode function.
/// If there is no Electrode at this position no toggling is performed.
/// @param x x-coordinate in device coordinates.
/// @param y y-coordinate in device coordinates.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::ToggleMarkerColor(int x, int y)
{
  // is x/y within the perimeters of a electrode?
  int index = GetElectrode(x,y);

  if (index!=-1) {
    // toggle the marker color
    electrodes[index]->ToggleMarkerColor();
    // render the change
    Render();
  }
}


///////////////////////////////////////////////////////////////////////////////
/// Renders the scale and legend for the framework of the collection.
/// The scale and legend alligned to the bottom right electrode.
/// The settings for rendering are provided by #RenderInit.
/// @warning #RenderInit has to be called before calling #RenderScale.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::RenderScale()
{
  TRect boundingbox = GetBoundingBoxInnerDevice();

  // scalex
  pscalex->Parent                  = this->powner;
  pscalex->Width                   = scalexvaluedevice + 3;
  pscalex->Height                  = 3;
  pscalex->Top                     = boundingboxelectrodesdevice.bottom + 10;
  pscalex->Left                    = boundingbox.right - scalexvaluedevice;
  pscalex->Pen->Color              = clBlack;
  pscalex->Pen->Width              = 3;
  pscalex->Visible                 = true;
  pscalex->Hint                    = "scale of x-axis in display";

  pscalexleft->Parent              = this->powner;
  pscalexleft->Width               = 3;
  pscalexleft->Height              = 10 + 3;
  pscalexleft->Top                 = pscalex->Top - 5;
  pscalexleft->Left                = pscalex->Left;
  pscalexleft->Pen->Color          = clBlack;
  pscalexleft->Pen->Width          = 3;
  pscalexleft->Visible             = true;

  pscalexright->Parent             = this->powner;
  pscalexright->Width              = 3;
  pscalexright->Height             = 10 + 3;
  pscalexright->Top                = pscalex->Top - 5;
  pscalexright->Left               = pscalex->Left + pscalex->Width - 3;
  pscalexright->Pen->Color         = clBlack;
  pscalexright->Pen->Width         = 3;
  pscalexright->Visible            = true;


  // scaley
  pscaley->Parent                  = this->powner;
  pscaley->Width                   = 3;
  pscaley->Height                  = scaleyvaluedevice + 3;
  pscaley->Top                     = boundingbox.bottom - scaleyvaluedevice;
  pscaley->Left                    = boundingboxelectrodesdevice.right + 10;
  pscaley->Pen->Color              = clBlack;
  pscaley->Pen->Width              = 3;
  pscaley->Visible                 = true;
  pscaley->Hint                    = "scale of y-axis in display";

  pscaleytop->Parent               = this->powner;
  pscaleytop->Width                = 10 + 3;
  pscaleytop->Height               = 3;
  pscaleytop->Top                  = pscaley->Top;
  pscaleytop->Left                 = pscaley->Left - 5;
  pscaleytop->Pen->Color           = clBlack;
  pscaleytop->Pen->Width           = 3;
  pscaleytop->Visible              = true;

  pscaleybottom->Parent            = this->powner;
  pscaleybottom->Width             = 10 + 3;
  pscaleybottom->Height            = 3;
  pscaleybottom->Top               = pscaley->Top + pscaley->Height - 3;;
  pscaleybottom->Left              = pscaley->Left - 5;
  pscaleybottom->Pen->Color        = clBlack;
  pscaleybottom->Pen->Width        = 3;
  pscaleybottom->Visible           = true;

  // labelx
  plegendx->Parent                 = this->powner;
  plegendx->Top                    = pscalex->Top + 10;
  plegendx->Left                   = pscalex->Left;
  plegendx->Font->Size             = 10;
  plegendx->Alignment              = taCenter;
  plegendx->Width                  = pscalex->Width;
  plegendx->Layout                 = tlBottom;
  plegendx->Visible                = true;
  plegendx->Hint                   = "scale of x-axis in display";

  // labely
  plegendy->Parent                 = this->powner;
  plegendy->Top                    = pscaley->Top  + scaleyvaluedevice / 2 - 5;
  plegendy->Left                   = pscaley->Left + 10;
  plegendy->Font->Size             = 10;
  plegendy->Alignment              = taLeftJustify;
  plegendy->Layout                 = tlBottom;
  plegendy->Visible                = true;
  plegendy->Hint                   = "scale of y-axis in display";


}

///////////////////////////////////////////////////////////////////////////////
/// Initiates the rendering of the track bars #ptrackbarinmin and #ptrackbarmax
/// as well as the corresponding text #ptextmin and #ptextmax.
/// @warning #RenderInit has to be called before calling #RenderTrackBarInit.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::RenderTrackBarInit()
{
  AnsiString szminvalueelectrodes;
  AnsiString szmaxvalueelectrodes;

  this->maxvalueelectrodesinitial  = this->maxvalueelectrodes;
  this->minvalueelectrodesinitial  = this->minvalueelectrodes;

  ptrackbarmin->Visible            = false;
  ptrackbarmax->Visible            = false;
  pupdownmin->Visible              = false;
  pupdownmax->Visible              = false;
  pcheckboxautoscale->Visible      = false;

  // trackbar minima
  ptrackbarmin->Parent             = this->powner;
  ptrackbarmin->Top                = boundingboxelectrodesdevice.bottom + this->pboundingrectangle->Pen->Width;
  ptrackbarmin->Left               = boundingboxelectrodesdevice.left + 15 * scalefactordevice;
  ptrackbarmin->Width              = 100.0 * scalefactordevice;
  ptrackbarmin->Height             = 20;
  if (!this->firsttimeinitialized) ptrackbarmin->Min                = -100;
  if (!this->firsttimeinitialized) ptrackbarmin->Max                = 200;
  if (!this->firsttimeinitialized) ptrackbarmin->Position           = minvalueelectrodes/maxvalueelectrodesinitial * 100.0;
  ptrackbarmin->Frequency          = 100;
  ptrackbarmin->Hint               = "minium value for display";

  // trackbar maxima
  ptrackbarmax->Parent             = this->powner;
  ptrackbarmax->Top                = boundingboxelectrodesdevice.bottom + this->pboundingrectangle->Pen->Width + ptrackbarmin->Height;
  ptrackbarmax->Left               = boundingboxelectrodesdevice.left + 15 * scalefactordevice;
  ptrackbarmax->Width              = 100.0 * scalefactordevice;
  ptrackbarmax->Height             = 20;
  if (!this->firsttimeinitialized) ptrackbarmax->Min                = -100;
  if (!this->firsttimeinitialized) ptrackbarmax->Max                = 200;
  if (!this->firsttimeinitialized) ptrackbarmax->Position           = 100;
  ptrackbarmax->Frequency          = 100;
  ptrackbarmax->Hint               = "maximum value for display";

  // updown minima
  pupdownmin->Parent             = this->powner;
  pupdownmin->Top                = boundingboxelectrodesdevice.bottom + this->pboundingrectangle->Pen->Width;
  pupdownmin->Left               = boundingboxelectrodesdevice.left;
  pupdownmin->Width              = 15.0 * scalefactordevice;
  pupdownmin->Height             = 40;
  if (!this->firsttimeinitialized) pupdownmin->Min                = 0;
  if (!this->firsttimeinitialized) pupdownmin->Max                = 200;
  if (!this->firsttimeinitialized) pupdownmin->Position           = 100;
  pupdownmin->Hint               = "minimum value for slider";

  // updown maxima
  pupdownmax->Parent             = this->powner;
  pupdownmax->Top                = boundingboxelectrodesdevice.bottom + this->pboundingrectangle->Pen->Width;
  pupdownmax->Left               = boundingboxelectrodesdevice.left + ptrackbarmax->Width + 15 * scalefactordevice;
  pupdownmax->Width              = 15.0 * scalefactordevice;
  pupdownmax->Height             = 40;
  if (!this->firsttimeinitialized) pupdownmax->Min                = 0;
  if (!this->firsttimeinitialized) pupdownmax->Max                = 200;
  if (!this->firsttimeinitialized) pupdownmax->Position           = 100;
  pupdownmax->Hint               = "maxima value for slider";


  // text minima
  ptextmin->Parent                 = this->powner;
  ptextmin->Top                    = boundingboxelectrodesdevice.bottom + this->pboundingrectangle->Pen->Width;
  ptextmin->Left                   = boundingboxelectrodesdevice.left   + pupdownmin->Width + pupdownmax->Width + ptrackbarmin->Width + 5;
  ptextmin->Font->Size             = 7.0 + scalefactordevice * 3.0;
  ptextmin->Font->Name             = "Courier New";
  ptextmin->Font->Style            = ptextmin->Font->Style << fsBold;
  ptextmin->Alignment              = taLeftJustify ;
  ptextmin->Width                  = pboundingrectangle->Left + pboundingrectangle->Width - pupdownmax->Left - pupdownmax->Width;
  ptextmin->Layout                 = tlBottom;
  ptextmin->Visible                = true;
  ptextmin->Hint                   = "minium value for display";
  if (!this->firsttimeinitialized) ptextmin->Caption                = szminvalueelectrodes.sprintf("%5.2f (%5.2f)",minvalueelectrodes,minvalueallelectrodes);

  // text maxima
  ptextmax->Parent                 = this->powner;
  ptextmax->Top                    = boundingboxelectrodesdevice.bottom + this->pboundingrectangle->Pen->Width + ptrackbarmin->Height;
  ptextmax->Left                   = boundingboxelectrodesdevice.left   + pupdownmin->Width + pupdownmax->Width + ptrackbarmax->Width + 5;
  ptextmax->Font->Size             = 7.0 + scalefactordevice * 3.0;
  ptextmax->Font->Name             = "Courier New";
  ptextmax->Font->Style            = ptextmax->Font->Style << fsBold;
  ptextmax->Alignment              = taLeftJustify;
  ptextmax->Width                  = pboundingrectangle->Left + pboundingrectangle->Width - pupdownmax->Left - pupdownmax->Width;
  ptextmax->Layout                 = tlBottom;
  ptextmax->Visible                = true;
  ptextmax->Hint                   = "minium value for display";
  if (!this->firsttimeinitialized) ptextmax->Caption                = szmaxvalueelectrodes.sprintf("%5.2f (%5.2f)",maxvalueelectrodes,maxvalueallelectrodes);

  pcheckboxautoscale->Parent                = this->powner;
  pcheckboxautoscale->Top                   = boundingboxelectrodesdevice.bottom + this->pboundingrectangle->Pen->Width + pupdownmin->Height + 5;
  pcheckboxautoscale->Left                  = boundingboxelectrodesdevice.left;
  pcheckboxautoscale->Font->Size            = 7.0 + scalefactordevice * 3.0;
  pcheckboxautoscale->Font->Style           = TFontStyles() << fsBold;
  pcheckboxautoscale->Alignment             = taCenter;
  pcheckboxautoscale->Visible               = true;
  pcheckboxautoscale->Caption               = "autoscale";
  pcheckboxautoscale->Checked               = bautoscale;
  pcheckboxautoscale->OnClick               = OnAutoscaleClick;
  pcheckboxautoscale->Width                 = pupdownmax->Left - pupdownmin->Left ;

  ptrackbarmin->Visible            = true;
  ptrackbarmax->Visible            = true;
  pupdownmin->Visible              = true;
  pupdownmax->Visible              = true;
  pcheckboxautoscale->Visible      = true;

}

///////////////////////////////////////////////////////////////////////////////
/// Renders the track bars #ptrackbarmin and #ptrackbarmax and the corresponding
/// text description #ptextmin and #ptestmax. The values of the trackbars are
/// read from the VCL object and the scale is recalculated and provided to all
/// electrodes added with #AddElectrodeCircle. Finally #Render is called to
/// update the electrodes on the device with the new scale.
/// Is called from event handlers #OnTrackBarMinChange and #OnTrackBarMaxChange.
/// @warning #RenderTrackBarInit has to be called before calling #RenderTrackBar.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::RenderTrackBar()
{
  AnsiString szminvalueelectrodes;
  AnsiString szmaxvalueelectrodes;

  // necessary to avoid subsequent callbacks
  if (ptrackbarmin->Position == ptrackbarmax->Position) {
    ptrackbarmax->Position = ptrackbarmin->Position+1;
  }

  // map the percentage to the minvalueelectrodes value
  minvalueelectrodes = maxvalueelectrodesinitial * (float)ptrackbarmin->Position / 100.0;

  // map the percentage to the maxvalueelectrodes value
  maxvalueelectrodes = maxvalueelectrodesinitial * (float)ptrackbarmax->Position / 100.0;

  // change the caption of the text indicating the min/max of the trackbar
  ptextmin->Caption                = szminvalueelectrodes.sprintf("%5.2f (%5.2f)",minvalueelectrodes,minvalueallelectrodes);
  ptextmax->Caption                = szmaxvalueelectrodes.sprintf("%5.2f (%5.2f)",maxvalueelectrodes,maxvalueallelectrodes);

  // initiate the electrodes
  for (unsigned int index=0; index < electrodes.size(); index++) {
    electrodes[index]->RenderUpdate(minvalueelectrodes,
                                    maxvalueelectrodes);
  }

  // render the change
//  Render();

}


///////////////////////////////////////////////////////////////////////////////
/// Event handler for changes of the track bar #ptrackbarmin.
/// Calls #RenderTrackBar.
///////////////////////////////////////////////////////////////////////////////
void __fastcall CElectrodeCollection::OnTrackBarMinChange(TObject *Sender)
{
  if (!btrackbarminignore && !bautoscale) {

    if (prenderer != NULL && bcommonslider) {
      prenderer->CallbackTrackBarMin(ptrackbarmin->Position,ptrackbarmin->Min,ptrackbarmin->Max);
    } else {
      SetTrackBarMinChange(ptrackbarmin->Position,ptrackbarmin->Min,ptrackbarmin->Max);
    }
  }

  btrackbarminignore = false;

}

///////////////////////////////////////////////////////////////////////////////
/// Changes the position of the minimum trackbar #ptrackbarmin.
/// Called from #OnTrackBarMinChange and OnUpDownMinClick.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::SetTrackBarMinChange(int position, int min, int max)
{
  if (ptrackbarmin->Visible) {

    // prevent subsequent events triggering SetTrackBarMinChange
    btrackbarminignore = true;

    // adjusting the position such that hte max is always higher than the min
    if (position >= ptrackbarmax->Position) {
      ptrackbarmin->Position = ptrackbarmax->Position-1;
    } else {
      ptrackbarmin->Position = position;
    }

    // set the min/max position
    ptrackbarmin->Min = min;
    ptrackbarmax->Min = min;
    ptrackbarmin->Max = max;
    ptrackbarmax->Max = max;

    // render the changes
    RenderTrackBar();
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Changes the position of the maximum trackbar #ptrackbarmax.
/// Called from #OnTrackBarMinChange and OnUpDownMinClick.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::SetTrackBarMaxChange(int position, int min, int max)
{
  if (ptrackbarmax->Visible) {

    // prevent subsequent events triggering SetTrackBarMinChange
    btrackbarmaxignore = true;

    // adjusting the position such that hte max is always higher than the min
    if (ptrackbarmax->Position <= ptrackbarmin->Position) {
      ptrackbarmax->Position = ptrackbarmin->Position+1;
    } else {
      ptrackbarmax->Position = position;
    }

    // set the min/max position
    ptrackbarmin->Min = min;
    ptrackbarmax->Min = min;
    ptrackbarmin->Max = max;
    ptrackbarmax->Max = max;

    // render the changes
    RenderTrackBar();
  }
}



///////////////////////////////////////////////////////////////////////////////
/// Event handler for changes of the track bar #ptrackbarmax.
/// Calls #RenderTrackBar.
///////////////////////////////////////////////////////////////////////////////
void __fastcall CElectrodeCollection::OnTrackBarMaxChange(TObject *Sender)
{
  if (!btrackbarmaxignore && !bautoscale) {

    if (prenderer != NULL && bcommonslider) {
      prenderer->CallbackTrackBarMax(ptrackbarmax->Position,ptrackbarmax->Min,ptrackbarmax->Max);
    } else {
      SetTrackBarMaxChange(ptrackbarmax->Position,ptrackbarmax->Min,ptrackbarmax->Max);
    }
  }

  // reset the marker that prevent subsequent calls of the OnTrackBarMaxChange function
  btrackbarmaxignore = false;

}


///////////////////////////////////////////////////////////////////////////////
/// Registers the renderer that renders this electrode collection.
/// @param prenderer Handle to the identity of the renderer. 
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::RegisterRenderer(CElectrodeRenderer *prenderer)
{
  this->prenderer = prenderer;
}

///////////////////////////////////////////////////////////////////////////////
/// Returns #bcommonslider.
///////////////////////////////////////////////////////////////////////////////
bool CElectrodeCollection::IsCommonSlider()
{
  return bcommonslider;
}

///////////////////////////////////////////////////////////////////////////////
/// Message handler for click on #pupdownmin.
/// Adjusts the minimum range of the sliders #ptrackbarmin and #ptrackbarmax.
/// increasing/decreasing by 50% at each click.
///////////////////////////////////////////////////////////////////////////////
void __fastcall CElectrodeCollection::OnUpDownMinClick(TObject *Sender, TUDBtnType Button)
{
  if (Button == Comctrls::btNext) {
    if (ptrackbarmin->Min <= -50) {
     // increase range only if below the initial 150% range 
      ptrackbarmin->Min = ptrackbarmin->Min + 50;
      ptrackbarmax->Min = ptrackbarmax->Min + 50;
    }
  } else {
    // decrease range
    ptrackbarmin->Min   = ptrackbarmin->Min - 50;
    ptrackbarmax->Min   = ptrackbarmax->Min - 50;
  }

  btrackbarminignore = false;
  btrackbarmaxignore = false;

  // adjust the sliders
  OnTrackBarMinChange(NULL);
  OnTrackBarMaxChange(NULL);

  // adjust the min/max range
  SetTrackBarMinChange(ptrackbarmin->Position,ptrackbarmin->Min,ptrackbarmin->Max);
  SetTrackBarMaxChange(ptrackbarmax->Position,ptrackbarmax->Min,ptrackbarmax->Max);

  // render changes
  RenderTrackBar();
}

///////////////////////////////////////////////////////////////////////////////
/// Message handler for click on #pupdownax.
/// Adjusts the maximum range of the sliders #ptrackbarmin and #ptrackbarmax.
/// increasing/decreasing by 50% at each click.
///////////////////////////////////////////////////////////////////////////////
void __fastcall CElectrodeCollection::OnUpDownMaxClick(TObject *Sender, TUDBtnType Button)
{
  if (Button == Comctrls::btNext) {
    // increase range
    ptrackbarmin->Max = ptrackbarmin->Max + 50;
    ptrackbarmax->Max = ptrackbarmax->Max + 50;
  } else {
    // decrease range only if above the initial 150% range 
    if (ptrackbarmax->Max >= 150) {
      ptrackbarmin->Max = ptrackbarmin->Max - 50;
      ptrackbarmax->Max = ptrackbarmax->Max - 50;
    }
  }

  btrackbarminignore = false;
  btrackbarmaxignore = false;

  // adjust the sliders
  OnTrackBarMinChange(NULL);
  OnTrackBarMaxChange(NULL);

  // adjust the min/max range
  SetTrackBarMinChange(ptrackbarmin->Position,ptrackbarmin->Min,ptrackbarmin->Max);
  SetTrackBarMaxChange(ptrackbarmax->Position,ptrackbarmax->Min,ptrackbarmax->Max);

  // render changes
  RenderTrackBar();
}

///////////////////////////////////////////////////////////////////////////////
/// Changes the color of the title label #plabel to red or black.
/// @param bhighlit Color will be red (highlighted) if true else black.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::Highlight(bool bhighlight)
{
  // only perform color change and subsequent rendering if bhighlight changes
  if (!this->bhighlight && bhighlight) {
    this->colortitle = clRed;
    this->bhighlight = bhighlight;
    RenderFramework();
  } else if (this->bhighlight && !bhighlight) {
    this->colortitle = clBlack;
    this->bhighlight = bhighlight;
    RenderFramework();
  } else {
    // do nothing
  }

}

///////////////////////////////////////////////////////////////////////////////
/// Checks if coordinates x and y are within the rectangle defined by the
/// parameters (x1,y1)(x2,y2).
///////////////////////////////////////////////////////////////////////////////
bool CElectrodeCollection::Within(int x, int y, int x1, int y1, int x2, int y2)
{
  if (x >= x1 && x <= x2 && y >= y1 && y <= y2) {
    return true;
  } else {
    return false;
  }

}

///////////////////////////////////////////////////////////////////////////////
/// Hides all VCL controls.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::Hide()
{
  this->pboundingrectangle->Visible=false;
  this->ptitle->Visible=false;
  this->pscalex->Visible=false;
  this->pscalexleft->Visible=false;
  this->pscalexright->Visible=false;
  this->pscaley->Visible=false;
  this->pscaleytop->Visible=false;
  this->pscaleybottom->Visible=false;
  this->pdetails->Visible=false;
  this->ptrackbarmax->Visible=false;
  this->ptrackbarmin->Visible=false;
  this->pupdownmin->Visible=false;
  this->pupdownmax->Visible=false;
  this->ptextmin->Visible=false;
  this->ptextmax->Visible=false;
}


///////////////////////////////////////////////////////////////////////////////
/// Shows all VCL controls.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::Show()
{
  this->pboundingrectangle->Visible=true;
  this->ptitle->Visible=true;
  this->pscalex->Visible=true;
  this->pscalexleft->Visible=true;
  this->pscalexright->Visible=true;
  this->pscaley->Visible=true;
  this->pscaleytop->Visible=true;
  this->pscaleybottom->Visible=true;
  this->pdetails->Visible=true;
  this->ptrackbarmax->Visible=true;
  this->ptrackbarmin->Visible=true;
  this->pupdownmin->Visible=true;
  this->pupdownmax->Visible=true;
  this->ptextmin->Visible=true;
  this->ptextmax->Visible=true;
}

///////////////////////////////////////////////////////////////////////////////
/// Returns the defined minimum value of the electrodes that will be mapped
/// into a circle in device coordinates. 
///////////////////////////////////////////////////////////////////////////////
float CElectrodeCollection::GetMinValueElectrodes()
{
  return minvalueelectrodes;
}

///////////////////////////////////////////////////////////////////////////////
/// Returns the defined maximum value of the electrodes that will be mapped
/// into a circle in device coordinates.
///////////////////////////////////////////////////////////////////////////////
float CElectrodeCollection::GetMaxValueElectrodes()
{
  return maxvalueelectrodes;
}


///////////////////////////////////////////////////////////////////////////////
/// Estimates the minimum and maximum of all electrodes and uses an 1st-order
/// IIR filter with the filter coefficient 0.99 to age the previously estimated
/// value with the new estimate. The estimation is based only on the channels
/// defined in #vautoscalechannellist. If this list is empty all avialible
/// channels are used.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::EstimateElectrodeMinMax()
{
  // initialize variables
  float value;
  float maxvalueallelectrodes = -1e32;
  float minvalueallelectrodes =  1e32;
//  float learningrate          = 0.99;


  // if list of channels to be used is empty use all avialible channels 
  if (vautoscalechannellist.size() == 0) {

    // find the minimum and maximum
    for (int index=0; index<GetSize(); index++) {
      value = electrodes[index]->GetValue();

      if (value > maxvalueallelectrodes) {
        maxvalueallelectrodes = value;
      } else if (value < minvalueallelectrodes) {
        minvalueallelectrodes = value;
      }
    }

  } else {

    // use only the channels defined in vautoscalechannellist to learn the
    // minimum and maximum
    for (int unsigned index=0; index<vautoscalechannellist.size(); index++) {
      value = electrodes[vautoscalechannellist[index]-1]->GetValue();

      if (value > maxvalueallelectrodes) {
        maxvalueallelectrodes = value;
      }
      if (value < minvalueallelectrodes) {
        minvalueallelectrodes = value;
      }
    }

  }


  if (!firsttimelearned) {
    // when this is the first time called used the estimated minmum/maximum
    this->maxvalueallelectrodes = maxvalueallelectrodes;
    this->minvalueallelectrodes = minvalueallelectrodes;
    firsttimelearned = true;
  } else {
    // when this was called before age the previously estimated value using
    // the 1st-order IIR filter with the filter coefficient 0.99.
    this->maxvalueallelectrodes = this->maxvalueallelectrodes * learningrateautoscale + maxvalueallelectrodes * (1-learningrateautoscale);
    this->minvalueallelectrodes = this->minvalueallelectrodes * learningrateautoscale + minvalueallelectrodes * (1-learningrateautoscale);
  }


}

///////////////////////////////////////////////////////////////////////////////
/// This function is called to signal that all electrode value were set.
/// Performs the autoscaling and estimation of the minimum and maximum value
/// of all or only the electrodes defined in #vautoscalechannellist.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::Process(bool bsilent)
{
  AnsiString szminvalueelectrodes;
  AnsiString szmaxvalueelectrodes;
  float trackbarminposition;
  float trackbarmaxposition;

  // estimate the minimum and maximum of all or only the electrodes
  // defined in vautoscalechannellist.
  if (!bsilent) {
    EstimateElectrodeMinMax();
  }

  // in the case of autoscaling
  if (bautoscale) {

    bcommonslider = false;

    minvalueelectrodes = minvalueallelectrodes;
    maxvalueelectrodes = maxvalueallelectrodes;

    // update the electrodes with the new scaling range
    for (unsigned int index=0; index < electrodes.size(); index++) {
      electrodes[index]->RenderUpdate(minvalueelectrodes,
                                      maxvalueelectrodes);
    }



    // calculate the new trackbar position
    trackbarminposition = minvalueallelectrodes / maxvalueelectrodesinitial * 100.0;
    trackbarmaxposition = maxvalueallelectrodes / maxvalueelectrodesinitial * 100.0;

    // if the lower range is not sufficient to accomodate the range call
    // OnUpDownMinClick to extend the lower range
    if (trackbarminposition < ptrackbarmin->Min) {
      OnUpDownMinClick(NULL,Comctrls::btPrev);
    }

    // if the higher range is not sufficient to accomodate the range call
    // OnUpDownMaxClick to extend the lower higher
    if (trackbarmaxposition > ptrackbarmax->Max) {
      OnUpDownMaxClick(NULL,Comctrls::btNext);
    }

    // set the new trackpar position
    ptrackbarmin->Position = trackbarminposition;
    ptrackbarmax->Position = trackbarmaxposition;

    // update the text for the minimum/maximum 
    ptextmin->Caption                = szminvalueelectrodes.sprintf("%5.2f (%5.2f)",minvalueelectrodes,minvalueallelectrodes);
    ptextmax->Caption                = szmaxvalueelectrodes.sprintf("%5.2f (%5.2f)",maxvalueelectrodes,maxvalueallelectrodes);


  }

}


///////////////////////////////////////////////////////////////////////////////
/// Message handler for click on the #pcheckboxautoscale control.
/// Reads the current status of the #pcheckboxautoscale control and
/// enables/disables the scaling trackbars and up/down controls in the
/// case of autoscale being enabled.
///////////////////////////////////////////////////////////////////////////////
void __fastcall CElectrodeCollection::OnAutoscaleClick(TObject *Sender)
{

  this->bautoscale = pcheckboxautoscale->Checked;

  if (bautoscale) {
    // autoscale enabled
    this->ptrackbarmax->Enabled = false;
    this->ptrackbarmin->Enabled = false;
    this->pupdownmax->Enabled   = false;
    this->pupdownmin->Enabled   = false;
  } else {
    // autoscale disabled
    this->ptrackbarmax->Enabled = true;
    this->ptrackbarmin->Enabled = true;
    this->pupdownmax->Enabled   = true;
    this->pupdownmin->Enabled   = true;
  }

}


///////////////////////////////////////////////////////////////////////////////
/// Calle by the form owner to inform all electrode collections about the
/// current mouse position in the case of an OnMouseMove event.
///////////////////////////////////////////////////////////////////////////////
void CElectrodeCollection::SetCursorPos(int cursorposx, int cursorposy)
{
  this->cursorposx = cursorposx;
  this->cursorposy = cursorposy;

  // invalidates the electrode index since the OnMouseMove event occoured
  // outside of the electrode collection.
  indexdetails = -1;

  // renders the updated details information
  RenderDetails();

//  OnAutoscaleClick(NULL);

}
