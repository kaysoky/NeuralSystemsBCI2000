object ImporterForm: TImporterForm
  Left = 628
  Top = 500
  Width = 420
  Height = 325
  Caption = 'Brain Vision Importer'
  Color = clBtnFace
  Constraints.MinHeight = 325
  Constraints.MinWidth = 420
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  DesignSize = (
    412
    298)
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 127
    Top = 8
    Width = 62
    Height = 13
    Caption = 'Import States'
  end
  object Label2: TLabel
    Left = 15
    Top = 8
    Width = 75
    Height = 13
    Caption = 'Channel Names'
  end
  object Label4: TLabel
    Left = 279
    Top = 8
    Width = 47
    Height = 13
    Caption = 'Drop area'
  end
  object DropPanel: TPanel
    Left = 280
    Top = 32
    Width = 118
    Height = 235
    Anchors = [akLeft, akTop, akRight, akBottom]
    BevelOuter = bvLowered
    TabOrder = 1
    DesignSize = (
      118
      235)
    object Label3: TLabel
      Left = 16
      Top = 80
      Width = 81
      Height = 154
      Alignment = taCenter
      Anchors = [akLeft, akTop, akRight, akBottom]
      AutoSize = False
      Caption = 
        'Dropping files here will create BrainVision import files at the ' +
        'original files'#39' location.'
      WordWrap = True
    end
  end
  object ChannelNamesMemo: TMemo
    Left = 15
    Top = 32
    Width = 90
    Height = 235
    Anchors = [akLeft, akTop, akBottom]
    Lines.Strings = (
      'FC5'
      'FC3'
      'FC1'
      'FCz'
      'FC2'
      'FC4'
      'FC6'
      'C5'
      'C3'
      'C1'
      'Cz'
      'C2'
      'C4'
      'C6'
      'CP5'
      'CP3'
      'CP1'
      'CPz'
      'CP2'
      'CP4'
      'CP6'
      'Fp1'
      'Fpz'
      'Fp2'
      'AF7'
      'AF3'
      'AFz'
      'AF4'
      'AF8'
      'F7'
      'F5'
      'F3'
      'F1'
      'Fz'
      'F2'
      'F4'
      'F6'
      'F8'
      'FT7'
      'FT8'
      'T7'
      'T8'
      'VEOGupper'
      'VEOGlower'
      'TP7'
      'TP8'
      'P7'
      'P5'
      'P3'
      'P1'
      'Pz'
      'P2'
      'P4'
      'P6'
      'P8'
      'PO7'
      'PO3'
      'POz'
      'PO4'
      'PO8'
      'O1'
      'Oz'
      'O2'
      'Iz'
      'marker')
    ScrollBars = ssVertical
    TabOrder = 0
  end
  object StatesList: TCheckListBox
    Left = 127
    Top = 32
    Width = 129
    Height = 235
    Anchors = [akLeft, akTop, akBottom]
    ItemHeight = 13
    Sorted = True
    TabOrder = 2
    OnKeyDown = StatesListKeyDown
  end
  object StatusBar: TStatusBar
    Left = 0
    Top = 282
    Width = 412
    Height = 16
    Panels = <
      item
        Alignment = taCenter
        Text = 'Idle'
        Width = 60
      end
      item
        Width = 330
      end>
    SimplePanel = False
  end
end
