object fHelp: TfHelp
  Left = 407
  Top = 389
  Width = 374
  Height = 208
  Caption = 'Help'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Memo: TMemo
    Left = 8
    Top = 8
    Width = 353
    Height = 161
    Color = clBtnFace
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    Lines.Strings = (
      'This program extracts minimum and maximum sample '
      'values for the first 5000 samples of the specified channels '
      'in BCI2000 *.DAT files. It does this by using a simple'
      'procedure to detect ANY local minimum or maximum.'
      'It assumes that the data files contain a calibration '
      'signal of a defined minimum- and maximum value. '
      'From this, it calculates an offset and a gain for each of the '
      'provided channels and saves it in a BCI2000 parameter file '
      'to be read into the BCI2000 system.')
    ParentFont = False
    TabOrder = 0
  end
end
