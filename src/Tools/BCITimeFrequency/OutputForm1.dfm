object OutputForm: TOutputForm
  Left = 569
  Top = 350
  Width = 391
  Height = 331
  Caption = 'OutputForm'
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
    Top = 16
    Width = 56
    Height = 13
    Caption = 'Start (msec)'
  end
  object Label2: TLabel
    Left = 8
    Top = 40
    Width = 53
    Height = 13
    Caption = 'End (msec)'
  end
  object Label3: TLabel
    Left = 16
    Top = 136
    Width = 106
    Height = 13
    Caption = 'Compute Means when'
    Visible = False
  end
  object Label4: TLabel
    Left = 248
    Top = 136
    Width = 42
    Height = 13
    Caption = 'Changes'
    Visible = False
  end
  object Label5: TLabel
    Left = 280
    Top = 152
    Width = 94
    Height = 16
    Caption = 'Time Periods'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    Visible = False
  end
  object Label6: TLabel
    Left = 16
    Top = 176
    Width = 45
    Height = 13
    Caption = 'Decimate'
  end
  object OutputOrder: TRadioGroup
    Left = 272
    Top = 16
    Width = 97
    Height = 81
    Caption = 'OutputOrder (XY)'
    ItemIndex = 0
    Items.Strings = (
      'ChanXTime'
      'TimeXChan'
      'Topographies')
    TabOrder = 0
    OnClick = OutputOrderClick
  end
  object vStart: TEdit
    Left = 80
    Top = 16
    Width = 49
    Height = 21
    TabOrder = 1
    Text = '0'
  end
  object vEnd: TEdit
    Left = 80
    Top = 40
    Width = 49
    Height = 21
    TabOrder = 2
    Text = '600'
  end
  object OverlapMode: TCheckBox
    Left = 24
    Top = 80
    Width = 97
    Height = 17
    Caption = 'OverlapMode'
    Checked = True
    State = cbChecked
    TabOrder = 3
  end
  object vCompuMeans: TEdit
    Left = 128
    Top = 136
    Width = 113
    Height = 21
    TabOrder = 4
    Text = 'SelectedTarget'
    Visible = False
  end
  object Statistics: TRadioGroup
    Left = 16
    Top = 216
    Width = 113
    Height = 73
    Caption = 'Statistics for Means'
    ItemIndex = 0
    Items.Strings = (
      'None'
      'r-squared'
      'Pearson'#39's r')
    TabOrder = 5
  end
  object SubGroups: TCheckBox
    Left = 24
    Top = 112
    Width = 129
    Height = 17
    Caption = 'Means for Subgroups'
    TabOrder = 6
    OnClick = SubGroupsClick
  end
  object Times: TMemo
    Left = 296
    Top = 168
    Width = 65
    Height = 121
    Lines.Strings = (
      '100'
      '200'
      '300'
      '400'
      '500'
      '600'
      '700'
      '800')
    TabOrder = 7
    Visible = False
  end
  object vDecimate: TEdit
    Left = 72
    Top = 176
    Width = 41
    Height = 21
    TabOrder = 8
    Text = '1'
  end
end
