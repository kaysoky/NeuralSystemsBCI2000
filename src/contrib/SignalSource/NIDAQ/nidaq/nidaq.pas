{
 ***************************************************************************
     NI-DAQ Header file for Borland Delphi (32-bit)
     Copyright    (C) National Instruments 2002.

 ***************************************************************************
}

unit NIDAQ;

interface

uses Windows;

{ special types }

type
   i8 = ShortInt;
type
   u8 = Byte;
type
   pi8 = PChar;
type
   i16 = SmallInt;
type
   u16 = Word;
type
   pi16 = ^i16;
type
   pu16 = ^u16;
type
   i32 = LongInt;
type
   u32 = Cardinal;
type
   pi32 = ^i32;
type
   pu32 = ^u32;
type
   f32 = Single;
type
   f64 = Double;
type
   pf64 = ^f64;
type
   nidaqStatus = i16;

const
   nidaqdll = 'nidaq32.dll';


{ NI-DAQ function prototypes }

function AI_Change_Parameter (
	slot:                 i16;
	channel:              i16;
	paramID:              u32;
	paramValue:           u32
	):nidaqStatus; stdcall; external nidaqdll;
function AI_Check (
	slot:                 i16;
	status:               pi16;
	value:                pi16
	):nidaqStatus; stdcall; external nidaqdll;
function AI_Clear (
	slot:                 i16
	):nidaqStatus; stdcall; external nidaqdll;
function AI_Configure (
	slot:                 i16;
	chan:                 i16;
	inputMode:            i16;
	inputRange:           i16;
	polarity:             i16;
	driveAIS:             i16
	):nidaqStatus; stdcall; external nidaqdll;
function AI_Mux_Config (
	slot:                 i16;
	numMuxBrds:           i16
	):nidaqStatus; stdcall; external nidaqdll;
function AI_Read (
	slot:                 i16;
	chan:                 i16;
	gain:                 i16;
	value:                pi16
	):nidaqStatus; stdcall; external nidaqdll;
function AI_Read32 (
	slot:                 i16;
	chan:                 i16;
	gain:                 i16;
	value:                pi32
	):nidaqStatus; stdcall; external nidaqdll;
function AI_Setup (
	slot:                 i16;
	chan:                 i16;
	gain:                 i16
	):nidaqStatus; stdcall; external nidaqdll;
function AI_VRead (
	slot:                 i16;
	chan:                 i16;
	gain:                 i16;
	volts:                pf64
	):nidaqStatus; stdcall; external nidaqdll;
function AI_VScale (
	slot:                 i16;
	chan:                 i16;
	gain:                 i16;
	gainAdjust:           f64;
	offset:               f64;
	reading:              i16;
	voltage:              pf64
	):nidaqStatus; stdcall; external nidaqdll;
function Align_DMA_Buffer (
	slot:                 i16;
	resource:             i16;
	buffer:               pi16;
	cnt:                  u32;
	bufSize:              u32;
	alignIndex:           pu32
	):nidaqStatus; stdcall; external nidaqdll;
function AO_Calibrate (
	board:                i16;
	operation:            i16;
	EEPROMloc:            i16
	):nidaqStatus; stdcall; external nidaqdll;
function AO_Configure (
	slot:                 i16;
	chan:                 i16;
	outputPolarity:       i16;
	IntOrExtRef:          i16;
	refVoltage:           f64;
	updateMode:           i16
	):nidaqStatus; stdcall; external nidaqdll;
function AO_Change_Parameter (
	slot:                 i16;
	channel:              i16;
	paramID:              u32;
	paramValue:           u32
	):nidaqStatus; stdcall; external nidaqdll;
function AO_Update (
	slot:                 i16
	):nidaqStatus; stdcall; external nidaqdll;
function AO_VWrite (
	slot:                 i16;
	chan:                 i16;
	voltage:              f64
	):nidaqStatus; stdcall; external nidaqdll;
function AO_Write (
	slot:                 i16;
	chan:                 i16;
	value:                i16
	):nidaqStatus; stdcall; external nidaqdll;
function Calibrate_E_Series (
	deviceNumber:         i16;
	calOp:                u32;
	setOfCalConst:        u32;
	calRefVolts:          f64
	):nidaqStatus; stdcall; external nidaqdll;
function Calibrate_59xx (
	deviceNumber:         i16;
	operation:            u32;
	refVoltage:           f64
	):nidaqStatus; stdcall; external nidaqdll;
function Calibrate_DSA (
	deviceNumber:         i16;
	operation:            u32;
	refVoltage:           f64
	):nidaqStatus; stdcall; external nidaqdll;
