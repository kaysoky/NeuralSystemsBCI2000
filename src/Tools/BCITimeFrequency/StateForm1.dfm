object UseStateForm: TUseStateForm
  Left = 151
  Top = 209
  Width = 633
  Height = 403
  Caption = 'UseStateForm'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object NumStates: TLabel
    Left = 32
    Top = 16
    Width = 52
    Height = 13
    Caption = 'NumStates'
  end
  object NumGroups: TLabel
    Left = 8
    Top = 48
    Width = 72
    Height = 13
    Caption = 'NumCategories'
  end
  object vNStates: TEdit
    Left = 88
    Top = 16
    Width = 49
    Height = 21
    TabOrder = 0
    Text = '2'
    OnChange = vNStatesChange
  end
  object vNValues: TEdit
    Left = 88
    Top = 48
    Width = 49
    Height = 21
    TabOrder = 1
    Text = '2'
    OnChange = vNValuesChange
  end
  object Grid: TStringGrid
    Left = 24
    Top = 112
    Width = 585
    Height = 217
    ColCount = 6
    DefaultColWidth = 80
    DefaultRowHeight = 20
    RowCount = 3
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goEditing]
    TabOrder = 2
  end
  object Clear: TButton
    Left = 24
    Top = 344
    Width = 75
    Height = 25
    Caption = 'Clear'
    TabOrder = 3
    OnClick = ClearClick
  end
  object Apply: TButton
    Left = 536
    Top = 344
    Width = 75
    Height = 25
    Caption = 'Apply'
    TabOrder = 4
    OnClick = ApplyClick
  end
  object vInput: TEdit
    Left = 264
    Top = 16
    Width = 345
    Height = 21
    TabOrder = 5
    Text = 'Input.b2g'
  end
  object vSave: TEdit
    Left = 264
    Top = 48
    Width = 345
    Height = 21
    TabOrder = 6
    Text = 'Save.b2g'
  end
  object Input: TButton
    Left = 176
    Top = 16
    Width = 75
    Height = 25
    Caption = 'Input'
    TabOrder = 7
    OnClick = InputClick
  end
  object Save: TButton
    Left = 176
    Top = 48
    Width = 75
    Height = 25
    Caption = 'Save'
    TabOrder = 8
    OnClick = SaveClick
  end
  object Exit: TButton
    Left = 384
    Top = 344
    Width = 75
    Height = 25
    Caption = 'Exit'
    TabOrder = 9
    OnClick = ExitClick
  end
  object IncludeNext: TCheckBox
    Left = 352
    Top = 80
    Width = 145
    Height = 17
    Caption = 'Include Next Sample'
    TabOrder = 10
  end
  object Use: TCheckBox
    Left = 96
    Top = 80
    Width = 169
    Height = 17
    Caption = ' Use second state regressively'
    TabOrder = 11
  end
  object OpenInput: TOpenDialog
    DefaultExt = 'b2g'
    Filter = '*.b2g'
    Left = 168
    Top = 16
  end
  object SaveOutput: TSaveDialog
    DefaultExt = 'b2g'
    Left = 168
    Top = 48
  end
end
