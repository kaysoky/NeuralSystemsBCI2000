object fMain: TfMain
  Left = 571
  Top = 239
  BorderStyle = bsSingle
  Caption = 'BCI2000FileInfo'
  ClientHeight = 159
  ClientWidth = 390
  Color = clCream
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object tFileName: TLabel
    Left = 8
    Top = 24
    Width = 20
    Height = 13
    Caption = 'N/A'
  end
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 51
    Height = 13
    Caption = 'Filename'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label2: TLabel
    Left = 8
    Top = 56
    Width = 121
    Height = 13
    Caption = 'Size of Sample Block'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label3: TLabel
    Left = 8
    Top = 72
    Width = 83
    Height = 13
    Caption = 'Sampling Rate'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label4: TLabel
    Left = 8
    Top = 88
    Width = 102
    Height = 13
    Caption = 'Feedback Update'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object tSampleBlockSize: TLabel
    Left = 152
    Top = 56
    Width = 20
    Height = 13
    Caption = 'N/A'
  end
  object tSamplingRate: TLabel
    Left = 152
    Top = 72
    Width = 20
    Height = 13
    Caption = 'N/A'
  end
  object tUpdateRate: TLabel
    Left = 152
    Top = 88
    Width = 20
    Height = 13
    Caption = 'N/A'
  end
  object Label5: TLabel
    Left = 8
    Top = 112
    Width = 109
    Height = 13
    Caption = 'File Format Version'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object tFileFormat: TLabel
    Left = 152
    Top = 112
    Width = 20
    Height = 13
    Caption = 'N/A'
  end
  object Label6: TLabel
    Left = 8
    Top = 128
    Width = 70
    Height = 13
    Caption = 'Data Format'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object tDataFormat: TLabel
    Left = 152
    Top = 128
    Width = 20
    Height = 13
    Caption = 'N/A'
  end
  object DropPanel: TPanel
    Left = 272
    Top = 56
    Width = 113
    Height = 57
    Caption = 'drop files here'
    TabOrder = 0
  end
  object bShowParams: TButton
    Left = 272
    Top = 120
    Width = 113
    Height = 33
    Caption = 'Show Parameters'
    Enabled = False
    TabOrder = 1
    OnClick = bShowParamsClick
  end
  object MainMenu1: TMainMenu
    Left = 320
    Top = 65528
    object File1: TMenuItem
      Caption = 'File'
      object Open1: TMenuItem
        Caption = 'Open...'
        ShortCut = 16463
        OnClick = Open1Click
      end
      object Quit1: TMenuItem
        Caption = 'Quit'
        ShortCut = 16465
        OnClick = Quit1Click
      end
    end
    object Edit1: TMenuItem
      Caption = 'Edit'
      object Cut1: TMenuItem
        Caption = 'Cut'
        Enabled = False
        ShortCut = 16472
      end
      object Copy: TMenuItem
        Caption = 'Copy'
        Enabled = False
        ShortCut = 16451
      end
      object Paste1: TMenuItem
        Caption = 'Paste'
        Enabled = False
        ShortCut = 16470
      end
    end
    object Help1: TMenuItem
      Caption = 'Help'
      object mHelpOpenHelp: TMenuItem
        Caption = 'BCI2000 Help'
        OnClick = HelpOpenHelp
      end
      object About1: TMenuItem
        Caption = 'About...'
        OnClick = HelpAbout
      end
    end
  end
  object OpenDialog1: TOpenDialog
    Filter = 'BCI2000 Data Files (*.DAT)|*.dat|All Files (*.*)|*.*'
    Left = 352
    Top = 65528
  end
end
