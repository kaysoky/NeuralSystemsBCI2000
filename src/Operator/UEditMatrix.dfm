object fEditMatrix: TfEditMatrix
  Left = 328
  Top = 259
  Width = 280
  Height = 355
  Caption = 'Edit Matrix'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 32
    Width = 61
    Height = 13
    Caption = '# of columns'
  end
  object Label2: TLabel
    Left = 88
    Top = 32
    Width = 44
    Height = 13
    Caption = '# of rows'
  end
  object tComment: TLabel
    Left = 8
    Top = 8
    Width = 52
    Height = 13
    Caption = 'Comment'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object StringGrid: TStringGrid
    Left = 8
    Top = 72
    Width = 259
    Height = 246
    Anchors = [akLeft, akTop, akRight, akBottom]
    DefaultColWidth = 32
    DefaultRowHeight = 16
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goDrawFocusSelected, goColSizing, goEditing]
    TabOrder = 0
  end
  object cColumnsMax: TCSpinEdit
    Left = 8
    Top = 48
    Width = 41
    Height = 22
    TabStop = True
    MaxValue = 256
    MinValue = 1
    ParentColor = False
    TabOrder = 1
    Value = 4
  end
  object cRowsMax: TCSpinEdit
    Left = 88
    Top = 48
    Width = 41
    Height = 22
    TabStop = True
    MaxValue = 256
    MinValue = 1
    ParentColor = False
    TabOrder = 2
    Value = 4
  end
  object bChangeMatrixSize: TButton
    Left = 144
    Top = 48
    Width = 97
    Height = 22
    Caption = 'set new matrix size'
    TabOrder = 3
    OnClick = bChangeMatrixSizeClick
  end
end
