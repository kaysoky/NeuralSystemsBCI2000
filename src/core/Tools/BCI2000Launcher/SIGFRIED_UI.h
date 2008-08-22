//---------------------------------------------------------------------------

#ifndef SIGFRIED_UIH
#define SIGFRIED_UIH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <FileCtrl.hpp>
#include <ActnList.hpp>
#include <ActnMan.hpp>
#include <ExtActns.hpp>
#include <Grids.hpp>
#include <Menus.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib>
#include <dir.h>

#include "EncodedString.h"
#include "functions.h"
using namespace std;

//---------------------------------------------------------------------------
class TSigfried_UIfrm : public TForm
{
__published:	// IDE-managed Components
    TGroupBox *GroupBox1;
    TEdit *directoryBox;
    TLabel *Label9;
    TButton *getDirBtn;
    TLabel *Label11;
    TEdit *sessionNumBox;
    TButton *baselinePrmBtn;
    TEdit *baselinePrmBox;
    TLabel *Label1;
    TGroupBox *GroupBox2;
    TButton *recordBaselineBtn;
    TLabel *Label2;
    TGroupBox *GroupBox3;
    TOpenDialog *OpenFileDlg;
    TLabel *Label3;
    TEdit *modelFileBox;
    TButton *modelDirBtn;
    TLabel *Label4;
    TEdit *modelIniBox;
    TButton *buildModelBtn;
    TGroupBox *GroupBox4;
    TEdit *parmOutputBox;
    TLabel *Label5;
    TButton *parmOutputBtn;
    TButton *returnBtn;
    TSaveDialog *SaveFileDlg;
    TPopupMenu *PopupMenu1;
    TListBox *modelList;
    TEdit *modelDescBox;
    TLabel *Label6;
    TButton *addModelBtn;
    TButton *remModelBtn;
    TEdit *visModelWidthBox;
    TEdit *visModelHeightBox;
    TLabel *Label7;
    TLabel *Label8;
    TCheckBox *visModelsCheck;
    TEdit *baselineFileBox;
    TButton *modelIniBtn;
    TButton *getBaselineFileBtn;
    TLabel *Label10;
    void __fastcall getDirBtnClick(TObject *Sender);
    void __fastcall sessionNumBoxExit(TObject *Sender);
    void __fastcall sessionNumBoxKeyPress(TObject *Sender, char &Key);
    void __fastcall baselinePrmBtnClick(TObject *Sender);
    void __fastcall directoryBoxChange(TObject *Sender);
    void __fastcall baselinePrmBoxChange(TObject *Sender);
    void __fastcall recordBaselineBtnClick(TObject *Sender);
    void __fastcall modelIniBtnClick(TObject *Sender);
    void __fastcall modelDirBtnClick(TObject *Sender);
    void __fastcall modelFileBoxChange(TObject *Sender);
    void __fastcall modelIniBoxChange(TObject *Sender);
    void __fastcall buildModelBtnClick(TObject *Sender);
    void __fastcall addModelBtnClick(TObject *Sender);
    void __fastcall remModelBtnClick(TObject *Sender);
    void __fastcall parmOutputBtnClick(TObject *Sender);
    void __fastcall parmOutputBoxChange(TObject *Sender);
    void __fastcall returnBtnClick(TObject *Sender);
    void __fastcall getBaselineFileBtnClick(TObject *Sender);
    void __fastcall baselineFileBoxChange(TObject *Sender);
private:	// User declarations

    struct Model
    {
        string iniFile;
        string modelOutput;
        string description;
    };
    vector<Model> models;
    void updateModelList();
    string mSigSource;
    string mProgDir;
    int mStatus;
    string mParmFile, mIniFile, mSigfriedFile;
    string mModelFile;
    string mBaselineFile;
    string mParmOutputFile;
    bool mRecordReady, mModelReady;
    void updateRecordReady();
public:		// User declarations
    __fastcall TSigfried_UIfrm(TComponent* Owner, string SigSource, string progDir);
    int Status(){return mStatus;};
    string ParmFile(){return mParmOutputFile;};
};
//---------------------------------------------------------------------------
extern PACKAGE TSigfried_UIfrm *Sigfried_UIfrm;
//---------------------------------------------------------------------------
#endif
