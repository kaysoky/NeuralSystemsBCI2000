object fShowParameters: TfShowParameters
  Left = 431
  Top = 333
  BorderStyle = bsSingle
  Caption = 'System Parameters'
  ClientHeight = 427
  ClientWidth = 249
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object ParameterListBox: TCheckListBox
    Left = 8
    Top = 8
    Width = 233
    Height = 412
    Anchors = [akLeft, akTop, akRight, akBottom]
    ItemHeight = 13
    TabOrder = 0
  end
end
