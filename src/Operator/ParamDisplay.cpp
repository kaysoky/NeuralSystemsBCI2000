#include "PCHIncludes.h"
#pragma hdrstop

#include "ParamDisplay.h"
#include "UParameter.h"
#include "UOperatorUtils.h"
#include "UPreferences.h"
#include "UEditMatrix.h"
#include "ParsedComment.h"

#include <vector>
#include <string>
#include <assert>

#define ALLFILES_FILTER "All files (*.*)|*.*"
#define MATRIX_EXTENSION ".txt"
#define MATRIX_FILTER "Space delimited matrix file (*" MATRIX_EXTENSION ")"  \
                                       "|*" MATRIX_EXTENSION "|" ALLFILES_FILTER
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay definitions
////////////////////////////////////////////////////////////////////////////////
map<ParamDisplay::DisplayBase*, int> ParamDisplay::RefCount;

ParamDisplay::ParamDisplay()
: mpDisplay( NULL )
{
}

ParamDisplay::ParamDisplay( const PARAM& inParam, TWinControl* inParent )
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
  ++RefCount[ mpDisplay ];
}

ParamDisplay::ParamDisplay( const ParamDisplay& inOriginal )
: mpDisplay( inOriginal.mpDisplay )
{
  ++RefCount[ mpDisplay ];
}

const ParamDisplay&
ParamDisplay::operator=( const ParamDisplay& inOriginal )
{
  --RefCount[ mpDisplay ];
  mpDisplay = inOriginal.mpDisplay;
  ++RefCount[ mpDisplay ];
  return *this;
}

