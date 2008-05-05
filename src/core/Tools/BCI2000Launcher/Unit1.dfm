object mainForm: TmainForm
  Left = 218
  Top = 200
  Width = 709
  Height = 470
  Caption = 'BCI2000 Launcher'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  OnClose = FormClose
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object GroupBox1: TGroupBox
    Left = 0
    Top = 0
    Width = 681
    Height = 201
    Caption = 'BCI Program Modules'
    TabOrder = 0
    object Label1: TLabel
      Left = 8
      Top = 24
      Width = 51
      Height = 13
      Caption = 'Acquisition'
    end
    object Label2: TLabel
      Left = 168
      Top = 24
      Width = 84
      Height = 13
      Caption = 'Signal Processing'
    end
    object Label3: TLabel
      Left = 328
      Top = 24
      Width = 52
      Height = 13
      Caption = 'Application'
    end
    object sourceList: TListBox
      Left = 8
      Top = 40
      Width = 153
      Height = 121
      DragMode = dmAutomatic
      ItemHeight = 13
      TabOrder = 0
      OnDragDrop = sourceListDragDrop
      OnDragOver = sourceListDragOver
      OnMouseDown = sourceListMouseDown
    end
    object sigProcList: TListBox
      Left = 168
      Top = 40
      Width = 153
      Height = 121
      DragMode = dmAutomatic
      ItemHeight = 13
      TabOrder = 1
      OnDragDrop = sigProcListDragDrop
      OnDragOver = sigProcListDragOver
      OnMouseDown = sigProcListMouseDown
    end
    object appList: TListBox
      Left = 328
      Top = 40
      Width = 153
      Height = 121
      DragMode = dmAutomatic
      ItemHeight = 13
      TabOrder = 2
      OnDragDrop = appListDragDrop
      OnDragOver = appListDragOver
      OnMouseDown = appListMouseDown
    end
    object GroupBox2: TGroupBox
      Left = 488
      Top = 24
      Width = 185
      Height = 137
      Caption = 'Others'
      TabOrder = 3
      object othersList: TListBox
        Left = 8
        Top = 16
        Width = 161
        Height = 113
        ItemHeight = 13
        MultiSelect = True
        TabOrder = 0
      end
    end
    object clearBut: TButton
      Left = 8
      Top = 168
      Width = 75
      Height = 25
      Caption = 'Clear All'
      TabOrder = 4
      OnClick = clearButClick
    end
  end
  object GroupBox3: TGroupBox
    Left = 0
    Top = 208
    Width = 329
    Height = 201
    Caption = 'Parameters'
    TabOrder = 1
    object Label5: TLabel
      Left = 16
      Top = 24
      Width = 67
      Height = 13
      Caption = 'Parameter File'
    end
    object Label9: TLabel
      Left = 16
      Top = 120
      Width = 76
      Height = 13
      Caption = 'Save Directory: '
    end
    object Label10: TLabel
      Left = 15
      Top = 155
      Width = 73
      Height = 13
      Caption = 'Subject Name: '
    end
    object Label11: TLabel
      Left = 173
      Top = 155
      Width = 50
      Height = 13
      Caption = 'Session #:'
    end
    object getParmBut: TButton
      Left = 288
      Top = 48
      Width = 27
      Height = 20
      Caption = '...'
      TabOrder = 0
      OnClick = getParmButClick
    end
    object delParmBut: TButton
      Left = 288
      Top = 72
      Width = 27
      Height = 20
      Caption = 'X'
      TabOrder = 1
      OnClick = delParmButClick
    end
    object parmList: TListBox
      Left = 16
      Top = 43
      Width = 266
      Height = 70
      ItemHeight = 13
      MultiSelect = True
      TabOrder = 2
    end
    object directoryBox: TEdit
      Left = 96
      Top = 119
      Width = 185
      Height = 21
      TabOrder = 3
    end
    object subjectNameBox: TEdit
      Left = 88
      Top = 151
      Width = 73
      Height = 21
      TabOrder = 4
    end
    object sessionNumBox: TEdit
      Left = 240
      Top = 151
      Width = 73
      Height = 21
      TabOrder = 5
      OnChange = sessionNumBoxChange
      OnExit = sessionNumBoxExit
      OnKeyPress = sessionNumBoxKeyPress
    end
    object getDirBtn: TButton
      Left = 284
      Top = 117
      Width = 27
      Height = 20
      Caption = '...'
      TabOrder = 6
      OnClick = getDirBtnClick
    end
  end
  object GroupBox4: TGroupBox
    Left = 336
    Top = 208
    Width = 345
    Height = 201
    Caption = 'Status'
    TabOrder = 2
    object Label4: TLabel
      Left = 147
      Top = 21
      Width = 30
      Height = 13
      Caption = 'Status'
    end
    object Label7: TLabel
      Left = 11
      Top = 73
      Width = 103
      Height = 13
      Alignment = taRightJustify
      Caption = 'Signal Processing IP: '
    end
    object Label8: TLabel
      Left = 11
      Top = 127
      Width = 71
      Height = 13
      Alignment = taRightJustify
      Caption = 'Application IP: '
    end
    object Label6: TLabel
      Left = 11
      Top = 23
      Width = 53
      Height = 13
      Alignment = taRightJustify
      Caption = 'Source IP: '
    end
    object launchBut: TButton
      Left = 150
      Top = 143
      Width = 75
      Height = 25
      Caption = 'Launch!'
      TabOrder = 0
      OnClick = launchButClick
    end
    object statusList: TListBox
      Left = 120
      Top = 40
      Width = 217
      Height = 89
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -8
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ItemHeight = 13
      ParentFont = False
      TabOrder = 1
    end
    object appIPBox: TEdit
      Left = 11
      Top = 147
      Width = 73
      Height = 21
      TabOrder = 2
      Text = '127.0.0.1'
    end
    object sigProcIPBox: TEdit
      Left = 11
      Top = 92
      Width = 73
      Height = 21
      TabOrder = 3
      Text = '127.0.0.1'
    end
    object sourceIPBox: TEdit
      Left = 11
      Top = 42
      Width = 73
      Height = 21
      TabOrder = 4
      Text = '127.0.0.1'
    end
  end
  object ActionManager1: TActionManager
    object FileRun1: TFileRun
      Category = 'File'
      Browse = False
      BrowseDlg.Title = 'Run'
      Caption = '&Run...'
      Hint = 'Run|Runs an application'
      Operation = 'open'
      ShowCmd = scShowNormal
    end
  end
  object OpenParmDlg: TOpenDialog
    Options = [ofHideReadOnly, ofAllowMultiSelect, ofEnableSizing]
    Left = 288
    Top = 224
  end
  object sourcePopup: TPopupMenu
    Left = 16
    Top = 128
    object Move1: TMenuItem
      Caption = 'Move To'
      object sourceToSP: TMenuItem
        Caption = 'Signal Processing'
        OnClick = sourceToSPClick
      end
      object sourceToApp: TMenuItem
        Caption = 'Application'
        OnClick = sourceToAppClick
      end
      object sourceToOthers: TMenuItem
        Caption = 'Others'
        OnClick = sourceToOthersClick
      end
    end
  end
  object spPopup: TPopupMenu
    Left = 176
    Top = 128
    object MoveTo1: TMenuItem
      Caption = 'Move To'
      object spToSource: TMenuItem
        Caption = 'Source'
        OnClick = spToSourceClick
      end
      object spToApp: TMenuItem
        Caption = 'Application'
        OnClick = spToAppClick
      end
      object spToOthers: TMenuItem
        Caption = 'Others'
        OnClick = spToOthersClick
      end
    end
  end
  object appPopup: TPopupMenu
    Left = 336
    Top = 128
    object MoveTo2: TMenuItem
      Caption = 'Move To'
      object appToSource: TMenuItem
        Caption = 'Source'
        OnClick = appToSourceClick
      end
      object appToSp: TMenuItem
        Caption = 'Signal Processing'
        OnClick = appToSpClick
      end
      object appToOthers: TMenuItem
        Caption = 'Others'
        OnClick = appToOthersClick
      end
    end
  end
  object othersPopup: TPopupMenu
    Left = 504
    Top = 120
    object MoveTo3: TMenuItem
      Caption = 'Move To'
      object othersToSource: TMenuItem
        Caption = 'Source'
        OnClick = othersToSourceClick
      end
      object othersToSp: TMenuItem
        Caption = 'Signal Processing'
        OnClick = othersToSpClick
      end
      object othersToApp: TMenuItem
        Caption = 'Application'
        OnClick = othersToAppClick
      end
    end
  end
  object MainMenu1: TMainMenu
    Left = 648
    Top = 8
    object Help1: TMenuItem
      Caption = 'Help'
      object helpMnu: TMenuItem
        Caption = 'BCILauncher Help'
        Hint = 'Run|Runs an application'
        OnClick = helpMnuClick
      end
      object About1: TMenuItem
        Caption = 'About...'
        OnClick = About1Click
      end
    end
    object Too1: TMenuItem
      Caption = 'Tools'
      object SIGFRIED1: TMenuItem
        Caption = 'SIGFRIED'
        OnClick = SIGFRIED1Click
      end
    end
  end
end
