//---------------------------------------------------------------------------

#ifndef BCI2000CertificationGUIH
#define BCI2000CertificationGUIH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>

#include "CertLauncher.h"
#include "Functions.h"
#include <Dialogs.hpp>
#include <vector>
#include <string>
#include <stdio.h>
#include <sstream>

using namespace std;


//---------------------------------------------------------------------------
class TBCICertificationGUI : public TForm
{
__published:	// IDE-managed Components
	TGroupBox *GroupBox1;
	TGroupBox *GroupBox2;
	TListView *taskList;
	TButton *addTaskBtn;
	TButton *delTaskBtn;
	TGroupBox *GroupBox3;
	TGroupBox *GroupBox4;
	TLabeledEdit *taskNameBox;
	TLabeledEdit *ampBox;
	TLabeledEdit *digAmpBox;
	TLabeledEdit *vidBox;
	TLabeledEdit *vidStateBox;
	TLabeledEdit *vidStateValuesBox;
	TLabeledEdit *audioBox;
	TLabeledEdit *audioStateBox;
	TLabeledEdit *audioStateValuesBox;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TLabeledEdit *winLeftBox;
	TLabeledEdit *winWidthBox;
	TLabeledEdit *winHeightBox;
	TLabel *Label4;
	TButton *addPrmBtn;
	TButton *delPrmBtn;
	TOpenDialog *OpenDialog1;
	TListView *parmsList;
	TEdit *dataSaveBox;
	TLabel *Label6;
	TButton *dataSaveBtn;
	TButton *startBtn;
	TButton *cancelBtn;
	TButton *analyzeBtn;
	TEdit *sigSourceBox;
	TButton *getSigSrcBtn;
	TEdit *sigProcBox;
	TButton *getSigProcBtn;
	TEdit *appBox;
	TButton *getAppBtn;
	TEdit *globalSigSrcBox;
	TButton *getGlobSigSrcBtn;
	TMemo *infoBox;
	TLabeledEdit *winTopBox;
	void __fastcall addPrmBtnClick(TObject *Sender);
	void __fastcall dataSaveBtnClick(TObject *Sender);
	void __fastcall taskListClick(TObject *Sender);
	void __fastcall parmsListInfoTip(TObject *Sender, TListItem *Item,
          AnsiString &InfoTip);
	void __fastcall taskNameBoxExit(TObject *Sender);
	void __fastcall sigSourceBoxExit(TObject *Sender);
	void __fastcall sigProcBoxExit(TObject *Sender);
	void __fastcall appBoxExit(TObject *Sender);
	void __fastcall ampBoxExit(TObject *Sender);
	void __fastcall digAmpBoxExit(TObject *Sender);
	void __fastcall vidBoxExit(TObject *Sender);
	void __fastcall vidStateBoxExit(TObject *Sender);
	void __fastcall vidStateValuesBoxExit(TObject *Sender);
	void __fastcall audioBoxExit(TObject *Sender);
	void __fastcall audioStateBoxExit(TObject *Sender);
	void __fastcall audioStateValuesBoxExit(TObject *Sender);
	void __fastcall getSigSrcBtnClick(TObject *Sender);
	void __fastcall getSigProcBtnClick(TObject *Sender);
	void __fastcall getAppBtnClick(TObject *Sender);
	void __fastcall winLeftBoxExit(TObject *Sender);
	void __fastcall winTopBoxExit(TObject *Sender);
	void __fastcall winWidthBoxExit(TObject *Sender);
	void __fastcall winHeightBoxExit(TObject *Sender);
	void __fastcall dataSaveBoxExit(TObject *Sender);
	void __fastcall startBtnClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall cancelBtnClick(TObject *Sender);
	void __fastcall globalSigSrcBoxExit(TObject *Sender);
	void __fastcall addTaskBtnClick(TObject *Sender);
	void __fastcall delTaskBtnClick(TObject *Sender);
private:	// User declarations
	bool init();
	void updateParmPanel();
	void updateParm();
	void updateGlobal();

	class RunThread;
	friend class RunThread;
	class RunThread : public TThread
	{
		friend class TBCICertificationGUI;
		public:
			bool threadRunning;
			__fastcall RunThread(bool, TBCICertificationGUI* );
		protected:
			void __fastcall Execute();
			TBCICertificationGUI* p;
	};
	RunThread *runThread;

	int mCurTask;

	CertLauncher mCT;
	string mDatDir, mResFile;
	double mThresh;
	vector<basicStats> mMinReqs;
public:		// User declarations
	__fastcall TBCICertificationGUI(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TBCICertificationGUI *BCICertificationGUI;
//---------------------------------------------------------------------------
#endif
