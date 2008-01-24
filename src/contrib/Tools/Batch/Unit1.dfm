object Form1: TForm1
  Left = 203
  Top = 119
  Width = 496
  Height = 438
  Caption = 'BCI2000 Batching'
  Color = clSkyBlue
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object IP: TLabel
    Left = 64
    Top = 152
    Width = 51
    Height = 13
    Caption = 'IP Address'
  end
  object Label1: TLabel
    Left = 80
    Top = 256
    Width = 49
    Height = 13
    Caption = 'Delay (ms)'
  end
  object Button1: TButton
    Left = 56
    Top = 376
    Width = 75
    Height = 25
    Caption = 'Start'
    TabOrder = 0
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 384
    Top = 376
    Width = 75
    Height = 25
    Caption = 'Exit'
    TabOrder = 1
    OnClick = Button2Click
  end
  object vOperator: TEdit
    Left = 136
    Top = 24
    Width = 321
    Height = 21
    TabOrder = 2
    Text = 'c:\bci2000\Operator\operat.exe'
  end
  object vSource: TEdit
    Left = 136
    Top = 56
    Width = 321
    Height = 21
    TabOrder = 3
    Text = 'c:\bci2000\EEGsource\DTADC\DT2000.exe'
  end
  object vSignalProcessing: TEdit
    Left = 136
    Top = 88
    Width = 321
    Height = 21
    TabOrder = 4
    Text = 'c:\bci2000\SignalProcessing\AR\ARSignalProcessing.exe'
  end
  object vApplication: TEdit
    Left = 136
    Top = 120
    Width = 321
    Height = 21
    TabOrder = 5
    Text = 'c:\bci2000\Application\RJB\RJB.exe'
  end
  object vIp: TEdit
    Left = 136
    Top = 152
    Width = 121
    Height = 21
    TabOrder = 6
    Text = '127.0.0.1'
  end
  object Save: TButton
    Left = 56
    Top = 184
    Width = 75
    Height = 25
    Caption = 'Save'
    TabOrder = 7
    OnClick = SaveClick
  end
  object Get: TButton
    Left = 56
    Top = 216
    Width = 75
    Height = 25
    Caption = 'Get'
    TabOrder = 8
    OnClick = GetClick
  end
  object vSave: TEdit
    Left = 136
    Top = 184
    Width = 321
    Height = 21
    TabOrder = 9
    Text = 'saving.asc'
  end
  object vGet: TEdit
    Left = 136
    Top = 216
    Width = 321
    Height = 21
    TabOrder = 10
    Text = 'saved.asc'
  end
  object Operator: TButton
    Left = 56
    Top = 24
    Width = 75
    Height = 25
    Caption = 'Operator'
    TabOrder = 11
    OnClick = OperatorClick
  end
  object EEGsource: TButton
    Left = 56
    Top = 56
    Width = 75
    Height = 25
    Caption = 'EEGsource'
    TabOrder = 12
    OnClick = EEGsourceClick
  end
  object SignalProcess: TButton
    Left = 56
    Top = 88
    Width = 75
    Height = 25
    Caption = 'SignalProcess'
    TabOrder = 13
    OnClick = SignalProcessClick
  end
  object Application: TButton
    Left = 56
    Top = 120
    Width = 75
    Height = 25
    Caption = 'Application'
    TabOrder = 14
    OnClick = ApplicationClick
  end
  object vDelay: TEdit
    Left = 136
    Top = 256
    Width = 65
    Height = 21
    TabOrder = 15
    Text = '500'
  end
  object Parmfile: TButton
    Left = 56
    Top = 296
    Width = 75
    Height = 25
    Caption = 'Parmfile'
    TabOrder = 16
    OnClick = ParmfileClick
  end
  object vParmfile: TEdit
    Left = 136
    Top = 296
    Width = 321
    Height = 21
    TabOrder = 17
  end
  object RunParm: TButton
    Left = 56
    Top = 328
    Width = 75
    Height = 25
    Caption = 'RunParm'
    TabOrder = 18
    OnClick = RunParmClick
  end
  object vRunParm: TEdit
    Left = 136
    Top = 328
    Width = 321
    Height = 21
    TabOrder = 19
  end
  object OpenSaved: TOpenDialog
    Left = 24
    Top = 184
  end
  object SaveFile: TSaveDialog
    Left = 24
    Top = 216
  end
  object OpenOperator: TOpenDialog
    Left = 24
    Top = 24
  end
  object OpenSource: TOpenDialog
    Left = 24
    Top = 56
  end
  object OpenSignalProcessing: TOpenDialog
    Left = 24
    Top = 88
  end
  object OpenApplication: TOpenDialog
    Left = 24
    Top = 120
  end
  object OpenParmfile: TOpenDialog
    Filter = '*.prm|*.prm|*.*|*.*'
    Left = 24
    Top = 296
  end
  object RunParmfile: TSaveDialog
    Left = 24
    Top = 328
  end
end
