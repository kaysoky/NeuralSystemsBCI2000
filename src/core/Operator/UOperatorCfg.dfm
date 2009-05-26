object fConfig: TfConfig
  Left = 403
  Top = 203
  HorzScrollBar.Visible = False
  VertScrollBar.Visible = False
  Caption = 'Configuration ...'
  ClientHeight = 534
  ClientWidth = 694
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
    Left = 7
    Top = 5
    Width = 573
    Height = 524
    Anchors = [akLeft, akTop, akRight, akBottom]
    TabOrder = 0
    OnChange = CfgTabControlChange
    OnChanging = CfgTabControlChanging
    object ScrollBox: TScrollBox
      Left = 4
      Top = 6
      Width = 565
      Height = 514
      HorzScrollBar.Visible = False
      Align = alClient
      BevelOuter = bvNone
      BorderStyle = bsNone
      TabOrder = 0
    end
  end
  object bLoadParameters: TButton
    Left = 586
    Top = 57
    Width = 100
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Load Parameters'
    TabOrder = 1
    OnClick = bLoadParametersClick
  end
  object bSaveParameters: TButton
    Left = 586
    Top = 25
    Width = 100
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Save Parameters'
    TabOrder = 2
    OnClick = bSaveParametersClick
  end
  object bConfigureSaveFilter: TButton
    Left = 586
    Top = 105
    Width = 100
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Configure Save'
    TabOrder = 3
    OnClick = bConfigureSaveFilterClick
  end
  object bConfigureLoadFilter: TButton
    Left = 586
    Top = 137
    Width = 100
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Configure Load'
    TabOrder = 4
    OnClick = bConfigureLoadFilterClick
  end
  object bHelp: TButton
    Left = 586
    Top = 185
    Width = 100
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Help'
    TabOrder = 5
    OnClick = bHelpClick
  end
  object LoadDialog: TOpenDialog
    DefaultExt = '.prm'
    Filter = 'BCI2000 parameter file (*.prm)|*.prm|Any file (*.*)|*.*'
    Options = [ofHideReadOnly, ofFileMustExist, ofEnableSizing]
    Left = 589
    Top = 232
  end
  object SaveDialog: TSaveDialog
    DefaultExt = '.prm'
    Filter = 'BCI2000 parameter file (*.prm)|*.prm|Any file (*.*)|*.*'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofEnableSizing]
    Left = 621
    Top = 232
  end
end
