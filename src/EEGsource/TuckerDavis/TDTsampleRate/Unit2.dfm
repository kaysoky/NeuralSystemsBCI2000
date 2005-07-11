object Form2: TForm2
  Left = 728
  Top = 323
  Width = 238
  Height = 237
  Caption = 'TDT Sample Rate'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 16
    Top = 16
    Width = 157
    Height = 13
    Caption = 'Enter Desired Sample Rate Here:'
  end
  object Label2: TLabel
    Left = 16
    Top = 96
    Width = 159
    Height = 13
    Caption = 'Enter this sample rate in BCI2000:'
  end
  object Label3: TLabel
    Left = 16
    Top = 144
    Width = 68
    Height = 13
    Caption = 'Sample Period'
  end
  object desiredSRbox: TEdit
    Left = 16
    Top = 32
    Width = 121
    Height = 21
    TabOrder = 0
    Text = '512'
  end
  object realSRbox: TEdit
    Left = 16
    Top = 112
    Width = 121
    Height = 21
    TabOrder = 1
  end
  object samplePeriodBox: TEdit
    Left = 16
    Top = 160
    Width = 65
    Height = 21
    Enabled = False
    TabOrder = 2
  end
  object calcBut: TButton
    Left = 80
    Top = 64
    Width = 75
    Height = 25
    Caption = 'Calculate'
    TabOrder = 3
    OnClick = calcButClick
  end
end
