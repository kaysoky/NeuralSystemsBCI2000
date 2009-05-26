#include "PCHIncludes.h"
#pragma hdrstop

#include "ParamDisplay.h"
#include "Param.h"
#include "Color.h"
#include "UOperatorUtils.h"
#include "UPreferences.h"
#include "UEditMatrix.h"
#include "ExecutableHelp.h"
#include "ParsedComment.h"
#include "Operator.h"
#include "VCLDefines.h"

#include <vector>
#include <string>
#include <sstream>
#include <cassert>

#define ALLFILES_FILTER "All files (*.*)|*.*"
#define MATRIX_EXTENSION ".txt"
#define MATRIX_FILTER "Tab delimited matrix file (*" MATRIX_EXTENSION ")"  \
                                       "|*" MATRIX_EXTENSION "|" ALLFILES_FILTER
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay definitions
// (C) 2000-2009, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
std::map<ParamDisplay::DisplayBase*, int>&
ParamDisplay::RefCount()
{
  static map<DisplayBase*, int> refCount;
  return refCount;
}

ParamDisplay::ParamDisplay()
: mpDisplay( NULL )
{
}

ParamDisplay::ParamDisplay( const Param& inParam, TWinControl* inParent )
: mpDisplay( NULL )
{
  ParsedComment parsedComment( inParam );
  switch( parsedComment.Kind() )
  {
    case ParsedComment::singleEntryEnum:
      mpDisplay = new SingleEntryEnum( parsedComment, inParent );
      break;
    case ParsedComment::singleEntryBoolean:
      mpDisplay = new SingleEntryBoolean( parsedComment, inParent );
      break;
    case ParsedComment::singleEntryInputFile:
      mpDisplay = new SingleEntryInputFile( parsedComment, inParent );
      break;
    case ParsedComment::singleEntryOutputFile:
      mpDisplay = new SingleEntryOutputFile( parsedComment, inParent );
      break;
    case ParsedComment::singleEntryDirectory:
      mpDisplay = new SingleEntryDirectory( parsedComment, inParent );
      break;
    case ParsedComment::singleEntryColor:
      mpDisplay = new SingleEntryColor( parsedComment, inParent );
      break;
    case ParsedComment::singleEntryGeneric:
      mpDisplay = new SingleEntryEdit( parsedComment, inParent );
      break;
    case ParsedComment::listGeneric:
      mpDisplay = new List( parsedComment, inParent );
      break;
    case ParsedComment::matrixGeneric:
      mpDisplay = new Matrix( parsedComment, inParent );
      break;
    default:
      assert( false );
  }
  ++RefCount()[ mpDisplay ];
}

ParamDisplay::ParamDisplay( const ParamDisplay& inOriginal )
: mpDisplay( inOriginal.mpDisplay )
{
  ++RefCount()[ mpDisplay ];
}

const ParamDisplay&
ParamDisplay::operator=( const ParamDisplay& inOriginal )
{
  mpDisplay = inOriginal.mpDisplay;
  ++RefCount()[ mpDisplay ];
  return *this;
}

ParamDisplay::~ParamDisplay()
{
  if( --RefCount()[ mpDisplay ] < 1 )
    delete mpDisplay;
}

void
ParamDisplay::SetTop( int top )
{
  mpDisplay->SetTop( top );
}

void
ParamDisplay::SetLeft( int left )
{
  mpDisplay->SetLeft( left );
}

int
ParamDisplay::GetBottom()
{
  return mpDisplay->GetBottom();
}

int
ParamDisplay::GetRight()
{
  return mpDisplay->GetRight();
}

void
ParamDisplay::Hide()
{
  mpDisplay->Hide();
}

void
ParamDisplay::Show()
{
  mpDisplay->Show();
}

void
ParamDisplay::WriteValuesTo( Param& p ) const
{
  mpDisplay->WriteValuesTo( p );
}

void
ParamDisplay::ReadValuesFrom( const Param& p )
{
  mpDisplay->ReadValuesFrom( p );
}

bool
ParamDisplay::Modified() const
{
  return mpDisplay->Modified();
}

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::DisplayBase definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::DisplayBase::DisplayBase( const ParsedComment& inParam,
                                        TWinControl* inParent )
