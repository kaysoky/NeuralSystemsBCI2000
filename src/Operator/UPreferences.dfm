object fPreferences: TfPreferences
  Left = 303
  Top = 361
  BorderIcons = [biMinimize, biMaximize]
  BorderStyle = bsSingle
  Caption = 'Preferences'
  ClientHeight = 286
  ClientWidth = 495
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 407
    Top = 176
    Width = 76
    Height = 16
    Caption = 'User Level'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object tUserLevel: TLabel
    Left = 424
    Top = 224
    Width = 42
    Height = 13
    Alignment = taCenter
    Caption = 'Beginner'
  end
  object Bevel1: TBevel
    Left = 8
    Top = 16
    Width = 241
    Height = 257
  end
  object Label2: TLabel
    Left = 16
    Top = 32
    Width = 131
    Height = 13
    Caption = 'After all modules connected'
  end
  object Label3: TLabel
    Left = 15
    Top = 8
    Width = 79
    Height = 16
    Caption = 'Script Files'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label4: TLabel
    Left = 16
    Top = 80
    Width = 89
    Height = 13
    Caption = 'On exit of BCI2000'
  end
  object Label5: TLabel
    Left = 16
    Top = 128
    Width = 56
    Height = 13
    Caption = 'On Resume'
  end
  object Label6: TLabel
    Left = 16
    Top = 176
    Width = 59
    Height = 13
    Caption = 'On Suspend'
  end
  object Label7: TLabel
    Left = 16
    Top = 224
    Width = 39
    Height = 13
    Caption = 'On Start'
  end
  object Bevel2: TBevel
    Left = 256
    Top = 16
    Width = 225
    Height = 145
  end
  object Label8: TLabel
    Left = 263
    Top = 8
    Width = 114
    Height = 16
    Caption = 'Function Buttons'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label9: TLabel
    Left = 264
    Top = 56
    Width = 49
    Height = 13
    Caption = 'Button 1'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label10: TLabel
    Left = 320
    Top = 40
    Width = 33
    Height = 13
    Caption = 'Name'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label11: TLabel
    Left = 264
    Top = 80
    Width = 49
    Height = 13
    Caption = 'Button 2'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label12: TLabel
    Left = 384
    Top = 40
    Width = 55
    Height = 13
    Caption = 'Command'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label13: TLabel
    Left = 264
    Top = 104
    Width = 49
    Height = 13
    Caption = 'Button 3'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label14: TLabel
    Left = 264
    Top = 128
    Width = 49
    Height = 13
    Caption = 'Button 4'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object TrackBar1: TTrackBar
    Left = 408
    Top = 192
    Width = 75
    Height = 33
    Hint = 'User Level'
    Max = 3
    Min = 1
    Orientation = trHorizontal
    ParentShowHint = False
    PageSize = 1
    Frequency = 1
    Position = 1
    SelEnd = 0
    SelStart = 0
    ShowHint = True
    TabOrder = 0
    TickMarks = tmBottomRight
    TickStyle = tsAuto
    OnChange = TrackBar1Change
  end
  object bClose: TButton
    Left = 408
    Top = 248
    Width = 75
    Height = 25
    Caption = 'Close'
    TabOrder = 1
    OnClick = bCloseClick
  end
  object eAfterModulesConnected: TEdit
    Left = 16
    Top = 48
    Width = 225
    Height = 21
    TabOrder = 2
  end
  object eOnExit: TEdit
    Left = 16
    Top = 96
    Width = 225
    Height = 21
    TabOrder = 3
  end
  object eOnResume: TEdit
    Left = 16
    Top = 144
    Width = 225
    Height = 21
    TabOrder = 4
  end
  object eOnSuspend: TEdit
    Left = 16
    Top = 192
    Width = 225
    Height = 21
    TabOrder = 5
  end
  object eOnStart: TEdit
    Left = 16
    Top = 240
    Width = 225
    Height = 21
    TabOrder = 6
  end
  object eButton1Name: TEdit
    Left = 320
    Top = 56
    Width = 57
    Height = 21
    TabOrder = 7
  end
  object eButton1Cmd: TEdit
    Left = 384
    Top = 56
    Width = 89
    Height = 21
    TabOrder = 8
  end
  object eButton2Name: TEdit
    Left = 320
    Top = 80
    Width = 57
    Height = 21
    TabOrder = 9
  end
  object eButton2Cmd: TEdit
    Left = 384
    Top = 80
    Width = 89
    Height = 21
    TabOrder = 10
  end
  object eButton3Name: TEdit
    Left = 320
    Top = 104
    Width = 57
    Height = 21
    TabOrder = 11
  end
  object eButton3Cmd: TEdit
    Left = 384
    Top = 104
    Width = 89
    Height = 21
    TabOrder = 12
  end
  object eButton4Name: TEdit
    Left = 320
    Top = 128
    Width = 57
    Height = 21
    TabOrder = 13
  end
  object eButton4Cmd: TEdit
    Left = 384
    Top = 128
    Width = 89
    Height = 21
    TabOrder = 14
  end
end
