object InputForm: TInputForm
  Left = 498
  Top = 235
  Width = 463
  Height = 352
  Caption = 'InputForm'
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
    Left = 336
    Top = 56
    Width = 101
    Height = 20
    Caption = 'Channel List'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label2: TLabel
    Left = 168
    Top = 256
    Width = 65
    Height = 13
    Caption = 'Start Baseline'
    Visible = False
  end
  object Label3: TLabel
    Left = 168
    Top = 280
    Width = 62
    Height = 13
    Caption = 'End Baseline'
    Visible = False
  end
  object ChanList: TMemo
    Left = 344
    Top = 88
    Width = 73
    Height = 201
    Lines.Strings = (
      '34'
      '11'
      '51'
      '56'
      '60')
    TabOrder = 0
  end
  object AllCh: TCheckBox
    Left = 336
    Top = 24
    Width = 97
    Height = 17
    Caption = 'All Channels'
    TabOrder = 1
    OnClick = CheckAllChClick
  end
  object SpatialFilter: TButton
    Left = 8
    Top = 48
    Width = 75
    Height = 25
    Caption = 'SpatialFilter'
    TabOrder = 2
    Visible = False
    OnClick = SpatialFilterClick
  end
  object vSpatialFile: TEdit
    Left = 88
    Top = 48
    Width = 201
    Height = 21
    TabOrder = 3
    Visible = False
  end
  object TemporalFilter: TButton
    Left = 8
    Top = 120
    Width = 75
    Height = 25
    Caption = 'TemporalFilter'
    TabOrder = 4
    Visible = False
    OnClick = TemporalFilterClick
  end
  object vTemporalFile: TEdit
    Left = 88
    Top = 120
    Width = 201
    Height = 21
    TabOrder = 5
    Visible = False
  end
  object CheckSpatialFilter: TCheckBox
    Left = 8
    Top = 16
    Width = 97
    Height = 17
    Caption = 'Spatial Filtering'
    TabOrder = 6
    OnClick = CheckSpatialFilterClick
  end
  object CheckTemporalFilter: TCheckBox
    Left = 8
    Top = 96
    Width = 113
    Height = 17
    Caption = 'Temporal Filtering'
    TabOrder = 7
    OnClick = CheckTemporalFilterClick
  end
  object CheckAlign: TCheckBox
    Left = 136
    Top = 16
    Width = 97
    Height = 17
    Caption = 'Align Channels'
    TabOrder = 8
    Visible = False
  end
  object CheckStateList: TCheckBox
    Left = 8
    Top = 160
    Width = 97
    Height = 17
    Caption = 'CheckStateList'
    TabOrder = 9
    OnClick = CheckStateListClick
  end
  object StateList: TMemo
    Left = 32
    Top = 184
    Width = 161
    Height = 49
    Lines.Strings = (
      'CursorPosX'
      'CursorPosY')
    TabOrder = 10
    Visible = False
  end
  object Baseline: TRadioGroup
    Left = 32
    Top = 248
    Width = 121
    Height = 57
    Caption = 'Baseline'
    ItemIndex = 0
    Items.Strings = (
      'None'
      'Prestimulus')
    TabOrder = 11
    OnClick = BaselineClick
  end
  object vStartBase: TEdit
    Left = 240
    Top = 256
    Width = 49
    Height = 21
    TabOrder = 12
    Text = '-100'
    Visible = False
  end
  object vEndBase: TEdit
    Left = 240
    Top = 280
    Width = 49
    Height = 21
    TabOrder = 13
    Text = '0'
    Visible = False
  end
  object OpenSpatialFile: TOpenDialog
    Filter = '*.flt'
    Left = 8
    Top = 48
  end
  object OpenTemporalFilter: TOpenDialog
    Filter = '*.flt'
    Left = 8
    Top = 120
  end
end
