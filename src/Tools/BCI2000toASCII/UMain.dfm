object fMain: TfMain
  Left = 379
  Top = 317
  Width = 438
  Height = 454
  Caption = 'BCI2000toASCII'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Icon.Data = {
    0000010001002020100000000000E80200001600000028000000200000004000
    0000010004000000000080020000000000000000000000000000000000000000
    000000008000008000000080800080000000800080008080000080808000C0C0
    C0000000FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00BB9B
    BBBBBB9BB99999BBBBB999BBBBBBBB9BBBBBBB9B9BBBBB9BBB9BBB9BBBBBBBB9
    BBBBB9BB9BBBBB9BB9BBBBB9BBBBBBB9999999BBBBBBBB9BB9BBBBBBBBBBBBBB
    9BBB9BBBBBBB99BBB9BBBBBBBBBBBBBB9BBB9BBBB999BBBBB9BBBBBBBBBBBBBB
    B9B9BBBB9BBBBBBBB9BBBBBBBBBBBBBBB9B9BBBB9BBBBB9BB9BBBBB9BBBBBBBB
    B9B9BBBB9BBBBB9BBB9BBB9BBBBBBBBBBB9BBBBBB99999BBBBB999BBBBBBBBBB
    BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB99999BBBBBBBBBBBBBBBBBB
    BBBBBBBBBB9BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB9BBBBBBBBBBBBBBBBBBBB
    BBBBBBBBBBBB9BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB9BBBBBBBBBBBBBBBBBB
    BBBBBBBBBBBBB9BBBBBBBBBBBBBBBBBBBBBBBBBBB9BBB9BBBBBBBBBBBBBBBBBB
    BBBBBBBBBB999BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
    BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB99999BB999BBB999BBB999BBBBBBBBBBB
    9BBBB9BBB9B9BBB9B9BBB9BBBBBBBBBBB9BBB9BBB9B9BBB9B9BBB9BBBBBBBBBB
    BB9BB9BBB9B9BBB9B9BBB9BBBBBBBBBBBBB9B9BBB9B9BBB9B9BBB9BBBBBBBBBB
    BBB9B9BBB9B9BBB9B9BBB9BBBBBBBBB9BBB9B9BBB9B9BBB9B9BBB9BBBBBBBBBB
    999BBB999BBB999BBB999BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
    BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB0000
    0000000000000000000000000000000000000000000000000000000000000000
    0000000000000000000000000000000000000000000000000000000000000000
    0000000000000000000000000000000000000000000000000000000000000000
    000000000000000000000000000000000000000000000000000000000000}
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 40
    Top = 32
    Width = 74
    Height = 16
    Caption = 'BCI2000 File'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label2: TLabel
    Left = 40
    Top = 88
    Width = 58
    Height = 16
    Caption = 'ASCII File'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Gauge: TCGauge
    Left = 0
    Top = 392
    Width = 425
    Height = 25
  end
  object Label3: TLabel
    Left = 296
    Top = 16
    Width = 34
    Height = 13
    Caption = 'first run'
  end
  object Label4: TLabel
    Left = 296
    Top = 48
    Width = 34
    Height = 13
    Caption = 'last run'
  end
  object Label5: TLabel
    Left = 0
    Top = 136
    Width = 63
    Height = 13
    Caption = 'Export States'
  end
  object Label6: TLabel
    Left = 0
    Top = 349
    Width = 113
    Height = 13
    Caption = 'Increment trial #, if state'
  end
  object Label7: TLabel
    Left = 136
    Top = 349
    Width = 53
    Height = 13
    Caption = 'switches to'
  end
  object Label8: TLabel
    Left = 40
    Top = 256
    Width = 89
    Height = 13
    Caption = 'Export only, if state'
  end
  object Label9: TLabel
    Left = 176
    Top = 256
    Width = 31
    Height = 13
    Caption = 'equals'
  end
  object Label10: TLabel
    Left = 224
    Top = 256
    Width = 18
    Height = 13
    Caption = 'and'
  end
  object Label11: TLabel
    Left = 264
    Top = 256
    Width = 31
    Height = 13
    Caption = 'if state'
  end
  object Label12: TLabel
    Left = 392
    Top = 256
    Width = 31
    Height = 13
    Caption = 'equals'
  end
  object Label13: TLabel
    Left = 40
    Top = 296
    Width = 31
    Height = 13
    Caption = 'if state'
  end
  object Label14: TLabel
    Left = 176
    Top = 296
    Width = 31
    Height = 13
    Caption = 'equals'
  end
  object Label15: TLabel
    Left = 224
    Top = 296
    Width = 18
    Height = 13
    Caption = 'and'
  end
  object Label16: TLabel
    Left = 264
    Top = 296
    Width = 31
    Height = 13
    Caption = 'if state'
  end
  object Label17: TLabel
    Left = 392
    Top = 296
    Width = 31
    Height = 13
    Caption = 'equals'
  end
  object Label18: TLabel
    Left = 0
    Top = 288
    Width = 31
    Height = 24
    Caption = 'OR'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -19
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object eSourceFile: TEdit
    Left = 40
    Top = 48
    Width = 225
    Height = 21
    TabOrder = 0
    Text = 'D:\BCI2000DATA\gal165\galS165R01.dat'
  end
  object eDestinationFile: TEdit
    Left = 40
    Top = 104
    Width = 225
    Height = 21
    TabOrder = 1
    Text = 'D:\BCI2000DATA\gal165\galS165.mat'
  end
  object bOpenFile: TButton
    Left = 0
    Top = 48
    Width = 33
    Height = 21
    Caption = 'File'
    TabOrder = 2
    OnClick = bOpenFileClick
  end
  object Button1: TButton
    Left = 0
    Top = 104
    Width = 33
    Height = 21
    Caption = 'File'
    TabOrder = 3
    OnClick = Button1Click
  end
  object bConvert: TButton
    Left = 344
    Top = 152
    Width = 81
    Height = 25
    Caption = 'Convert'
    Enabled = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 4
    OnClick = ContinueClick
  end
  object frun: TEdit
    Left = 344
    Top = 16
    Width = 81
    Height = 21
    Enabled = False
    TabOrder = 5
    Text = '1'
    OnChange = frunChange
  end
  object lrun: TEdit
    Left = 344
    Top = 48
    Width = 81
    Height = 21
    Enabled = False
    TabOrder = 6
    Text = '8'
  end
  object bDefineInput: TButton
    Left = 344
    Top = 120
    Width = 81
    Height = 25
    Caption = 'Define Input'
    TabOrder = 7
    OnClick = bDefineInputClick
  end
  object cStateListBox: TCheckListBox
    Left = 0
    Top = 152
    Width = 121
    Height = 97
    ItemHeight = 13
    TabOrder = 8
  end
  object eITIStateValue: TEdit
    Left = 136
    Top = 365
    Width = 33
    Height = 21
    TabOrder = 9
    Text = '1'
  end
  object eState1aVal: TEdit
    Left = 176
    Top = 272
    Width = 33
    Height = 21
    TabOrder = 10
    Text = '1'
  end
  object eState1bVal: TEdit
    Left = 392
    Top = 272
    Width = 33
    Height = 21
    TabOrder = 11
  end
  object eState2bVal: TEdit
    Left = 392
    Top = 312
    Width = 33
    Height = 21
    TabOrder = 13
  end
  object eState2aVal: TEdit
    Left = 176
    Top = 312
    Width = 33
    Height = 21
    TabOrder = 12
  end
  object ListBox1a: TComboBox
    Left = 40
    Top = 272
    Width = 113
    Height = 21
    ItemHeight = 13
    TabOrder = 14
  end
  object ListBox1b: TComboBox
    Left = 264
    Top = 272
    Width = 113
    Height = 21
    ItemHeight = 13
    TabOrder = 15
  end
  object ListBox2a: TComboBox
    Left = 40
    Top = 312
    Width = 113
    Height = 21
    ItemHeight = 13
    TabOrder = 16
  end
  object ListBox2b: TComboBox
    Left = 264
    Top = 312
    Width = 113
    Height = 21
    ItemHeight = 13
    TabOrder = 17
  end
  object ITIstateListBox: TComboBox
    Left = 0
    Top = 365
    Width = 113
    Height = 21
    ItemHeight = 13
    TabOrder = 18
  end
  object rExportMatlab: TRadioButton
    Left = 136
    Top = 152
    Width = 113
    Height = 17
    Caption = 'Export Matlab'
    Checked = True
    TabOrder = 19
    TabStop = True
  end
  object rExportFile: TRadioButton
    Left = 136
    Top = 176
    Width = 113
    Height = 17
    Caption = 'Export File'
    TabOrder = 20
  end
  object OpenDialog: TOpenDialog
    Filter = 'BCI2000 EEG files (*.DAT)|*.DAT|All Files (*.*)|*.*'
    Options = [ofHideReadOnly, ofFileMustExist, ofEnableSizing]
    Left = 40
  end
  object SaveDialog: TSaveDialog
    Left = 80
  end
  object OpenParameter: TOpenDialog
    Filter = '*.prm|*.prm|all files|*.*'
  end
  object OpenDialog1: TOpenDialog
    Filter = '*.prm|*.prm|all files|*.*'
  end
  object OpenDialog2: TOpenDialog
    Filter = '*.prm|*.prm|all files|*.*'
  end
end
