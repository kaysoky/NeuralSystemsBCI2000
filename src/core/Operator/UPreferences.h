/* (C) 2000-2010, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef UPreferencesH
#define UPreferencesH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>

#define USERLEVEL_BEGINNER      1
#define USERLEVEL_INTERMEDIATE  2
#define USERLEVEL_ADVANCED      3

#define USERLEVELTXT_BEGINNER           "Beginner"
#define USERLEVELTXT_INTERMEDIATE       "Intermediate"
#define USERLEVELTXT_ADVANCED           "Advanced"

class PREFERENCES
{
private:  // User declarations
public:   // User declarations
        PREFERENCES();

        int     UserLevel;

        enum
        {
          AfterModulesConnected = 0,
          OnSetConfig,
          OnExit,
          OnResume,
          OnSuspend,
          OnStart,
          numScriptEvents
        };
        AnsiString Script[ numScriptEvents ];
        bool       mCmdlineSpecified[ numScriptEvents ];

        enum
        {
          numButtons = 4,
        };
        struct
        {
          AnsiString Name,
                     Cmd;
        } Buttons[ numButtons + 1 ];

        void          GetDefaultSettings();
        void          SetDefaultSettings();
};



//---------------------------------------------------------------------------
class TfPreferences : public TForm
{
__published:  // IDE-managed Components
        TTrackBar *TrackBar1;
        TLabel *Label1;
        TLabel *tUserLevel;
        TButton *bClose;
        TBevel *Bevel1;
        TLabel *Label2;
        TLabel *Label3;
        TEdit *eAfterModulesConnected;
        TEdit *eOnSetConfig;
        TLabel *Label4;
        TLabel *Label5;
        TEdit *eOnResume;
        TLabel *Label6;
        TEdit *eOnSuspend;
        TLabel *Label7;
        TEdit *eOnStart;
        TBevel *Bevel2;
        TLabel *Label8;
        TLabel *Label9;
        TEdit *eButton1Name;
        TLabel *Label10;
        TLabel *Label11;
        TLabel *Label12;
        TEdit *eButton1Cmd;
        TLabel *Label13;
        TLabel *Label14;
        TEdit *eButton2Name;
        TEdit *eButton2Cmd;
        TEdit *eButton3Name;
        TEdit *eButton3Cmd;
        TEdit *eButton4Name;
        TEdit *eButton4Cmd;
        TLabel *Label15;
        TEdit *eOnExit;
        void __fastcall TrackBar1Change(TObject *Sender);
        void __fastcall bCloseClick(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
private:  // User declarations
public:   // User declarations
        __fastcall TfPreferences(TComponent* Owner);
        PREFERENCES *preferences;
};
//---------------------------------------------------------------------------
extern PACKAGE TfPreferences *fPreferences;
//---------------------------------------------------------------------------
#endif

