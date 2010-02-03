////////////////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Main form for an application demonstrating the use of
//   the 3D Scene GraphObject class.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef UDemoWindowH
#define UDemoWindowH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "GraphDisplay.h"
#include <ExtCtrls.hpp>

class Scene;
class sceneObj;
class twoDText;
class twoDCursor;
class twoDOverlay;

//---------------------------------------------------------------------------
class TDemoWindow : public TForm
{
__published:    // IDE-managed Components
    TTimer *FrameUpdate;
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall FormResize(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall FrameUpdateTimer(TObject *Sender);

private:    // User declarations
    void __fastcall WMEraseBkgnd( TWMEraseBkgnd& ) {}
    BEGIN_MESSAGE_MAP
      VCL_MESSAGE_HANDLER( WM_ERASEBKGND, TWMEraseBkgnd, WMEraseBkgnd )
    END_MESSAGE_MAP( TForm )

  void SetupScene();
  void ResetPositions();
  void StopMovement();
  void RandomTranslation( float speed );
  void RandomRotation( float speed );
  static void RandomTranslation( sceneObj&, float speed );
  static void RandomRotation( sceneObj&, float speed );
  static float Randf();
  static void OnCollide( sceneObj&, sceneObj& );

  sceneObj*    mpBound,
          *    mpEarth,
          *    mpMoon,
          *    mpCube,
          *    mpCuboid,
          *    mp3DText,
          *    mpFace;
  twoDText*    mpHelpText,
          *    mpFPSText;
  twoDCursor*  mpCursor;
  twoDOverlay* mpOverlay;

  float mFieldOfView;

  Scene*            mpScene;
  GUI::GraphDisplay mGraphDisplay;
  
public:     // User declarations
    __fastcall TDemoWindow(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TDemoWindow *DemoWindow;
//---------------------------------------------------------------------------
#endif