function Config_Alarm_Deadband (
	slot:                 i16;
	mode:                 i16;
	chanStr:              pi8;
	trigLvl:              f64;
	deadbandWidth:        f64;
	handle:               HWND;
	alarmOnMsg:           i16;
	alarmOffMsg:          i16;
	callbackAddr:         u32
	):nidaqStatus; stdcall; external nidaqdll;
function Config_ATrig_Event_Message (
	slot:                 i16;
	mode:                 i16;
	chanStr:              pi8;
	trigLvl:              f64;
	winSize:              f64;
	trigSlope:            i16;
	skipCnt:              u32;
	preTrigScans:         u32;
	postTrigScans:        u32;
	handle:               HWND;
	msg:                  i16;
	callBackAddr:         u32
	):nidaqStatus; stdcall; external nidaqdll;
function Config_DAQ_Event_Message (
	slot:                 i16;
	mode:                 i16;
	chanStr:              pi8;
	DAQEvent:             i16;
	trigVal0:             i32;
	trigVal1:             i32;
	skipCnt:              u32;
	preTrigScans:         u32;
	postTrigScans:        u32;
	handle:               HWND;
	msg:                  i16;
	callBackAddr:         u32
	):nidaqStatus; stdcall; external nidaqdll;
function Configure_HW_Analog_Trigger (
	deviceNumber:         i16;
	onOrOff:              u32;
	lowValue:             i32;
	highValue:            i32;
	mode:                 u32;
	trigSource:           u32
	):nidaqStatus; stdcall; external nidaqdll;
function CTR_Config (
	slot:                 i16;
	ctr:                  i16;
	edgeMode:             i16;
	gateMode:             i16;
	outType:              i16;
	outPolarity:          i16
	):nidaqStatus; stdcall; external nidaqdll;
function CTR_EvCount (
	slot:                 i16;
	ctr:                  i16;
	timebase:             i16;
	cont:                 i16
	):nidaqStatus; stdcall; external nidaqdll;
function CTR_EvRead (
	slot:                 i16;
	ctr:                  i16;
	overflow:             pi16;
	counts:               pu16
	):nidaqStatus; stdcall; external nidaqdll;
function CTR_FOUT_Config (
	slot:                 i16;
	FOUT:                 i16;
	mode:                 i16;
	timebase:             i16;
	division:             i16
	):nidaqStatus; stdcall; external nidaqdll;
function CTR_Period (
	slot:                 i16;
	ctr:                  i16;
	timebase:             i16
	):nidaqStatus; stdcall; external nidaqdll;
function CTR_Pulse (
	slot:                 i16;
	ctr:                  i16;
	timebase:             i16;
	delay:                u16;
	pulseWidth:           u16
	):nidaqStatus; stdcall; external nidaqdll;
function CTR_Rate (
	freq:                 f64;
	duty:                 f64;
	timebase:             pi16;
	period1:              pu16;
	period2:              pu16
	):nidaqStatus; stdcall; external nidaqdll;
function CTR_Reset (
	slot:                 i16;
	ctr:                  i16;
	outState:             i16
	):nidaqStatus; stdcall; external nidaqdll;
function CTR_Restart (
	slot:                 i16;
	ctr:                  i16
	):nidaqStatus; stdcall; external nidaqdll;
function CTR_Simul_Op (
	slot:                 i16;
	numCtrs:              i16;
	ctrList:              pi16;
	mode:                 i16
	):nidaqStatus; stdcall; external nidaqdll;
function CTR_Square (
	slot:                 i16;
	ctr:                  i16;
	timebase:             i16;
	period1:              u16;
	period2:              u16
	):nidaqStatus; stdcall; external nidaqdll;
function CTR_State (
	slot:                 i16;
	ctr:                  i16;
	outState:             pi16
	):nidaqStatus; stdcall; external nidaqdll;
function CTR_Stop (
	slot:                 i16;
	ctr:                  i16
	):nidaqStatus; stdcall; external nidaqdll;
function DAQ_Check (
	slot:                 i16;
	progress:             pi16;
	retrieved:            pu32
	):nidaqStatus; stdcall; external nidaqdll;
function DAQ_Clear (
	slot:                 i16
	):nidaqStatus; stdcall; external nidaqdll;
function DAQ_Config (
	slot:                 i16;
	startTrig:            i16;
	extConv:              i16
	):nidaqStatus; stdcall; external nidaqdll;
function DAQ_DB_Config (
	slot:                 i16;
	dbMode:               i16
	):nidaqStatus; stdcall; external nidaqdll;
function DAQ_DB_HalfReady (
	slot:                 i16;
	halfReady:            pi16;
	status:               pi16
	):nidaqStatus; stdcall; external nidaqdll;
function DAQ_DB_Transfer (
	slot:                 i16;
	hbuffer:              pi16;
	ptsTfr:               pu32;
	status:               pi16
	):nidaqStatus; stdcall; external nidaqdll;
