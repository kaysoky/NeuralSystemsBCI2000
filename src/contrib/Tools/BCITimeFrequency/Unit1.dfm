object OutputForm: TOutputForm
  Left = 599
  Top = 354
  Width = 360
  Height = 237
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
  object OutputOrder: TRadioGroup
    Left = 232
    Top = 24
    Width = 97
    Height = 65
    Caption = 'OutputOrder (XY)'
    ItemIndex = 1
    Items.Strings = (
      'ChanXTime'
      'TimeXChan')
    TabOrder = 0
  end
end
