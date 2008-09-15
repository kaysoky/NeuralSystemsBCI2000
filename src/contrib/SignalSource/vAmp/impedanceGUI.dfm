object impGUI: TimpGUI
  Left = 0
  Top = 0
  Width = 1120
  Height = 154
  Caption = 'impGUI'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  DesignSize = (
    1112
    127)
  PixelsPerInch = 96
  TextHeight = 13
  object impGrid: TStringGrid
    Left = 0
    Top = -4
    Width = 1110
    Height = 132
    Anchors = [akLeft, akTop, akRight, akBottom]
    ColCount = 17
    DefaultDrawing = False
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goColSizing]
    TabOrder = 0
    OnDrawCell = impGridDrawCell
  end
end
