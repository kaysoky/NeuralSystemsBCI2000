object fConfig: TfConfig
  Left = 368
  Top = 250
  Width = 702
  Height = 561
  Caption = 'Configuration ...'
  Color = clBtnFace
  Constraints.MinWidth = 702
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  DesignSize = (
    694
    534)
  PixelsPerInch = 96
  TextHeight = 13
  object CfgTabControl: TTabControl
    Left = 5
    Top = 5
    Width = 556
    Height = 521
    Anchors = [akLeft, akTop, akRight]
    TabOrder = 0
    OnChange = CfgTabControlChange
    OnChanging = CfgTabControlChanging
  end
  object bLoadParameters: TButton
    Left = 589
    Top = 37
    Width = 100
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Load Parameters'
    TabOrder = 1
    OnClick = bLoadParametersClick
  end
  object bSaveParameters: TButton
    Left = 589
    Top = 5
    Width = 100
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Save Parameters'
    TabOrder = 2
    OnClick = bSaveParametersClick
  end
  object bConfigureSaveFilter: TButton
    Left = 589
    Top = 125
    Width = 100
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Configure Save'
    TabOrder = 3
    OnClick = bConfigureSaveFilterClick
  end
  object bConfigureLoadFilter: TButton
    Left = 589
    Top = 157
    Width = 100
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Configure Load'
    TabOrder = 4
    OnClick = bConfigureLoadFilterClick
  end
  object LoadDialog: TOpenDialog
    DefaultExt = '.prm'
    Filter = 'BCI2000 parameter file (*.prm)|*.prm|Any file (*.*)|*.*'
    Options = [ofHideReadOnly, ofFileMustExist, ofEnableSizing]
    Left = 592
    Top = 72
  end
  object SaveDialog: TSaveDialog
    DefaultExt = '.prm'
    Filter = 'BCI2000 parameter file (*.prm)|*.prm|Any file (*.*)|*.*'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofEnableSizing]
    Left = 624
    Top = 72
  end
end
