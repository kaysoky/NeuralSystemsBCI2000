object fVisConfig: TfVisConfig
  Left = 532
  Top = 134
  Width = 232
  Height = 284
  Caption = 'Visualization Cfg'
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
    Left = 8
    Top = 8
    Width = 140
    Height = 16
    Caption = 'Graph Visualisation:'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label2: TLabel
    Left = 8
    Top = 35
    Width = 45
    Height = 13
    Caption = 'SourceID'
  end
  object Label3: TLabel
    Left = 8
    Top = 67
    Width = 19
    Height = 13
    Caption = 'Top'
  end
  object Label4: TLabel
    Left = 8
    Top = 91
    Width = 18
    Height = 13
    Caption = 'Left'
  end
  object Label5: TLabel
    Left = 8
    Top = 115
    Width = 28
    Height = 13
    Caption = 'Width'
  end
  object Label6: TLabel
    Left = 8
    Top = 139
    Width = 31
    Height = 13
    Caption = 'Height'
  end
  object Label7: TLabel
    Left = 8
    Top = 171
    Width = 51
    Height = 13
    Caption = 'DisplayMin'
  end
  object Label8: TLabel
    Left = 8
    Top = 195
    Width = 54
    Height = 13
    Caption = 'DisplayMax'
  end
  object Label9: TLabel
    Left = 136
    Top = 67
    Width = 81
    Height = 13
    Caption = 'Display Channels'
  end
  object Label10: TLabel
    Left = 136
    Top = 91
    Width = 23
    Height = 13
    Caption = 'From'
  end
  object Label11: TLabel
    Left = 136
    Top = 115
    Width = 13
    Height = 13
    Caption = 'To'
  end
  object Label12: TLabel
    Left = 8
    Top = 227
    Width = 53
    Height = 13
    Caption = '# Samples:'
  end
  object cSourceID: TCSpinEdit
    Left = 72
    Top = 32
    Width = 49
    Height = 22
    TabStop = True
    MaxValue = 255
    ParentColor = False
    TabOrder = 0
  end
  object cTop: TCSpinEdit
    Left = 72
    Top = 64
    Width = 49
    Height = 22
    TabStop = True
    Enabled = False
    MaxValue = 255
    ParentColor = False
    TabOrder = 1
  end
  object cLeft: TCSpinEdit
    Left = 72
    Top = 88
    Width = 49
    Height = 22
    TabStop = True
    Enabled = False
    MaxValue = 255
    ParentColor = False
    TabOrder = 2
  end
  object cWidth: TCSpinEdit
    Left = 72
    Top = 112
    Width = 49
    Height = 22
    TabStop = True
    Enabled = False
    MaxValue = 255
    ParentColor = False
    TabOrder = 3
  end
  object cHeight: TCSpinEdit
    Left = 72
    Top = 136
    Width = 49
    Height = 22
    TabStop = True
    Enabled = False
    MaxValue = 255
    ParentColor = False
    TabOrder = 4
  end
  object eDisplayMin: TEdit
    Left = 72
    Top = 168
    Width = 49
    Height = 21
    TabOrder = 5
    Text = '-20000'
  end
  object eDisplayMax: TEdit
    Left = 72
    Top = 192
    Width = 49
    Height = 21
    TabOrder = 6
    Text = '+20000'
  end
  object CSpinEdit5: TCSpinEdit
    Left = 168
    Top = 88
    Width = 49
    Height = 22
    TabStop = True
    Enabled = False
    MaxValue = 255
    ParentColor = False
    TabOrder = 7
  end
  object CSpinEdit6: TCSpinEdit
    Left = 168
    Top = 112
    Width = 49
    Height = 22
    TabStop = True
    Enabled = False
    MaxValue = 255
    ParentColor = False
    TabOrder = 8
  end
  object bSet: TButton
    Left = 144
    Top = 224
    Width = 75
    Height = 21
    Caption = 'Set'
    TabOrder = 9
    OnClick = bSetClick
  end
  object cNumSamples: TCSpinEdit
    Left = 72
    Top = 224
    Width = 49
    Height = 22
    TabStop = True
    MaxValue = 255
    ParentColor = False
    TabOrder = 10
  end
end
