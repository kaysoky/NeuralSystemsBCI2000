from ctypes import *
import os,sys,os.path

import numpy
import WavTools as wav
try:
	from PrecisionTiming import prectime
except:
	print __name__,"module failed to import prectime: using less precise time.time() instead"
	import time
	def prectime(): return time.time() * 1000.0
	
class tac:
	#driverdir = os.path.abspath(os.path.join(os.path.dirname(__file__), '../quaerosys'))
	driverdir = os.path.abspath(os.path.join(os.path.dirname(__file__), '.'))

	def __init__(self, verbose=False, stimulators=2, dacs=8, bits=12, working_range=(0.195,0.928), clock_rate=2000, load=True):
		"""
		Initialize with the number of stimulators, number of DACs, bit
		depth, working range, and clock rate. Incoming signals of range
		[-1, +1] will be mapped to the range
			[(2**bits-1)*min(working_range), (2**bits-1)*max(working_range)]
		"""###
		self.verbose = verbose
		self.stimulators = stimulators
		self.dacs = dacs
		self.bits = bits
		self.working_range = working_range
		self.clock_rate = clock_rate
		self.dll = None
		if not load: return
		self.LoadDLL()

		self.initStimulator() # do this synchronously since the reset call will reset the background_queue
		self.reset()
		
		
	def __del__(self):
		self.UnloadDLL()
		
	def LoadDLL(self):
		self.UnloadDLL()
		d = cdll.LoadLibrary(os.path.join(tac.driverdir, 'stimlib0.dll'))
		funcs = (
			d.resetStimulator, d.initStimulator, d.closeStimulator,
			d.setDAC, d.setPinBlock, d.wait, d.stopStimulation,
			d.startStimulation, d.waitForTrigger, d.triggerOut,
			d.setProperty, d.getProperty,
		)
		self.dll = d
	
	def CallDLL(self, fnstr, *pargs, **kwargs):
		output = kwargs.pop('output', False)
		if len(kwargs): raise TypeError, "%s() got an unexpected keyword argument '%s'"%(sys._getframe(1).f_code.co_name,kwargs.keys()[0])
		if self.dll == None: raise WindowsError, "no dll object loaded"
		fn = getattr(self.dll, fnstr)
		res = fn(*pargs)
		if res and not output: raise WindowsError, "got result %d from call to %s" % (res, fnstr)
		return res
		
	def UnloadDLL(self):
		del(self.dll)
		self.dll = None
			
	################################################################

	def sec2ticks(self, sec):
		return int(round(float(self.clock_rate) * float(sec)))

	def msec2ticks(self, msec):
		return self.sec2ticks(float(msec)/1000.0)

	################################################################

	def resetStimulator(self):
		self.CallDLL('resetStimulator')

	def initStimulator(self, license='68105517796932851779Y5KB0A3AAG'):
		self.CallDLL('initStimulator', c_char_p(license))

	def closeStimulator(self):
		self.CallDLL('closeStimulator')

	def setDAC(self, dac, val):
		self.CallDLL('setDAC', c_uint8(dac), c_uint16(val))

	def setPinBlock(self, slot, int_trigger, dac_ind):
		if isinstance(dac_ind,float) or isinstance(dac_ind,int): dac_ind = [dac_ind] * 8
		dac_ind = list(dac_ind)
		if len(dac_ind) != 8: raise TypeError, "dac_ind should be an 8-tuple or a list of length 8"
		for i in range(len(dac_ind)): dac_ind[i] = c_uint8(dac_ind[i])
		self.CallDLL('setPinBlock', c_uint8(slot), c_uint8(int_trigger), *dac_ind)

	def wait(self, int_trigger, howlong, units='ticks'):
		if   units=='msec': howlong = self.msec2ticks(howlong)
		elif units=='sec': howlong = self.sec2ticks(howlong)
		elif units!='ticks': raise ValueError, 'units should be "ticks", "msec" or "sec"'
		self.CallDLL('wait', c_uint8(int_trigger), c_int32(int(round(howlong))))

	def stopStimulation(self):
		self.CallDLL('stopStimulation')

	def terminateSequence(self):
		"""A more intuitive synonym for stopStimulation"""###
		self.CallDLL('stopStimulation')

	def startStimulation(self):
		self.CallDLL('startStimulation')

	def waitForTrigger(self, slot=16, trigger=0):
		self.CallDLL('waitForTrigger', c_uint8(slot), c_uint8(trigger))
	
	def triggerOut(self, slot=16, trigger=1):
		self.CallDLL('triggerOut', c_uint8(slot), c_uint8(trigger))
	
	def setProperty(self, key, val):
		self.CallDLL('setProperty', c_char_p(key), c_int32(val))

	def getProperty(self, key):
		return self.CallDLL('getProperty', c_char_p(key), output=1)
		
	################################################################

	def quaerify(self, y, fs=None, rise_msec=20, fall_msec=None):
		"""
		Given either
			- a wav object y,  or
			- a numpy.array y plus a sampling frequency fs
		return (y,fs),  where y is a numpy.array appropriately scaled
		and rounded for writing to the DACs, with the requested length
		of rising and falling shoulder at the beginning and end.
		
		If, on entry, y is already a numpy.array of type uint16,
		then it is assumed to require no further processing and is
		returned unchanged. Otherwise, the samples are scaled and
		such that the range [-1.0, +1.0] is mapped to the
		working_range of the stimulator.
		"""###
		if fs == None: fs = getattr(y, 'samplingfreq_hz', None)
		if fs == None: fs = getattr(y, 'fs', None)
		if fs == None: raise TypeError, "sampling frequency not specified" 
		y = getattr(y, 'y', y)
		if isinstance(y,numpy.ndarray) and y.dtype==numpy.dtype('uint16'):
			return (y, fs)

		y = numpy.array(y, dtype='float')
		if len(y.shape)==1: y.shape=(y.shape[0], 1)
		nsamp,nchan = y.shape
		y *= (max(self.working_range)-min(self.working_range))/2.0
		y += (max(self.working_range)+min(self.working_range))/2.0
		
		if fall_msec==None: fall_msec = rise_msec
		rise_samp = round(float(fs) * float(rise_msec) / 1000.0)
		rise = 0.5 + 0.5 * numpy.sin(numpy.linspace(-numpy.pi/2.0, +numpy.pi/2.0, rise_samp))
		rise.shape = (rise_samp,1)
		fall_samp = round(float(fs) * float(fall_msec) / 1000.0)
		fall = 0.5 + 0.5 * numpy.sin(numpy.linspace(+numpy.pi/2.0, -numpy.pi/2.0, fall_samp))
		fall.shape = (fall_samp,1)
		plateau = numpy.ones((nsamp-rise_samp-fall_samp,1))
		rise_and_fall = numpy.concatenate((rise, plateau, fall), axis=0)
		y = y * rise_and_fall
		
		y = y.clip(0.0, 1.0)
		y *= 2**self.bits-1
		y = numpy.round(y)
		y = numpy.uint16(y)
		return (y,fs)

	def load(self, y, dac=None, fs=None, rise_msec=20, fall_msec=None, terminate=True):		
		(y,fs) = self.quaerify(y, rise_msec=rise_msec, fall_msec=fall_msec, fs=fs)
		wait_ticks = self.sec2ticks(1.0/float(fs))
		nsamp,nchan = y.shape
		if dac==None: dac = range(nchan)
		if numpy.isscalar(dac): dac = [int(dac)]
		if isinstance(dac,numpy.ndarray) and len(dac.shape)==0: dac = [int(dac)]
		if len(dac) != nchan: raise ValueError, "number of DAC indices should match number of channels in signal array"
		for i in range(nsamp):
			for j in range(nchan):
				self.setDAC(dac[j], int(y[i,j]))
			self.wait(1, wait_ticks)
		if terminate: self.terminateSequence()
			
	def start_timer(self):
		if self.verbose: print "\n"+repr(self),"is busy"
		self.time_started = prectime()

	def stop_timer(self):
		self.time_taken = (prectime()-self.time_started)/1000.0
		if self.verbose: print "\n"+repr(self),"has finished after",self.time_taken,"seconds"

	def append(self, func, *pargs, **kwargs):
		if not hasattr(self, 'queue'): self.queue = wav.background_queue()
		self.queue.append(func, *pargs, **kwargs)

	def reset(self):
		self.time_started = None
		self.time_taken = None
		self.append(None) # makes sure the queue exists and is running
		self.queue.reset()
		self.append(self.resetStimulator)
		for i in range(self.stimulators): self.append(self.setPinBlock, i, i, [i]*8)
		for i in range(self.dacs): self.append(self.setDAC, i, 0)
		self.append(self.terminateSequence)
		self.append(self.startStimulation)
		
	def arm(self, stim, dac=0, fs=None, delay_msec=0, rise_msec=20, fall_msec=None):
		"""
		q.arm(stim, dac)     # where stim is a wav object 
		q.arm(stim, dac, fs) # where stim is a numpy.array and fs
		                     # denotes the sampling frequency
		
		In a background thread, transfer stimulus stim to specified
		DAC, and arm the sequence to start on the next external input
		trigger signal.
		"""###
		self.append(self.start_timer)
		self.append(self.waitForTrigger)
		if delay_msec: self.append(self.wait, 1, delay_msec, units='msec')
		self.append(self.load, stim, dac=dac, fs=fs, rise_msec=rise_msec, fall_msec=fall_msec, terminate=True)
		self.append(self.startStimulation) # start it! but it immediately hits the waitForTrigger
		self.append(self.stop_timer)


