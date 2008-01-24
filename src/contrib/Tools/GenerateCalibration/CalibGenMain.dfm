object fMain: TfMain
  Left = 372
  Top = 272
  BorderStyle = bsSingle
  Caption = 'BCI2000 Calibration Generator V1.2 ... Gerwin Schalk '#39'01'#39'04'
  ClientHeight = 317
  ClientWidth = 456
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object tFilename: TLabel
    Left = 112
    Top = 232
    Width = 20
    Height = 13
    Caption = 'N/A'
  end
  object Label1: TLabel
    Left = 144
    Top = 120
    Width = 23
    Height = 20
    Caption = '-->'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label2: TLabel
    Left = 336
    Top = 120
    Width = 23
    Height = 20
    Caption = '-->'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object tCalibFile: TLabel
    Left = 112
    Top = 272
    Width = 20
    Height = 13
    Caption = 'N/A'
  end
  object Label3: TLabel
    Left = 16
    Top = 232
    Width = 72
    Height = 13
    Caption = 'Input Filename:'
  end
  object Label4: TLabel
    Left = 16
    Top = 272
    Width = 71
    Height = 13
    Caption = 'Calibration File:'
  end
  object Label6: TLabel
    Left = 16
    Top = 248
    Width = 93
    Height = 13
    Caption = '# channels in input:'
  end
  object tNumChannelsInput: TLabel
    Left = 112
    Top = 248
    Width = 20
    Height = 13
    Caption = 'N/A'
  end
  object Label5: TLabel
    Left = 48
    Top = 56
    Width = 47
    Height = 24
    Caption = 'Input'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -19
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label7: TLabel
    Left = 216
    Top = 56
    Width = 63
    Height = 24
    Caption = 'Output'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -19
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label8: TLabel
    Left = 16
    Top = 176
    Width = 266
    Height = 13
    Caption = 'Peak-to-Peak value of the input sine wave in microVolts:'
  end
  object bSelectInput: TButton
    Left = 16
    Top = 88
    Width = 121
    Height = 81
    Caption = 'Select Input'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 0
    OnClick = bSelectInputClick
  end
  object bSelectCalib: TButton
    Left = 176
    Top = 88
    Width = 153
    Height = 81
    Caption = 'Select Calibration'
    Enabled = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 1
    OnClick = bSelectCalibClick
  end
  object bGo: TButton
    Left = 367
    Top = 88
    Width = 75
    Height = 81
    Caption = 'Go'
    Enabled = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 2
    OnClick = bGoClick
  end
  object bHelp: TButton
    Left = 368
    Top = 8
    Width = 75
    Height = 25
    Caption = 'Help'
    TabOrder = 3
    OnClick = bHelpClick
  end
  object eTargetVal: TEdit
    Left = 16
    Top = 192
    Width = 49
    Height = 21
    TabOrder = 4
    Text = '100.0'
  end
  object OpenDialog: TOpenDialog
    Filter = 'BCI2000 EEG files (*.DAT)|*.DAT|All Files (*.*)|*.*'
    Options = [ofHideReadOnly, ofFileMustExist, ofEnableSizing]
    Left = 16
    Top = 8
  end
  object SaveDialog: TSaveDialog
    DefaultExt = '.prm'
    Filter = 'BCI2000 parameter file (*.prm)|*.prm|All Files (*.*)|*.*'
    Left = 56
    Top = 8
  end
end