: mpUserLevel( NULL ),
  mTop( 0 ),
  mLeft( 0 ),
  mModified( false )
{
  // render the parameter's name
  TStaticText* label = new TStaticText( static_cast<TComponent*>( NULL ) );
  label->Left = labelsOffsetX;
  label->Top = labelsOffsetY;
  label->Caption = inParam.Name().c_str();
  label->Font->Style = TFontStyles() << fsBold;
  label->Visible = false;
  label->Hint = inParam.Comment().c_str();
  label->ShowHint = true;
  label->Parent = inParent;
  AddControl( label );
  AddHelp( inParam, label );

  // render the parameter's User Level track bar
  // ONLY, if the current user level is "advanced"
  if( OperatorUtils::UserLevel() == USERLEVEL_ADVANCED )
  {
    mpUserLevel = new TTrackBar( static_cast<TComponent*>( NULL ) );
    mpUserLevel->Left = userlevelOffsetX;
    mpUserLevel->Top = userlevelOffsetY;
    mpUserLevel->Width = userlevelWidth;
    mpUserLevel->Height = userlevelHeight;
    mpUserLevel->Min = 1;
    mpUserLevel->Max = 3;
    mpUserLevel->PageSize = 1;
    mpUserLevel->Visible = false;
    mpUserLevel->Parent = inParent;
    mpUserLevel->OnChange = DisplayBase::OnContentChange;
    mpUserLevel->TabStop = false;
    AddControl( mpUserLevel );
  }
}

ParamDisplay::DisplayBase::~DisplayBase()
{
  for( WndProcIterator i = mHelpWndProcs.begin(); i != mHelpWndProcs.end(); ++i )
    delete *i;
  for( ControlIterator i = mControls.begin(); i != mControls.end(); ++i )
    delete *i;
}

void
ParamDisplay::DisplayBase::SetTop( int inTop )
{
  for( ControlIterator i = mControls.begin(); i != mControls.end(); ++i )
    ( *i )->Top = ( *i )->Top - mTop + inTop;
  mTop = inTop;
}

void
ParamDisplay::DisplayBase::SetLeft( int inLeft )
{
  for( ControlIterator i = mControls.begin(); i != mControls.end(); ++i )
    ( *i )->Left = ( *i )->Left - mLeft + inLeft;
  mLeft = inLeft;
}

int
ParamDisplay::DisplayBase::GetBottom()
{
  int bottom = 0;
  for( ControlIterator i = mControls.begin(); i != mControls.end(); ++i )
    bottom = max( bottom, ( *i )->Top + ( *i )->Height );
  return bottom;
}

int
ParamDisplay::DisplayBase::GetRight()
{
  int right = 0;
  for( ControlIterator i = mControls.begin(); i != mControls.end(); ++i )
    right = max( right, ( *i )->Left + ( *i )->Width );
  return right;
}

void
ParamDisplay::DisplayBase::Hide()
{
  for( ControlIterator i = mControls.begin(); i != mControls.end(); ++i )
    ( *i )->Hide();
}

void
ParamDisplay::DisplayBase::Show()
{
  for( ControlIterator i = mControls.begin(); i != mControls.end(); ++i )
    ( *i )->Show();
}

void
ParamDisplay::DisplayBase::WriteValuesTo( Param& inParam ) const
{
  if( mpUserLevel )
    OperatorUtils::SetUserLevel( inParam.Name().c_str(), mpUserLevel->Position );
}

void
ParamDisplay::DisplayBase::ReadValuesFrom( const Param& inParam )
{
  if( mpUserLevel )
    mpUserLevel->Position = OperatorUtils::GetUserLevel( inParam.Name().c_str() );
  mModified = false;
}

ParamDisplay::DisplayBase::HelpWndProc::HelpWndProc( const ParsedComment& inParam, TWinControl* iopControl )
: mpControl( iopControl ),
  mParamName( inParam.Name() ),
  mHelpContext( inParam.HelpContext() )
{
  this->mWndProc = mpControl->WindowProc;
  mpControl->WindowProc = &( this->WndProc );
}