ParamDisplay::~ParamDisplay()
{
  if( --RefCount[ mpDisplay ] < 1 )
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
ParamDisplay::WriteValuesTo( PARAM& p ) const
{
  mpDisplay->WriteValuesTo( p );
}

void
ParamDisplay::ReadValuesFrom( const PARAM& p )
{
  mpDisplay->ReadValuesFrom( p );
}

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::DisplayBase definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::DisplayBase::DisplayBase( const ParsedComment& inParam,
                                        TWinControl* inParent )
: mpUserLevel( NULL ),
  mTop( 0 ),
  mLeft( 0 )
{
  // render the parameter's name
  TLabel* label = new TLabel( static_cast<TComponent*>( NULL ) );
  label->Left = LABELS_OFFSETX;
  label->Top = LABELS_OFFSETY;
  label->Caption = inParam.Name().c_str();
  label->Font->Style = TFontStyles() << fsBold;
  label->Visible = false;
  label->Hint = inParam.Comment().c_str();
  label->ShowHint = true;
  label->Parent = inParent;
  mControls.insert( label );

  // render the parameter's User Level track bar
  // ONLY, if the current user level is "advanced"
  if( OperatorUtils::UserLevel() == USERLEVEL_ADVANCED )
  {
    mpUserLevel = new TTrackBar( static_cast<TComponent*>( NULL ) );
    mpUserLevel->Left = USERLEVEL_OFFSETX;
    mpUserLevel->Top = USERLEVEL_OFFSETY;
    mpUserLevel->Width = USERLEVEL_WIDTH;
    mpUserLevel->Height = USERLEVEL_HEIGHT;
    mpUserLevel->Min = 1;
    mpUserLevel->Max = 3;
    mpUserLevel->PageSize = 1;
    mpUserLevel->Visible = false;
    mpUserLevel->Parent = inParent;
    mControls.insert( mpUserLevel );
  }
}

ParamDisplay::DisplayBase::~DisplayBase()
{
  for( set<TControl*>::iterator i = mControls.begin(); i != mControls.end(); ++i )
    delete *i;
}

void
ParamDisplay::DisplayBase::SetTop( int inTop )
{
  for( set<TControl*>::iterator i = mControls.begin(); i != mControls.end(); ++i )
    ( *i )->Top = ( *i )->Top - mTop + inTop;
  mTop = inTop;
}

void
ParamDisplay::DisplayBase::SetLeft( int inLeft )
{
  for( set<TControl*>::iterator i = mControls.begin(); i != mControls.end(); ++i )
    ( *i )->Left = ( *i )->Left - mLeft + inLeft;
  mLeft = inLeft;
}

int
ParamDisplay::DisplayBase::GetBottom()
{
  int bottom = 0;
  for( set<TControl*>::iterator i = mControls.begin(); i != mControls.end(); ++i )
    bottom = max( bottom, ( *i )->Top + ( *i )->Height );
  return bottom;
}

int
ParamDisplay::DisplayBase::GetRight()
{
  int right = 0;
  for( set<TControl*>::iterator i = mControls.begin(); i != mControls.end(); ++i )
    right = max( right, ( *i )->Left + ( *i )->Width );
  return right;
}

void
ParamDisplay::DisplayBase::Hide()
{
  for( set<TControl*>::iterator i = mControls.begin(); i != mControls.end(); ++i )
    ( *i )->Hide();
}

void
ParamDisplay::DisplayBase::Show()
{
  for( set<TControl*>::iterator i = mControls.begin(); i != mControls.end(); ++i )
    ( *i )->Show();
}

void
ParamDisplay::DisplayBase::WriteValuesTo( PARAM& inParam ) const
{
  if( mpUserLevel )
    OperatorUtils::SetUserLevel( inParam.GetName(), mpUserLevel->Position );
}

void
ParamDisplay::DisplayBase::ReadValuesFrom( const PARAM& inParam )
{
  if( mpUserLevel )
    mpUserLevel->Position = OperatorUtils::GetUserLevel( inParam.GetName() );
}

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::SeparateComment definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::SeparateComment::SeparateComment( const ParsedComment& inParam,
                                                TWinControl* inParent )
: DisplayBase( inParam, inParent )
{
  // render the parameter's comment
  TLabel* comment = new TLabel( static_cast<TComponent*>( NULL ) );
  comment->Left = COMMENT_OFFSETX;
  comment->Top = COMMENT_OFFSETY;
  comment->Caption = inParam.Comment().c_str();
  comment->Hint = comment->Caption;
  comment->ShowHint = true;
  comment->Font->Style = TFontStyles() << fsItalic;
  comment->Visible = false;
  comment->Parent = inParent;
  mControls.insert( comment );
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
  mpEdit->Left = VALUE_OFFSETX;
  mpEdit->Top = VALUE_OFFSETY;
  mpEdit->Width = VALUE_WIDTH;
  mpEdit->ReadOnly = false;
  mpEdit->OnChange = OnEditChange;
  mpEdit->ShowHint = true;
  mpEdit->Visible = false;
  mpEdit->Parent = inParent;
  mControls.insert( mpEdit );
}

void
ParamDisplay::SingleEntryEdit::WriteValuesTo( PARAM& outParam ) const
{
  DisplayBase::WriteValuesTo( outParam );
  outParam.SetValue( mpEdit->Text.c_str() );
}

void
ParamDisplay::SingleEntryEdit::ReadValuesFrom( const PARAM& inParam )
{
  DisplayBase::ReadValuesFrom( inParam );
  mpEdit->Text = inParam.GetValue();
}

void
__fastcall
ParamDisplay::SingleEntryEdit::OnEditChange( TObject* )
{
  mpEdit->Hint = mpEdit->Text;
}

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::List definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::List::List( const ParsedComment& inParam, TWinControl* inParent )
: SingleEntryEdit( inParam, inParent )
{
}

void
ParamDisplay::List::WriteValuesTo( PARAM& outParam ) const
{
  DisplayBase::WriteValuesTo( outParam );
  istringstream is( mpEdit->Text.c_str() );
  PARAM::encodedString value;
  int index = 0;
  while( is >> value )
    outParam.SetValue( value, index++ );
  outParam.SetNumValues( index );
}

void
ParamDisplay::List::ReadValuesFrom( const PARAM& inParam )
{
  DisplayBase::ReadValuesFrom( inParam );
  ostringstream oss;
  oss << PARAM::encodedString( inParam.GetValue( 0 ) );
  for( size_t i = 1; i < inParam.GetNumValues(); ++i )
    oss << ' ' << PARAM::encodedString( inParam.GetValue( i ) );
  mpEdit->Text = oss.str().c_str();
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
  editButton->Left = VALUE_OFFSETX;
  editButton->Top = VALUE_OFFSETY;
  editButton->Width = BUTTON_WIDTH;
  editButton->Height = BUTTON_HEIGHT;
  editButton->OnClick = OnEditButtonClick;
  editButton->Visible = false;
  editButton->Parent = inParent;
  mControls.insert( editButton );

  TButton* loadButton = new TButton( static_cast<TComponent*>( NULL ) );
  loadButton->Caption = "Load Matrix";
  loadButton->Left = VALUE_OFFSETX + BUTTON_WIDTH + BUTTON_SPACINGX;
  loadButton->Top = VALUE_OFFSETY;
  loadButton->Width = BUTTON_WIDTH;
  loadButton->Height = BUTTON_HEIGHT;
  loadButton->OnClick = OnLoadButtonClick;
  loadButton->Visible = false;
  loadButton->Parent = inParent;
  mControls.insert( loadButton );

  TButton* saveButton = new TButton( static_cast<TComponent*>( NULL ) );
  saveButton->Caption = "Save Matrix";
  saveButton->Left = VALUE_OFFSETX + 2 * ( BUTTON_WIDTH + BUTTON_SPACINGX );
  saveButton->Top = VALUE_OFFSETY;
  saveButton->Width = BUTTON_WIDTH;
  saveButton->Height = BUTTON_HEIGHT;
  saveButton->OnClick = OnSaveButtonClick;
  saveButton->Visible = false;
  saveButton->Parent = inParent;
  mControls.insert( saveButton );
}

void
ParamDisplay::Matrix::WriteValuesTo( PARAM& outParam ) const
{
  DisplayBase::WriteValuesTo( outParam );
  outParam = mParam;
}

void
ParamDisplay::Matrix::ReadValuesFrom( const PARAM& inParam )
{
  DisplayBase::ReadValuesFrom( inParam );
  mParam = inParam;
  if( mMatrixWindowOpen )
    fEditMatrix->SetDisplayedParam( &mParam );
}

void
__fastcall
ParamDisplay::Matrix::OnEditButtonClick( TObject* )
{
  mMatrixWindowOpen = true;
  fEditMatrix->SetDisplayedParam( &mParam );
  fEditMatrix->ShowModal();
  mMatrixWindowOpen = false;
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
    int result = OperatorUtils::LoadMatrix( loadDialog->FileName.c_str(), mParam );
    switch( result )
    {
      case ERR_NOERR:
        break;
      case ERR_MATLOADCOLSDIFF:
        Application->MessageBox( "Number of columns in rows is different", "Error", MB_OK );
        break;
      case ERR_MATNOTFOUND:
        Application->MessageBox( "Could not open matrix data file", "Error", MB_OK );
        break;
      default:
        Application->MessageBox(" Error loading the matrix file", "Error", MB_OK );
    }
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
    int result = OperatorUtils::SaveMatrix( saveDialog->FileName.c_str(), mParam );
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
          Application->MessageBox( message.c_str(), "Error", MB_OK );
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
  button->Left = VALUE_OFFSETX + VALUE_WIDTH + BUTTON_HEIGHT / 2;
  button->Top = VALUE_OFFSETY;
  button->Width = BUTTON_HEIGHT;
  button->Height = BUTTON_HEIGHT;
  button->Visible = false;
  button->OnClick = OnButtonClick;
  button->Parent = inParent;
  mControls.insert( button );
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
  try
  {
    dialog->Color = StringToColor( mpEdit->Text );
  }
  catch( EConvertError& )
  {
    dialog->Color = clBlack;
  }
  if( dialog->Execute() )
    mpEdit->Text = AnsiString( "0x" ) + IntToHex( ColorToRGB( dialog->Color ), 6 );
  delete dialog;
}
////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::SingleEntryEnum definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::SingleEntryEnum::SingleEntryEnum( const ParsedComment& inParam,
                                                TWinControl* inParent )