function DAQ_Monitor (
	slot:                 i16;
	chan:                 i16;
	seq:                  i16;
	monitorCnt:           u32;
	monitorBuf:           pi16;
	newestIndex:          pu32;
	status:               pi16
	):nidaqStatus; stdcall; external nidaqdll;
function DAQ_Op (
	slot:                 i16;
	chan:                 i16;
	gain:                 i16;
	buffer:               pi16;
	cnt:                  u32;
	sampleRate:           f64
	):nidaqStatus; stdcall; external nidaqdll;
function DAQ_Rate (
	rate:                 f64;
	units:                i16;
	timebase:             pi16;
	sampleInt:            pu16
	):nidaqStatus; stdcall; external nidaqdll;
function DAQ_Start (
	slot:                 i16;
	chan:                 i16;
	gain:                 i16;
	buffer:               pi16;
	cnt:                  u32;
	timebase:             i16;
	sampInt:              u16
	):nidaqStatus; stdcall; external nidaqdll;
function DAQ_StopTrigger_Config (
	slot:                 i16;
	preTrig:              i16;
	preTrigCnt:           u32
	):nidaqStatus; stdcall; external nidaqdll;
function DAQ_to_Disk (
	slot:                 i16;
	chan:                 i16;
	gain:                 i16;
	fileName:             pi8;
	cnt:                  u32;
	sampleRate:           f64;
	concat:               i16
	):nidaqStatus; stdcall; external nidaqdll;