ParamDisplay::DisplayBase::HelpWndProc::~HelpWndProc()
{
  mpControl->WindowProc = this->mWndProc;
}

void
__fastcall
ParamDisplay::DisplayBase::HelpWndProc::WndProc( TMessage& inMessage )
{
  if( inMessage.Msg == WM_HELP )
  {
    if( ExecutableHelp().ParamHelp().Exists( mParamName ) )
      ExecutableHelp().ParamHelp().Open( mParamName, mHelpContext );
    else
      ::MessageBeep( MB_ICONASTERISK );
  }
  this->mWndProc( inMessage );
}
////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::SeparateComment definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::SeparateComment::SeparateComment( const ParsedComment& inParam,
                                                TWinControl* inParent )
: DisplayBase( inParam, inParent )
{
  // render the parameter's comment
  TStaticText* comment = new TStaticText( static_cast<TComponent*>( NULL ) );
  comment->AutoSize = true;
  comment->Left = commentOffsetX;
  comment->Top = commentOffsetY;
  comment->Caption = inParam.Comment().c_str();
  comment->Hint = comment->Caption;
  comment->ShowHint = true;
  comment->Font->Style = TFontStyles() << fsItalic;
  comment->Visible = false;
  static const AnsiString ellipsis = "...";
  while( comment->Caption != ellipsis
          && comment->Left + comment->Width > userlevelOffsetX + userlevelWidth )
    comment->Caption
      = comment->Caption.SubString( 0, comment->Caption.LastDelimiter( " \t\n\r" ) - 1 )
        + ellipsis;
  comment->Caption = comment->Caption + ' ';
  comment->Height = comment->Height - 2;
  comment->Parent = inParent;
  AddControl( comment );
  AddHelp( inParam, comment );
}

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::SingleEntryEdit definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::SingleEntryEdit::SingleEntryEdit( const ParsedComment& inParam,
                                                TWinControl* inParent )
: SeparateComment( inParam, inParent ),
  mpEdit( NULL )
{
  mpEdit = new TEdit( static_cast<TComponent*>( NULL ) );
  mpEdit->Left = valueOffsetX;
  mpEdit->Top = valueOffsetY;
  mpEdit->Width = valueWidth;
  mpEdit->ReadOnly = false;
  mpEdit->OnChange = OnEditChange;
  mpEdit->ShowHint = true;
  mpEdit->Visible = false;
  mpEdit->Parent = inParent;
  AddControl( mpEdit );
  AddHelp( inParam, mpEdit );
}

void
ParamDisplay::SingleEntryEdit::WriteValuesTo( Param& outParam ) const
{
  outParam.Value() = AnsiString( mpEdit->Text ).c_str();
  DisplayBase::WriteValuesTo( outParam );
}

void
ParamDisplay::SingleEntryEdit::ReadValuesFrom( const Param& inParam )
{
  mpEdit->Text = inParam.Value().c_str();
  DisplayBase::ReadValuesFrom( inParam );
}

void
__fastcall
ParamDisplay::SingleEntryEdit::OnEditChange( TObject* )
{
  mpEdit->Hint = mpEdit->Text;
  DisplayBase::OnContentChange();
}

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::List definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::List::List( const ParsedComment& inParam, TWinControl* inParent )
: SingleEntryEdit( inParam, inParent )
{
}

void
ParamDisplay::List::WriteValuesTo( Param& outParam ) const
{
  istringstream is( AnsiString( mpEdit->Text ).c_str() );
  EncodedString value;
  int index = 0;
  outParam.SetNumValues( 0 );
  while( is >> value )
  {
    outParam.SetNumValues( index + 1 );
    outParam.Value( index++ ) = value;
  }

  DisplayBase::WriteValuesTo( outParam );
}

