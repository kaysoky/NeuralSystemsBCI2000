//---------------------------------------------------------------------------

#ifndef UsrH
#define UsrH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "UGenericFilter.h"
#include <ExtCtrls.hpp>

/*shidong starts*/
#include <errno.h>
#include "Task.h"
#include <stddef.h>     /* _threadid variable */
#include <process.h>    /* _beginthread, _endthread */
#include "../Application/Shared/3DAPI/Environment3D.cpp"




bool    glWindowRun;
/*shidong end*/

void SetUsr( PARAMLIST *plist, STATELIST *slist);

//---------------------------------------------------------------------------
class TUser : public TForm
{
__published:	// IDE-managed Components
        TShape *Cursor;
        TShape *Target;
        TLabel *tT;
        TLabel *tO;
private:	// User declarations
        int Wx;                 // task window x location
        int Wy;                 // task window y location
        int Wxl;                // task window x size
        int Wyl;                // task window y size
        int CursorSize;         // cursor size
        float cursor_y_start;   // y starting position of cursor
        float cursor_x_start;   // x starting position of cursor
        float limit_top;
        float limit_bottom;
        float limit_left;
        float limit_right;
        float xscalef;
        float yscalef;
       
public:		// User declarations
        bool  TrackingTarget;
        int HalfCursorSize;     // 1/2 of cursor size
        float scalex;
        float scaley;   
        void Scale( float , float );
        float ran1( long *idem );       // from Press et al
        void GetLimits( float *, float *, float *, float * );
        void GetSize( float *, float *, float *, float *);
       // void TestCursorLocation( float x, float y );
        void PutCursor( float *y, float *x, TColor color );      // self explanitory
        void PutTrackingTarget( float *y, float *x, TColor color );      // self explanitory
        void PutTarget( float x, float y, float sizex, float sizey, TColor color, int target );
        void PutT(bool);
        void PutO(bool);
        void Clear( void );
        void __fastcall Initialize(PARAMLIST *plist, STATELIST *slist, AnsiString borderTexture, AnsiString targetTexture, AnsiString cursorTexture,  int totalTarg, bool TrackingTarget);
        __fastcall TUser(TComponent* Owner);
        __fastcall ~TUser();

        /*shidong starts*/
        FILE *a;
        AnsiString cursorColorF;
        AnsiString cursorColorB;
        float startZ;           //curosr initial z coord
        float posZ;
        void suspend();
        void resume();
        void open3D();
        bool boundV;
        void setCursorColor(AnsiString front, AnsiString back);
        void setCursor(float posX, float posY, float posZ, float radius, float clR, float clG, float clB, float bright, AnsiString cTexture);
        void setTarget(int target[][NTARGS], int col, int row, AnsiString tTexture, AnsiString bTexture, int visible);

        float zR;
        float zG;
        float zB;
        void calculateCursorColor(float curPos, AnsiString frontColor, AnsiString backColor);
        int WinHeight;
        int WinWidth;
        int WinXpos;
        int WinYpos;
        void setWindow(int h, int w, int x, int y);

        //camera and light
        int CameraX;
        int CameraY;
        int CameraZ;
        int CameraAimX;
        int CameraAimY;
        int CameraAimZ;
        int LightSourceX;
        int LightSourceY;
        int LightSourceZ;
        int LightSourceColorR;
        int LightSourceColorG;
        int LightSourceColorB;
        int LightSourceintensity;
        void setCameraLight(
                int CameraX,
                int CameraY,
                int CameraZ,
                int CameraAimX,
                int CameraAimY,
                int CameraAimZ,
                int LightSourceX,
                int LightSourceY,
                int LightSourceZ,
                int LightSourceColorR,
                int LightSourceColorG,
                int LightSourceColorB,
                int LightSourceIntensity);
        /*shidong ends*/
};
//---------------------------------------------------------------------------
extern PACKAGE TUser *User;
//---------------------------------------------------------------------------
#endif
