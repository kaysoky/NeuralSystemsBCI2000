//---------------------------------------------------------------------------

#ifndef SWUserH
#define SWUserH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <math.h>
#include <MPlayer.hpp>
#include "UGenericVisualization.h"
#include "UGenericFilter.h"
#include "mmsystem.h"

#define GMFBChannel   0
#define GMBIChannel   1
#define GMFIChannel   2
#define GMRIChannel   3
#define GMIVChannel   4
#define GMDEVICEID    1

//------------------ Speller Class Definition ------------------------
//                  programed by Dr. Thilo Hinterberger 2000
//--------------------------------------------------------------------------------------------------

class TSTree {    // defines a speller-tree
public:
        int MaxIndex;
        int CurIndex;
        int OnSelect[255];
        int OnReject[255];
        int OnNoChoice[255];
        AnsiString Caption[255];
        AnsiString WrittenText;
        bool LoadTree(char *FName);
        void ClearText();
};

//---------------------------------------------------------------------------

class TFBForm : public TForm
{
__published:	// IDE-managed Components
        TShape *UpperGoal;
        TShape *LowerGoal;
        TShape *Ball;
        TShape *MiddleGoal;
        TLabel *BottomText;
        TLabel *TopText;
        TPaintBox *PB1;
        TShape *ZeroBar;
        TTimer *GMTimer;
        TMediaPlayer *MediaPlayer1;
        TMediaPlayer *MediaPlayer2;
        TMediaPlayer *MediaPlayer4;
        TMediaPlayer *ResponsePlayer1;
        TMediaPlayer *ResponsePlayer4;
        TMediaPlayer *QuestionPlayer;
        TLabel *Label1;
        void __fastcall FormResize(TObject *Sender);
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall FormDestroy(TObject *Sender);
        void __fastcall GMTimerTimer(TObject *Sender);
private:	// User declarations
        PARAMLIST *paramlist;
        STATEVECTOR *statevector;
        TSTree *STree;
        bool TaskActive;
        bool BIState;
        int VisOptionMode;
        int AudOptionMode;
        int VisReinforcement;
        int AudReinforcement;
        int VisTaskMode;
        int AudTaskMode;
        int VisResultMode;
        int AudResultMode;
        int VisMarker;
        int AudMarker;
        int VisFBMode;
        int AudFBMode;
        int VisInvalid;
        int AudInvalid;

        int PosInTrial;
        int FBBegin;
        int FBEnd;
        int ZeroPosition;
        int XStepSize;
    // MIDI variables
        float GMFBInterval;
        int GMFBInstrument;
        int GMFBVelocity;
        int GMFBCenterNote;
        int GMBIInstrument;
        int GMBIVelocity;
        int GMBINote;
        int GMFIInstrument;
        int GMFIVelocity;
        int GMFINote;
        int GMRIInstrument;
        int GMRIVelocity;
        int GMIVInstrument;
        int GMIVVelocity;
        int GMIVNote;
        unsigned long result;
        HMIDISTRM outHandle;
        MIDIHDR midiheader;
        int ProCounter;

        int CalcAcNote(int FBValue);
        void Reinforce();
	void Invalid();
        void ShowTask();
        void ShowLetters2Select();
        void ShowResult();
        void ShowFB(int FBValue);
        AnsiString OddballStr[10];
        void ReInitialize(); //Initialize after Resize

public:		// User declarations
        __fastcall TFBForm(TComponent* AOwner);
        void SetFBForm(PARAMLIST *NewParamlist, STATELIST *statelist);
        void Initialize(STATEVECTOR *Newstatevector);
        bool LoadTree(char *FName);
        void Process(int FBValue);
};
//---------------------------------------------------------------------------
extern PACKAGE TFBForm *FBForm;
//---------------------------------------------------------------------------
#endif

