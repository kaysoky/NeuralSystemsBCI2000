/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
//---------------------------------------------------------------------------

#ifndef UMainH
#define UMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "CGAUGES.h"
#include <Dialogs.hpp>
#include <vector>
//---------------------------------------------------------------------------

class TfMain : public TForm
{
__published:	// IDE-managed Components
        TButton *bConvert;
        TEdit *eDestinationFile;
        TCGauge *Gauge;
        TOpenDialog *OpenDialog;
        TButton *bOpenFile;
        TButton *Button1;
        TSaveDialog *SaveDialog;
        TButton *Button2;
        TEdit *ParameterFile;
        TOpenDialog *OpenParameter;
        TMemo *mSourceFiles;
    TCheckBox *AutoscaleCheckbox;
    TGroupBox *mOptionsGroupBox;
    TLabel *Label3;
    TGroupBox *mInputGroupBox;
    TGroupBox *mOutputGroupBox;
    TLabel *mProgressLegend;
    TCheckBox *mSubSetCheckbox;
    TEdit *mSubSetEdit;
    TLabel *Label1;

        void __fastcall bConvertClick(TObject *Sender);
        void __fastcall bOpenFileClick(TObject *Sender);
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall Button2Click(TObject *Sender);
        void __fastcall mSourceFilesChange(TObject *Sender);
    void __fastcall mSubSetCheckboxClick(TObject *Sender);
    void __fastcall mSubSetEditChange(TObject *Sender);

private:	// User declarations
        typedef __int16 gab_type;
        const short* mpGabTargets;
        int mTargetMax;
        __int16 ConvertState(BCI2000DATA *bci2000data);
        enum { eInvalidState = -1 };

        std::vector<float> mOffset, mGain;
        bool CalibFromFile() { return mGain.size() != 0; }
        void __fastcall CheckCalibrationFile( void );

        void __fastcall DoStartupProcessing( TObject*, bool& );
        bool mBatchMode;

        int __fastcall UserChannels( int defaultChannels ) const;

        typedef enum { Message, Warning, Error } msgtypes;
        void UserMessage( const AnsiString&, msgtypes ) const;

        void __fastcall SourceFilesWindowProc( TMessage& msg );
        TWndMethod defaultSourceFilesWindowProc;

public:		// User declarations
        __fastcall TfMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif


