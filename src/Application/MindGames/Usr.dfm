object User: TUser
  Left = 353
  Top = 164
  BorderStyle = bsNone
  Caption = 'Usr'
  ClientHeight = 474
  ClientWidth = 528
  Color = clNone
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Target: TShape
    Left = 496
    Top = 16
    Width = 25
    Height = 145
    Brush.Color = clBlack
  end
  object tT: TLabel
    Left = 88
    Top = 192
    Width = 346
    Height = 75
    Caption = 'Game Over'
    Font.Charset = ANSI_CHARSET
    Font.Color = clLime
    Font.Height = -67
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object Cursor: TShape
    Left = 8
    Top = 432
    Width = 100
    Height = 25
    Brush.Color = clYellow
  end
  object Ball: TShape
    Left = 224
    Top = 312
    Width = 20
    Height = 20
    Brush.Color = clLime
    Shape = stCircle
  end
end
