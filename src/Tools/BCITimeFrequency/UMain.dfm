object fMain: TfMain
  Left = 271
  Top = 132
  Width = 550
  Height = 517
  Caption = 'BCI TimeFrequency 1/22/04'
  Color = clSkyBlue
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 48
    Top = 16
    Width = 87
    Height = 16
    Caption = 'BCI2000 File'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label2: TLabel
    Left = 48
    Top = 64
    Width = 75
    Height = 16
    Caption = 'Output File'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Gauge: TCGauge
    Left = 40
    Top = 400
    Width = 441
    Height = 25
  end
  object Label5: TLabel
    Left = 48
    Top = 112
    Width = 110
    Height = 16
    Caption = 'Calibration  File'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label3: TLabel
    Left = 48
    Top = 160
    Width = 103
    Height = 16
    Caption = 'Parameter File'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Convert: TButton
    Left = 392
    Top = 224
    Width = 89
    Height = 25
    Caption = 'Convert'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 0
    OnClick = ConvertClick
  end
  object eSourceFile: TEdit
    Left = 48
    Top = 32
    Width = 329
    Height = 21
    TabOrder = 1
    Text = 'c:\p300\aah.dat'
  end
  object eDestinationFile: TEdit
    Left = 48
    Top = 80
    Width = 329
    Height = 21
    TabOrder = 2
    Text = 'c:\p300\aah.asc'
  end
  object bOpenFile: TButton
    Left = 16
    Top = 32
    Width = 33
    Height = 21
    Caption = 'File'
    TabOrder = 3
    OnClick = bOpenFileClick
  end
  object Button1: TButton
    Left = 16
    Top = 80
    Width = 33
    Height = 21
    Caption = 'File'
    TabOrder = 4
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 16
    Top = 128
    Width = 33
    Height = 21
    Caption = 'File'
    TabOrder = 5
    OnClick = Button2Click
  end
  object vCalibrationFile: TEdit
    Left = 48
    Top = 128
    Width = 329
    Height = 21
    TabOrder = 6
    Text = 'c:\p300\calib.prm'
  end
  object FileList: TListBox
    Left = 40
    Top = 256
    Width = 441
    Height = 137
    ItemHeight = 13
    TabOrder = 7
  end
  object AddFile: TButton
    Left = 40
    Top = 224
    Width = 89
    Height = 25
    Caption = 'AddFile'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 8
    OnClick = AddFileClick
  end
  object AddDirectory: TButton
    Left = 160
    Top = 224
    Width = 81
    Height = 25
    Caption = 'AddDirectory'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 9
    OnClick = AddDirectoryClick
  end
  object Clear: TButton
    Left = 272
    Top = 224
    Width = 89
    Height = 25
    Caption = 'Clear'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 10
    OnClick = ClearClick
  end
  object StateControl: TButton
    Left = 408
    Top = 16
    Width = 100
    Height = 25
    Caption = 'StateControl'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 11
    OnClick = StateControlClick
  end
  object Button3: TButton
    Left = 424
    Top = 432
    Width = 75
    Height = 25
    Caption = 'Exit'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 12
    OnClick = Button3Click
  end
  object OutputControl: TButton
    Left = 408
    Top = 136
    Width = 100
    Height = 25
    Caption = 'OutputControl'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 13
    OnClick = OutputControlClick
  end
  object InputControl: TButton
    Left = 408
    Top = 56
    Width = 100
    Height = 25
    Caption = 'InputControl'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 14
    OnClick = InputControlClick
  end
  object ProcessControl: TButton
    Left = 408
    Top = 96
    Width = 100
    Height = 25
    Caption = 'ProcessControl'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 15
    OnClick = ProcessControlClick
  end
  object Button4: TButton
    Left = 16
    Top = 176
    Width = 33
    Height = 21
    Caption = 'File'
    TabOrder = 16
    OnClick = Button4Click
  end
  object vParmFile: TEdit
    Left = 48
    Top = 176
    Width = 329
    Height = 21
    TabOrder = 17
    Text = 'c:\p300\parameters.asc'
  end
  object Button5: TButton
    Left = 408
    Top = 176
    Width = 100
    Height = 25
    Caption = 'Save Parameters'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 18
    OnClick = Button5Click
  end
  object FileType: TCheckBox
    Left = 224
    Top = 112
    Width = 97
    Height = 17
    Caption = 'OldDataFileTypeCheckBox1'
    Checked = True
    State = cbChecked
    TabOrder = 19
  end
  object OpenDialog: TOpenDialog
    Filter = 'BCI2000 EEG files (*.DAT)|*.DAT|All Files (*.*)|*.*'
    Options = [ofHideReadOnly, ofFileMustExist, ofEnableSizing]
    Left = 8
    Top = 32
  end
  object SaveDialog: TSaveDialog
    Left = 8
    Top = 80
  end
  object OpenParameter: TOpenDialog
    Filter = '*.prm|*.prm|all files|*.*'
    Left = 8
    Top = 128
  end
  object OpenParameterFile: TOpenDialog
    Left = 8
    Top = 176
  end
  object SaveParameterFile: TSaveDialog
    Filter = 'tfp'
    Left = 8
    Top = 184
  end
end
