////////////////////////////////////////////////////////////////////////////////
// $Id$
// Authors: juergen.mellinger@uni-tuebingen.de
// Description: The BCI2000 Viewer application's main window.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include <vcl.h>
#include <Clipbrd.hpp>
#include <Registry.hpp>

#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <limits>

#include "ExecutableHelp.h"
#include "AboutBox.h"
#include "MainWindow.h"
#include "Label.h"
#include "TimeValue.h"
#include "MouseCursor.h"
#include "BCIError.h"
#include "defines.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

static const char* cProgramName = "BCI2000Viewer";

using namespace std;

const RGBColor TMainForm::cAxisColor = RGBColor::White;
const RGBColor TMainForm::cChannelColorsDefault[] =
{
  RGBColor::White,
  RGBColor::White,
  RGBColor::White,
  RGBColor::White,
  RGBColor::Yellow,
  ColorList::End
};

TMainForm::ActionEntry TMainForm::sActions[] =
{
  { FileOpen,  NULL,              NULL, "mFileOpen" },
  { FileClose, FileClose_Enabled, NULL, "mFileClose" },
  { FileQuit,  NULL,              NULL, "mFileQuit" },
  { EditCopy,  EditCopy_Enabled,  NULL, "mEditCopy" },
  { HelpOpenHelp, NULL,           NULL, "mHelpOpenHelp" },
  { HelpAbout,    NULL,           NULL, "mHelpAbout" },

  { Invert, Invert_Enabled, Invert_Checked, "mViewInvert" },
  { EnlargeSignal, ChangeResolution_Enabled, NULL, "mViewEnlargeSignal" },
  { ReduceSignal,  ChangeResolution_Enabled, NULL, "mViewReduceSignal" },
  { MoreChannels,  MoreChannels_Enabled, NULL,  "mViewMoreChannels" },
  { FewerChannels, FewerChannels_Enabled, NULL, "mViewFewerChannels" },
  { ChooseChannelColors, ChooseChannelColors_Enabled, NULL,
      "mViewChooseChannelColors" },
  { ToggleBaselines, ToggleBaselines_Enabled, ToggleBaselines_Checked,
      "mViewShowBaselines" },
  { ToggleUnit, ToggleUnit_Enabled, ToggleUnit_Checked,
      "mViewShowUnit" },
  { SampleZoomIn, SampleZoomIn_Enabled,   NULL, "mViewZoomIn" },
  { SampleZoomOut, SampleZoomOut_Enabled, NULL, "mViewZoomOut" },

  { ToFirstSample, GoBack_Enabled,    NULL, "mToBegin" },
  { ToLastSample,  GoForward_Enabled, NULL, "mToEnd" },
  { ToPrevBlock,   GoBack_Enabled,    NULL, "mBlockRew" },
  { ToNextBlock,   GoForward_Enabled, NULL, "mBlockFwd" },
  { ToPrevPage,    GoBack_Enabled,    NULL, "mPageRew" },
  { ToNextPage,    GoForward_Enabled, NULL, "mPageFwd" },

  { SampleZoomIn,  SampleZoomIn_Enabled,  NULL, "mSampleZoomIn" },
  { SampleZoomOut, SampleZoomOut_Enabled, NULL, "mSampleZoomOut" },

  { ShowSelectedChannels, ChannelsSelected, NULL, "mShowChannel" },
  { HideSelectedChannels, ChannelsSelected, NULL, "mHideChannel" },
};

//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm( TComponent* inOwner )
: TForm( inOwner ),
  mSamplePos( 0 ),
  mNumSignalChannels( 0 )
{
  mDefaultWindowProc = this->WindowProc;
  this->WindowProc = DragDropWindowProc;
  ::DragAcceptFiles( this->Handle, true );
  this->KeyPreview = true;

  mDragDropHint->BoundsRect = mSignalArea->BoundsRect;
  mDragDropHint->Anchors = mSignalArea->Anchors;
  mChannelListBox->MultiSelect = true;

  SetupActions();
  mDisplay.SetValueUnitVisible( true )
          .SetAxisColor( cAxisColor )
          .SetChannelColors( cChannelColorsDefault );
  RestoreFromRegistry();
  UpdateSignalDisplayContext();

  if( ParamCount() > 0 )
    DoFileOpen( ParamStr( 1 ).c_str() );
  else
    DoFileOpen( NULL );
}

__fastcall TMainForm::~TMainForm()
{
  SaveToRegistry();
}

