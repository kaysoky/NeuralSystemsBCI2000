object FBForm: TFBForm
  Left = 290
  Top = 276
  Width = 705
  Height = 528
  HorzScrollBar.Visible = False
  VertScrollBar.Visible = False
  Caption = 'CTP-Application V1.11'
  Color = clNavy
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Visible = True
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnResize = FormResize
  PixelsPerInch = 96
  TextHeight = 13
  object ZeroBar: TShape
    Left = 0
    Top = 240
    Width = 697
    Height = 17
    Brush.Color = clGray
    Pen.Color = clGray
  end
  object UpperGoal: TShape
    Left = 8
    Top = 8
    Width = 673
    Height = 73
    Brush.Color = clPurple
    Pen.Color = clGreen
    Pen.Width = 10
  end
  object LowerGoal: TShape
    Left = 8
    Top = 416
    Width = 673
    Height = 73
    Brush.Color = clPurple
    Pen.Color = clGreen
    Pen.Width = 10
  end
  object MiddleGoal: TShape
    Left = 624
    Top = 192
    Width = 65
    Height = 121
    Brush.Color = clPurple
    Pen.Color = clGreen
    Pen.Width = 10
    Visible = False
  end
  object BottomText: TLabel
    Left = 48
    Top = 432
    Width = 600
    Height = 42
    Alignment = taCenter
    AutoSize = False
    Caption = 'T'
    Color = clPurple
    Font.Charset = ANSI_CHARSET
    Font.Color = clYellow
    Font.Height = -37
    Font.Name = 'Times New Roman'
    Font.Style = []
    ParentColor = False
    ParentFont = False
    Transparent = True
    Layout = tlCenter
    Visible = False
  end
  object TopText: TLabel
    Left = 40
    Top = 24
    Width = 93
    Height = 31
    Caption = 'TopText'
    Font.Charset = ANSI_CHARSET
    Font.Color = clYellow
    Font.Height = -27
    Font.Name = 'Times New Roman'
    Font.Style = []
    ParentFont = False
    Transparent = True
    Visible = False
  end
  object PB1: TPaintBox
    Left = 184
    Top = 112
    Width = 313
    Height = 289
  end
  object Ball: TShape
    Left = 32
    Top = 224
    Width = 49
    Height = 49
    Brush.Color = clYellow
    Shape = stCircle
  end
  object Label1: TLabel
    Left = 8
    Top = 136
    Width = 32
    Height = 13
    Caption = 'Label1'
    Color = clSilver
    ParentColor = False
  end
  object MediaPlayer1: TMediaPlayer
    Left = 8
    Top = 96
    Width = 253
    Height = 30
    AutoOpen = True
    FileName = 'Task1.WAV'
    Visible = False
    TabOrder = 0
  end
  object MediaPlayer2: TMediaPlayer
    Left = 352
    Top = 240
    Width = 253
    Height = 30
    AutoOpen = True
    FileName = 'Task2.WAV'
    Visible = False
    TabOrder = 1
  end
  object MediaPlayer4: TMediaPlayer
    Left = 8
    Top = 376
    Width = 253
    Height = 30
    AutoOpen = True
    FileName = 'Task4.wav'
    Visible = False
    TabOrder = 2
  end
  object ResponsePlayer1: TMediaPlayer
    Left = 352
    Top = 40
    Width = 253
    Height = 30
    Visible = False
    TabOrder = 3
  end
  object ResponsePlayer4: TMediaPlayer
    Left = 400
    Top = 424
    Width = 253
    Height = 30
    Visible = False
    TabOrder = 4
  end
  object QuestionPlayer: TMediaPlayer
    Left = 16
    Top = 192
    Width = 253
    Height = 30
    Visible = False
    TabOrder = 5
  end
  object GMTimer: TTimer
    Interval = 500
    OnTimer = GMTimerTimer
  end
end
