////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: The BCI2000 Viewer application's main window.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef MainWindowH
#define MainWindowH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ActnList.hpp>
#include <CheckLst.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>

#include "BCI2000FileReader.h"
#include "SignalDisplay.h"
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
 public:
  __fastcall TMainForm(TComponent* Owner);
  __fastcall ~TMainForm();

  __published: // IDE-managed Components
    TButton *mPageFwd;
    TButton *mPageRew;
    TButton *mBlockRew;
    TButton *mBlockFwd;
    TButton *mToBegin;
    TButton *mToEnd;
    TBevel *mBevel;
    TPaintBox *mSignalArea;
    TEdit *mEditPosition;
    TButton *mSampleZoomIn;
    TButton *mSampleZoomOut;
    TScrollBar *mVerticalScroller;
    TMainMenu *mMainMenu;
    TMenuItem *mFileMenu;
    TMenuItem *mEditMenu;
    TMenuItem *mEditCopy;
    TMenuItem *mViewMenu;
    TMenuItem *mHelpMenu;
    TMenuItem *mFileOpen;
    TMenuItem *mFileQuit;
    TMenuItem *mHelpOpenHelp;
    TMenuItem *mHelpAbout;
    TMenuItem *mEditCut;
    TMenuItem *mEditPaste;
    TMenuItem *mViewEnlargeSignal;
    TMenuItem *mViewReduceSignal;
    TMenuItem *N1;
    TMenuItem *mViewFewerChannels;
    TMenuItem *mViewMoreChannels;
    TMenuItem *N2;
    TMenuItem *mViewChooseChannelColors;
    TMenuItem *mViewInvert;
    TMenuItem *mViewShowBaselines;
    TMenuItem *mViewShowUnit;
    TMenuItem *N4;
    TMenuItem *mViewZoomOut;
    TMenuItem *mViewZoomIn;
    TCheckListBox *mChannelListBox;
    TActionList *mActionList;
    TPopupMenu *mListBoxPopupMenu;
    TMenuItem *mShowChannel;
    TMenuItem *mHideChannel;
    TMenuItem *mFileClose;
    TLabel *mDragDropHint;
    TMenuItem *mHelpOnChannel;
    TMenuItem *N3;
    void __fastcall DragDropWindowProc( TMessage& msg );
    void __fastcall FormCanResize(TObject *Sender, int &NewWidth,
          int &NewHeight, bool &Resize);
    void __fastcall FormResize(TObject *Sender);
    void __fastcall SignalAreaPaint(TObject *Sender);
    void __fastcall EditPositionExit(TObject *Sender);
    void __fastcall EditPositionKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall ChannelListBoxClickCheck(TObject *Sender);
    void __fastcall VerticalScrollerChange(TObject *Sender);
    void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall HelpOnChannelClick(TObject *Sender);
    void __fastcall mChannelListBoxContextPopup(TObject *Sender,
          TPoint &MousePos, bool &Handled);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);

  private:
    void __fastcall WMEraseBkgnd( TWMEraseBkgnd& );
    BEGIN_MESSAGE_MAP
      VCL_MESSAGE_HANDLER( WM_ERASEBKGND, TWMEraseBkgnd, WMEraseBkgnd )
    END_MESSAGE_MAP( TForm )

    static struct ActionEntry
    {
      typedef void (TMainForm::*Action)();
      typedef bool (TMainForm::*StateGetter)() const;
      Action      action;
      StateGetter enabled,
                  checked;
      const char* control;
    } sActions[];
    void __fastcall ActionUpdateHandler( TObject* );
    void __fastcall ActionExecuteHandler( TObject* );
    void SetupActions();

  private:
    // User actions
    // Standard actions
    void FileOpen();
    void FileClose();
    bool FileClose_Enabled() const;
    void FileQuit();
    void EditCopy();
    bool EditCopy_Enabled() const;
    void HelpOpenHelp();
    void HelpAbout();
    // Temporal movement
    void ToFirstSample();
    void ToLastSample();
    void ToPrevBlock();
    void ToNextBlock();
    void ToPrevPage();
    void ToNextPage();
    bool GoBack_Enabled() const;
    bool GoForward_Enabled() const;
    // Temporal resolution
    void SampleZoomIn();
    bool SampleZoomIn_Enabled() const;
    void SampleZoomOut();
    bool SampleZoomOut_Enabled() const;
    // Number of displayed channels
    void FewerChannels();
    bool FewerChannels_Enabled() const;
    void MoreChannels();
    bool MoreChannels_Enabled() const;
    // Channel scrolling
    void ChannelUp();
    void ChannelDown();
    void ChannelPageNext();
    void ChannelPagePrev();
    void ChannelPageFirst();
    void ChannelPageLast();
    bool ChannelUp_Enabled();
    bool ChannelDown_Enabled();
    // Signal resolution
    void EnlargeSignal();
    void ReduceSignal();
    bool ChangeResolution_Enabled() const;
    // Display attributes
    void ChooseChannelColors();
    bool ChooseChannelColors_Enabled() const;
    void Invert();
    bool Invert_Checked() const;
    bool Invert_Enabled() const;
    void ToggleBaselines();
    bool ToggleBaselines_Checked() const;
    bool ToggleBaselines_Enabled() const;
    void ToggleUnit();
    bool ToggleUnit_Checked() const;
    bool ToggleUnit_Enabled() const;
    // Channel List
    void ShowSelectedChannels();
    void HideSelectedChannels();
    bool ChannelsSelected() const;

    // Internal functions
    void DoFileOpen( const char* name );
    void FillChannelList();
    void RestoreFromRegistry();
    void SaveToRegistry() const;
    void UpdateSignalDisplayContext();
    void UpdateSamplePos();
    void UpdateTimeField();
    void UpdateChannelLabels();
    void UpdateVerticalScroller();
    const GenericSignal& ConstructDisplaySignal( long samplePos, long length );
    void SetSamplePos( long sampleIndex );
    void MoveSamplePos( long sampleIndexDiff );

    static const RGBColor cAxisColor;
    static const RGBColor cChannelColorsDefault[];
    SignalDisplay      mDisplay;
    BCI2000FileReader  mFile;
    long               mSamplePos;
    int                mNumSignalChannels;
    TWndMethod         mDefaultWindowProc;
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif // MainWindowH