void __fastcall TMainForm::DragDropWindowProc( TMessage& msg )
{
  switch( msg.Msg )
  {
    case WM_DROPFILES:
      {
        HDROP handle = ( HDROP )msg.WParam;
        size_t numFiles = ::DragQueryFile( handle, -1, NULL, 0 );
        if( numFiles > 1 )
          Application->MessageBox( "You can only drop one file at a time", "Warning", MB_OK );
        if( numFiles > 0 )
        {
          size_t nameLen = ::DragQueryFile( handle, 0, NULL, 0 );
          char* name = new char[ nameLen + 1 ];
          ::DragQueryFile( handle, 0, name, nameLen + 1 );
          DoFileOpen( name );
          delete[] name;
        }
        ::DragFinish( handle );
      }
      break;
    default:
      mDefaultWindowProc( msg );
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormCanResize(TObject*, int &NewWidth,
      int &NewHeight, bool &Resize)
{
  Resize = NewWidth >= this->Constraints->MinWidth
           && NewHeight >= this->Constraints->MinHeight;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormResize( TObject* )
{
  if ( !Application->Terminated )
  {
    const cControlDist = 10;
    mEditPosition->Left = mSignalArea->Left
      + ( mSignalArea->Width + mVerticalScroller->Width - mEditPosition->Width ) / 2;
    mSampleZoomOut->Left = mEditPosition->Left - mSampleZoomOut->Width - cControlDist;
    mSampleZoomIn->Left = mEditPosition->Left + mEditPosition->Width + cControlDist;
    UpdateSignalDisplayContext();
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SignalAreaPaint( TObject* )
{
  HourglassCursor cursor;
  mDisplay.Paint( NULL );
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditPositionExit( TObject* inSender )
{
  TEdit* editField = dynamic_cast<TEdit*>( inSender );
  if( editField != NULL && editField->Modified )
  {
    TimeValue t;
    istringstream iss( editField->Text.c_str() );
    if( iss >> t )
      SetSamplePos( t * mFile.SamplingRate() - mDisplay.NumSamples() / 2 );
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditPositionKeyUp(TObject *Sender, WORD &Key,
      TShiftState )
{
  if( Key == VK_RETURN )
    EditPositionExit( Sender );
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ChannelListBoxClickCheck( TObject* )
{
  UpdateChannelLabels();
  SetSamplePos( mSamplePos );
  UpdateVerticalScroller();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::VerticalScrollerChange( TObject *inSender )
{
  TScrollBar* scroller = dynamic_cast<TScrollBar*>( inSender );
  if( scroller != NULL )
    mDisplay.SetTopGroup( scroller->Position );
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::WMEraseBkgnd( TWMEraseBkgnd& msg )
{
  HDC dc = msg.DC;
  // Avoid flicker by erasing outside the signal display only:
  HRGN eraseRgn = ::CreateRectRgnIndirect( &ClientRect ),
       rgn = ::CreateRectRgnIndirect( &( mSignalArea->BoundsRect ) );
  ::CombineRgn( eraseRgn, eraseRgn, rgn, RGN_DIFF );
  ::DeleteObject( rgn );
  ::FillRgn( dc, eraseRgn, Brush->Handle );
  ::DeleteObject( eraseRgn );
}

// User actions
//---------------------------------------------------------------------------
// Standard actions
void TMainForm::FileOpen()
{
  TOpenDialog* pDialog = new TOpenDialog( ( TComponent* )NULL );
  pDialog->Filter = "BCI2000 data files (*.dat)|*.DAT";
  if( pDialog->Execute() )
    DoFileOpen( pDialog->FileName.c_str() );
  delete pDialog;
}

void TMainForm::FileClose()
{ DoFileOpen( NULL ); }
bool TMainForm::FileClose_Enabled() const
{ return mFile.IsOpen(); }

void TMainForm::FileQuit()
{ Application->Terminate(); }

void TMainForm::EditCopy()
{
  TMetafile* pMetafile = new TMetafile;
  pMetafile->Height = mSignalArea->Height;
  pMetafile->Width = mSignalArea->Width;
  TMetafileCanvas* pCanvas = new TMetafileCanvas( pMetafile, 0 );
  GUI::DrawContext dc =
  {
    pCanvas->Handle,
    { 0, 0, mSignalArea->Width, mSignalArea->Height }
  };
  mDisplay.SetContext( dc );
  mDisplay.Paint();
  UpdateSignalDisplayContext();
  delete pCanvas;
  Clipboard()->Assign( pMetafile );
  delete pMetafile;
}
bool TMainForm::EditCopy_Enabled() const
{ return mFile.IsOpen(); }

void TMainForm::HelpOpenHelp()
{
  ExecutableHelp().Display();
}

void TMainForm::HelpAbout()
{
  AboutBox().SetApplicationName( cProgramName )
            .Display();
}

// Temporal movement
void TMainForm::ToFirstSample()
{ SetSamplePos( 0 ); }
void TMainForm::ToLastSample()
{ SetSamplePos( max( 0L, mFile.NumSamples() - mDisplay.NumSamples() ) ); }
void TMainForm::ToNextBlock()
{ MoveSamplePos( mFile.SignalProperties().Elements() ); }
void TMainForm::ToPrevBlock()
{ MoveSamplePos( -long( mFile.SignalProperties().Elements() ) ); }
void TMainForm::ToPrevPage()
{ SetSamplePos( max( 0L, mSamplePos - mDisplay.NumSamples() ) ); }
void TMainForm::ToNextPage()
{ SetSamplePos( min( mFile.NumSamples() - mDisplay.NumSamples(), mSamplePos + mDisplay.NumSamples() ) ); }

bool TMainForm::GoBack_Enabled() const
{ return mSamplePos > 0; }
bool TMainForm::GoForward_Enabled() const
{ return mSamplePos < long( mFile.NumSamples() ) - mDisplay.NumSamples(); }

// Temporal resolution
void TMainForm::SampleZoomIn()
{
  int prevNumSamples = mDisplay.NumSamples(),
      newNumSamples = prevNumSamples / 2;
  mDisplay.SetNumSamples( newNumSamples );
  SetSamplePos( mSamplePos + ( prevNumSamples - newNumSamples ) / 2 );
}
bool TMainForm::SampleZoomIn_Enabled() const
{ return mFile.IsOpen() && mDisplay.NumSamples() / 2 >= mFile.SignalProperties().Elements(); }

void TMainForm::SampleZoomOut()
{
  int prevNumSamples = mDisplay.NumSamples(),
      newNumSamples = prevNumSamples * 2;
  if( newNumSamples > mFile.NumSamples() )
    newNumSamples = mFile.NumSamples();
  mDisplay.SetNumSamples( newNumSamples );
  SetSamplePos( mSamplePos + ( prevNumSamples - newNumSamples ) / 2 );
}
bool TMainForm::SampleZoomOut_Enabled() const
{ return mDisplay.NumSamples() < mFile.NumSamples(); }

// Number of displayed channels
void TMainForm::FewerChannels()
{
  int newNumChannels = mDisplay.DisplayGroups() / 2;
  if( newNumChannels < 1 )
    newNumChannels = 1;
  mDisplay.SetDisplayGroups( newNumChannels )
          .SetTopGroup( mVerticalScroller->Position );
  UpdateVerticalScroller();
}
bool TMainForm::FewerChannels_Enabled() const
{ return mDisplay.DisplayGroups() > 1; }

void TMainForm::MoreChannels()
{
  int newNumChannels = mDisplay.DisplayGroups() * 2;
  newNumChannels = min( newNumChannels, mNumSignalChannels );
  mDisplay.SetDisplayGroups( newNumChannels )
          .SetTopGroup( mVerticalScroller->Position );
  UpdateVerticalScroller();
}
bool TMainForm::MoreChannels_Enabled() const
{ return mDisplay.DisplayGroups() < mNumSignalChannels; }

// Channel scrolling
void TMainForm::ChannelUp()
{ --mVerticalScroller->Position; }
void TMainForm::ChannelDown()
{ ++mVerticalScroller->Position; }
void TMainForm::ChannelPageNext()
{ mVerticalScroller->Position
  = min( mVerticalScroller->Max, mVerticalScroller->Position + mVerticalScroller->LargeChange ); }
void TMainForm::ChannelPagePrev()
{ mVerticalScroller->Position
  = max( mVerticalScroller->Min, mVerticalScroller->Position - mVerticalScroller->LargeChange ); }
void TMainForm::ChannelPageFirst()
{ mVerticalScroller->Position = mVerticalScroller->Min; }
void TMainForm::ChannelPageLast()
{ mVerticalScroller->Position = mVerticalScroller->Max; }

bool TMainForm::ChannelUp_Enabled()
{ return mVerticalScroller->Enabled && mVerticalScroller->Position > mVerticalScroller->Min; }
bool TMainForm::ChannelDown_Enabled()
{ return mVerticalScroller->Enabled
         && mVerticalScroller->Position < mVerticalScroller->Max - mVerticalScroller->PageSize + 1; }

// Signal resolution
void TMainForm::EnlargeSignal()
{
  mDisplay.SetMinValue( mDisplay.MinValue() / 2 );
  mDisplay.SetMaxValue( mDisplay.MaxValue() / 2 );
}

void TMainForm::ReduceSignal()
{
  mDisplay.SetMinValue( mDisplay.MinValue() * 2 );
  mDisplay.SetMaxValue( mDisplay.MaxValue() * 2 );
}

bool TMainForm::ChangeResolution_Enabled() const
{ return mDisplay.NumSamples() > 0; }

// Display attributes
void TMainForm::ChooseChannelColors()
{
  const numCustomColors = 16;
  COLORREF customColors[ numCustomColors ];
  ColorList channelColors = mDisplay.ChannelColors();
  for( int i = 0; i < ::min<int>( channelColors.size(), numCustomColors ); ++i )
    customColors[ i ] = channelColors[ i ].ToWinColor();
  for( int i = channelColors.size(); i < numCustomColors; ++i )
    customColors[ i ] = RGBColor( RGBColor::Black ).ToWinColor();
  CHOOSECOLOR chooserParams =
  {
    sizeof( CHOOSECOLOR ),
    this->Handle,
    NULL,
    0x0,
    customColors,
    CC_FULLOPEN,
    NULL,
    NULL,
    NULL
  };
  if( ::ChooseColor( &chooserParams ) )
  {
    int numUserColors = 0;
    while( numUserColors < numCustomColors && customColors[ numUserColors ] != RGBColor::Black )
      ++numUserColors;
    if( numUserColors == 0 )
      channelColors.resize( 1, RGBColor::White );
    else
    {
      channelColors.resize( numUserColors );
      for( int i = 0; i < numUserColors; ++i )
        channelColors[ i ] = RGBColor::FromWinColor( customColors[ i ] );
    }
    mDisplay.SetChannelColors( channelColors );
  }
}
bool TMainForm::ChooseChannelColors_Enabled() const
{ return mFile.IsOpen(); }

void TMainForm::Invert()
{ mDisplay.SetInverted( !mDisplay.Inverted() ); }
bool TMainForm::Invert_Checked() const
{ return mDisplay.Inverted(); }
bool TMainForm::Invert_Enabled() const
{ return mNumSignalChannels > 0; }

void TMainForm::ToggleBaselines()
{ mDisplay.SetBaselinesVisible( !mDisplay.BaselinesVisible() ); }
bool TMainForm::ToggleBaselines_Checked() const
{ return mDisplay.BaselinesVisible(); }
bool TMainForm::ToggleBaselines_Enabled() const
{ return mNumSignalChannels > 0; }

void TMainForm::ToggleUnit()
{ mDisplay.SetValueUnitVisible( !mDisplay.ValueUnitVisible() ); }
bool TMainForm::ToggleUnit_Checked() const
{ return mDisplay.ValueUnitVisible(); }
bool TMainForm::ToggleUnit_Enabled() const
{ return mNumSignalChannels > 0; }

void TMainForm::ShowSelectedChannels()
{
  for( int i = 0; i < mChannelListBox->Count; ++i )
    if( mChannelListBox->Selected[ i ] )
      mChannelListBox->Checked[ i ] = true;
  mChannelListBox->OnClickCheck( mChannelListBox );
}
void TMainForm::HideSelectedChannels()
{
  for( int i = 0; i < mChannelListBox->Count; ++i )
    if( mChannelListBox->Selected[ i ] )
      mChannelListBox->Checked[ i ] = false;
  mChannelListBox->OnClickCheck( mChannelListBox );
}
bool TMainForm::ChannelsSelected() const
{ return mChannelListBox->SelCount > 0; }

// Internal functions
//----------------------------------------------------------------------------
void
TMainForm::DoFileOpen( const char* inName )
{
  mFile.Open( inName );
  if( !mFile.IsOpen() )
  {
    if( inName != NULL )
    {
      ostringstream oss;
      oss << "Could not open \n\"" << inName << "\"\nas a BCI2000 file." << endl;
      Application->MessageBox( oss.str().c_str(), cProgramName, MB_OK | MB_ICONERROR );
    }
    mDragDropHint->Visible = true;
    mSignalArea->Visible = false;
    this->Caption = cProgramName;
    mDisplay.SetNumSamples( 0 );
  }
  else
  {
    mDragDropHint->Visible = false;
    mSignalArea->Visible = true;
    Caption = AnsiString( cProgramName ) + ": " + inName;
    mDisplay.SetNumSamples( mFile.SamplingRate() * 10 )
            .SetUnitsPerValue( 1.0 ).SetValueUnit( "muV" )
            .SetMinValue( -100 ).SetMaxValue( 100 )
            .SetUnitsPerSample( 1.0 / mFile.SamplingRate() )
            .SetSampleUnit( ":s" );
  }
  FillChannelList();
  UpdateChannelLabels();
  SetSamplePos( 0 );
  mDisplay.SetDisplayGroups( mFile.SignalProperties().Channels() );
  UpdateVerticalScroller();
}

void
TMainForm::FillChannelList()
{
  mChannelListBox->Clear();
  if( mFile.IsOpen() )
  {
    mChannelListBox->Items->Add( "States" );
    mChannelListBox->Header[ 0 ] = true;
    const StateList* states = mFile.States();
    for( int i = 0; i < states->Size(); ++i )
      mChannelListBox->Items->Add( ( *states )[ i ].Name().c_str() );

    mChannelListBox->Items->Add( "Channels" );
    int base = mChannelListBox->Count;
    mChannelListBox->Header[ base - 1 ] = true;
    int channelsInFile = mFile.SignalProperties().Channels();
    if( mFile.Parameters()->Exists( "ChannelNames" ) )
    {
      ParamRef ChannelNames = mFile.Parameter( "ChannelNames" );
      for( int i = 0; i < ChannelNames->NumValues() && i < channelsInFile; ++i )
        mChannelListBox->Items->Add( ChannelNames( i ).c_str() );
    }
    for( int i = mChannelListBox->Count - base; i < channelsInFile; ++i )
      mChannelListBox->Items->Add( AnsiString( "Channel " ) + IntToStr( i + 1 ) );

    int i = 0;
    for( ++i; i < mChannelListBox->Count && !mChannelListBox->Header[ i ]; ++i )
      mChannelListBox->Checked[ i ] = false;
    for( ++i; i < mChannelListBox->Count && !mChannelListBox->Header[ i ]; ++i )
      mChannelListBox->Checked[ i ] = true;
  }
}

void
TMainForm::SaveToRegistry() const
{
  TRegistry* pReg = NULL;
  try
  {
    pReg = new TRegistry( KEY_WRITE );
    pReg->RootKey = HKEY_CURRENT_USER;
    pReg->OpenKey( KEY_BCI2000 KEY_VIEWER KEY_CONFIG, true );
    if( this->WindowState == wsNormal )
    {
      TMainForm* this_ = const_cast<TMainForm*>( this );
      pReg->WriteInteger( "Left", this_->Left );
      pReg->WriteInteger( "Top", this_->Top );
      pReg->WriteInteger( "Height", this_->Height );
      pReg->WriteInteger( "Width", this_->Width );
    }
    ostringstream oss;
    if( oss << mDisplay.ChannelColors() )
      pReg->WriteString( "ChannelColors", oss.str().c_str() );
    pReg->WriteInteger( "Invert", mDisplay.Inverted() ? 1 : 0 );
  }
  catch( ERegistryException& )
  {
  }
  delete pReg;
}

void
TMainForm::RestoreFromRegistry()
{
  TRegistry* pReg = NULL;
  try
  {
    pReg = new TRegistry( KEY_READ );
    pReg->RootKey = HKEY_CURRENT_USER;
    pReg->OpenKey( KEY_BCI2000 KEY_VIEWER KEY_CONFIG, false );
    TRect storedRect;
    storedRect.Left = pReg->ReadInteger( "Left" );
    storedRect.Top = pReg->ReadInteger( "Top" );
    storedRect.Right = storedRect.Left + pReg->ReadInteger( "Width" );
    storedRect.Bottom = storedRect.Top + pReg->ReadInteger( "Height" );
    const int minDistance = 10; // Make sure that at least that much of the window is
                                // inside the screen.
    TRect commonRect;
    if( Types::IntersectRect( commonRect, storedRect, Screen->WorkAreaRect )
        && commonRect.Height() >= minDistance
        && commonRect.Width() >= minDistance )
    {
      this->Left = storedRect.Left;
      this->Top = storedRect.Top;
      this->Height = storedRect.Height();
      this->Width = storedRect.Width();
    }
    istringstream iss;
    iss.str( pReg->ReadString( "ChannelColors" ).c_str() );
    ColorList colors;
    if( iss >> colors )
      mDisplay.SetChannelColors( colors );
    mDisplay.SetInverted( 1 == pReg->ReadInteger( "Invert" ) );
  }
  catch( ERegistryException& )
  {
  }
  delete pReg;
}

void
TMainForm::UpdateSignalDisplayContext()
{
  TRect rect = mSignalArea->BoundsRect;
  GUI::DrawContext dc =
  {
    ::GetDC( this->Handle ),
    {
      rect.left,
      rect.top,
      rect.right,
      rect.bottom
    }
  };
  mDisplay.SetContext( dc );
}

void
TMainForm::UpdateSamplePos()
{
  mDisplay.SetSampleOffset( mSamplePos );
  UpdateTimeField();
}

void
TMainForm::UpdateTimeField()
{
  TNotifyEvent handler = mEditPosition->OnChange;
  mEditPosition->OnChange = NULL;
  if( mFile.IsOpen() )
  {
    TimeValue pos = ( 2 * mSamplePos + mDisplay.NumSamples() ) / 2
                    / mFile.SamplingRate(),
              length = mFile.NumSamples() / mFile.SamplingRate();
    ostringstream oss;
    oss << pos << "/" << length;
    mEditPosition->Text = oss.str().c_str();
  }
  else
    mEditPosition->Text = "";
  mEditPosition->Enabled = mFile.IsOpen();
  mEditPosition->Modified = false;
  mEditPosition->OnChange = handler;
}

void
TMainForm::UpdateChannelLabels()
{
  if( mFile.IsOpen() )
  {
    vector<string> signalLabels;
    if( mFile.Parameters()->Exists( "ChannelLabels" ) )
    {
      ParamRef labelParam = mFile.Parameter( "ChannelLabels" );
      for( int k = 0; k < labelParam->NumValues(); ++k )
        signalLabels.push_back( labelParam( k ) );
    }
    for( int i = signalLabels.size(); i < mFile.SignalProperties().Channels(); ++i )
    {
      ostringstream oss;
      oss << i + 1;
      signalLabels.push_back( oss.str() );
    }

    LabelList channelLabels;
    size_t numMarkerChannels = 0;
    int i = 1;
    while( i < mChannelListBox->Count && !mChannelListBox->Header[ i ] )
      ++i;
    int chBase = ++i;
    for( ; i < mChannelListBox->Count && !mChannelListBox->Header[ i ]; ++i )
      if( mChannelListBox->Checked[ i ] )
        channelLabels.push_back(
          Label(
            channelLabels.size(),
            signalLabels[ i - chBase ]
          )
        );
    for( int i = 1; i < mChannelListBox->Count && !mChannelListBox->Header[ i ]; ++i )
      if( mChannelListBox->Checked[ i ] )
      {
        channelLabels.push_back(
          Label(
            channelLabels.size(),
            mChannelListBox->Items->Strings[ i ].c_str()
          )
        );
        ++numMarkerChannels;
      }
    mNumSignalChannels = channelLabels.size() - numMarkerChannels;
    mDisplay.SetNumMarkerChannels( numMarkerChannels );
    mDisplay.SetChannelLabels( channelLabels );
    mDisplay.SetChannelLabelsVisible( true );
  }
  else
  {
    mNumSignalChannels = 0;
    mDisplay.SetNumMarkerChannels( 0 );
    mDisplay.SetChannelLabels( LabelList() );
  }
}

void
TMainForm::UpdateVerticalScroller()
{
  TNotifyEvent scrollerChange = mVerticalScroller->OnChange;
  mVerticalScroller->OnChange = NULL;
  int numChannels = mNumSignalChannels,
      pageSize = mDisplay.DisplayGroups();
  if( pageSize < 1 )
    pageSize = 1;
  int scrollMax = numChannels - 1;
  if( scrollMax < 0 )
    scrollMax = 0;
  if( pageSize > scrollMax )
  {
    mVerticalScroller->Enabled = false;
  }
  else
  {
    mVerticalScroller->PageSize = pageSize; // Due to a VCL bug, this needs to be set first.
    mVerticalScroller->LargeChange = mVerticalScroller->PageSize;
    mVerticalScroller->SmallChange = 1;
    mVerticalScroller->Min = 0;
    mVerticalScroller->Max = scrollMax;
    mVerticalScroller->Position = mDisplay.TopGroup();
    mVerticalScroller->Enabled = true;
  }
  mVerticalScroller->OnChange = scrollerChange;
}

const GenericSignal&
TMainForm::ConstructDisplaySignal( long inPos, long inLength )
{
  static GenericSignal result;
  if( mFile.IsOpen() )
  {
    HourglassCursor cursor;
    int i = 1;
    vector<StateRef> states;
    for( ; i < mChannelListBox->Count && !mChannelListBox->Header[ i ]; ++i )
      if( mChannelListBox->Checked[ i ] )
        states.push_back( mFile.State( mChannelListBox->Items->Strings[ i ].c_str() ) );
    vector<size_t> channels;
    int base = ++i;
    for( ; i < mChannelListBox->Count && !mChannelListBox->Header[ i ]; ++i )
      if( mChannelListBox->Checked[ i ] )
        channels.push_back( i - base );

    int numChannels = channels.size() + states.size();
    GenericSignal signal( numChannels, inLength );
    long sampleInFile = inPos;
    for( long sample = 0;
         sample < signal.Elements() && sampleInFile < mFile.NumSamples();
         ++sample, ++sampleInFile )
    {
      size_t channelIdx = 0;
      for( ; channelIdx < channels.size(); ++channelIdx )
        signal( channelIdx, sample )
          = mFile.CalibratedValue( channels[ channelIdx ], sampleInFile );
      if( !states.empty() )
      {
        mFile.ReadStateVector( sampleInFile );
        for( vector<StateRef>::const_iterator i = states.begin(); i != states.end(); ++i )
          signal( channelIdx++, sample ) = *i;
      }
    }
    result = signal;
  }
  else
  {
    result = GenericSignal( 0, 0 );
  }
  return result;
}

void
TMainForm::SetSamplePos( long inPos )
{
  if( mFile.IsOpen() )
  {
    mSamplePos = inPos;
    if( mSamplePos > long( mFile.NumSamples() ) - mDisplay.NumSamples() )
          mSamplePos = mFile.NumSamples() - mDisplay.NumSamples();
    if( mSamplePos < 0 )
      mSamplePos = 0;
    const GenericSignal& signal = ConstructDisplaySignal( mSamplePos, mDisplay.NumSamples() );
    mDisplay.WrapForward( signal );
  }
  else
  {
    mDisplay.SetNumSamples( 0 )
            .WrapForward( GenericSignal( 0, 0 ) );
    mSamplePos = 0;
  }
  UpdateSamplePos();
}

void
TMainForm::MoveSamplePos( long inPosDiff )
{
  if( mSamplePos + inPosDiff < 0 )
  {
    ToFirstSample();
    return;
  }
  if( mSamplePos + inPosDiff >= mFile.NumSamples() - mDisplay.NumSamples() )
  {
    ToLastSample();
    return;
  }
  if( inPosDiff != 0 && mFile.IsOpen() )
  {
    long readCursor = inPosDiff > 0 ?
                      mSamplePos + mDisplay.NumSamples() :
                      mSamplePos + inPosDiff;
    const GenericSignal& signal = ConstructDisplaySignal( readCursor, ::abs( inPosDiff ) );
    mSamplePos += inPosDiff;
    if( inPosDiff > 0 )
      mDisplay.ScrollForward( signal );
    else
      mDisplay.ScrollBack( signal );
    UpdateSamplePos();
  }
}

void
TMainForm::SetupActions()
{
  for( size_t i = 0; i < sizeof( sActions ) / sizeof( *sActions ); ++i )
  {
    TAction* action = new TAction( this );
    action->Tag = i;
    action->OnUpdate = ActionUpdateHandler;
    action->OnExecute = ActionExecuteHandler;
    TComponent* component = FindComponent( sActions[ i ].control );
    TControl* control = dynamic_cast<TControl*>( component );
    TMenuItem* menuItem = dynamic_cast<TMenuItem*>( component );
    if( control != NULL )
    {
      int size = control->GetTextLen();
      char* caption = new char[ size + 1 ];
      control->GetTextBuf( caption, size + 1 );
      action->Caption = caption;
      delete[] caption;
      action->Hint = control->Hint;
      control->Action = action;
    }
    else if( menuItem != NULL )
    {
      action->Caption = menuItem->Caption;
      action->ShortCut = menuItem->ShortCut;
      action->Hint = menuItem->Hint;
      menuItem->Action = action;
    }
    else
      bcierr__ << "Component not accessible" << endl;
  }
  TPopupMenu* popupMenu = new TPopupMenu( this );
  for( int i = 0; i < mViewMenu->Count; ++i )
  {
    TMenuItem* item = new TMenuItem( this ),
             * viewItem = mViewMenu->Items[ i ];
    item->Caption = viewItem->Caption;
    item->Action = viewItem->Action;
    popupMenu->Items->Add( item );
  }
  mSignalArea->PopupMenu = popupMenu;
};

void __fastcall TMainForm::ActionUpdateHandler( TObject* inSender )
{
  TAction* action = dynamic_cast<TAction*>( inSender );
  if( action != NULL )
  {
    const ActionEntry& actionEntry = sActions[ action->Tag ];
    if( actionEntry.enabled != NULL )
      action->Enabled = ( this->*actionEntry.enabled )();
    if( actionEntry.checked != NULL )
      action->Checked = ( this->*actionEntry.checked )();
  }
}

void __fastcall TMainForm::ActionExecuteHandler( TObject* inSender )
{
  TAction* action = dynamic_cast<TAction*>( inSender );
  if( action != NULL )
  {
    const ActionEntry& actionEntry = sActions[ action->Tag ];
    if( actionEntry.action != NULL )
      ( this->*actionEntry.action )();
  }
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::FormKeyDown(TObject*, WORD &Key, TShiftState Shift)
{
  switch( Key )
  {
    case VK_ESCAPE:
      ActiveControl = NULL;
      break;
  }
  if( dynamic_cast<TEdit*>( ActiveControl )
      || dynamic_cast<TCheckListBox*>( ActiveControl ) )
    return;

  bool scrollerActive = ( ActiveControl == mVerticalScroller );
  static int acc = 0;
  int wipe_acc = 1;
  bool shift = Shift.Contains( ssShift ) || Shift.Contains( ssAlt ) || Shift.Contains( ssCtrl );
  switch( Key )
  {
    case VK_UP:
      if( !scrollerActive && ChannelUp_Enabled() )
      {
        if( shift )
          ChannelPagePrev();
        else
          ChannelUp();
      }
      break;
    case VK_DOWN:
      if( !scrollerActive && ChannelDown_Enabled() )
      {
        if( shift )
          ChannelPageNext();
        else
          ChannelDown();
      }
      break;

    case VK_PRIOR:
    case 'B':
    case 'b':
      if( ChannelUp_Enabled() )
        ChannelPagePrev();
      break;
    case VK_NEXT:
    case VK_SPACE:
      if( ChannelDown_Enabled() )
        ChannelPageNext();
      break;

    case VK_RIGHT:
      if( GoForward_Enabled() )
      {
        if( shift )
          ToNextPage();
        else
          ToNextBlock();
      }
      break;
    case VK_LEFT:
      if( GoBack_Enabled() )
      {
        if( shift )
          ToPrevPage();
        else
          ToPrevBlock();
      }
      break;
    case VK_OEM_COMMA:
      if(FewerChannels_Enabled())
        FewerChannels();
      break;
    case VK_OEM_PERIOD:
      if(MoreChannels_Enabled())
        MoreChannels();
      break;
    case VK_SUBTRACT:
    case VK_OEM_MINUS:
      if(ChangeResolution_Enabled())
        ReduceSignal();
      break;
    case VK_ADD:
    case VK_OEM_PLUS:
      if(ChangeResolution_Enabled())
        EnlargeSignal();
      break;
    case VK_HOME:
      if( ChannelUp_Enabled() )
        ChannelPageFirst();
      break; 
    case VK_END:
      if( ChannelDown_Enabled() )
        ChannelPageLast();
      break;
    case VK_RETURN:
    case 'G':
    case 'g':
      ChannelPageFirst();
      for( int i = 1; i < acc; ++i )
        ChannelDown();
      break;      
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      acc = acc * 10 + (Key - '0');
      wipe_acc = 0;
      break;
  }
  if(wipe_acc)
    acc = 0;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::HelpOnChannelClick(TObject *Sender)
{
  TMenuItem* pItem = dynamic_cast<TMenuItem*>( Sender );
  if( pItem )
  {
    string name = pItem->Caption.c_str();
    size_t p1 = name.find( '\"' ),
           p2 = name.find( '\"', p1 + 1 );
    if( p1 != string::npos && p2 != string::npos )
    {
      name = name.substr( p1 + 1, p2 - p1 - 1 );

      if( ExecutableHelp().StateHelp().Exists( name ) )
        ExecutableHelp().StateHelp().Open( name );
      else
        ::MessageBeep( MB_ICONASTERISK );
    }
    else
      HelpOpenHelp();
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mChannelListBoxContextPopup(TObject* Sender,
      TPoint& MousePos, bool& )
{
  TCheckListBox* pListBox = dynamic_cast<TCheckListBox*>( Sender );
  if( pListBox )
  {
    mHelpOnChannel->Caption = "BCI2000 Help";
    int section = 0,
        item = pListBox->ItemAtPos( MousePos, true );
    if( item >= 0 && !pListBox->Header[item] )
    {
      for( int i = 0; i <= item; ++i )
        if( pListBox->Header[i] )
          ++section;
      if( section == 1 )
      {
        string name = pListBox->Items->Strings[item].c_str();
        if( ExecutableHelp().StateHelp().Exists( name ) )
          mHelpOnChannel->Caption =
            ( string( "Help on the \"" ) + name + "\" state variable" ).c_str();

      }
    }
  }
}
//---------------------------------------------------------------------------


