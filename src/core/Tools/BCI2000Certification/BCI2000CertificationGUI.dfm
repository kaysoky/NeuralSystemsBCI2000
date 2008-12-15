object BCICertificationGUI: TBCICertificationGUI
  Left = 296
  Top = 225
  Caption = 'BCI2000 Certification'
  ClientHeight = 701
  ClientWidth = 620
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object GroupBox1: TGroupBox
    Left = 224
    Top = 8
    Width = 388
    Height = 517
    Caption = 'Task Details'
    Color = clBtnFace
    ParentColor = False
    TabOrder = 0
    object Label1: TLabel
      Left = 16
      Top = 77
      Width = 164
      Height = 13
      Alignment = taRightJustify
      Caption = 'Signal Source (Overwrites Global):'
    end
    object Label2: TLabel
      Left = 16
      Top = 125
      Width = 89
      Height = 13
      Alignment = taRightJustify
      Caption = 'Signal Processing: '
    end
    object Label3: TLabel
      Left = 16
      Top = 173
      Width = 59
      Height = 13
      Alignment = taRightJustify
      Caption = 'Application: '
    end
    object taskNameBox: TLabeledEdit
      Left = 16
      Top = 40
      Width = 345
      Height = 21
      EditLabel.Width = 52
      EditLabel.Height = 13
      EditLabel.Caption = 'Task Name'
      TabOrder = 0
      OnExit = taskNameBoxExit
    end
    object ampBox: TLabeledEdit
      Left = 32
      Top = 385
      Width = 65
      Height = 21
      BiDiMode = bdLeftToRight
      EditLabel.Width = 63
      EditLabel.Height = 13
      EditLabel.BiDiMode = bdLeftToRight
      EditLabel.Caption = 'Amp Channel'
      EditLabel.ParentBiDiMode = False
      ParentBiDiMode = False
      TabOrder = 9
      Text = '1'
      OnExit = ampBoxExit
    end
    object digAmpBox: TLabeledEdit
      Left = 120
      Top = 385
      Width = 65
      Height = 21
      BiDiMode = bdLeftToRight
      EditLabel.Width = 95
      EditLabel.Height = 13
      EditLabel.BiDiMode = bdLeftToRight
      EditLabel.Caption = 'Digital Amp Channel'
      EditLabel.ParentBiDiMode = False
      ParentBiDiMode = False
      TabOrder = 10
      Text = '16'
      OnExit = digAmpBoxExit
    end
    object vidBox: TLabeledEdit
      Left = 32
      Top = 433
      Width = 65
      Height = 21
      BiDiMode = bdLeftToRight
      EditLabel.Width = 68
      EditLabel.Height = 13
      EditLabel.BiDiMode = bdLeftToRight
      EditLabel.Caption = 'Video Channel'
      EditLabel.ParentBiDiMode = False
      ParentBiDiMode = False
      TabOrder = 11
      Text = '2'
      OnExit = vidBoxExit
    end
    object vidStateBox: TLabeledEdit
      Left = 120
      Top = 433
      Width = 105
      Height = 21
      BiDiMode = bdLeftToRight
      EditLabel.Width = 26
      EditLabel.Height = 13
      EditLabel.BiDiMode = bdLeftToRight
      EditLabel.Caption = 'State'
      EditLabel.ParentBiDiMode = False
      ParentBiDiMode = False
      TabOrder = 12
      Text = 'StimulusCode'
      OnExit = vidStateBoxExit
    end
    object vidStateValuesBox: TLabeledEdit
      Left = 240
      Top = 433
      Width = 105
      Height = 21
      BiDiMode = bdLeftToRight
      EditLabel.Width = 31
      EditLabel.Height = 13
      EditLabel.BiDiMode = bdLeftToRight
      EditLabel.Caption = 'Values'
      EditLabel.ParentBiDiMode = False
      ParentBiDiMode = False
      TabOrder = 13
      Text = '3'
      OnExit = vidStateValuesBoxExit
    end
    object audioBox: TLabeledEdit
      Left = 32
      Top = 481
      Width = 65
      Height = 21
      BiDiMode = bdLeftToRight
      EditLabel.Width = 69
      EditLabel.Height = 13
      EditLabel.BiDiMode = bdLeftToRight
      EditLabel.Caption = 'Audio Channel'
      EditLabel.ParentBiDiMode = False
      ParentBiDiMode = False
      TabOrder = 14
      Text = '2'
      OnExit = audioBoxExit
    end
    object audioStateBox: TLabeledEdit
      Left = 120
      Top = 481
      Width = 105
      Height = 21
      BiDiMode = bdLeftToRight
      EditLabel.Width = 26
      EditLabel.Height = 13
      EditLabel.BiDiMode = bdLeftToRight
      EditLabel.Caption = 'State'
      EditLabel.ParentBiDiMode = False
      ParentBiDiMode = False
      TabOrder = 15
      Text = 'StimulusCode'
      OnExit = audioStateBoxExit
    end
    object audioStateValuesBox: TLabeledEdit
      Left = 239
      Top = 481
      Width = 105
      Height = 21
      BiDiMode = bdLeftToRight
      EditLabel.Width = 31
      EditLabel.Height = 13
      EditLabel.BiDiMode = bdLeftToRight
      EditLabel.Caption = 'Values'
      EditLabel.ParentBiDiMode = False
      ParentBiDiMode = False
      TabOrder = 16
      Text = '3'
      OnExit = audioStateValuesBoxExit
    end
    object addPrmBtn: TButton
      Left = 336
      Top = 231
      Width = 27
      Height = 25
      Caption = '+'
      TabOrder = 8
      OnClick = addPrmBtnClick
    end
    object delPrmBtn: TButton
      Left = 303
      Top = 231
      Width = 27
      Height = 25
      Caption = '-'
      TabOrder = 7
      OnClick = delPrmBtnClick
    end
    object parmsList: TListView
      Left = 200
      Top = 77
      Width = 161
      Height = 148
      Columns = <
        item
          AutoSize = True
          Caption = 'Parameters'
        end>
      ParentShowHint = False
      ShowHint = True
      TabOrder = 17
      ViewStyle = vsReport
      OnInfoTip = parmsListInfoTip
    end
    object sigSourceBox: TEdit
      Left = 17
      Top = 96
      Width = 144
      Height = 21
      TabOrder = 1
      OnExit = sigSourceBoxExit
    end
    object getSigSrcBtn: TButton
      Left = 160
      Top = 96
      Width = 27
      Height = 21
      Caption = '...'
      TabOrder = 2
      OnClick = getSigSrcBtnClick
    end
    object sigProcBox: TEdit
      Left = 17
      Top = 144
      Width = 144
      Height = 21
      TabOrder = 3
      OnExit = sigProcBoxExit
    end
    object getSigProcBtn: TButton
      Left = 160
      Top = 144
      Width = 27
      Height = 21
      Caption = '...'
      TabOrder = 4
      OnClick = getSigProcBtnClick
    end
    object appBox: TEdit
      Left = 17
      Top = 192
      Width = 144
      Height = 21
      TabOrder = 5
      OnExit = appBoxExit
    end
    object getAppBtn: TButton
      Left = 160
      Top = 192
      Width = 27
      Height = 21
      Caption = '...'
      TabOrder = 6
      OnClick = getAppBtnClick
    end
    object sampleRateBox: TLabeledEdit
      Left = 16
      Top = 280
      Width = 81
      Height = 21
      EditLabel.Width = 60
      EditLabel.Height = 13
      EditLabel.Caption = 'Sample Rate'
      TabOrder = 18
      OnExit = sampleRateBoxExit
    end
    object SBSbox: TLabeledEdit
      Left = 120
      Top = 280
      Width = 81
      Height = 21
      EditLabel.Width = 83
      EditLabel.Height = 13
      EditLabel.Caption = 'Sample Block Size'
      TabOrder = 19
      OnExit = SBSboxExit
    end
  end
  object GroupBox2: TGroupBox
    Left = 8
    Top = 8
    Width = 210
    Height = 419
    Caption = 'Task List'
    TabOrder = 1
    object addTaskBtn: TButton
      Left = 168
      Top = 383
      Width = 27
      Height = 25
      Caption = '+'
      TabOrder = 2
      OnClick = addTaskBtnClick
    end
    object delTaskBtn: TButton
      Left = 135
      Top = 383
      Width = 27
      Height = 25
      Caption = '-'
      TabOrder = 1
      OnClick = delTaskBtnClick
    end
    object taskList: TListView
      Left = 3
      Top = 24
      Width = 204
      Height = 353
      Checkboxes = True
      Columns = <
        item
          AutoSize = True
          Caption = 'Tasks'
        end>
      TabOrder = 0
      ViewStyle = vsReport
      OnClick = taskListClick
    end
    object copyBtn: TButton
      Left = 80
      Top = 383
      Width = 49
      Height = 25
      Caption = 'Copy'
      TabOrder = 3
      OnClick = copyBtnClick
    end
  end
  object GroupBox3: TGroupBox
    Left = 224
    Top = 531
    Width = 388
    Height = 162
    Caption = 'Global Settings'
    TabOrder = 2
    object Label4: TLabel
      Left = 16
      Top = 61
      Width = 64
      Height = 13
      Caption = 'Signal Source'
    end
    object Label6: TLabel
      Left = 17
      Top = 102
      Width = 97
      Height = 13
      Caption = 'Data Save Directory'
    end
    object winLeftBox: TLabeledEdit
      Left = 16
      Top = 36
      Width = 65
      Height = 21
      BiDiMode = bdLeftToRight
      EditLabel.Width = 60
      EditLabel.Height = 13
      EditLabel.BiDiMode = bdLeftToRight
      EditLabel.Caption = 'Window Left'
      EditLabel.ParentBiDiMode = False
      ParentBiDiMode = False
      TabOrder = 0
      Text = '0'
      OnExit = winLeftBoxExit
    end
    object winWidthBox: TLabeledEdit
      Left = 176
      Top = 36
      Width = 65
      Height = 21
      BiDiMode = bdLeftToRight
      EditLabel.Width = 69
      EditLabel.Height = 13
      EditLabel.BiDiMode = bdLeftToRight
      EditLabel.Caption = 'Window Width'
      EditLabel.ParentBiDiMode = False
      ParentBiDiMode = False
      TabOrder = 2
      Text = '800'
      OnExit = winWidthBoxExit
    end
    object winHeightBox: TLabeledEdit
      Left = 256
      Top = 36
      Width = 65
      Height = 21
      BiDiMode = bdLeftToRight
      EditLabel.Width = 72
      EditLabel.Height = 13
      EditLabel.BiDiMode = bdLeftToRight
      EditLabel.Caption = 'Window Height'
      EditLabel.ParentBiDiMode = False
      ParentBiDiMode = False
      TabOrder = 3
      Text = '800'
      OnExit = winHeightBoxExit
    end
    object dataSaveBox: TEdit
      Left = 16
      Top = 121
      Width = 329
      Height = 21
      TabOrder = 6
      Text = 'data'
      OnExit = dataSaveBoxExit
    end
    object dataSaveBtn: TButton
      Left = 344
      Top = 120
      Width = 25
      Height = 21
      Caption = '...'
      TabOrder = 7
      OnClick = dataSaveBtnClick
    end
    object globalSigSrcBox: TEdit
      Left = 16
      Top = 75
      Width = 145
      Height = 21
      TabOrder = 4
      OnExit = globalSigSrcBoxExit
    end
    object getGlobSigSrcBtn: TButton
      Left = 160
      Top = 73
      Width = 26
      Height = 21
      Caption = '...'
      TabOrder = 5
    end
    object winTopBox: TLabeledEdit
      Left = 96
      Top = 36
      Width = 65
      Height = 21
      BiDiMode = bdLeftToRight
      EditLabel.Width = 59
      EditLabel.Height = 13
      EditLabel.BiDiMode = bdLeftToRight
      EditLabel.Caption = 'Window Top'
      EditLabel.ParentBiDiMode = False
      ParentBiDiMode = False
      TabOrder = 1
      Text = '0'
      OnExit = winTopBoxExit
    end
  end
  object GroupBox4: TGroupBox
    Left = 8
    Top = 433
    Width = 210
    Height = 260
    Caption = 'Controls'
    TabOrder = 3
    object startBtn: TButton
      Left = 16
      Top = 25
      Width = 49
      Height = 25
      Caption = 'Start'
      TabOrder = 0
      OnClick = startBtnClick
    end
    object cancelBtn: TButton
      Left = 71
      Top = 25
      Width = 49
      Height = 25
      Caption = 'Cancel'
      TabOrder = 1
      OnClick = cancelBtnClick
    end
    object analyzeBtn: TButton
      Left = 126
      Top = 25
      Width = 49
      Height = 25
      Caption = 'Analyze'
      TabOrder = 2
      OnClick = analyzeBtnClick
    end
    object infoBox: TMemo
      Left = 3
      Top = 56
      Width = 204
      Height = 201
      TabOrder = 3
    end
  end
  object OpenDialog1: TOpenDialog
    Left = 488
    Top = 240
  end
  object MainMenu1: TMainMenu
    Left = 344
    Top = 8
    object File1: TMenuItem
      Caption = 'File'
      object Openini1: TMenuItem
        Caption = 'Open *.ini'
        OnClick = Openini1Click
      end
      object Saveini1: TMenuItem
        Caption = 'Save *.ini'
        OnClick = Saveini1Click
      end
    end
  end
  object SaveDialog1: TSaveDialog
    Left = 376
    Top = 8
  end
end
