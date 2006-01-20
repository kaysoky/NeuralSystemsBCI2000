//---------------------------------------------------------------------------

#ifndef UTargetH
#define UTargetH

#define TARGETID_NOID           -999
#define TARGETID_ROOT           -1
#define TARGETID_BLANK          0
#define TARGETID_BACKUP         1
#define TARGETID_A              14
#define TARGETID__              40
#define TARGETID_ABCDEFGHI      2
#define TARGETID_YZ_            13

#define TARGETTYPE_NOTYPE       -1
#define TARGETTYPE_WORD         1
#define TARGETTYPE_CHARACTERS   2
#define TARGETTYPE_CHARACTER    3
#define TARGETTYPE_CONTROL      4
#define TARGETTYPE_DELETE       5

#define MODE_NORMAL             1
#define MODE_PREDICTION         2

#include <syncobjs.hpp>

#include "UCursor.h"
#include "WavePlayer.h"

class TARGET
{
private: 	// User declarations
        TShape  *shape;
        TImage  *icon;
        TLabel  *caption;

        //VK
        TImage  *mono_icon;
	TWavePlayer *wavplayer;

        /*shidong starts*/
        void __fastcall clickTarget( TObject* );
        /*shidong ends*/
public:		// User declarations
        TARGET::TARGET(int my_targetID);
        TARGET::~TARGET();
        TARGET  *CloneTarget();
        TARGET  *TargetSelected(CURSOR *cursor);
        AnsiString Caption, IconFile;
        int     targetID, parentID;
        int     targettype;
        BYTE    targetposition;
        int     Top, Left, Width, Height;
        int     TextHeight;
        TColor  Color, TextColor;
        TPen    *Pen;
        void    RenderTarget(TForm *form, TRect destrect);
        void    HideTarget();
        void    ShowTarget();
        void    HighlightTarget();
        void    SetTextColor(TColor new_color);
        /*shidong starts*/
        AnsiString CharDisplayInMatrix;
        AnsiString CharDisplayInResult;
        float FontSizeFactor;
        bool    clickedOn;
        bool    IsClickedOn();

        /*shidong ends*/
        // VK
	void	HighlightIcon(bool intensify);
        AnsiString IconHighlightMethod;
	float   IconHighlightFactor;
	AnsiString SoundFile;
	void 	PlaySound();
};


class TARGETLIST
{
private: 	// User declarations
        TCriticalSection        *critsec;               // critical section for screen update
        TList   *target_list;
public:		// User declarations
        TARGETLIST::TARGETLIST();
        TARGETLIST::~TARGETLIST();
        bool    Add(TARGET *new_target);
        int     parentID;
        int     predictionmode;
        TARGET  *GetTargetPtr(AnsiString caption);
        TARGET  *GetTargetPtr(int my_targetID);
        TARGET  *GetTargetPtr(int my_targetID, int displaypos);
        TARGET  *TargetsSelected(CURSOR *cursor);
        int     GetTargetID(int displaypos);
        int     GetTargetID(AnsiString caption, int targettype);
        int     GetMaxTargetID();
        void    DeleteTargets();
        void    HideTargets();
        void    ShowTargets();
        void    HighlightTargets();
        void    RenderTargets(TForm *form, TRect destrect);
        int     GetCurrentBackupPosition();
        int     GetNumTargets();
};
//---------------------------------------------------------------------------
#endif

