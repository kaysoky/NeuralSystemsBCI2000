object fMain: TfMain
  Left = 583
  Top = 279
  Width = 425
  Height = 343
  Caption = 'BCI2000toGAB'
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
  DesignSize = (
    417
    316)
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 48
    Top = 8
    Width = 89
    Height = 16
    Caption = 'BCI2000 File(s)'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label2: TLabel
    Left = 48
    Top = 134
    Width = 94
    Height = 16
    Anchors = [akLeft, akBottom]
    Caption = 'Output GAB File'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Gauge: TCGauge
    Left = 48
    Top = 243
    Width = 353
    Height = 25
    Anchors = [akLeft, akRight, akBottom]
  end
  object Label5: TLabel
    Left = 48
    Top = 182
    Width = 139
    Height = 16
    Anchors = [akLeft, akBottom]
    Caption = 'External Parameter File'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object bConvert: TButton
    Left = 328
    Top = 280
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Convert'
    Default = True
    TabOrder = 0
    OnClick = bConvertClick
  end
  object eDestinationFile: TEdit
    Left = 48
    Top = 150
    Width = 353
    Height = 21
    Anchors = [akLeft, akRight, akBottom]
    TabOrder = 1
    Text = 'c:\shared\raw\em180.raw'
  end
  object bOpenFile: TButton
    Left = 8
    Top = 24
    Width = 33
    Height = 21
    Caption = 'File(s)'
    TabOrder = 2
    OnClick = bOpenFileClick
  end
  object Button1: TButton
    Left = 8
    Top = 150
    Width = 33
    Height = 21
    Anchors = [akLeft, akBottom]
    Caption = 'File'
    TabOrder = 3
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 8
    Top = 198
    Width = 33
    Height = 21
    Anchors = [akLeft, akBottom]
    Caption = 'File'
    TabOrder = 4
    OnClick = Button2Click
  end
  object ParameterFile: TEdit
    Left = 48
    Top = 198
    Width = 353
    Height = 21
    Anchors = [akLeft, akRight, akBottom]
    Enabled = False
    TabOrder = 5
    Text = '<none>'
  end
  object mSourceFiles: TMemo
    Left = 48
    Top = 24
    Width = 353
    Height = 95
    Anchors = [akLeft, akTop, akRight, akBottom]
    Lines.Strings = (
      'c:\shared\raw\em180.dat')
    ScrollBars = ssVertical
    TabOrder = 6
  end
  object OpenDialog: TOpenDialog
    Filter = 'BCI2000 EEG files (*.DAT)|*.DAT|All Files (*.*)|*.*'
    Options = [ofHideReadOnly, ofFileMustExist, ofEnableSizing]
    Left = 8
    Top = 40
  end
  object SaveDialog: TSaveDialog
    Left = 8
    Top = 168
  end
  object OpenParameter: TOpenDialog
    Filter = '*.prm|*.prm|all files|*.*'
    Left = 8
    Top = 216
  end
end