: SeparateComment( inParam, inParent ),
  mComboBox( NULL ),
  mIndexBase( inParam.IndexBase() )
{
  mComboBox = new TComboBox( static_cast<TComponent*>( NULL ) );
  mComboBox->Left = VALUE_OFFSETX;
  mComboBox->Top = VALUE_OFFSETY;
  mComboBox->Width = VALUE_WIDTH;
  mComboBox->Sorted = false;
  mComboBox->Style = csDropDownList;
  mComboBox->Parent = inParent;
  for( size_t i = 0; i < inParam.Values().size(); ++i )
    mComboBox->Items->Add( inParam.Values()[ i ].c_str() );
  mComboBox->Hint = inParam.Comment().c_str();
  mComboBox->ShowHint = true;
  mComboBox->Visible = false;
  mControls.insert( mComboBox );
}

void
ParamDisplay::SingleEntryEnum::WriteValuesTo( PARAM& outParam ) const
{
  ostringstream oss;
  oss << mComboBox->ItemIndex + mIndexBase;
  outParam.SetValue( oss.str().c_str() );
}

void
ParamDisplay::SingleEntryEnum::ReadValuesFrom( const PARAM& inParam )
{
  mComboBox->ItemIndex = ::atoi( inParam.GetValue() ) - mIndexBase;
}

////////////////////////////////////////////////////////////////////////////////
// ParamDisplay::SingleEntryBoolean definitions
////////////////////////////////////////////////////////////////////////////////
ParamDisplay::SingleEntryBoolean::SingleEntryBoolean( const ParsedComment& inParam,
                                                      TWinControl* inParent )
: DisplayBase( inParam, inParent ),
  mCheckBox( NULL )
{
  mCheckBox = new TCheckBox( static_cast<TComponent*>( NULL ) );
  mCheckBox->Left = VALUE_OFFSETX;
  mCheckBox->Top = VALUE_OFFSETY;
  mCheckBox->Width = VALUE_WIDTH;
  mCheckBox->Caption = inParam.Comment().c_str();
  mCheckBox->Hint = mCheckBox->Caption;
  mCheckBox->ShowHint = true;
  mCheckBox->Visible = false;
  mCheckBox->Parent = inParent;
  mControls.insert( mCheckBox );
}

void
ParamDisplay::SingleEntryBoolean::WriteValuesTo( PARAM& outParam ) const
{
  outParam.SetValue( mCheckBox->Checked ? "1" : "0" );
}

void
ParamDisplay::SingleEntryBoolean::ReadValuesFrom( const PARAM& inParam )
{
  mCheckBox->Checked = ::atoi( inParam.GetValue() );
}


