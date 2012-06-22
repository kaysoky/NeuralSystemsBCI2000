#! ../prog/BCI2000Shell
@cls & ..\prog\BCI2000Shell %0 %* #! && exit /b 0 || exit /b 1

#TODO:
# if [${VAR}==val ]  always returns true:  very easy to miss this

reset system

########################################################################################

set environment BATCHDIR  ${canonical path ${parent directory $0}}
set environment PARMSDIR  ${canonical path ${BATCHDIR}/../parms}
set environment PYWD      ${canonical path ${BATCHDIR}/../python}
set environment PYLOGDIR  ${canonical path ${BATCHDIR}/../log}
set environment LOCALPROG ${canonical path ${BATCHDIR}/../prog}

set environment SUBJECT TestSubject
if [ $1 ]; set environment SUBJECT $1; end

set environment CONDITION 002
if [ $2 ]; set environment CONDITION $2; end

set environment MODE CALIB
if [ $3 ]; set environment MODE $3; end

set environment SRC gUSBampSource
#set environment SRC SignalGenerator ; warn SignalGenerator is the default
if [ $4 ]; set environment SRC $4; end

set environment MONTAGE B
if [ $5 ]; set environment MONTAGE $5; end

########################################################################################

set environment TIMINGFLAG "--EvaluateTiming=1" # otherwise an --EvaluateTiming might hang over from a previous launch
if [ ${SRC} == SignalGenerator ]; set environment TIMINGFLAG --EvaluateTiming=0; end

change directory $BCI2000LAUNCHDIR
show window; set title ${Extract file base $0}
reset system
startup system

start executable ${SRC}                 AUTOSTART 127.0.0.1 --SignalSourceIP=127.0.0.1     $TIMINGFLAG
start executable PythonSignalProcessing AUTOSTART 127.0.0.1 --SignalProcessingIP=127.0.0.1 --PythonSigWD=${PYWD} --PythonSigClassFile=Streaming.py       --PythonSigLog=${PYLOGDIR}/###-sig.txt --PythonSigShell=1
start executable PythonApplication      AUTOSTART 127.0.0.1 --ApplicationIP=127.0.0.1      --PythonAppWD=${PYWD} --PythonAppClassFile=TrialStructure.py  --PythonAppLog=${PYLOGDIR}/###-app.txt --PythonAppShell=0

wait for connected 600

########################################################################################

if [ ${MONTAGE} == B ]
	load parameterfile ${PARMSDIR}/gUSBamp-Cap8-SMR.prm
elseif [ ${MONTAGE} == D ]
	load parameterfile ${PARMSDIR}/gUSBamp-Cap8-SMR-REVERSED-WIRING.prm
elseif [ ${MONTAGE} == 16 ]
	load parameterfile ${PARMSDIR}/gUSBamp-Cap16.prm
	load parameterfile ${PARMSDIR}/TransmitOnly8Channels.prm
else
	error unrecognized montage ${MONTAGE}
end

load parameterfile ${PARMSDIR}/realbase.prm 
load parameterfile ${PARMSDIR}/condition${CONDITION}.prm

if [ ${MODE} == FREE ]
	load parameterfile ${PARMSDIR}/realfree.prm 
elseif [ ${MODE} != CALIB ]
	error unrecognized mode ${MODE}
end

if [ ${SRC} == Emotiv ]
	load parameterfile ${PARMSDIR}/epoc.prm
end

########################################################################################

set parameter SubjectName    ${SUBJECT}${MODE}
set parameter SubjectSession ${CONDITION}

########################################################################################

# TODO:  find upstairs data directories if necessary
set environment DATADIR ${canonical path ${get parameter DataDirectory}/${get parameter SubjectName}${get parameter SubjectSession}}
set environment WEIGHTS ${canonical path ${DATADIR}/ChosenWeights.prm}

if [ ${exists file ${WEIGHTS}} ]
	load parameterfile ${WEIGHTS}
	if [ ${exists parameter SignalProcessingDescription} ]
		log classifier loaded: ${get parameter SignalProcessingDescription}
	end
else
	warn No classifier loaded - could not find ${WEIGHTS}
end

setconfig
