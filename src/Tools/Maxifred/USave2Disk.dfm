object fSave2Disk: TfSave2Disk
  Left = 343
  Top = 360
  Width = 249
  Height = 326
  Caption = 'Save 2 Disk'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 56
    Top = 24
    Width = 131
    Height = 16
    Caption = 'You are about to save'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label2: TLabel
    Left = 50
    Top = 50
    Width = 143
    Height = 16
    Caption = 'the currently visible data'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label3: TLabel
    Left = 36
    Top = 77
    Width = 171
    Height = 16
    Caption = '(selected time and channels)'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label4: TLabel
    Left = 61
    Top = 103
    Width = 120
    Height = 16
    Caption = 'into the following file:'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label5: TLabel
    Left = 25
    Top = 155
    Width = 192
    Height = 16
    Caption = 'data will be space delimited and'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label6: TLabel
    Left = 20
    Top = 182
    Width = 203
    Height = 16
    Caption = 'and the channels are placed in the'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label7: TLabel
    Left = 78
    Top = 208
    Width = 86
    Height = 16
    Caption = 'rows of the file.'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object eFilename: TEdit
    Left = 61
    Top = 129
    Width = 121
    Height = 21
    TabOrder = 0
    Text = 'c:\temp\test.dat'
  end
  object bSave: TButton
    Left = 160
    Top = 264
    Width = 75
    Height = 25
    Caption = 'Save'
    Default = True
    TabOrder = 1
    OnClick = bSaveClick
  end
  object bCancel: TButton
    Left = 72
    Top = 264
    Width = 75
    Height = 25
    Caption = 'Ooooops ...'
    TabOrder = 2
    OnClick = bCancelClick
  end
end
