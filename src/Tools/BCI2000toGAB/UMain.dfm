object fMain: TfMain
  Left = 492
  Top = 265
  Width = 470
  Height = 416
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
    462
    389)
  PixelsPerInch = 96
  TextHeight = 13
  object Gauge: TCGauge
    Left = 16
    Top = 316
    Width = 433
    Height = 25
    Anchors = [akLeft, akRight, akBottom]
  end
  object mProgressLegend: TLabel
    Left = 16
    Top = 296
    Width = 3
    Height = 13
    Anchors = [akTop, akBottom]
  end
  object GroupBox3: TGroupBox
    Left = 16
    Top = 117
    Width = 433
    Height = 49
    Anchors = [akLeft, akRight, akBottom]
    Caption = 'Output File'
    TabOrder = 10
  end
  object GroupBox2: TGroupBox
    Left = 16
    Top = 8
    Width = 433
    Height = 94
    Anchors = [akLeft, akTop, akRight, akBottom]
    Caption = 'BCI2000 File(s)'
    TabOrder = 9
  end
  object mSourceFiles: TMemo
    Left = 64
    Top = 24
    Width = 377
    Height = 70
    Anchors = [akLeft, akTop, akRight, akBottom]
    Lines.Strings = (
      'c:\shared\raw\em180.dat')
    ScrollBars = ssVertical
    TabOrder = 6
    OnChange = mSourceFilesChange
  end
  object GroupBox1: TGroupBox
    Left = 16
    Top = 176
    Width = 433
    Height = 102
    Anchors = [akLeft, akRight, akBottom]
    Caption = 'Options'
    TabOrder = 8
    DesignSize = (
      433
      102)
    object Label3: TLabel
      Left = 8
      Top = 45
      Width = 128
      Height = 13
      Anchors = [akLeft, akBottom]
      Caption = 'Use external parameter file:'
    end
  end
  object bConvert: TButton
    Left = 373
    Top = 353
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Convert'
    Default = True
    TabOrder = 0
    OnClick = bConvertClick
  end
  object eDestinationFile: TEdit
    Left = 64
    Top = 135
    Width = 377
    Height = 21
    Anchors = [akLeft, akRight, akBottom]
    TabOrder = 1
    Text = 'c:\shared\raw\em180.raw'
  end
  object bOpenFile: TButton
    Left = 24
    Top = 24
    Width = 33
    Height = 21
    Caption = 'Find...'
    TabOrder = 2
    OnClick = bOpenFileClick
  end
  object Button2: TButton
    Left = 24
    Top = 239
    Width = 33
    Height = 21
    Anchors = [akLeft, akBottom]
    Caption = 'Find...'
    TabOrder = 4
    OnClick = Button2Click
  end
  object ParameterFile: TEdit
    Left = 64
    Top = 239
    Width = 377
    Height = 21
    Anchors = [akLeft, akRight, akBottom]
    Enabled = False
    TabOrder = 5
    Text = '<none>'
  end
  object AutoscaleCheckbox: TCheckBox
    Left = 24
    Top = 192
    Width = 273
    Height = 23
    Anchors = [akLeft, akBottom]
    Caption = 'Auto-scale input signal to fit into 16-bit output signal'
    Checked = True
    State = cbChecked
    TabOrder = 7
  end
  object Button1: TButton
    Left = 24
    Top = 135
    Width = 33
    Height = 21
    Anchors = [akLeft, akBottom]
    Caption = 'Find...'
    TabOrder = 3
    OnClick = Button1Click
  end
  object OpenDialog: TOpenDialog
    Filter = 'BCI2000 EEG files (*.DAT)|*.DAT|All Files (*.*)|*.*'
    Options = [ofHideReadOnly, ofFileMustExist, ofEnableSizing]
    Left = 256
    Top = 48
  end
  object SaveDialog: TSaveDialog
    Left = 256
    Top = 136
  end
  object OpenParameter: TOpenDialog
    Filter = '*.prm|*.prm|all files|*.*'
    Left = 288
    Top = 240
  end
end
