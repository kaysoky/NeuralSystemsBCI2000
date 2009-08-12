object ImporterForm: TImporterForm
  Left = 795
  Top = 242
  Width = 420
  Height = 355
  Caption = 'BCI2000Export'
  Color = clBtnFace
  Constraints.MinHeight = 325
  Constraints.MinWidth = 420
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Menu = MainMenu
  OldCreateOrder = False
  Position = poScreenCenter
  DesignSize = (
    412
    309)
  PixelsPerInch = 96
  TextHeight = 13
  object StatusBar: TStatusBar
    Left = 0
    Top = 288
    Width = 412
    Height = 21
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
  object Panel: TPanel
    Left = 0
    Top = 0
    Width = 409
    Height = 289
    Anchors = [akLeft, akTop, akRight, akBottom]
    BevelInner = bvLowered
    BevelOuter = bvNone
    TabOrder = 1
    DesignSize = (
      409
      289)
    object Label4: TLabel
      Left = 258
      Top = 8
      Width = 48
      Height = 13
      Caption = 'Drop Area'
    end
    object Label2: TLabel
      Left = 7
      Top = 8
      Width = 75
      Height = 13
      Caption = 'Channel Names'
    end
    object Label1: TLabel
      Left = 115
      Top = 8
      Width = 62
      Height = 13
      Caption = 'Import States'
    end
    object Label5: TLabel
      Left = 11
      Top = 263
      Width = 67
      Height = 13
      Anchors = [akLeft, akBottom]
      Caption = 'Output Format'
    end
    object StatesList: TCheckListBox
      Left = 112
      Top = 24
      Width = 129
      Height = 225
      Anchors = [akLeft, akTop, akBottom]
      ItemHeight = 13
      Sorted = True
      TabOrder = 0
      OnKeyDown = StatesListKeyDown
    end
    object DropPanel: TPanel
      Left = 256
      Top = 24
      Width = 145
      Height = 225
      Anchors = [akLeft, akTop, akRight, akBottom]
      BevelOuter = bvLowered
      TabOrder = 1
    end
    object ChannelNamesMemo: TMemo
      Left = 7
      Top = 24
      Width = 90
      Height = 225
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
      TabOrder = 2
    end
    object FormatsBox: TComboBox
      Left = 112
      Top = 259
      Width = 289
      Height = 21
      Style = csDropDownList
      Anchors = [akLeft, akRight, akBottom]
      ItemHeight = 13
      TabOrder = 3
    end
  end
  object MainMenu: TMainMenu
    Left = 376
    Top = 65528
    object File1: TMenuItem
      Caption = 'File'
      object FileOpen: TMenuItem
        Caption = 'Open...'
        ShortCut = 16463
        OnClick = FileOpenClick
      end
      object FileQuit: TMenuItem
        Caption = 'Quit'
        ShortCut = 16465
        OnClick = FileQuitClick
      end
    end
    object Edit1: TMenuItem
      Caption = 'Edit'
      object Cut1: TMenuItem
        Action = EditCut
      end
      object Copy: TMenuItem
        Action = EditCopy
      end
      object Paste1: TMenuItem
        Action = EditPaste
      end
    end
    object Help1: TMenuItem
      Caption = 'Help'
      object HelpOpenHelp: TMenuItem
        Caption = 'BCI2000 Help'
        OnClick = HelpOpenHelpClick
      end
      object HelpAbout: TMenuItem
        Caption = 'About...'
        OnClick = HelpAboutClick
      end
    end
  end
  object ActionList: TActionList
    Left = 344
    Top = 65528
    object EditCopy: TEditCopy
      Category = 'Edit'
      Caption = '&Copy'
      Hint = 'Copy|Copies the selection and puts it on the Clipboard'
      ImageIndex = 1
      ShortCut = 16451
    end
    object EditCut: TEditCut
      Category = 'Edit'
      Caption = 'Cu&t'
      Hint = 'Cut|Cuts the selection and puts it on the Clipboard'
      ImageIndex = 0
      ShortCut = 16472
    end
    object EditPaste: TEditPaste
      Category = 'Edit'
      Caption = '&Paste'
      Hint = 'Paste|Inserts Clipboard contents'
      ImageIndex = 2
      ShortCut = 16470
    end
  end
end