void
ParamDisplay::List::ReadValuesFrom( const Param& inParam )
{
  ostringstream oss;
  if( inParam.NumValues() > 0 )
  {
    oss << EncodedString( inParam.Value( 0 ) );
    for( int i = 1; i < inParam.NumValues(); ++i )
      oss << ' ' << EncodedString( inParam.Value( i ) );
  }
  mpEdit->Text = oss.str().c_str();
  
  DisplayBase::ReadValuesFrom( inParam );
}

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::Matrix definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::Matrix::Matrix( const ParsedComment& inParam, TWinControl* inParent )
: SeparateComment( inParam, inParent ),
  mMatrixWindowOpen( false )
{
  TButton* editButton = new TButton( static_cast<TComponent*>( NULL ) );
  editButton->Caption = "Edit Matrix";
  editButton->Left = valueOffsetX;
  editButton->Top = valueOffsetY;
  editButton->Width = buttonWidth;
  editButton->Height = buttonHeight;
  editButton->OnClick = OnEditButtonClick;
  editButton->Visible = false;
  editButton->Parent = inParent;
  AddControl( editButton );

  TButton* loadButton = new TButton( static_cast<TComponent*>( NULL ) );
  loadButton->Caption = "Load Matrix";
  loadButton->Left = valueOffsetX + buttonWidth + buttonSpacingX;
  loadButton->Top = valueOffsetY;
  loadButton->Width = buttonWidth;
  loadButton->Height = buttonHeight;
  loadButton->OnClick = OnLoadButtonClick;
  loadButton->Visible = false;
  loadButton->Parent = inParent;
  AddControl( loadButton );

  TButton* saveButton = new TButton( static_cast<TComponent*>( NULL ) );
  saveButton->Caption = "Save Matrix";
  saveButton->Left = valueOffsetX + 2 * ( buttonWidth + buttonSpacingX );
  saveButton->Top = valueOffsetY;
  saveButton->Width = buttonWidth;
  saveButton->Height = buttonHeight;
  saveButton->OnClick = OnSaveButtonClick;
  saveButton->Visible = false;
  saveButton->Parent = inParent;
  AddControl( saveButton );
}

void
ParamDisplay::Matrix::WriteValuesTo( Param& outParam ) const
{
  outParam = mParam;
  DisplayBase::WriteValuesTo( outParam );
}

void
ParamDisplay::Matrix::ReadValuesFrom( const Param& inParam )
{
  mParam = inParam;
  if( mMatrixWindowOpen )
    fEditMatrix->SetDisplayedParam( &mParam );
  DisplayBase::ReadValuesFrom( inParam );
}

void
__fastcall
ParamDisplay::Matrix::OnEditButtonClick( TObject* )
{
  mMatrixWindowOpen = true;
  fEditMatrix->SetDisplayedParam( &mParam );
  fEditMatrix->ShowModal();
  mMatrixWindowOpen = false;
  DisplayBase::OnContentChange();
}

void
__fastcall
ParamDisplay::Matrix::OnLoadButtonClick( TObject* )
{
  TOpenDialog* loadDialog = new TOpenDialog( static_cast<TComponent*>( NULL ) );
  loadDialog->DefaultExt = MATRIX_EXTENSION;
  loadDialog->Filter = MATRIX_FILTER;
  loadDialog->Options << ofFileMustExist;
  if( loadDialog->Execute() )
  {
	AnsiString name = loadDialog->FileName;
	int result = OperatorUtils::LoadMatrix( name.c_str(), mParam );
    switch( result )
    {
      case ERR_NOERR:
        break;
      case ERR_MATLOADCOLSDIFF:
		Application->MessageBox(
		  VCLSTR( "Number of columns differs across rows" ),
		  VCLSTR( "Error" ),
		  MB_ICONERROR | MB_OK );
        break;
      case ERR_MATNOTFOUND:
		Application->MessageBox(
		  VCLSTR( "Could not open matrix data file" ),
		  VCLSTR( "Error" ),
		  MB_ICONERROR | MB_OK );
        break;
      default:
		Application->MessageBox(
		  VCLSTR( " Error loading the matrix file" ),
		  VCLSTR( "Error" ),
		  MB_ICONERROR | MB_OK );
    }
    DisplayBase::OnContentChange();
  }
  delete loadDialog;
}

