object fMain: TfMain
  Left = 673
  Top = 408
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
    Top = 342
    Width = 87
    Height = 13
    Anchors = [akBottom]
    Caption = '<progress legend>'
  end
  object mOptionsGroupBox: TGroupBox
    Left = 16
    Top = 176
    Width = 433
    Height = 129
    Anchors = [akLeft, akRight, akBottom]
    Caption = 'Options'
    TabOrder = 3
    DesignSize = (
      433
      129)
    object Label3: TLabel
      Left = 8
      Top = 80
      Width = 128
      Height = 13
      Anchors = [akLeft, akBottom]
      Caption = 'Use external parameter file:'
    end
    object Label1: TLabel
      Left = 112
      Top = 25
      Width = 65
      Height = 13
      Caption = 'channels only'
    end
    object mSubSetCheckbox: TCheckBox
      Left = 8
      Top = 24
      Width = 65
      Height = 17
      Caption = 'Use first'
      TabOrder = 0
      OnClick = mSubSetCheckboxClick
    end
    object mSubSetEdit: TEdit
      Left = 72
      Top = 22
      Width = 33
      Height = 21
      Enabled = False
      TabOrder = 1
      Text = '1'
      OnChange = mSubSetEditChange
    end
    object Button2: TButton
      Left = 8
      Top = 100
      Width = 33
      Height = 21
      Anchors = [akLeft, akBottom]
      Caption = 'Find...'
      TabOrder = 3
      OnClick = Button2Click
    end
    object AutoscaleCheckbox: TCheckBox
      Left = 8
      Top = 50
      Width = 273
      Height = 23
      Anchors = [akLeft, akBottom]
      Caption = 'Auto-scale input signal to fit into 16-bit output signal'
      Checked = True
      State = cbChecked
      TabOrder = 2
    end
    object ParameterFile: TEdit
      Left = 48
      Top = 100
      Width = 377
      Height = 21
      Anchors = [akLeft, akRight, akBottom]
      Enabled = False
      TabOrder = 4
      Text = '<none>'
    end
  end
  object mOutputGroupBox: TGroupBox
    Left = 16
    Top = 117
    Width = 433
    Height = 49
    Anchors = [akLeft, akRight, akBottom]
    Caption = 'Output File'
    TabOrder = 2
    DesignSize = (
      433
      49)
    object eDestinationFile: TEdit
      Left = 48
      Top = 20
      Width = 377
      Height = 21
      Anchors = [akLeft, akRight, akBottom]
      TabOrder = 1
      Text = 'c:\shared\raw\em180.raw'
    end
    object Button1: TButton
      Left = 8
      Top = 20
      Width = 33
      Height = 21
      Anchors = [akLeft, akBottom]
      Caption = 'Find...'
      TabOrder = 0
      OnClick = Button1Click
    end
  end
  object mInputGroupBox: TGroupBox
    Left = 16
    Top = 8
    Width = 433
    Height = 94
    Anchors = [akLeft, akTop, akRight, akBottom]
    Caption = 'BCI2000 File(s)'
    TabOrder = 1
    DesignSize = (
      433
      94)
    object bOpenFile: TButton
      Left = 8
      Top = 16
      Width = 33
      Height = 21
      Caption = 'Find...'
      TabOrder = 0
      OnClick = bOpenFileClick
    end
    object mSourceFiles: TMemo
      Left = 48
      Top = 16
      Width = 377
      Height = 70
      Anchors = [akLeft, akTop, akRight, akBottom]
      Lines.Strings = (
        'c:\shared\raw\em180.dat')
      ScrollBars = ssVertical
      TabOrder = 1
      OnChange = mSourceFilesChange
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
    Left = 312
    Top = 216
  end
end
