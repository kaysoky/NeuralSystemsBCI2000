object fMain: TfMain
  Left = 304
  Top = 245
  Width = 555
  Height = 344
  Caption = 'BCI2000 Calibration Generator V1.0 ... Gerv '#39'01'
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
  object Label9: TLabel
    Left = 352
    Top = 56
    Width = 89
    Height = 24
    Caption = 'Channels'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -19
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label10: TLabel
    Left = 440
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
  object Label11: TLabel
    Left = 376
    Top = 200
    Width = 131
    Height = 13
    Caption = '(otherwise, all the channels)'
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
    Left = 463
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
    Left = 464
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
  object mTransmitChannels: TMemo
    Left = 360
    Top = 88
    Width = 73
    Height = 81
    Lines.Strings = (
      '1'
      '2'
      '3'
      '4'
      '5'
      '6'
      '7'
      '8'
      '9'
      '10'
      '11'
      '12'
      '13'
      '14'
      '15'
      '16'
      '17'
      '18'
      '19'
      '20'
      '21'
      '22'
      '23'
      '24'
      '25'
      '26'
      '27'
      '28'
      '29'
      '30'
      '31'
      '32'
      '33'
      '34'
      '35'
      '36'
      '37'
      '38'
      '39'
      '40'
      '41'
      '42'
      '43'
      '44'
      '45'
      '46'
      '47'
      '48'
      '49'
      '50'
      '51'
      '52'
      '53'
      '54'
      '55'
      '56'
      '57'
      '58'
      '59'
      '60'
      '61'
      '62'
      '63'
      '64'
      '')
    ScrollBars = ssVertical
    TabOrder = 5
  end
  object cEnableChannellist: TCheckBox
    Left = 360
    Top = 176
    Width = 113
    Height = 17
    Caption = 'Enable Channellist'
    TabOrder = 6
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
