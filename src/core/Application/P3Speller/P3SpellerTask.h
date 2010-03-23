////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: schalk@wadsworth.org, vkamat@cambridgeconsultants.com,
//   pbrunner@wadsworth.org, shzeng, juergen.mellinger@uni-tuebingen.de
// Description: The task filter for a P300 based speller providing multiple
//   menus.
//
// (C) 2000-2010, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef P3_SPELLER_TASK_H
#define P3_SPELLER_TASK_H

#include "StimulusTask.h"
#include "Speller.h"
#include "StatusBar.h"
#include "TextWindow.h"
#include "SockStream.h"

#include <stack>
#include <vector>
#include <ctime>

class P3SpellerTask : public StimulusTask, public Speller
{
 public:
  P3SpellerTask();
  ~P3SpellerTask();

 protected:
  // StimulusTask events
  virtual void    OnPreflight( const SignalProperties& Input ) const;
  virtual void    OnInitialize( const SignalProperties& Input );
  virtual void    OnStartRun();
  virtual void    OnStopRun();
  virtual void    OnPreSequence();
  virtual void    OnSequenceBegin();
  virtual void    OnPostRun();
  virtual void    DoPostSequence( const GenericSignal&, bool& doProgress );
  virtual Target* OnClassResult( const ClassResult& );
  virtual int     OnNextStimulusCode();
  // Speller events
  virtual void    OnEnter( const std::string& );
  // Speller commands dispatched from OnEnter
  virtual void    OnText( const std::string& );
  virtual void    OnBackspace();
  virtual void    OnDeleteWord();
  virtual void    OnUndo();
  virtual void    OnEnd();
  virtual void    OnSleep();
  virtual void    OnPause();
  virtual void    OnGoto( int );
  virtual void    OnBack();
  virtual void    OnSave();
  virtual void    OnRetrieve();

 private:
  void DetermineAttendedTarget();
  void InitSequence();
  void ClearSequence();
  void ClearTextHistory();
  // The LoadMenu function fills its output containers with stimuli/graph objects.
  void LoadMenu( int                inMenuIdx,
                 GUI::Rect&         ioRect,
                 GUI::GraphDisplay& ioDisplay,
                 SetOfStimuli&      ioStimuli,
                 AssociationMap&    ioAssociations,
                 Speller&           ioSpeller ) const;

  bool MultipleMenus() const;
  int  NumMenus() const;
  int  MenuRows( int inMenuIdx ) const;
  int  MenuCols( int inMenuIdx ) const;
  ParamRef MenuParam( const std::string& inParamName, int inMenuIdx ) const;

  // Log file, text window file.
  static std::string TimeStamp();
  static std::string StringTime();
  static std::string StringDate();
  static std::string SummaryFileExtension();
  std::string DirectoryFileName() const;

 private:
  // Configuration parameters.
  int  mNumberOfSequences,
       mInterpretMode_;
  bool mDisplayResults,
       mTestMode;
  std::string mTextToSpell;

  // Properties of the current menu.
  int mCurMenu;
  GUI::Rect mMatrixRect;
  int mNumMatrixRows,
      mNumMatrixCols;

  // Internal state.
  int mSequenceCount;
  std::vector<int> mSequence;
  std::vector<int>::const_iterator mSequencePos;
  bool mAvoidStimulusRepetition;

  enum
  {
    dontSleep = 0,
    sleep1 = 1,
    sleep2 = 2,

    unsleepAfter

  };
  int mSleepMode;
  bool mPaused;

  std::string mGoalText;
  // Histories for "back" and "undo" functionality.
  std::stack<int>         mMenuHistory;
  std::stack<std::string> mTextHistory;

  // Set of all existing stimuli.
  SetOfStimuli       mStimuli;

  // Display elements.
  StatusBar*         mpStatusBar;
  TextWindow*        mpTextWindow;

  // UDP output.
  sending_udpsocket  mSocket;
  sockstream         mConnection;

 private:
  // Summary log file information.
  LogFile            mSummaryFile;
  std::ostringstream mSelectionSummary;
  int                mRunCount,
                     mNumSelections,
                     mSleepDuration;
  std::time_t        mStartPause;
};


#endif // P3_SPELLER_TASK_H