####################################################################
####################################################################

from SigTools import amstim,zap

def demo(q=None, w=None, dac=0):
	if q == None: q = tac(verbose=True) # q is the tac() instance
	if w == None: w = amstim()          # w is a signal, wrapped for convenience in a wav object	
	q.arm(w, dac)
	return q,w


####################################################################
####################################################################

def low_level_demo(q=None, w=None):
	"""
	demo sequence: stimulate, wait for trigger, stimulate
	
	(q,w)=quaerosys.demo()    # create new objects
	(q,w)=quaerosys.demo(q,w) # or use pre-initialized objects	

	q is a quaerosys.tac() instance for communicating with the hardware
	w is a wav() instance containing a signal

	Once this function has returned, you can call q.startStimulation()
	to start.

	The trouble is, even with pre-initialized q and w objects,
	the DLL calls in this function are synchronous and slow: anything
	from 20 to 120 msec for q.load(), which involves hundreds/thousands
	of setDAC and wait calls, and several seconds for stopStimulation()
	to finalize the sequence.

	A solution to this is showcased in quaerosys.low_level_demo_bg()
	"""

	if q == None: q = tac() # q is the tac() instance
	if w == None: w = amstim() # w is a signal, wrapped for convenience in a wav object	
	q.load(w, dac=0, terminate=False)
	q.waitForTrigger() # input triggers half-way tested (need to solder a cable that will allow lpt-triggering)
	q.load(w, dac=1, terminate=False)
	q.triggerOut() # output triggers not yet tested
	q.terminateSequence()
	return (q, w)