void
__fastcall
ParamDisplay::Matrix::OnSaveButtonClick( TObject* )
{
  TSaveDialog* saveDialog = new TSaveDialog( static_cast<TComponent*>( NULL ) );
  saveDialog->DefaultExt = MATRIX_EXTENSION;
  saveDialog->Filter = MATRIX_FILTER;
  if( saveDialog->Execute() )
  {
	AnsiString name = saveDialog->FileName;
	int result = OperatorUtils::SaveMatrix( name.c_str(), mParam );
    switch( result )
    {
      case ERR_NOERR:
        break;
      case ERR_COULDNOTWRITE:
      default:
        {
          AnsiString message;
          message = "Could not write to file ";
          message += saveDialog->FileName;
		  Application->MessageBox(
			VCLSTR( message.c_str() ),
			VCLSTR( "Error" ),
			MB_OK );
        }
        break;
    }
  }
  delete saveDialog;
}

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::SingleEntryButton definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::SingleEntryButton::SingleEntryButton( const ParsedComment& inParam,
                                                    TWinControl* inParent )
: SingleEntryEdit( inParam, inParent )
{
  TButton* button = new TButton( static_cast<TComponent*>( NULL ) );
  button->Caption = "...";
  button->Left = valueOffsetX + valueWidth + buttonHeight / 2;
  button->Top = valueOffsetY;
  button->Width = buttonHeight;
  button->Height = buttonHeight;
  button->Visible = false;
  button->OnClick = OnButtonClick;
  button->Parent = inParent;
  AddControl( button );
  mComment = inParam.Comment();
}

void
__fastcall
ParamDisplay::SingleEntryButton::OnButtonClick( TObject* )
{
  ButtonClick();
}

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::SingleEntryInputFile definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::SingleEntryInputFile::SingleEntryInputFile( const ParsedComment& inParam,
                                                          TWinControl* inParent )
: SingleEntryButton( inParam, inParent )
{
}

void
ParamDisplay::SingleEntryInputFile::ButtonClick()
{
  TOpenDialog* dialog = new TOpenDialog( static_cast<TComponent*>( NULL ) );
  dialog->Title = AnsiString( "Choosing " ) + mComment.c_str();
  dialog->InitialDir = ::ExtractFilePath( mpEdit->Text );
  dialog->DefaultExt = "";
  dialog->Filter = ALLFILES_FILTER;
  dialog->Options.Clear();
  dialog->Options << ofFileMustExist << ofHideReadOnly
                  << ofNoChangeDir << ofDontAddToRecent;
  if( dialog->Execute() )
    mpEdit->Text = dialog->FileName;
  delete dialog;
}

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::SingleEntryOutputFile definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::SingleEntryOutputFile::SingleEntryOutputFile( const ParsedComment& inParam,
                                                            TWinControl* inParent )
: SingleEntryButton( inParam, inParent )
{
}

void
ParamDisplay::SingleEntryOutputFile::ButtonClick()
{
  TSaveDialog* dialog = new TSaveDialog( static_cast<TComponent*>( NULL ) );
  dialog->Title = AnsiString( "Choosing " ) + mComment.c_str();
  dialog->InitialDir = ::ExtractFilePath( mpEdit->Text );
  dialog->DefaultExt = "";
  dialog->Filter = ALLFILES_FILTER;
  dialog->Options.Clear();
  dialog->Options << ofPathMustExist << ofHideReadOnly
                  << ofNoChangeDir << ofDontAddToRecent
                  << ofNoReadOnlyReturn << ofOverwritePrompt;
  if( dialog->Execute() )
    mpEdit->Text = dialog->FileName;
  delete dialog;
}

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::SingleEntryDirectory definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::SingleEntryDirectory::SingleEntryDirectory( const ParsedComment& inParam,
                                                          TWinControl* inParent )
: SingleEntryButton( inParam, inParent )
{
}

void
ParamDisplay::SingleEntryDirectory::ButtonClick()
{
  LPMALLOC shellMalloc;
  if( ::SHGetMalloc( &shellMalloc ) == NO_ERROR )
  {
    char buffer[ MAX_PATH ];
    BROWSEINFO browseinfo =
    {
      mpEdit->Handle,
      NULL,
      buffer,
      mComment.c_str(),
      BIF_RETURNONLYFSDIRS,
      NULL, 0, NULL
    };
    ITEMIDLIST* result = ::SHBrowseForFolder( &browseinfo );
    if( result )
    {
      if( ::SHGetPathFromIDList( result, buffer ) )
        mpEdit->Text = buffer;
      shellMalloc->Free( result );
    }
    shellMalloc->Release();
  }
}

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::SingleEntryColor definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::SingleEntryColor::SingleEntryColor( const ParsedComment& inParam,
                                                  TWinControl* inParent )
