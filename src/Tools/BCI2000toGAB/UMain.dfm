object fMain: TfMain
  Left = 490
  Top = 221
  Width = 425
  Height = 284
  Caption = 'BCI2000toGAB  v0.3  9/13/01'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Icon.Data = {
    0000010001002020100000000000E80200001600000028000000200000004000
    0000010004000000000080020000000000000000000000000000000000000000
    000000008000008000000080800080000000800080008080000080808000C0C0
    C0000000FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00BBBB
    BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB999BBB9BBBBB9B99999BBBBBBBBBBB
    B9BBB9BB9BBBBB9B9BBBB9BBBBBBBBBB9BBBBB9BB99999BB9BBBB9BBBBBBBBBB
    9BBB999BB9BBB9BB9BBBB9BBBBBBBBBB9BBBBBBBBB9B9BBB999999BBBBBBBBBB
    9BBBBB9BBB9B9BBB9BBBB9BBBBBBBBBBB9BBB9BBBB9B9BBB9BBBB9BBBBBBBBBB
    BB999BBBBBB9BBBB99999BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
    BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB99999BBBBBBBBBBBBBBBBBB
    BBBBBBBBBB9BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB9BBBBBBBBBBBBBBBBBBBB
    BBBBBBBBBBBB9BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB9BBBBBBBBBBBBBBBBBB
    BBBBBBBBBBBBB9BBBBBBBBBBBBBBBBBBBBBBBBBBB9BBB9BBBBBBBBBBBBBBBBBB
    BBBBBBBBBB999BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
    BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB99999BB999BBB999BBB999BBBBBBBBBBB
    9BBBB9BBB9B9BBB9B9BBB9BBBBBBBBBBB9BBB9BBB9B9BBB9B9BBB9BBBBBBBBBB
    BB9BB9BBB9B9BBB9B9BBB9BBBBBBBBBBBBB9B9BBB9B9BBB9B9BBB9BBBBBBBBBB
    BBB9B9BBB9B9BBB9B9BBB9BBBBBBBBB9BBB9B9BBB9B9BBB9B9BBB9BBBBBBBBBB
    999BBB999BBB999BBB999BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
    BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB0000
    0000000000000000000000000000000000000000000000000000000000000000
    0000000000000000000000000000000000000000000000000000000000000000
    0000000000000000000000000000000000000000000000000000000000000000
    000000000000000000000000000000000000000000000000000000000000}
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 40
    Top = 32
    Width = 74
    Height = 16
    Caption = 'BCI2000 File'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label2: TLabel
    Left = 40
    Top = 88
    Width = 53
    Height = 16
    Caption = 'GAB File'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Gauge: TCGauge
    Left = 40
    Top = 208
    Width = 353
    Height = 25
  end
  object Label3: TLabel
    Left = 272
    Top = 48
    Width = 34
    Height = 13
    Caption = 'first run'
  end
  object Label4: TLabel
    Left = 272
    Top = 80
    Width = 34
    Height = 13
    Caption = 'last run'
  end
  object Label5: TLabel
    Left = 40
    Top = 144
    Width = 155
    Height = 16
    Caption = 'Calibration Parameter File'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object bConvert: TButton
    Left = 328
    Top = 8
    Width = 75
    Height = 25
    Caption = 'Load List'
    TabOrder = 0
    OnClick = bConvertClick
  end
  object eSourceFile: TEdit
    Left = 40
    Top = 48
    Width = 201
    Height = 21
    TabOrder = 1
    Text = 'c:\shared\raw\em180.dat'
  end
  object eDestinationFile: TEdit
    Left = 40
    Top = 104
    Width = 201
    Height = 21
    TabOrder = 2
    Text = 'c:\shared\raw\em180.raw'
  end
  object bOpenFile: TButton
    Left = 0
    Top = 48
    Width = 33
    Height = 21
    Caption = 'File'
    TabOrder = 3
    OnClick = bOpenFileClick
  end
  object Button1: TButton
    Left = 0
    Top = 104
    Width = 33
    Height = 21
    Caption = 'File'
    TabOrder = 4
    OnClick = Button1Click
  end
  object Continue: TButton
    Left = 328
    Top = 112
    Width = 75
    Height = 25
    Caption = 'Convert'
    Enabled = False
    TabOrder = 5
    OnClick = ContinueClick
  end
  object frun: TEdit
    Left = 320
    Top = 48
    Width = 89
    Height = 21
    TabOrder = 6
    Text = 'firstr'
  end
  object lrun: TEdit
    Left = 320
    Top = 80
    Width = 89
    Height = 21
    TabOrder = 7
    Text = 'lastr'
  end
  object Button2: TButton
    Left = 0
    Top = 160
    Width = 33
    Height = 21
    Caption = 'File'
    TabOrder = 8
    OnClick = Button2Click
  end
  object ParameterFile: TEdit
    Left = 40
    Top = 160
    Width = 201
    Height = 21
    Enabled = False
    TabOrder = 9
    Text = '<none>'
  end
  object OpenDialog: TOpenDialog
    Filter = 'BCI2000 EEG files (*.DAT)|*.DAT|All Files (*.*)|*.*'
    Options = [ofHideReadOnly, ofFileMustExist, ofEnableSizing]
    Top = 72
  end
  object SaveDialog: TSaveDialog
    Top = 128
  end
  object OpenParameter: TOpenDialog
    Filter = '*.prm|*.prm|all files|*.*'
    Top = 176
  end
end