def low_level_demo_bg(q=None, w=None, b=None):
	"""
	demo sequence: stimulate, wait for trigger, stimulate
	
	(q,w,b)=quaerosys.demo()    # create new objects
	(q,w,b)=quaerosys.demo(q,w,b) # or use pre-initialized objects	
	
	q is a quaerosys.tac() instance for communicating with the hardware
	w is a wav() instance containing a signal
	b is a background_queue() instance, which is a general-purpose
	  object for getting things done in a background thread

	Construction of these objects takes time, but if you pass in
	pre-initialized objects, this function should return in
	microseconds. Provided b.going is True---which it should be,
	but if not then you can call b.go()---you simply have to
	keep polling b.busy.  As soon as b.busy becomes False, you
	can call q.startStimulation(). However, that itself is synchronous
	and may take 200 msec to return. So instead you should probably
	call b.append(q.startStimulation). If the first thing in the
	sequence is a waitForTrigger, then you don't even need to wait
	for b.busy: just call b.append(q.startStimulation) immediately after
	b.append(q.terminateSequence) .
	"""
	
	if q == None: q = tac()
	if w == None: w = amstim()
	if b == None: b = wav.background_queue()
	
	# The following calls are identical to the calls in demo(), above.
	b.append(q.load, w, dac=0, terminate=False)
	b.append(q.waitForTrigger) # input triggers half-way tested (need to solder a cable that will allow lpt-triggering)
	b.append(q.load, w, dac=1, terminate=False)
	b.append(q.triggerOut) # output triggers not yet tested
	b.append(q.terminateSequence)

	return (q, w, b)


