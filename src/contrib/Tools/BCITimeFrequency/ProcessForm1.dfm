object ProcessForm: TProcessForm
  Left = 557
  Top = 283
  BorderStyle = bsDialog
  Caption = 'ProcessForm'
  ClientHeight = 381
  ClientWidth = 332
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 56
    Top = 56
    Width = 50
    Height = 13
    Caption = 'Hz to Start'
  end
  object Label2: TLabel
    Left = 56
    Top = 80
    Width = 47
    Height = 13
    Caption = 'Hz to End'
  end
  object Label3: TLabel
    Left = 40
    Top = 104
    Width = 68
    Height = 13
    Caption = 'Hz line density'
  end
  object Label4: TLabel
    Left = 56
    Top = 128
    Width = 50
    Height = 13
    Caption = 'Bandwidth'
  end
  object Label5: TLabel
    Left = 24
    Top = 240
    Width = 85
    Height = 13
    Caption = 'Mem Data Length'
    Visible = False
  end
  object Label6: TLabel
    Left = 48
    Top = 152
    Width = 58
    Height = 13
    Caption = 'Model Order'
  end
  object Label7: TLabel
    Left = 32
    Top = 216
    Width = 76
    Height = 13
    Caption = 'Mem Block Size'
    Visible = False
  end
  object Label8: TLabel
    Left = 40
    Top = 192
    Width = 70
    Height = 13
    Caption = 'Mem Windows'
    Visible = False
  end
  object UseMEM: TCheckBox
    Left = 24
    Top = 16
    Width = 97
    Height = 17
    Caption = 'UseMEM'
    TabOrder = 0
  end
  object Remove: TRadioGroup
    Left = 200
    Top = 56
    Width = 121
    Height = 73
    Caption = 'Remove Trend'
    ItemIndex = 0
    Items.Strings = (
      'No'
      'Mean'
      'Linear Trend')
    TabOrder = 1
  end
  object Exit: TButton
    Left = 248
    Top = 344
    Width = 75
    Height = 25
    Caption = 'Exit'
    TabOrder = 2
    OnClick = ExitClick
  end
  object vStart: TEdit
    Left = 112
    Top = 48
    Width = 65
    Height = 21
    TabOrder = 3
    Text = '0'
  end
  object vEnd: TEdit
    Left = 112
    Top = 72
    Width = 65
    Height = 21
    TabOrder = 4
    Text = '64.0'
  end
  object vDensity: TEdit
    Left = 112
    Top = 96
    Width = 65
    Height = 21
    TabOrder = 5
    Text = '0.2'
  end
  object vBandwidth: TEdit
    Left = 112
    Top = 120
    Width = 65
    Height = 21
    TabOrder = 6
    Text = '1'
  end
  object vMemDataLength: TEdit
    Left = 112
    Top = 232
    Width = 65
    Height = 21
    TabOrder = 7
    Text = '64'
    Visible = False
    OnChange = vMemDataLengthChange
  end
  object vModel: TEdit
    Left = 112
    Top = 144
    Width = 65
    Height = 21
    TabOrder = 8
    Text = '16'
  end
  object vMemWindows: TEdit
    Left = 112
    Top = 184
    Width = 65
    Height = 21
    TabOrder = 9
    Text = '1'
    Visible = False
    OnChange = vMemWindowsChange
  end
  object vMemBlockSize: TEdit
    Left = 112
    Top = 208
    Width = 65
    Height = 21
    TabOrder = 10
    Text = '64'
    Visible = False
    OnChange = vMemBlockSizeChange
  end
  object MemWinType: TRadioGroup
    Left = 200
    Top = 168
    Width = 121
    Height = 49
    Caption = 'Windows'
    ItemIndex = 0
    Items.Strings = (
      'Data Length'
      'Fixed')
    TabOrder = 11
    OnClick = MemWinTypeClick
  end
  object GroupBox1: TGroupBox
    Left = 200
    Top = 272
    Width = 121
    Height = 49
    Caption = 'Sidelobe Suppression'
    TabOrder = 12
    object cbSidelobeSuppression: TComboBox
      Left = 16
      Top = 16
      Width = 97
      Height = 21
      Style = csDropDownList
      ItemHeight = 0
      TabOrder = 0
    end
  end
end
