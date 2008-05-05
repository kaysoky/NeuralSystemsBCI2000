object Sigfried_UIfrm: TSigfried_UIfrm
  Left = 427
  Top = 370
  BorderStyle = bsDialog
  Caption = 'SIGFRIED Startup'
  ClientHeight = 500
  ClientWidth = 438
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object GroupBox1: TGroupBox
    Left = 8
    Top = 8
    Width = 425
    Height = 97
    Caption = 'Baseline Data Information'
    TabOrder = 0
    object Label9: TLabel
      Left = 6
      Top = 31
      Width = 76
      Height = 13
      Caption = 'Save Directory: '
    end
    object Label11: TLabel
      Left = 291
      Top = 64
      Width = 50
      Height = 13
      Caption = 'Session #:'
    end
    object Label1: TLabel
      Left = 18
      Top = 62
      Width = 64
      Height = 13
      Caption = 'Baseline Prm:'
    end
    object directoryBox: TEdit
      Left = 84
      Top = 28
      Width = 277
      Height = 21
      TabOrder = 0
      OnChange = directoryBoxChange
    end
    object getDirBtn: TButton
      Left = 370
      Top = 29
      Width = 27
      Height = 20
      Caption = '...'
      TabOrder = 1
      OnClick = getDirBtnClick
    end
    object sessionNumBox: TEdit
      Left = 344
      Top = 60
      Width = 65
      Height = 21
      TabOrder = 2
      OnExit = sessionNumBoxExit
      OnKeyPress = sessionNumBoxKeyPress
    end
    object baselinePrmBtn: TButton
      Left = 242
      Top = 60
      Width = 27
      Height = 20
      Caption = '...'
      TabOrder = 4
      OnClick = baselinePrmBtnClick
    end
    object baselinePrmBox: TEdit
      Left = 84
      Top = 59
      Width = 157
      Height = 21
      TabOrder = 3
      OnChange = baselinePrmBoxChange
    end
  end
  object GroupBox2: TGroupBox
    Left = 8
    Top = 112
    Width = 425
    Height = 73
    Caption = 'Step 1: Record Baseline'
    TabOrder = 1
    object Label2: TLabel
      Left = 96
      Top = 36
      Width = 177
      Height = 13
      Caption = 'Record baseline using above settings'
    end
    object recordBaselineBtn: TButton
      Left = 14
      Top = 32
      Width = 75
      Height = 25
      Caption = 'Record'
      Enabled = False
      TabOrder = 0
      OnClick = recordBaselineBtnClick
    end
  end
  object GroupBox3: TGroupBox
    Left = 8
    Top = 192
    Width = 425
    Height = 193
    Caption = 'Step 2: Build SIGFRIED Model'
    TabOrder = 2
    object Label3: TLabel
      Left = 8
      Top = 49
      Width = 67
      Height = 13
      Caption = 'Model Output:'
    end
    object Label4: TLabel
      Left = 8
      Top = 25
      Width = 71
      Height = 13
      Caption = 'Model *.ini File:'
    end
    object Label6: TLabel
      Left = 23
      Top = 78
      Width = 56
      Height = 13
      Caption = 'Description:'
    end
    object Label7: TLabel
      Left = 8
      Top = 144
      Width = 28
      Height = 13
      Caption = 'Width'
    end
    object Label8: TLabel
      Left = 56
      Top = 144
      Width = 31
      Height = 13
      Caption = 'Height'
    end
    object modelFileBox: TEdit
      Left = 84
      Top = 46
      Width = 157
      Height = 21
      TabOrder = 0
      OnChange = modelFileBoxChange
    end
    object modelDirBtn: TButton
      Left = 242
      Top = 47
      Width = 27
      Height = 20
      Caption = '...'
      TabOrder = 1
      OnClick = modelDirBtnClick
    end
    object modelIniBox: TEdit
      Left = 84
      Top = 22
      Width = 157
      Height = 21
      TabOrder = 2
      OnChange = modelIniBoxChange
    end
    object modelIniBtn: TButton
      Left = 242
      Top = 23
      Width = 27
      Height = 20
      Caption = '...'
      TabOrder = 3
      OnClick = modelIniBtnClick
    end
    object buildModelBtn: TButton
      Left = 341
      Top = 144
      Width = 75
      Height = 25
      Caption = 'Build Models'
      Enabled = False
      TabOrder = 4
      OnClick = buildModelBtnClick
    end
    object modelList: TListBox
      Left = 288
      Top = 24
      Width = 121
      Height = 113
      ItemHeight = 13
      TabOrder = 5
    end
    object modelDescBox: TEdit
      Left = 84
      Top = 74
      Width = 121
      Height = 21
      TabOrder = 6
    end
    object addModelBtn: TButton
      Left = 285
      Top = 144
      Width = 25
      Height = 25
      Caption = '+'
      TabOrder = 7
      OnClick = addModelBtnClick
    end
    object remModelBtn: TButton
      Left = 309
      Top = 144
      Width = 27
      Height = 25
      Caption = '-'
      TabOrder = 8
      OnClick = remModelBtnClick
    end
    object visModelWidthBox: TEdit
      Left = 8
      Top = 160
      Width = 41
      Height = 21
      TabOrder = 9
      Text = '200'
    end
    object visModelHeightBox: TEdit
      Left = 56
      Top = 160
      Width = 41
      Height = 21
      TabOrder = 10
      Text = '200'
    end
    object visModelsCheck: TCheckBox
      Left = 8
      Top = 120
      Width = 97
      Height = 17
      Caption = 'Visualize Models?'
      Checked = True
      State = cbChecked
      TabOrder = 11
    end
  end
  object GroupBox4: TGroupBox
    Left = 8
    Top = 392
    Width = 425
    Height = 105
    Caption = 'Step 3: Create Parm Fragment and Return'
    TabOrder = 3
    object Label5: TLabel
      Left = 8
      Top = 30
      Width = 62
      Height = 13
      Caption = 'Parm Output:'
    end
    object parmOutputBox: TEdit
      Left = 84
      Top = 27
      Width = 285
      Height = 21
      TabOrder = 0
      OnChange = parmOutputBoxChange
    end
    object parmOutputBtn: TButton
      Left = 378
      Top = 28
      Width = 27
      Height = 20
      Caption = '...'
      TabOrder = 1
      OnClick = parmOutputBtnClick
    end
    object returnBtn: TButton
      Left = 16
      Top = 64
      Width = 91
      Height = 25
      Caption = 'Save - Return...'
      TabOrder = 2
      OnClick = returnBtnClick
    end
  end
  object OpenFileDlg: TOpenDialog
    Left = 264
  end
  object ActionManager1: TActionManager
    Left = 240
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
  object SaveFileDlg: TSaveDialog
    Left = 208
  end
  object PopupMenu1: TPopupMenu
    Left = 304
    Top = 8
  end
end