: SingleEntryButton( inParam, inParent )
{
}

void
ParamDisplay::SingleEntryColor::ButtonClick()
{
  TColorDialog* dialog = new TColorDialog( static_cast<TComponent*>( NULL ) );
  RGBColor color;
  istringstream iss( AnsiString( mpEdit->Text ).c_str() );
  if( !( iss >> color ) )
    color = RGBColor::Black;
  dialog->Color = TColor( color.ToWinColor() );
  if( dialog->Execute() )
  {
    ostringstream oss;
    oss << RGBColor::FromWinColor( dialog->Color );
    mpEdit->Text = oss.str().c_str();
  }
  delete dialog;
}
////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::SingleEntryEnum definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::SingleEntryEnum::SingleEntryEnum( const ParsedComment& inParam,
                                                TWinControl* inParent )
: SeparateComment( inParam, inParent ),
  mpComboBox( NULL ),
  mIndexBase( inParam.IndexBase() )
{
  mpComboBox = new TComboBox( static_cast<TComponent*>( NULL ) );
  mpComboBox->Left = valueOffsetX;
  mpComboBox->Top = valueOffsetY;
  mpComboBox->Width = valueWidth;
  mpComboBox->Sorted = false;
  mpComboBox->Style = Stdctrls::csDropDownList;
  mpComboBox->Parent = inParent;
  for( size_t i = 0; i < inParam.Values().size(); ++i )
    mpComboBox->Items->Add( inParam.Values()[ i ].c_str() );
  mpComboBox->Hint = inParam.Comment().c_str();
  mpComboBox->ShowHint = true;
  mpComboBox->Visible = false;
  mpComboBox->OnChange = DisplayBase::OnContentChange;
  AddControl( mpComboBox );
  AddHelp( inParam, mpComboBox );
}

void
ParamDisplay::SingleEntryEnum::WriteValuesTo( Param& outParam ) const
{
  ostringstream oss;
  oss << mpComboBox->ItemIndex + mIndexBase;
  outParam.Value() = oss.str();
  SeparateComment::WriteValuesTo( outParam );
}

void
ParamDisplay::SingleEntryEnum::ReadValuesFrom( const Param& inParam )
{
  mpComboBox->ItemIndex = ::atoi( inParam.Value().c_str() ) - mIndexBase;
  SeparateComment::ReadValuesFrom( inParam );
}

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::SingleEntryBoolean definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::SingleEntryBoolean::SingleEntryBoolean( const ParsedComment& inParam,
                                                      TWinControl* inParent )
: DisplayBase( inParam, inParent ),
  mpCheckBox( NULL )
{
  mpCheckBox = new TCheckBox( static_cast<TComponent*>( NULL ) );
  mpCheckBox->Left = valueOffsetX;
  mpCheckBox->Top = valueOffsetY;
  mpCheckBox->Width = valueWidth;
  mpCheckBox->Caption = inParam.Comment().c_str();
  mpCheckBox->Hint = mpCheckBox->Caption;
  mpCheckBox->ShowHint = true;
  mpCheckBox->Visible = false;
  mpCheckBox->Parent = inParent;
  mpCheckBox->OnClick = DisplayBase::OnContentChange;
  AddControl( mpCheckBox );
  AddHelp( inParam, mpCheckBox );
}

void
ParamDisplay::SingleEntryBoolean::WriteValuesTo( Param& outParam ) const
{
  outParam.Value() = ( mpCheckBox->Checked ? "1" : "0" );
  DisplayBase::WriteValuesTo( outParam );
}

void
ParamDisplay::SingleEntryBoolean::ReadValuesFrom( const Param& inParam )
{
  mpCheckBox->Checked = ::atoi( inParam.Value().c_str() );
  DisplayBase::ReadValuesFrom( inParam );
}


