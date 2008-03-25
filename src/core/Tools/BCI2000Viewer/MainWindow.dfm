object MainForm: TMainForm
  Left = 329
  Top = 269
  Width = 718
  Height = 472
  Caption = 'BCI2000 Viewer'
  Color = clBtnFace
  Constraints.MinHeight = 200
  Constraints.MinWidth = 680
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Menu = mMainMenu
  OldCreateOrder = False
  Position = poScreenCenter
  OnCanResize = FormCanResize
  OnKeyDown = FormKeyDown
  OnResize = FormResize
  DesignSize = (
    710
    426)
  PixelsPerInch = 96
  TextHeight = 13
  object mBevel: TBevel
    Left = 4
    Top = 0
    Width = 577
    Height = 385
    Anchors = [akLeft, akTop, akRight, akBottom]
  end
  object mSignalArea: TPaintBox
    Left = 5
    Top = 1
    Width = 557
    Height = 383
    Anchors = [akLeft, akTop, akRight, akBottom]
    OnPaint = SignalAreaPaint
  end
  object mDragDropHint: TLabel
    Left = 200
    Top = 128
    Width = 169
    Height = 137
    Alignment = taCenter
    AutoSize = False
    Caption = 
      'Drag-and-drop a BCI2000 data file into this window to display it' +
      's contents.'
    Layout = tlCenter
    WordWrap = True
  end
  object mPageFwd: TButton
    Left = 509
    Top = 395
    Width = 33
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = '>>'
    TabOrder = 7
  end
  object mPageRew: TButton
    Left = 43
    Top = 395
    Width = 33
    Height = 25
    Anchors = [akLeft, akBottom]
    Caption = '<<'
    TabOrder = 1
  end
  object mBlockRew: TButton
    Left = 83
    Top = 395
    Width = 33
    Height = 25
    Anchors = [akLeft, akBottom]
    Caption = '<'
    TabOrder = 2
  end
  object mBlockFwd: TButton
    Left = 469
    Top = 395
    Width = 33
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = '>'
    TabOrder = 6
  end
  object mToBegin: TButton
    Left = 3
    Top = 395
    Width = 33
    Height = 25
    Anchors = [akLeft, akBottom]
    Caption = '|<<'
    TabOrder = 0
  end
  object mToEnd: TButton
    Left = 549
    Top = 395
    Width = 33
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = '>>|'
    TabOrder = 8
  end
  object mEditPosition: TEdit
    Left = 251
    Top = 396
    Width = 97
    Height = 21
    Anchors = [akRight, akBottom]
    BevelInner = bvSpace
    BevelOuter = bvNone
    TabOrder = 4
    OnExit = EditPositionExit
    OnKeyUp = EditPositionKeyUp
  end
  object mSampleZoomIn: TButton
    Left = 360
    Top = 395
    Width = 65
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Zoom In'
    TabOrder = 5
  end
  object mSampleZoomOut: TButton
    Left = 176
    Top = 395
    Width = 65
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Zoom Out'
    TabOrder = 3
  end
  object mVerticalScroller: TScrollBar
    Left = 563
    Top = 2
    Width = 17
    Height = 382
    Anchors = [akTop, akRight, akBottom]
    Kind = sbVertical
    PageSize = 0
    TabOrder = 9
    OnChange = VerticalScrollerChange
  end
  object mChannelListBox: TCheckListBox
    Left = 590
    Top = 0
    Width = 115
    Height = 385
    OnClickCheck = ChannelListBoxClickCheck
    Anchors = [akTop, akRight, akBottom]
    ItemHeight = 13
    PopupMenu = mListBoxPopupMenu
    TabOrder = 10
    OnContextPopup = mChannelListBoxContextPopup
  end
  object mMainMenu: TMainMenu
    object mFileMenu: TMenuItem
      Caption = 'File'
      object mFileOpen: TMenuItem
        Caption = 'Open...'
        ShortCut = 16463
      end
      object mFileClose: TMenuItem
        Caption = 'Close'
        ShortCut = 16471
      end
      object mFileQuit: TMenuItem
        Caption = 'Quit'
        ShortCut = 16465
      end
    end
    object mEditMenu: TMenuItem
      Caption = 'Edit'
      object mEditCut: TMenuItem
        Caption = 'Cut'
        Enabled = False
        ShortCut = 16472
      end
      object mEditCopy: TMenuItem
        Caption = 'Copy to Clipboard'
        ShortCut = 16451
      end
      object mEditPaste: TMenuItem
        Caption = 'Paste'
        Enabled = False
        ShortCut = 16470
      end
    end
    object mViewMenu: TMenuItem
      Caption = 'View'
      ShortCut = 16571
      object mViewEnlargeSignal: TMenuItem
        Caption = 'Enlarge Signal'
      end
      object mViewReduceSignal: TMenuItem
        Caption = 'Reduce Signal'
      end
      object N1: TMenuItem
        Caption = '-'
      end
      object mViewMoreChannels: TMenuItem
        Caption = 'More Channels'
      end
      object mViewLessChannels: TMenuItem
        Caption = 'Less Channels'
      end
      object N2: TMenuItem
        Caption = '-'
      end
      object mViewChooseChannelColors: TMenuItem
        Caption = 'Choose Channel Colors...'
      end
      object mViewInvert: TMenuItem
        Caption = 'Invert'
      end
      object mViewShowBaselines: TMenuItem
        AutoCheck = True
        Caption = 'Show Baselines'
      end
      object mViewShowUnit: TMenuItem
        AutoCheck = True
        Caption = 'Show Unit'
      end
      object N4: TMenuItem
        Caption = '-'
      end
      object mViewZoomOut: TMenuItem
        Caption = 'Zoom Out'
      end
      object mViewZoomIn: TMenuItem
        Caption = 'Zoom In'
      end
    end
    object mHelpMenu: TMenuItem
      Caption = 'Help'
      object mHelpOpenHelp: TMenuItem
        Caption = 'BCI2000 Help'
      end
      object mHelpAbout: TMenuItem
        Caption = 'About...'
      end
    end
  end
  object mActionList: TActionList
    Left = 32
  end
  object mListBoxPopupMenu: TPopupMenu
    Left = 592
    object mShowChannel: TMenuItem
      Caption = 'Show Channel(s)'
    end
    object mHideChannel: TMenuItem
      Caption = 'Hide Channel(s)'
    end
    object N3: TMenuItem
      Caption = '-'
    end
    object mHelpOnChannel: TMenuItem
      Caption = 'Help'
      OnClick = HelpOnChannelClick
    end
  end
end
