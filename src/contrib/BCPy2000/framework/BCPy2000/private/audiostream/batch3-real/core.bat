#! ../prog/BCI2000Shell
@cls & ..\prog\BCI2000Shell %0 %* #! && exit /b 0 || exit /b 1

#TODO: if [${VAR}==val ]  always returns true:  very easy to miss this
# first time after restarting Windows, parameter file claims to be loaded but the parameter values do not reflect what was in the file
# quit operator and re-launch from same script without changing anything: all is fine.

reset system

########################################################################################

set environment BATCHDIR  ${CD}
set environment PARMSDIR  ${canonical path ${BATCHDIR}/../parms}
set environment PYWD      ${canonical path ${BATCHDIR}/../python}
set environment PYLOGDIR  ${canonical path ${BATCHDIR}/../log}
set environment LOCALPROG ${canonical path ${BATCHDIR}/../prog}

set environment SUBJECT TestSubject
if [ ${Arg1} ]; set environment SUBJECT ${Arg1}; end

set environment CONDITION 002
if [ ${Arg2} ]; set environment CONDITION ${Arg2}; end

set environment MODE CALIB
if [ ${Arg3} ]; set environment MODE ${Arg3}; end

set environment SRC gUSBampSource
#set environment SRC SignalGenerator ; warn SignalGenerator is the default #TODO: remove
if [ ${Arg4} ]; set environment SRC ${Arg4}; end

set environment MONTAGE D
if [ ${Arg5} ]; set environment MONTAGE ${Arg5}; end

########################################################################################

set environment TIMINGFLAG "--EvaluateTiming=1" # otherwise an --EvaluateTiming might hang over from a previous launch
if [ ${SRC} == SignalGenerator ]; set environment TIMINGFLAG --EvaluateTiming=0; end

change directory ${BCI2000LAUNCHDIR}
show window; set title ${Extract file base ${Arg0}}
reset system
startup system

start executable ${SRC}                 AUTOSTART 127.0.0.1 --SignalSourceIP=127.0.0.1     ${get environment TIMINGFLAG} # TODO: $TIMINGFLAG would be nicer
start executable PythonSignalProcessing AUTOSTART 127.0.0.1 --SignalProcessingIP=127.0.0.1 --PythonSigWD=${PYWD} --PythonSigClassFile=Streaming.py       --PythonSigLog=${PYLOGDIR}/###-sig.txt --PythonSigShell=1
start executable PythonApplication      AUTOSTART 127.0.0.1 --ApplicationIP=127.0.0.1      --PythonAppWD=${PYWD} --PythonAppClassFile=TrialStructure.py  --PythonAppLog=${PYLOGDIR}/###-app.txt --PythonAppShell=0

wait for connected

########################################################################################

if [ ${MONTAGE} == D ]
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