function DAQ_VScale (
	slot:                 i16;
	chan:                 i16;
	gain:                 i16;
	gainAdjust:           f64;
	offset:               f64;
	cnt:                  u32;
	binArray:             pi16;
	voltArray:            pf64
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Block_Check (
	slot:                 i16;
	grp:                  i16;
	remaining:            pu32
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Block_Clear (
	slot:                 i16;
	grp:                  i16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Block_In (
	slot:                 i16;
	grp:                  i16;
	buffer:               pi16;
	cnt:                  u32
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Block_Out (
	slot:                 i16;
	grp:                  i16;
	buffer:               pi16;
	cnt:                  u32
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Block_PG_Config (
	slot:                 i16;
	grp:                  i16;
	PGmode:               i16;
	reqSource:            i16;
	timebase:             i16;
	interval:             u16;
	externalGate:         i16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_DB_Config (
	slot:                 i16;
	grp:                  i16;
	DBMode:               i16;
	oldDataStop:          i16;
	partialTransfer:      i16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_DB_HalfReady (
	slot:                 i16;
	grp:                  i16;
	halfReady:            pi16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_DB_Transfer (
	slot:                 i16;
	grp:                  i16;
	halfBuffer:           pi16;
	ptsTfr:               u32
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Grp_Config (
	slot:                 i16;
	grp:                  i16;
	grpsize:              i16;
	port:                 i16;
	direction:            i16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Grp_Mode (
	slot:                 i16;
	grp:                  i16;
	sigType:              i16;
	edge:                 i16;
	reqpol:               i16;
	ackpol:               i16;
	settleTime:           i16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Grp_Status (
	slot:                 i16;
	grp:                  i16;
	status:               pi16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_In_Grp (
	slot:                 i16;
	grp:                  i16;
	grp_pat:              pi16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_In_Line (
	slot:                 i16;
	port:                 i16;
	linenum:              i16;
	state:                pi16
	):nidaqStatus; stdcall; external nidaqdll;
function Query_Optimizations_GFS (
	device:               i16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_In_Port_GFS (
	slot:                 i16;
	port:                 i16;
	pattern:              pi16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_In_Port (
	slot:                 i16;
	port:                 i16;
	pattern:              pi16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Line_Config (
	slot:                 i16;
	port:                 i16;
	linenum:              i16;
	direction:            i16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Out_Grp (
	slot:                 i16;
	grp:                  i16;
	grp_pat:              i16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Out_Line (
	slot:                 i16;
	port:                 i16;
	linenum:              i16;
	state:                i16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Out_Port_GFS (
	slot:                 i16;
	port:                 i16;
	pattern:              i16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Out_Port (
	slot:                 i16;
	port:                 i16;
	pattern:              i16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Prt_Config (
	slot:                 i16;
	port:                 i16;
	latch_mode:           i16;
	direction:            i16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Prt_Status (
	slot:                 i16;
	port:                 i16;
	status:               pi16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_SCAN_Setup (
	slot:                 i16;
	grp:                  i16;
	numPorts:             i16;
	portList:             pi16;
	direction:            i16
	):nidaqStatus; stdcall; external nidaqdll;
function Get_DAQ_Device_Info (
	deviceNumber:         i16;
	infoType:             u32;
	infoVal:              pu32
	):nidaqStatus; stdcall; external nidaqdll;
function Get_DAQ_Event (
	timeOut:              u32;
	handle:               pi16;
	msg:                  pi16;
	wParam:               pi16;
	lParam:               pi32
	):nidaqStatus; stdcall; external nidaqdll;
function Get_NI_DAQ_Version (
	version:              pu32
	):nidaqStatus; stdcall; external nidaqdll;
function GPCTR_Config_Buffer (
	deviceNumber:         i16;
	gpCounterNumber:      u32;
	reserved:             u32;
	numPoints:            u32;
	buffer:               pu32
	):nidaqStatus; stdcall; external nidaqdll;
function GPCTR_Read_Buffer (
	deviceNumber:         i16;
	gpCounterNumber:      u32;
	readMode:             u32;
	readOffset:           i32;
	numPointsToRead:      u32;
	timeOut:              f64;
	numPointsRead:        pu32;
	buffer:               pu32
	):nidaqStatus; stdcall; external nidaqdll;
function Line_Change_Attribute (
	deviceNumber:         i16;
	lineNumber:           u32;
	attribID:             u32;
	attribValue:          u32
	):nidaqStatus; stdcall; external nidaqdll;
function GPCTR_Control (
	deviceNumber:         i16;
	gpCounterNumber:      u32;
	action:               u32
	):nidaqStatus; stdcall; external nidaqdll;
function GPCTR_Set_Application (
	deviceNumber:         i16;
	gpCounterNumber:      u32;
	application:          u32
	):nidaqStatus; stdcall; external nidaqdll;
function GPCTR_Watch (
	deviceNumber:         i16;
	gpCounterNumber:      u32;
	watchID:              u32;
	watchValue:           pu32
	):nidaqStatus; stdcall; external nidaqdll;
function ICTR_Read (
	slot:                 i16;
	counter:              i16;
	cnt:                  pu16
	):nidaqStatus; stdcall; external nidaqdll;
function ICTR_Reset (
	slot:                 i16;
	counter:              i16;
	state:                i16
	):nidaqStatus; stdcall; external nidaqdll;
function ICTR_Setup (
	slot:                 i16;
	counter:              i16;
	mode:                 i16;
	cnt:                  u16;
	binBCD:               i16
	):nidaqStatus; stdcall; external nidaqdll;
function Init_DA_Brds (
	slot:                 i16;
	brdCode:              pi16
	):nidaqStatus; stdcall; external nidaqdll;
function Lab_ISCAN_Check (
	slot:                 i16;
	status:               pi16;
	retrieved:            pu32;
	finalScanOrder:       pi16
	):nidaqStatus; stdcall; external nidaqdll;
function Lab_ISCAN_Op (
	slot:                 i16;
	numChans:             i16;
	gain:                 i16;
	buffer:               pi16;
	cnt:                  u32;
	sampleRate:           f64;
	scanRate:             f64;
	finalScanOrder:       pi16
	):nidaqStatus; stdcall; external nidaqdll;
function Lab_ISCAN_Start (
	slot:                 i16;
	numChans:             i16;
	gain:                 i16;
	buffer:               pi16;
	cnt:                  u32;
	timebase:             i16;
	sampleInt:            u16;
	scanInt:              u16
	):nidaqStatus; stdcall; external nidaqdll;
function Lab_ISCAN_to_Disk (
	slot:                 i16;
	numChans:             i16;
	gain:                 i16;
	fileName:             pi8;
	cnt:                  u32;
	sampleRate:           f64;
	scanRate:             f64;
	concat:               i16
	):nidaqStatus; stdcall; external nidaqdll;
function LPM16_Calibrate (
	slot:                 i16
	):nidaqStatus; stdcall; external nidaqdll;
function MIO_Config (
	slot:                 i16;
	dither:               i16;
	useAMUX:              i16
	):nidaqStatus; stdcall; external nidaqdll;
function Peek_DAQ_Event (
	timeOut:              u32;
	handle:               pi16;
	msg:                  pi16;
	wParam:               pi16;
	lParam:               pi32
	):nidaqStatus; stdcall; external nidaqdll;
function REG_Level_Read (
	slot:                 i16;
	registerIndex:        i16;
	registerValue:        pu32
	):nidaqStatus; stdcall; external nidaqdll;
function REG_Level_Write (
	slot:                 i16;
	registerIndex:        i16;
	bitsAffected:         u32;
	bitSettings:          u32;
	registerValue:        pu32
	):nidaqStatus; stdcall; external nidaqdll;
function RTSI_Clear (
	slot:                 i16
	):nidaqStatus; stdcall; external nidaqdll;
function RTSI_Clock (
	slot:                 i16;
	connect:              i16;
	direction:            i16
	):nidaqStatus; stdcall; external nidaqdll;
function RTSI_Conn (
	slot:                 i16;
	brdSignal:            i16;
	busLine:              i16;
	direction:            i16
	):nidaqStatus; stdcall; external nidaqdll;
function RTSI_DisConn (
	slot:                 i16;
	brdSignal:            i16;
	busLine:              i16
	):nidaqStatus; stdcall; external nidaqdll;
function SC_2040_Config (
	deviceNumber:         i16;
	channel:              i16;
	sc2040Gain:           i16
	):nidaqStatus; stdcall; external nidaqdll;
function SCAN_Demux (
	buffer:               pi16;
	cnt:                  u32;
	numChans:             i16;
	muxMode:              i16
	):nidaqStatus; stdcall; external nidaqdll;
function SCAN_Op (
	slot:                 i16;
	numChans:             i16;
	chans:                pi16;
	gains:                pi16;
	buffer:               pi16;
	cnt:                  u32;
	sampleRate:           f64;
	scanRate:             f64
	):nidaqStatus; stdcall; external nidaqdll;
function SCAN_Sequence_Demux (
	numChans:             i16;
	chanVector:           pi16;
	bufferSize:           u32;
	buffer:               pi16;
	samplesPerSequence:   i16;
	scanSequenceVector:   pi16;
	samplesPerChanVector: pu32
	):nidaqStatus; stdcall; external nidaqdll;
function SCAN_Sequence_Retrieve (
	deviceNumber:         i16;
	samplesPerSequence:   i16;
	scanSequenceVector:   pi16
	):nidaqStatus; stdcall; external nidaqdll;
function SCAN_Sequence_Setup (
	deviceNumber:         i16;
	numChans:             i16;
	chanVector:           pi16;
	gainVector:           pi16;
	scanRateDivVector:    pi16;
	scansPerSequence:     pi16;
	samplesPerSequence:   pi16
	):nidaqStatus; stdcall; external nidaqdll;
function SCAN_Setup (
	slot:                 i16;
	num_chans:            i16;
	chans:                pi16;
	gains:                pi16
	):nidaqStatus; stdcall; external nidaqdll;
function SCAN_Start (
	slot:                 i16;
	buffer:               pi16;
	cnt:                  u32;
	tb1:                  i16;
	si1:                  u16;
	tb2:                  i16;
	si2:                  u16
	):nidaqStatus; stdcall; external nidaqdll;
function SCAN_to_Disk (
	slot:                 i16;
	numChans:             i16;
	chans:                pi16;
	gains:                pi16;
	fileName:             pi8;
	cnt:                  u32;
	sampleRate:           f64;
	scanRate:             f64;
	concat:               i16
	):nidaqStatus; stdcall; external nidaqdll;
function Calibrate_1200 (
	deviceNumber:         i16;
	calOP:                i16;
	saveNewCal:           i16;
	EEPROMloc:            i16;
	calRefChan:           i16;
	grndRefChan:          i16;
	DAC0chan:             i16;
	DAC1chan:             i16;
	calRefVolts:          f64;
	gain:                 f64
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_AO_Write (
	chassisID:            i16;
	moduleSlot:           i16;
	DACchannel:           i16;
	opCode:               i16;
	rangeCode:            i16;
	voltCurrentData:      f64;
	binaryDat:            i16;
	binaryWritten:        pi16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Cal_Constants (
	chassisID:            i16;
	moduleSlot:           i16;
	SCXIchannel:          i16;
	operation:            i16;
	calArea:              i16;
	rangeCode:            i16;
	SCXIgain:             f64;
	DAQdevice:            i16;
	DAQchannel:           i16;
	DAQgain:              i16;
	TBgain:               f64;
	volt1:                f64;
	binary1:              f64;
	volt2:                f64;
	binary2:              f64;
	binEEprom1:           pf64;
	binEEprom2:           pf64
	):nidaqStatus; stdcall; external nidaqdll;
function InitChannelWizardStrainCal (
	chassisID:            i16;
	moduleSlot:           i16;
	moduleChan:           i16;
	device:               i16;
	loLim:                f32;
	hiLim:                f32;
	moduleGain:           pf64;
	DAQgain:              pi16;
	DAQchan:              pi16;
	excitation:           f32;
	connectionType:       i16
	):nidaqStatus; stdcall; external nidaqdll;
function ChannelWizardSCXIStrainCal (
	chassisID:            i16;
	moduleSlot:           i16;
	moduleChan:           i16;
	device:               i16;
	DAQchan:              i16;
	moduleGain:           f64;
	DAQgain:              i16;
	engageShuntA:         u8;
	engageShuntB:         u8;
	voltage:              pf64;
	pos_excitation:       pf64;
	neg_excitation:       pf64
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_1520_Transducer_Cal (
	chassisID:            i16;
	moduleSlot:           i16;
	moduleChan:           i16;
	SCXIgain:             f64;
	DAQdevice:            i16;
	DAQchan:              i16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Calibrate (
	chassisID:            i16;
	moduleSlot:           i16;
	moduleChan:           i16;
	operation:            i16;
	calArea:              i16;
	SCXIgain:             f64;
	inputRefVoltage:      f64;
	DAQdevice:            i16;
	DAQchan:              i16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Strain_Null (
	chassisID:            i16;
	slot:                 i16;
	moduleChan:           i16;
	device:               i16;
	DAQchan:              i16;
	imbalances:           pf32
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Strain_Null_Ex (
	chassisID:            i16;
	slot:                 i16;
	moduleChan:           i16;
	device:               i16;
	DAQchan:              i16;
	imbalances:           pf32;
	excitation:           f32;
	bridgeType:           u32;
	finePot:              pi16;
	coarsePot:            pi16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Calibrate_Setup (
	chassisID:            i16;
	moduleSlot:           i16;
	calOp:                i16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Change_Chan (
	chassisID:            i16;
	moduleSlot:           i16;
	chan:                 i16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Set_Potentiometer (
	chassisID:            i16;
	moduleSlot:           i16;
	channel:              i16;
	value:                u16;
	pottype:              u8
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Set_Excitation (
	chassisID:            i16;
	moduleSlot:           i16;
	channel:              i16;
	excitationType:       i16;
	excitation:           f32;
	actualExcitation:     pf32
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Configure_Connection (
	chassisID:            i16;
	moduleSlot:           i16;
	channel:              i16;
	connectionType:       i16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Configure_Filter (
	chassisID:            i16;
	moduleSlot:           i16;
	channel:              i16;
	filterMode:           i16;
	freq:                 f64;
	cutoffDivDown:        u16;
	outClkDivDown:        u16;
	actFreq:              pf64
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Configure_Filter_NoStatusCheck (
	chassisID:            i16;
	moduleSlot:           i16;
	channel:              i16;
	filterMode:           i16;
	freq:                 f64;
	cutoffDivDown:        u16;
	outClkDivDown:        u16;
	actFreq:              pf64
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Get_Chassis_Info (
	chassisID:            i16;
	chassisType:          pi16;
	address:              pi16;
	commMode:             pi16;
	commPath:             pi16;
	numSlots:             pi16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Get_Module_Info (
	chassisID:            i16;
	slot:                 i16;
	modulePresent:        pi32;
	opMode:               pi16;
	DAQboard:             pi16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Get_State (
	chassisID:            i16;
	moduleSlot:           i16;
	port:                 i16;
	channel:              i16;
	data:                 pu32
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Get_Status (
	chassisID:            i16;
	moduleSlot:           i16;
	wait:                 i16;
	data:                 pu32
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Load_Config (
	chassisID:            i16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_MuxCtr_Setup (
	slot:                 i16;
	enable:               i16;
	scanDiv:              i16;
	muxCtrVal:            u16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Reset (
	chassisID:            i16;
	moduleSlot:           i16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Scale (
	chassisID:            i16;
	moduleSlot:           i16;
	SCXIchannel:          i16;
	SCXIgain:             f64;
	TBgain:               f64;
	DAQdevice:            i16;
	DAQchannel:           i16;
	DAQgain:              i16;
	numPoints:            u32;
	binArray:             pi16;
	voltArray:            pf64
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_SCAN_Setup (
	chassisID:            i16;
	numModules:           i16;
	modules:              pi16;
	numChans:             pi16;
	startChans:           pi16;
	DAQboard:             i16;
	modeFlag:             i16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Set_Config (
	chassisID:            i16;
	chassisType:          i16;
	address:              i16;
	commMode:             i16;
	slotOrCOMM:           i16;
	numSlots:             i16;
	moduleTypes:          pi32;
	opModes:              pi16;
	DAQboards:            pi16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Set_Gain (
	chassisID:            i16;
	moduleSlot:           i16;
	channel:              i16;
	gain:                 f64
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Set_Gain_NoStatusCheck (
	chassisID:            i16;
	moduleSlot:           i16;
	channel:              i16;
	gain:                 f64
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Set_Input_Mode (
	chassisID:            i16;
	moduleSlot:           i16;
	inputMode:            i16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Set_State (
	chassisID:            i16;
	moduleSlot:           i16;
	port:                 i16;
	channel:              i16;
	data:                 u32
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Single_Chan_Setup (
	chassisID:            i16;
	moduleSlot:           i16;
	chan:                 i16;
	DAQboard:             i16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Track_Hold_Control (
	chassisID:            i16;
	moduleSlot:           i16;
	state:                i16;
	DAQboard:             i16
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Track_Hold_Setup (
	chassisID:            i16;
	moduleSlot:           i16;
	mode:                 i16;
	source:               i16;
	send:                 i16;
	holdCnt:              i16;
	DAQboard:             i16
	):nidaqStatus; stdcall; external nidaqdll;
function Select_Signal (
	deviceNumber:         i16;
	signal:               u32;
	source:               u32;
	sourceSpec:           u32
	):nidaqStatus; stdcall; external nidaqdll;
function Set_DAQ_Device_Info (
	deviceNumber:         i16;
	infoType:             u32;
	infoVal:              u32
	):nidaqStatus; stdcall; external nidaqdll;
function Timeout_Config (
	slot:                 i16;
	numTicks:             i32
	):nidaqStatus; stdcall; external nidaqdll;
function WFM_Chan_Control (
	slot:                 i16;
	channel:              i16;
	operation:            i16
	):nidaqStatus; stdcall; external nidaqdll;
function WFM_Check (
	slot:                 i16;
	channel:              i16;
	progress:             pi16;
	itersDone:            pu32;
	pointsDone:           pu32
	):nidaqStatus; stdcall; external nidaqdll;
function WFM_ClockRate (
	slot:                 i16;
	group:                i16;
	whickClock:           i16;
	timebase:             i16;
	updateInterval:       u32;
	mode:                 i16
	):nidaqStatus; stdcall; external nidaqdll;
function WFM_DB_Config (
	slot:                 i16;
	numChans:             i16;
	chanVect:             pi16;
	DBMode:               i16;
	oldDataStop:          i16;
	partialTransfer:      i16
	):nidaqStatus; stdcall; external nidaqdll;
function WFM_DB_HalfReady (
	slot:                 i16;
	numChans:             i16;
	chanVect:             pi16;
	halfReady:            pi16
	):nidaqStatus; stdcall; external nidaqdll;
function WFM_DB_Transfer (
	slot:                 i16;
	numChans:             i16;
	chanVect:             pi16;
	buffer:               pi16;
	cnt:                  u32
	):nidaqStatus; stdcall; external nidaqdll;
function WFM_from_Disk (
	slot:                 i16;
	numChans:             i16;
	chanVect:             pi16;
	fileName:             pi8;
	startPts:             u32;
	endPts:               u32;
	iterations:           u32;
	rate:                 f64
	):nidaqStatus; stdcall; external nidaqdll;
function WFM_Group_Control (
	slot:                 i16;
	group:                i16;
	operation:            i16
	):nidaqStatus; stdcall; external nidaqdll;
function WFM_Group_Setup (
	slot:                 i16;
	numChans:             i16;
	chanVect:             pi16;
	group:                i16
	):nidaqStatus; stdcall; external nidaqdll;
function WFM_Load (
	slot:                 i16;
	numChans:             i16;
	chanVect:             pi16;
	buffer:               pi16;
	cnt:                  u32;
	iterations:           u32;
	mode:                 i16
	):nidaqStatus; stdcall; external nidaqdll;
function WFM_Op (
	slot:                 i16;
	numChans:             i16;
	chanVect:             pi16;
	buffer:               pi16;
	cnt:                  u32;
	iterations:           u32;
	rate:                 f64
	):nidaqStatus; stdcall; external nidaqdll;
function WFM_Rate (
	rate:                 f64;
	units:                i16;
	timebase:             pi16;
	updateInterval:       pu32
	):nidaqStatus; stdcall; external nidaqdll;
function WFM_Scale (
	slot:                 i16;
	chan:                 i16;
	cnt:                  u32;
	gain:                 f64;
	voltArray:            pf64;
	binArray:             pi16
	):nidaqStatus; stdcall; external nidaqdll;
function AI_Read_Scan (
	slot:                 i16;
	reading:              pi16
	):nidaqStatus; stdcall; external nidaqdll;
function AI_VRead_Scan (
	slot:                 i16;
	reading:              pf64
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_ModuleID_Read (
	scxiID:               i16;
	moduleSlot:           i16;
	id:                   pi32
	):nidaqStatus; stdcall; external nidaqdll;
function AO_VScale (
	slot:                 i16;
	chan:                 i16;
	voltage:              f64;
	value:                pi16
	):nidaqStatus; stdcall; external nidaqdll;
function GPCTR_Change_Parameter (
	deviceNumber:         i16;
	gpCounterNumber:      u32;
	paramID:              u32;
	paramValue:           u32
	):nidaqStatus; stdcall; external nidaqdll;
function USE_MIO 
	 :nidaqStatus; stdcall; external nidaqdll;
function USE_LPM 
	 :nidaqStatus; stdcall; external nidaqdll;
function USE_LAB 
	 :nidaqStatus; stdcall; external nidaqdll;
function USE_DIO_96 
	 :nidaqStatus; stdcall; external nidaqdll;
function USE_DIO_32F 
	 :nidaqStatus; stdcall; external nidaqdll;
function USE_DIO_24 
	 :nidaqStatus; stdcall; external nidaqdll;
function USE_AO_610 
	 :nidaqStatus; stdcall; external nidaqdll;
function USE_AO_2DC 
	 :nidaqStatus; stdcall; external nidaqdll;
function DIG_Trigger_Config (
	slot:                 i16;
	grp:                  i16;
	startTrig:            i16;
	startPol:             i16;
	stopTrig:             i16;
	stopPol:              i16;
	ptsAfterStopTrig:     u32;
	pattern:              u32;
	patternMask:          u32
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_Set_Threshold (
	chassisID:            i16;
	moduleSlot:           i16;
	channel:              i16;
	threshHold:           f64;
	hysteresis:           f64
	):nidaqStatus; stdcall; external nidaqdll;
function WFM_Set_Clock (
	slot:                 i16;
	group:                i16;
	whichClock:           u32;
	desiredRate:          f64;
	units:                u32;
	actualRate:           pf64
	):nidaqStatus; stdcall; external nidaqdll;
function DAQ_Set_Clock (
	slot:                 i16;
	whichClock:           u32;
	desiredRate:          f64;
	units:                u32;
	actualRate:           pf64
	):nidaqStatus; stdcall; external nidaqdll;
function Tio_Select_Signal (
	deviceNumber:         i16;
	signal:               u32;
	source:               u32;
	sourceSpec:           u32
	):nidaqStatus; stdcall; external nidaqdll;
function Tio_Combine_Signals (
	deviceNumber:         i16;
	internalLine:         u32;
	logicalExpression:    u32
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_In_Prt (
	slot:                 i16;
	port:                 i16;
	pattern:              pi32
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Out_Prt (
	slot:                 i16;
	port:                 i16;
	pattern:              i32
	):nidaqStatus; stdcall; external nidaqdll;
function AI_Get_Overloaded_Channels (
	deviceNumber:         i16;
	numChannels:          pi16;
	channelList:          pi16
	):nidaqStatus; stdcall; external nidaqdll;
function Calibrate_TIO (
	deviceNumber:         i16;
	operation:            u32;
	setOfCalConst:        u32;
	referenceFreq:        f64
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Change_Message_Config (
	deviceNumber:         i16;
	operation:            i16;
	riseChanStr:          pi8;
	fallChanStr:          pi8;
	handle:               HWND;
	msg:                  i16;
	callBackAddr:         u32
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Change_Message_Control (
	deviceNumber:         i16;
	ctrlCode:             i16
	):nidaqStatus; stdcall; external nidaqdll;
function DIG_Filter_Config (
	deviceNumber:         i16;
	mode:                 i16;
	chanStr:              pi8;
	interval:             f64
	):nidaqStatus; stdcall; external nidaqdll;
function SCXI_TerminalBlockID_Read (
	scxiID:               i16;
	moduleSlot:           i16;
	id:                   pi32
	):nidaqStatus; stdcall; external nidaqdll;
function ni62xx_CalStart (
	device:               i16;
	password:             pi8
	):nidaqStatus; stdcall; external nidaqdll;
function ni62xx_CalAdjust (
	device:               i16;
	calMode:              u32;
	measuredData:         pf64;
	numOfValues:          i16
	):nidaqStatus; stdcall; external nidaqdll;
function ni62xx_CalEnd (
	device:               i16;
	calAction:            u32
	):nidaqStatus; stdcall; external nidaqdll;
function ni62xx_SelfCalibrate (
	device:               i16
	):nidaqStatus; stdcall; external nidaqdll;
function CalFetchInternalReference (
	device:               i16;
	referenceValue:       pf64
	):nidaqStatus; stdcall; external nidaqdll;
function CalChangePassword (
	device:               i16;
	oldPassword:          pi8;
	newPassword:          pi8
	):nidaqStatus; stdcall; external nidaqdll;
function CalFetchCount (
	device:               i16;
	calType:              u32;
	calCount:             pi32
	):nidaqStatus; stdcall; external nidaqdll;
function CalFetchDate (
	device:               i16;
	calType:              u32;
	year:                 pi32;
	month:                pi32;
	day:                  pi32
	):nidaqStatus; stdcall; external nidaqdll;
function CalFetchTemperature (
	device:               i16;
	calType:              u32;
	temperature:          pf64
	):nidaqStatus; stdcall; external nidaqdll;
function CalFetchMiscInfo (
	device:               i16;
	miscInfo:             pi8
	):nidaqStatus; stdcall; external nidaqdll;
function CalStoreMiscInfo (
	device:               i16;
	miscInfo:             pi8
	):nidaqStatus; stdcall; external nidaqdll;


implementation

end.


