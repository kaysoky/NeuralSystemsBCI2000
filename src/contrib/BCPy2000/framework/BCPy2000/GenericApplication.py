#   $Id$
#   
#   This file is part of the BCPy2000 framework, a Python framework for
#   implementing modules that run on top of the BCI2000 <http://bci2000.org/>
#   platform, for the purpose of realtime biosignal processing.
# 
#   Copyright (C) 2007-8  Thomas Schreiner, Jeremy Hill
#                         Christian Puzicha, Jason Farquhar
#   
#   bcpy2000@bci2000.org
#   
#   The BCPy2000 framework is free software: you can redistribute it
#   and/or modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation, either version 3 of
#   the License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
import BCPy2000.Generic as Core
from BCPy2000.Generic import *
__all__ = ['BciGenericApplication', 'SetDefaultFont'] + Core.__all__

import os
import sys
import time
import copy
import numpy
import pygame
import VisionEgg.Core
import VisionEgg.Text

import PrecisionTiming

#################################################################
#################################################################

def SetDefaultFont(name=None, size=None):
	"""
	Set the name and/or size of the font that VisionEgg uses
	by default for Text stimuli. Returns True if the named font
	can be found, False if not.
	"""###
	d = VisionEgg.Text.Text.constant_parameters_and_defaults
	if name != None:
		font = pygame.font.match_font(name)
		if font == None: return False
		d['font_name'] = (font,) + d['font_name'][1:]
	if size != None:
		d['font_size'] = (size,) + d['font_size'][1:]
	return True

#################################################################
#################################################################

class oops(Exception): pass # oops messages are directed at jez

#################################################################
#################################################################

class BciGenericApplication(Core.BciCore):
	"""
	The BciGenericApplication class is part of the BCPy2000 
	framework.  You create a subclass of it called BciApplication
	in order to specify your application module's behaviour.  The
	BCPy2000 application API is somewhat more extensive than that
	of the other modules: it allows you to schedule stimulus
	events using an automatic real-time "phase machine" which you
	implement in your Phases() and Transition() methods, update
	stimuli on a frame-by-frame basis by implementing a Frame()
	method, and respond to keyboard and mouse events by implementing
	an Event() method.

	Hook methods (which you can overshadow in your subclasses)
	have names beginning with a capital letter (Construct, Preflight,
	Initialize, Process, etc...).   API methods, which are useful
	calls that help you in writing your subclass implementation,
	are lower-case. Anything beginning with underscores should be
	avoided---you should not need to call such methods directly,
	and should certainly not overshadow them (so, for example, do
	not implement __init__ and __del__, but rather use Construct,
	Initialize and StartRun for initialization, and use StopRun,
	Halt and Destruct for cleanup).
	"""###

	#############################################################
	#### hooks called by the C++
	#############################################################

	def __init__(self):
		super(BciGenericApplication, self).__init__()
				
		fonts = ('lucida console', 'monaco', 'courier new', 'courier')
		self.monofont = (filter(None,map(pygame.font.match_font,fonts))+[None])[0]
		SetDefaultFont(self.monofont, 26)
		self.screen = 0
		self.viewport = 0
		self.frame_count = 0
		self.frame_timer = VisionEgg.Core.FrameTimer()
		self.current_presentation_phase = None
		self.estimated = Core.BciDict(lazy=True)
		self._regfs = Core.BciDict(lazy=True)
		self._block_structure = {}
		self._previous_srctime_stateval = None
		self._creation_parameters = None

		self.forget('transition')
		self.forget('packet')
		self.forget('frame')
		self.forget('trial')
		self.forget('block')
		self.forget('cycle')

		self._optimize_threads = False
		self._ve_sleep_msec = None
			# 10 msec sleep was what we used for 16.666 ms/frame to prevent VE from eating 100% CPU.
			# Can be increased if desired, but remember that you are living on the edge, depending
			# how long your Frame, Process and Transition calls take.  Set to None for auto.

		self._add_thread('phase machine', self._phase_machine)
		self._add_thread('vision egg', self._vision_egg)
		#self._add_thread('share', self._share)  # four-thread model doesn't seem to improve frame timing in practice
		self._redraw_lock = False  # doesn't seem to improve frame timing
		
	#############################################################

	def _Construct(self):
		if self._optimize_threads: PrecisionTiming.SetThreadAffinity([0])
		parameters,states = super(BciGenericApplication, self)._Construct()    # superclass
		appdesc = self.Description()
		appdesc = appdesc.replace("%", "%%")
		appdesc = appdesc.replace(" ", "%20")
		parameters += [ 
			"PythonApp        string ApplicationDescription= " + appdesc + " % a z // Identifies the stimulus presentation module",
			"PythonApp        int    ShowSignalTime=  0    0  0 1 // show a timestamp based on the number of processed samples (boolean)",
			"PythonApp:Design int    TrialsPerBlock= 20   20  1 % // number of trials in one block",
			"PythonApp:Design int    BlocksPerRun=    1   20  1 % // number of sub-blocks in one run",
		]
		states += [
			"VEStimulusTime    16 0 0 0",
			"EventOffset       10 0 0 0",
			"PresentationPhase  5 0 0 0",
			"CurrentTrial       9 0 0 0",
			"CurrentBlock       7 0 0 0",
		]
		subclass_parameters,subclass_states = self.Construct()                 # subclass	
		parameters += list(subclass_parameters)
		if isinstance(subclass_states, dict):
			for name,state in subclass_states.items():
				states.append(name + " " + str(state["bits"]) + " 0 0 0")
		else:
			states += list(subclass_states)		
		# Memorize internal variables, to delete all additional ones during restart
		if self._creation_parameters == None: self._creation_parameters = dir(self)
		
		return (parameters, states)
	
	#############################################################

	def _Halt(self):
		super(BciGenericApplication, self)._Halt()                      # superclass
		self._lock.release()
		self._lock.reset()
		self._lock.acquire('Halt')
		self.Halt()                                                     # subclass
		self._run_callbacks('Halt')
		self._lock.release('Halt')
		for threadname in ['phase machine', 'vision egg']:
			th = self._threads[threadname]
			if not th.read('ready'):
				th.post('stop')
				th.read('ready', wait=True)
		self._check_threads()
		for i in dir(self):
			if i not in self._creation_parameters: delattr(self, i)

	#############################################################

	def _Preflight(self, in_signal_props):
		super(BciGenericApplication, self)._Preflight(in_signal_props)    # superclass
		VisionEgg.config.VISIONEGG_MAX_PRIORITY = 0
		out_signal_props = self.Preflight(self.in_signal_props)           # subclass
		self._store_out_signal_props(out_signal_props)
		return self.out_signal_props
		
	#############################################################

	def _Initialize(self, in_signal_props, out_signal_props):
		super(BciGenericApplication, self)._Initialize(in_signal_props, out_signal_props)  # superclass		
		self._slave = self.states.read_only = (int(self.params['EnslavePython']) != 0)
		if self._slave:
			print
			print
			print "The application module is running in \"slave\" mode:"
			print "state variables will not be writeable from this module."
			print "NB: the application will not replay its previous behaviour"
			print "exactly unless a number of criteria are met.  See the"
			print "documentation on \"replaying\"."
			print
			
		self.forget('transition')
		self.forget('packet')
		self.forget('frame')
		self.forget('trial')
		self.forget('block')
		self.forget('cycle')

		th = self._threads['vision egg']
		ready = th.read('ready', remove=True)
		if not ready: raise oops, 'vision egg thread is not running'
		th.post('init', wait=True)		
		
	#############################################################

	def _StartRun(self):
		self.frame_count = 0
		super(BciGenericApplication, self)._StartRun()  # superclass
		self.forget('cycle')
		self._previous_srctime_stateval = None
		self.current_presentation_phase = None
		self._regress_sampling_rate(init=True)
		self._estimate_rate('SamplesPerSecond', init=2.0)
		self._estimate_rate('FramesPerSecond', init=2.0)
		self.states['CurrentTrial'] = 0
		self.states['CurrentBlock'] = 0	
		self._lock.acquire('StartRun')
		self.StartRun()                                 # subclass
		self._run_callbacks('StartRun')
		self._lock.release('StartRun')
		if not self._lock.enabled:
			print "\nWARNING: mutex disabled"
		if self._lock.record_timing:
			print "\nWARNING: mutex is recording timing information, which will eat memory"
		
		for threadname in ['phase machine']:
			th = self._threads[threadname]
			th.read('ready', wait=True, remove=True)
			th.post('go', wait=True)
		
		# playback support
		self._slave_memory = None
		if 'SignalStopRun' in self.states: self.states.__setitem__('SignalStopRun', 0, 'really')
		
	#############################################################

	def _Process(self, in_signal):
		self._lock.acquire('Process')
		if self.states['EventOffset'] and self.since('transition')['packets'] > 0:
			#self.debug('EventOffsetZeroed', val=self.states['EventOffset'])
			self.states['EventOffset'] =  0

		t = self.prectime()

		packet = self.since('run', t)['packets']
		if packet > 0:
			srctime = self._update_srctime(t)
			self._regress_sampling_rate(srctime)  # online regression over whole run so far
		if packet > 2:
			self._estimate_rate('SamplesPerSecond', t) # instantaneous (but smoothed) estimate
		self.remember('packet', t)
		fallback_signal = super(BciGenericApplication, self)._Process(in_signal)  # superclass
		# fallback_signal is set by superclass to be a copy of the input if same dims, or zeros if not
		out_signal = self.Process(self.in_signal)                                 # subclass
		self._run_callbacks('Process')
		self._store_out_signal(out_signal, fallback_signal)

		# playback support
		if self._slave:
			# Even if enslaved, only the app module can stop a run cleanly.
			# So, detect and respond to the special state 'SignalStopRun' if it exists.
			if self.states.get('SignalStopRun', 0): self.states.__setitem__('Running', 0, 'really')
			# Handle phase transitions
			pp,eo = self.states['PresentationPhase'],self.states['EventOffset']
			if self._slave_memory == None: change = False
			elif pp != self._slave_memory['pp']: change = True
			elif eo == 0: change = False
			elif eo != self._slave_memory['eo']: change = True
			else: change = False
			if change: self.change_phase(self._phasedefs['bynumber'][pp]['name'])
			self._slave_memory = {'pp':pp, 'eo':eo}

		self._lock.release('Process')
		return self.out_signal
		
	#############################################################

	def _StopRun(self):
		super(BciGenericApplication, self)._StopRun()    # superclass
		self._lock.acquire('StopRun')
		self.StopRun()                                   # subclass
		self._run_callbacks('StopRun')
		self._lock.release('StopRun')

	#############################################################

	def _Resting(self):
		super(BciGenericApplication, self)._Resting()    # superclass
		self._lock.acquire('Resting')
		self.Resting()                                   # subclass
		self._lock.release('Resting')

	#############################################################

	def _Destruct(self):
		super(BciGenericApplication, self)._Destruct()    # superclass
		self.Destruct()                                   # subclass

	#############################################################
	#### low-level helper methods mostly used by the superclass
	#############################################################

	def _update_srctime(self, t=None):
		if t == None: t = self.prectime()
		stateval = self.states['SourceTime']
		statebits = self.bits['SourceTime']
		previous_stateval = getattr(self, '_previous_srctime_stateval', None)
		if previous_stateval == None:
			tt = uintwrap(t + self._prectime_zero, statebits)
			# This recreates what the source module should be doing to make the
			# SourceTime stamp. So, if src and app modules are on the same machine,
			# tt should now be comparable to the SourceTime state, and should be
			# only a small number of msec ahead of it (but possibly wrapped-around) 
			SourceTimeToNow = float(Core.unwrapdiff(stateval, tt, statebits))
			if SourceTimeToNow < 0.0 or SourceTimeToNow/1000.0 > 0.75 * self.nominal['SecondsPerPacket']:
				SourceTimeToNow = 0.0
			# If SourceTimeToNow is negative or too large, we conclude that the two modules
			# are not using the same clock chip. So let's pretend that there's no
			# offset (if we SourceTimeToNow is too small then we might compute our event
			# times a bit early as a result, but that's better than too late).
			srctime = t - SourceTimeToNow - self.nominal['SecondsPerPacket']*1000.0
			# Having unwrapped the difference between the SourceTime and now, re-apply
			# that difference to now to give a floating-point non-wrapped SourceTime
			# that is comparable to future calls to self.prectime()
		else:
			elapsed = Core.unwrapdiff(previous_stateval, stateval, statebits)
			srctime = self.last['cycle']['msec'] + float(elapsed)
		self.remember('cycle', timestamp=srctime)
		self._previous_srctime_stateval = stateval
		return srctime
		
	#############################################################

	def _trial_update(self):
		start = self._block_structure.get('start')
		if self.current_presentation_phase == None: self.change_phase(start)
		if self.current_presentation_phase == None: self.change_phase('idle')
		
		inc = self._block_structure.get('new_trial')
		if inc != self.current_presentation_phase: return 

		self.states['CurrentTrial'] += 1
		if self.states['CurrentBlock'] == 0: self.states['CurrentBlock'] = 1
		
		
		if self.states['CurrentTrial'] > int(self.params['TrialsPerBlock']):
			self.states['CurrentTrial'] = 0
			self.states['CurrentBlock'] += 1
			interblock = self._block_structure.get('interblock')
			if interblock != None: self.change_phase(interblock)

		if self.states['CurrentBlock'] > int(self.params['BlocksPerRun']):
			self.states['CurrentTrial'] = 0
			self.states['CurrentBlock'] = 0
			endphase = self._block_structure.get('end')
			if endphase == None: # by default, automatically press suspend after the last block of the run
				self.states['Running'] = 0
			else:                # ...unless the 'end' key has been defined, in which case pop into that phase
				self.change_phase(endphase)
		
		if self.states['CurrentTrial'] >  0: self.remember('trial')
		if self.states['CurrentTrial'] == 1: self.remember('block')

	#############################################################

	def _estimate_rate(self, what, t=None, init=0.0):
		if t==None: t = self.prectime()
		if what == 'FramesPerSecond':
			event_type = 'frame'
			batch = 1
		elif what == 'SamplesPerSecond':
			event_type = 'packet'
			batch = self.nominal['SamplesPerPacket']
		else:
			raise oops,what
			
		if not hasattr(self, 'estimated'): self.estimated = Core.BciDict(lazy=True)
		if init or not self.estimated.has_key(what): self.estimated[what] = Core.BciDict(lazy=True)
		d = self.estimated[what]
		if len(d) == 0:
			if init==0.0: init = 4.0
			nominal_rate = self.nominal[what]
			nominal_delta = 1.0 / nominal_rate
			batch_rate = nominal_rate / float(batch)
			d['global'] = nominal_rate
			d['running'] = nominal_rate
			d['buffer'] = numpy.array([nominal_delta] * int(round(float(init) * batch_rate)))
			d['n'] = int(round(batch_rate)) # assume ~ 1 sec worth of prior knowledge
		if init: return
		elapsed = self.since(event_type, t)
		delta = elapsed['msec']
		if delta == None or delta < 0.0: return
		if elapsed[event_type + 's'] != 1: return
		delta /= (1000.0 * batch)
		i = d['n'] % len(d['buffer'])
		oldval,d['buffer'][i] = d['buffer'][i], delta
		d['running'] = 1.0/(1.0/d['running'] + (delta-oldval)/float(len(d['buffer'])))
		persistence = float(d['n']) / float(d['n'] + 1)
		d['global'] = 1.0/(persistence * 1.0/d['global'] + (1.0 - persistence) * delta)
		d['n'] += 1
		
	#############################################################

	def _regress_sampling_rate(self, t=None, init=False):
		if t==None: t = self.prectime()
		if init or not hasattr(self, '_regfs'): self._regfs = Core.BciDict(lazy=True)
		d = self._regfs
		if init or len(d) == 0:
			d['Packets'] = None
			d['SamplesPerPacket'] = self.nominal['SamplesPerPacket']
			d['SamplesPerSecond'] = self.nominal['SamplesPerSecond']
			d['PacketStartSamples'] = 0.0
			d['PacketStartSeconds'] = 0.0
			d['OffsetSamples'] = 0.0
			d['FakePackets'] = round(0.5 * d['SamplesPerSecond'] / d['SamplesPerPacket'])
		if init: return
		if d['Packets'] == None: d['Packets'] = 0
		y = d['Packets'] * d['SamplesPerPacket']  # y is measured in samples, 
		x = float(t) / 1000.0                     # x is measured seconds,
		n = d['Packets'] + 1.0                    # so in  y = a * x + b,   a is SamplesPerSecond, and b is Samples
		a = d['SamplesPerSecond']
		b = d['OffsetSamples']
		if n < 2.0:
			y = numpy.array([0.0, 1.0]) * d['SamplesPerPacket']
			x = (y-b)/a
			ym = numpy.mean(y)
			xm = numpy.mean(x)
			x -= xm
			p = numpy.inner(y,x)/(len(x)**3.0)
			q = numpy.inner(x,x)/(len(x)**3.0)
		else:
			n += d['FakePackets']
			oldxm = d['xm']
			oldym = d['ym']
			xm = oldxm + (x - oldxm)/n
			ym = oldym + (y - oldym)/n
			f = (n-1.0)/n
			incp = y*(x-xm) + oldym*(oldxm-x)*f
			incq = xm*oldxm - x*xm*(1+1/f) + x*x/f
			p = d['p'] * f**3.0 + incp/n**3.0
			q = d['q'] * f**3.0 + incq/n**3.0
			a = p/q
			b = ym - a*xm
			
		y = d['Packets']*d['SamplesPerPacket']
		x = (y-b)/a
		d.update({'SamplesPerSecond':a, 'OffsetSamples':b, 'PacketStartSamples':y, 'PacketStartSeconds':x, 'p':p, 'q':q, 'xm':xm, 'ym':ym})
		d['Packets'] += 1.0

	#############################################################

	def event_offset(self, timestamp=None, state=None):
		if timestamp == None: timestamp = self.prectime()
		d = self._regfs
		self.states["VEStimulusTime"] = uintwrap(timestamp, self.bits["VEStimulusTime"])
		EventTimeSeconds = timestamp/1000.0
		SamplesSinceStart = d['OffsetSamples'] + d['SamplesPerSecond'] * timestamp / 1000.0
		EventOffset = int(round(SamplesSinceStart - d['PacketStartSamples']))
		
		#EventOffset = self.samples_since_packet(timestamp=timestamp)
		# This would be a safer and *much* simpler method than all the regression-based stuff above
		# However, it measured up with +/- 5 msec in an 68-channel RDA test, whereas the regression
		# method yielded +/-1.4.  The regression is only unsafe when late packets occur, which real EEG
		# source modules shouldn't allow
		
		signedval = EventOffset
		
		if state != None:
			# put the signed value into an unsigned state variable in a slightly unusual way
			bits = self.bits[state]
			maxabsval = 2**(bits-1)-1 # for example, 127 for 8 bits
			if EventOffset < -maxabsval or EventOffset > maxabsval:  # reserve -128 (will become 0, below)
				r,firsttime = self.debug('BadEventOffsets', val=EventOffset, statename=state, bits=bits)
				if firsttime: print "WARNING: %s out of range"%state
				EventOffset = maxabsval * (EventOffset/abs(EventOffset))
			val = 1 + maxabsval + EventOffset # add 1, because 0 is reserved for "no event"
			self.states[state] = val
			
		return signedval

	#############################################################

	def add_callback(self, hookname, func, pargs=(), kwargs={}):
		class CallbackRegErr(Exception): pass
		if not hasattr(self, '_callbacks'): self._callbacks = {'StartRun':[], 'Process':[], 'Frame':[], 'StopRun':[], 'Halt':[]}
		if not self._callbacks.has_key(hookname): raise CallbackRegErr, 'cannot register callbacks for %s, only for %s'%(hookname,str(self._callbacks.keys()))
		c = {'func':func,'pargs':pargs,'kwargs':kwargs}
		if not c in self._callbacks[hookname]: self._callbacks[hookname].append(c)
		
	#############################################################

	def _run_callbacks(self, hookname):
		if not hasattr(self, '_callbacks'): return
		if not self._callbacks.has_key(hookname): return
		for c in self._callbacks[hookname]: c['func'](*c['pargs'],**c['kwargs'])
			
	#############################################################

	def _initfocus(self):
		try:
			import ctypes  # !! Windows-specific code.
			dll = ctypes.windll.user32
			self._focus = {'func': (dll.SetForegroundWindow, dll.SetActiveWindow),
			               'arg': ctypes.c_voidp(dll.GetForegroundWindow()),}
			self.add_callback('StartRun', self.focus)
		except:
			self._focus = None

	#############################################################

	def focus(self):
		f = getattr(self,'_focus')
		if f != None:
			for func in f['func']: func(f['arg'])

	#############################################################
	#### main thread controllers
	#############################################################
	
	def _phase_machine(self, mythread):
		try:
			class PhaseChangeErr(Exception): pass
			if self._optimize_threads: PrecisionTiming.SetThreadAffinity([0])
			mythread.read('stop', remove=True)
			mythread.post('ready')
			mythread.read('go', wait=True, remove=True)
			while not self.states['Running']: time.sleep(0.001)
			previous_phase = None
			self.current_presentation_phase = None
			self.phase(name='idle', duration=None, next='idle', id=0)
			while self.states['Running'] and not mythread.read('stop'):
				self._lock.acquire('Transition')
				self.Phases()
				self._trial_update()
				if not self.states['Running']: break
					
				if not isinstance(self.current_presentation_phase, str): raise PhaseChangeErr, 'phase names must be strings'
				rec = self._phasedefs['byname'].get(self.current_presentation_phase)
				if rec == None: raise PhaseChangeErr, 'unrecognized phase "'+self.current_presentation_phase+'"'
				
				t = self.prectime()
				if previous_phase != None:
					self.event_offset(state="EventOffset", timestamp=t)
				self.states['PresentationPhase'] = rec['id']
				elapsed = self.since('transition', timestamp=t)
				self.remember('transition', timestamp=t)
				#self.debug('transition', from_phase=previous_phase, to_phase=self.current_presentation_phase, after=elapsed['msec'], pp=self.states['PresentationPhase'], eo=self.states['EventOffset'])
				self.Transition(self.current_presentation_phase)
				if elapsed['packets'] == 0 and previous_phase != None:
					evt = 'MultipleTransitions'
					dbrec,firsttime=self.debug(evt, from_phase=previous_phase, to_phase=self.current_presentation_phase, after_msec=elapsed['msec'])
					if firsttime: sys.stderr.write("WARNING: multiple phase transitions per packet\n")
				self._lock.release('Transition')
				
				previous_phase = rec['name']
				duration = rec['duration']
				next = rec['next']
				self._phase_must_change = False
				while not self._phase_must_change:
					if not self.states['Running'] or mythread.read('stop'): break
					if duration != None and next != None and not self._slave:
						elapsed = self.since('transition')
						if elapsed['msec'] >= duration: self.change_phase(next)
					if not self._phase_must_change:
						time.sleep(0.001)		
		except:
			mythread.fail()
		self._lock.release('Transition')
			
	#############################################################

	def _vision_egg(self, mythread):
					
		try:
			mythread.read('init', remove=True)
			mythread.read('stop', remove=True)
			mythread.post('ready')
			mythread.read('init', wait=True) # removed when initialization is complete
			
			import logging; logging.raiseExceptions = 0 # suppresses the "No handlers could be found" chatter

			self.screen = VisionEgg.Core.get_default_screen()
			self._initfocus() # TODO: seems to raise the correct window on some machines/experiments, and the wrong one (the operator) on others
			self.stimuli = Core.BciDict(lazy=True)
			self._stimlist = []
			self._stimz = []
			self._stimq = () # non-list object is a sign to stimulus() that stimuli can be processed immediately, since we are in the right thread
			self.Initialize(self.in_signal_dim, self.out_signal_dim)    # subclass
			self._stimq = [] # list object is a sign that stimuli must be queued
			self._hud_setup()
			self.viewport = VisionEgg.Core.Viewport (screen=self.screen, stimuli=self._stimlist)
			self.frame_count = 0
			self.nominal['FramesPerSecond'] = float(self.screen.query_refresh_rate())
			self.nominal['SecondsPerFrame'] = 1.0 / self.nominal['FramesPerSecond']
			self._estimate_rate('FramesPerSecond', init=2.0) # will be re-initialized at StartRun
			
			if self._optimize_threads: PrecisionTiming.SetThreadAffinity([1])
			if self._optimize_threads: PrecisionTiming.SetThreadPriority(3)

			mythread.read('init', remove=True)
			redraw_time = None
						
			while not mythread.read('stop'):
				self.ftdb('newframe') # first column of frame timing log is filled in now
				events = pygame.event.get()
				self.ftdb()
				if self.states['Running']:
					self._lock.acquire('Frame')
					for event in events: self.Event(self.current_presentation_phase, event)
					self.Frame(self.current_presentation_phase)
					self._run_callbacks('Frame')
					self._lock.release('Frame')
				self.ftdb()
				
				sleeptime = self._ve_sleep_msec
				safety_margin = 3.0
				if sleeptime == None:
					if redraw_time == None:
						sleeptime = 1
					else:
						elapsed = self.since('frame')['msec']
						allowed = self.nominal['SecondsPerFrame'] * 1000.0
						sleeptime = int(allowed - elapsed - redraw_time - safety_margin)
						sleeptime = max(0, min(int(allowed - safety_margin), sleeptime))
						self._sleeptime = (elapsed, redraw_time, sleeptime)
				self.ftdb()
				time.sleep(float(sleeptime)/1000.0)
				self.ftdb()
				if self._redraw_lock: self._lock.acquire('Frame')
				self.ftdb()
				self.screen.clear()  # at least on some graphics cards, THIS is what blocks (and takes full CPU) until the interrupt
				self.ftdb()
				t = self.prectime()
				self._update_stimlist()
				self.ftdb()
				self.viewport.draw()
				self.ftdb()
				redraw_time = self.prectime() - t
				VisionEgg.Core.swap_buffers()
				self.ftdb()
				self.frame_timer.tick()
				t = self.prectime()
				if self.frame_count > 0: self._estimate_rate('FramesPerSecond', t)
				self.remember('frame', t)
				self.frame_count += 1
				if self._redraw_lock: self._lock.release('Frame')
		except:
			einfo = mythread.fail()
			self._lock.release('Frame')
			if not isinstance(einfo[1], EndUserError):
				while mythread.exception != None and not mythread.read('stop'):
					time.sleep(0.001)  # cannot use mythread.read to wait for 'stop' in the normal way until the exception is cleared
				mythread.read('stop', wait=True) # because waits normally fall through if the thread has posted an exception
		self._lock.release('Frame')	
		self.screen.close()
		self.screen = 0
		self.viewport = 0
		if hasattr(self, 'stimuli'): delattr(self, 'stimuli')
		if hasattr(self, '_stimlist'): delattr(self, '_stimlist')
		if hasattr(self, '_stimz'): delattr(self, '_stimz')
		if hasattr(self, '_stimq'): delattr(self, '_stimq')
		VisionEgg.Text._font_objects = {}
		# VisionEgg 1.1 allowed these cached pygame.font.Font objects to persist even
		# after pygame quits or is reloaded: this causes a crash the second time around.
		# VisionEgg 1.0 didn't cache, so we never ran across the problem under Python 2.4.
		# Andrew fixed it in VE 1.1.1.
		pygame.quit()
	
	#############################################################

	def ftdb(self, subcmd=None, *pargs):
		if subcmd == 'setup': # create an array to hold the timings (one row per frame)
			(nframes,ntimings) = pargs
			self._ftlog = {'t':numpy.zeros((nframes,ntimings),dtype=numpy.float64), 'i':0, 'j':0, 'started':False, 'rows_used':0, 'cols_used':0}
			return
		ftlog = getattr(self, '_ftlog', None)
		if ftlog == None: return
		t = self.prectime()
		if subcmd == 'start':
			ftlog['i'] = 0
			ftlog['j'] = 0
			ftlog['rows_used'] = 0
			ftlog['cols_used'] = 0
			ftlog['started'] = True
			return
		if subcmd == 'stop':
			ftlog['started'] = False
			return
		if subcmd == 'save':
			(filename,) = pargs
			m = ftlog['t'][:ftlog['rows_used'],:ftlog['cols_used']]
			f = open(filename, 'wt')
			f.write('[\n')
			for i in xrange(m.shape[0]):
				f.write('  [')
				for j in xrange(m.shape[1]):
					f.write('%8.3f, '%m[i,j])
				f.write('  ],\n')
			f.write(']\n')
			f.close()
			return
	
		if not ftlog['started']: return
		if subcmd == 'newframe':
			if ftlog['j']: ftlog['i'] += 1
			ftlog['j'] = 0
			ftlog['rows_used'] = max([ftlog['rows_used'], ftlog['i']])
		elif subcmd != None:
			raise oops,'unknown subcommand'
		
		m,i,j = ftlog['t'],ftlog['i'],ftlog['j']
		if i < m.shape[0] and j < m.shape[1]: m[i,j] = t
		ftlog['j'] += 1
		ftlog['cols_used'] = max([ftlog['cols_used'], ftlog['j']])
			
	#############################################################
	#### extra visual widgets
	#############################################################

	def _hud_setup(self):
		if int(self.params['ShowSignalTime']):
			t = self.params.get('PlaybackStart','0')
			t = t.split(':'); t.reverse()
			if len(t) > 3: t = [0]
			t = map(float,t) + [0]*(3-len(t))
			stim1 = self.stimulus('_signalclock1', VisionEgg.Text.Text(text=' ', color=(1,1,1),on=False, position=(self.screen.size[0]-5,5), anchor='lowerright',font_name=self.monofont,font_size=20), z=100)
			stim2 = self.stimulus('_signalclock2', VisionEgg.Text.Text(text=' ', color=(1,1,1),on=False, position=(self.screen.size[0]-5,25), anchor='lowerright',font_name=self.monofont,font_size=20), z=100)
			self._signalclock = {
				'stim1'  : stim1.parameters,
				'stim2'  : stim2.parameters,
				'offset': 1000.0 * (t[0] + 60.0 * t[1] + 3600.0 * t[2]),
				'mspp'  : 1000.0 * float(self.params['SampleBlockSize']) / self.samplingrate()
			}
			self.add_callback('Process', self._signalclock_update) # by definition only updates per packet
		# playback support
		if len(self.params.get('PlaybackFileName','')):
			self._replay = self.stimulus('_replay', VisionEgg.Text.Text(text='REPLAY',color=(1,1,1),on=False,font_size=35,position=(5,5),font_name=self.monofont), z=100)
			self.forget('_replay')
			self.add_callback('Frame', self._replay_update) # keep flashing even while paused
			self.add_callback('StartRun', self.forget, ('_replay',))
		
	#############################################################

	def _signalclock_update(self):
		packets = self.since('run')['packets']
		msecs = round(packets * self._signalclock['mspp'] + self._signalclock['offset'])
		secs,msecs  = divmod(int(msecs), 1000)
		mins,secs   = divmod(int(secs),  60)
		hours,mins  = divmod(int(mins),  60)
		self._signalclock['stim1'].text = '%02d:%02d:%02d.%03d' % (hours,mins,secs,msecs)
		self._signalclock['stim1'].on = True
		self._signalclock['stim2'].text = 'packet %05d' % (packets)
		self._signalclock['stim2'].on = True

	#############################################################

	def _replay_update(self):
		if self.since('_replay')['msec'] < 500: return
		self.remember('_replay')
		if self.since('packet')['msec'] > 15.0 * self.nominal['SecondsPerPacket']*1000.0: self._replay.parameters.text = 'PAUSED'
		else: self._replay.parameters.text = 'REPLAY'
		self._replay.parameters.on = not self._replay.parameters.on
		

	#############################################################
	#### useful callbacks for the developer
	#############################################################

	def forget(self, event_type):
		"""
		Sets the counter for the specified event_type such that the
		object's "remembers" its last occurrence as having occurred at
		time 0, packet 0, frame 0. See self.remember() and self.since().
		"""###
		super(BciGenericApplication, self).forget(event_type=event_type)
		self.last[event_type]['frame'] = 0

	#############################################################

	def remember(self, event_type, timestamp=None):
		"""
		event_type is a string describing something whose time of
		occurrence you wish to remember.

		The application framework automatically remembers events of
		type 'frame', 'packet', 'transition', 'trial', 'block' and
		'run'. To find out how long in milliseconds it was since the
		last packet, use
		
			time_since_last_packet = self.since('packet')['msec']
		
		With an explicit call to self.remember(), you can do the
		same trick with arbitrary events. Events are timestamped
		in 'msec', 'packets' and 'frames'.
		"""###
		super(BciGenericApplication, self).remember(event_type=event_type, timestamp=timestamp)
		self.last[event_type]['frame'] = self.frame_count

	#############################################################

	def since(self, event_type, timestamp=None):
		"""
		event_type is a string that you have previously remembered
		with self.remember(event_type), or at least initialized with
		self.forget(event_type).  self.since(event_type) returns a
		dict containing the number of elapsed milliseconds, the
		number of elapsed signal packets, and (for the application
		module) the number of elapsed frames, since the specified
		event_type was last remembered.
		
		The application framework automatically remembers events of
		type 'frame', 'packet', 'transition', 'trial', 'block' and
		'run'. To find out how long in milliseconds it was since the
		last packet, use
		
			time_since_last_packet = self.since('packet')['msec']
		
		Use explicit calls to self.remember(), to do the same trick
		with arbitrary events. Events are timestamped in 'msec',
		'packets' and 'frames'.
		"""###
		d = super(BciGenericApplication, self).since(event_type=event_type, timestamp=timestamp)
		rec = self.last.get(event_type)
		if rec == None:
			d['frames'] = None
		else:
			d['frames'] = self.frame_count - rec['frame']
		return d

	#############################################################

	def samples_since_packet(self, timestamp=None, fs=None):
		if timestamp == None: timestamp = self.prectime()
		if fs == None: fs = self.nominal['SamplesPerSecond']
		msec = self.since('cycle', timestamp)['msec']
		if msec == None: return None
		samples = int(round(float(fs) * float(msec)/1000.0))
		return samples

	#############################################################

	def debug(self, ref, **kwargs):
		"""
		ref is the name of a kind of occurrence which you wish
		to record (for example "frame skips").  When you call
		self.debug, the dict entry self.db[ref] is initialized
		to an empty list if it didn't already exist. Then a
		record is appended to this list.  The contents of this
		record are partly automatically generated: a timestamp
		in milliseconds, a timestamp in terms of packets, and
		(for the application module) a timestamp in terms of
		video frames. The rest of the contents can optionally
		be specified by you, using additional named keyword
		arguments. For example:

		t = self.prectime()
		if t > deadline:
			self.debug("frame skips",  lateness=t-deadline)

		Play with this at the shell prompt in order to get a feel
		for how it works: set an experiment running, then issue
		a few self.debug() calls manually.  Then examine self.db
		"""###
		r,firsttime = super(BciGenericApplication, self).debug(ref, **kwargs)
		r.update({'frame':self.frame_count})
		return r,firsttime
		
	#############################################################

	def animate(self, list, mode=None, countername='transition'):
		"""
		Given a list, this API function will return one element of
		that list, depending on how many video frames have elapsed
		since the last occurrence of whatever event countername
		refers to ('transition' by default).

		If going past the end of the list, loop the animation if
		mode is "loop"; otherwise return None or the element passed
		as "mode", if any.		
		
		For example, in your Frame() hook, you could animate the
		colour of a stimulus s, flickering it from black to white
		on alternate frames, as follows
		
		def Frame(self, phasename):
			s = self.stimuli['some_stimulus']
			s.parameters.color = self.animate([(0,0,0), (1,1,1)], mode='loop')
		
		"""###
		fc =  self.frame_count
		if countername != None: fc -= self.last[countername]['frame']
		if mode == 'loop': fc %= len(list)
		if fc >= len(list): return mode
		return list[fc]

	#############################################################

	def set_ve_window_pos(self, pos):
		"""
		This is an API method for setting the (x,y) screen offset, in
		pixels, of the upper-left corner of the VisionEgg window before
		it opens. See also self.get_ve_window_pos() and
		self.shift_ve_window_pos()
		"""###
		if pos==None: return
		if not isinstance(pos, str): pos = ','.join(map(lambda x:str(int(round(x))),pos))
		os.environ['SDL_VIDEO_WINDOW_POS'] = pos

	#############################################################

	def get_ve_window_pos(self):
		"""
		This is an API method for getting the screen offset, in pixels,
		of the setting for the upper-left corner of the VisionEgg window.
		A tuple (x,y) is returned. See also self.set_ve_window_pos() and
		self.shift_ve_window_pos().
		"""###
		pos = os.environ.get('SDL_VIDEO_WINDOW_POS', '')
		if len(pos) == 0: pos = "0,0"
		return tuple(map(float, pos.split(",")))
		
	#############################################################

	def shift_ve_window_pos(self, xy=None, x=None, y=None):
		"""
		This is an API method with which you can alter the screen offset
		of the upper-left corner of the VisionEgg window before it opens.
		Supply an (x,y) tuple of pixel offsets as argument xy, or
		alternatively supply the x and y offsets as separate named
		arguments. See also self.set_ve_window_pos() and
		self.get_ve_window_pos().
		"""###
		pos = list(self.get_ve_window_pos())
		if xy != None: x,y=xy
		if x != None: pos[0] += x
		if y != None: pos[1] += y
		self.set_ve_window_pos(pos)
			
	#############################################################

	def stimulus(self, name, stim, z=0, **kwargs):
		"""
		This is an API method for registering VisionEgg objects as stimuli to
		be rendered automatically.
		
		name is a string identifying the stimulus object.
		
		stim is a VisionEgg stimulus object (such as a VisionEgg.Text.Text
		instance).
		
		z is the optional depth parameter, determining the order in which
		stimuli are drawn.
		
		Call this function (and any higher-level AppTools functions that make
		use of it) from within your Initialize() hook. Once Initialize has
		returned, the stimulus objects are available in the dictionary
		self.stimuli, and in the depth-ordered list self._stimlist. However,
		nothing stops you from storing them additional ways, as other custom
		attributes of self (for example, you could store cue stimuli in a list
		indexed by target class, or whatever can be accessed conveniently).
		
		Each registered stimulus is drawn and updated automatically, so you
		only need to play with its .parameters attribute to change its
		appearance during run-time.
		"""###
		class StimDefErr(Exception): pass
		if not hasattr(self, 'stimuli'): self.stimuli = Core.BciDict(lazy=True)
		if not hasattr(self, '_stimlist'): self._stimlist = []
		if not hasattr(self, '_stimz'): self._stimz = []
		if not hasattr(self, '_stimq'): self._stimq = []
		self._stimz = self._stimz[:len(self._stimlist)] + [0.0] * (len(self._stimlist) - len(self._stimz))
		if self.stimuli.has_key(name): raise StimDefErr, 'duplicate stimulus name "'+name+'"'
		
		s = BciStimulus(self, name, z)
		if callable(stim):
			maker = stim
			if not isinstance(maker, Core.BciFunc):
				maker = Core.BciFunc(maker)
			maker.kwargs.update(kwargs)
			s._maker = maker
		elif isinstance(stim, VisionEgg.Core.Stimulus):
			if len(kwargs): raise TypeError, "extra keyword arguments are only expected when stim is callable"
			s.Stimulus = stim
		else:
			raise TypeError, "stim should be a VisionEgg stimulus instance, or a function for making one"
		s.enter()
		return s

	#############################################################

	def _update_stimlist(self, s=None):
		if s==None:
			while len(self._stimq):
				self._update_stimlist(self._stimq.pop(0))
		else:
			vestim = s.Stimulus
			if vestim == None:
				vestim = s.Stimulus = s._maker()
			if vestim in self._stimlist:
				ind = self._stimlist.index(vestim)
				self._stimlist.pop(ind)
				self._stimz.pop(ind)
			newz = s.z
			ind = [ z > newz for z in self._stimz+[newz+1] ].index(True)
			self._stimz.insert(ind, newz)
			self._stimlist.insert(ind, s.Stimulus)
			
	#############################################################

	def phase(self, name, duration=None, next='idle', id=None):
		"""
		This is an API method for defining or redefining a presentation
		phase. This call should occur within the Phases hook, which is
		called at transition time.
		
		name is the string name which will get passed to the Frame,
		Transition and Event hooks in order to let your code know what must
		happen at each phase of an experimental trial. next is the name of
		the subsequent phase, to which the phase machine will change once a
		number of milliseconds specified by duration have elapsed. If
		duration is None, then the phase never times out and would have to
		be changed manually with a call to self.change_phase().
		
		Once a phase is defined, it can be updated to change its 'next' and
		'duration' entries by a subsequent call. However, the numerical id
		associated with a given name cannot be changed after it is first
		defined.  id is the actual value of the BCI2000 state variable
		PresentationPhase that indicates that that phase is currently
		occurring. If you do not supply explicit ids, they are generated
		automatically. Id 0 is already reserved for the builtin state 'idle'.
		
		Leaving the ids to be generated automatically is not as foolish as it
		sounds: really, a well-designed experiment will *not* rely on these
		arbitrary numerical values to know what is going on when. The
		PresentationPhase state variable should be treated as an 'internal'.
		The best approach is to use your Transition hook to react to the
		string value of the phase name, and set a number of more transparent
		state variables accordingly. For example:
		
		# define some 1-bit states and one 3-bit state
		def Construct(self):
			return [],[
				"Baseline    1 0 0 0",
				"Cue         1 0 0 0",
				"Imagination 1 0 0 0",
				"TargetClass 3 0 0 0",
				# ...
			]
			
		# define the phase machine
		def Phases(self):
			self.phase('baseline',   duration=3000, next='startcue')  
			self.phase('startcue',   duration=1000, next='imagination')
			self.phase('imagination',duration=5000, next='stopcue')
			# ...
			
			self.design(start='baseline',  new_trial='startcue')
		
		
		def Transition(self, phase):  # interpret the phases accordingly
			self.states['Baseline']    = int(phase == 'baseline')
			self.states['Cue']         = int(phase == 'startcue')
			self.states['Imagination'] = int(phase == 'imagination')

			if phase=='cue':
				self.states['TargetClass'] = randint(1,8)
				self.target = self.states['TargetClass']
				self.my_cue_stimuli[self.target-1].parameters.on = True
				
			if phase=='imagination':
				self.my_cue_stimuli[self.target-1].parameters.on = False

			# ...
			# (NB: my_cue_stimuli will presumably have been initialized by
			# you in self.Initialize)
		"""###
		class PhaseDefErr(Exception): pass
		if not hasattr(self, '_phasedefs'): self._phasedefs = None
		if self._phasedefs == None: self._phasedefs = {'byname':{},  'bynumber':{}}
		byname = self._phasedefs['byname']
		bynumber = self._phasedefs['bynumber']
		if len(byname.items()) == 0:
			rec = {'name':name, 'id':0, 'duration':None, 'next':'idle'}
			byname[rec['name']] = rec
			bynumber[rec['id']] = rec
		if not isinstance(name, str): raise PhaseDefErr, 'phase names must be strings'
		if not isinstance(next, (str,type(None))): raise PhaseDefErr, 'phase names must be strings'
		if not isinstance(duration, (float,int,type(None))): raise PhaseDefErr, 'duration (in msec) must be a float or an int'
		if byname.has_key(name):
			rec = byname[name]
			if id != None and id != rec['id']: raise PhaseDefErr, 'cannot change presentation phase id for phase "'+name+'"'
			rec.update({'duration':duration, 'next':next})
		else:
			if id == None:
				id = 0
				while bynumber.has_key(id): id += 1
			if not isinstance(id, int): raise PhaseDefErr, 'id must be an integer'
			if id < 0: raise PhaseDefErr, 'cannot use a negative phase id'
			if id >= 2**self.bits['PresentationPhase']: raise PhaseDefErr, 'too many phases! maximum is %d'%(2**self.bits['PresentationPhase']-1)
			if bynumber.has_key(id): raise PhaseDefErr, 'cannot use id '+str(id)+' for phase "'+name+'" because it is already in use for phase "'+bynumber[id]['name']+'"'
			rec = {'name':name, 'id':id, 'duration':duration, 'next':next}
			byname[rec['name']] = rec
			bynumber[rec['id']] = rec
	
	#############################################################

	def design(self, start=None, new_trial=None, interblock=None, end=None):
		"""
		This is another API call that should be made in your self.Phases
		hook, before or after multiple calls to self.phase().  It specifies
		how "trials", and the odd Farquharian concept of "sub-blocks", should
		be defined.
		
		start specifies the name of the phase that each run should start with.
		If you don't specify that, and don't explicitly call self.change_phase(),
		then you will be stuck in the 'idle' phase forever.
		
		new_trial specifies the name of the phase upon transition to which
		the builtin state CurrentTrial will be incremented.  When the value
		of CurrentTrial tries to exceed the value of the builtin parameter
		TrialsPerBlock, the block is over. If, as is usual, BlocksPerRun
		is set to 1, then that also means that the run is over. 
		
		end specifies the phase that the phase machine will drop into when
		the run is over. If you have left it undefined, then the Running state
		will simply be set to 0 after the last trial and the recording will be
		ended.
		
		What happens when you have multiple BlocksPerRun?  Then, between
		blocks, the phase machine falls into the phase that you have dictated
		to be the 'interblock' phase.  How one gets from interblock back to
		start, to begin a new block of trials within the same run file, is
		then up to you: perhaps you could use the Event() hook to react to a
		keypress and call self.change_phase(), or you could use the Process()
		hook to monitor a state variable that you have associated with one of
		the customizable buttons on the BCI2000 operator.
		"""###
		self._block_structure.update({'start':start, 'new_trial':new_trial, 'interblock':interblock, 'end':end})
		
	#############################################################

	def in_phase(self, phasename, min_packets=1):
		"""
		This API method queries whether we are in the presentation phase
		denoted by <phasename> and have been so for at least <min_packets>
		packets.
		"""###
		if self.states['PresentationPhase'] != getattr(self,'_phasedefs',{}).get('byname',{}).get(phasename, {}).get('id'): return False
		return self.since('transition')['packets'] >= min_packets
		
	#############################################################

	def change_phase(self, phasename=None):
		"""
		This is an API method which immediately, manually changes the
		presentation phase. The target phase is specified by name.
		"""###
		if phasename==None:
			rec = self._phasedefs['byname'].get(self.current_presentation_phase)
			if rec != None: phasename = rec.get('next')
		if phasename != None:
			self.current_presentation_phase = phasename
		self._phase_must_change = True
			
	#############################################################
	#### application-specific hooks (or hooks with application-
	#### specific documentation) for the developer to overshadow
	#############################################################
	
	def Preflight(self, in_signal_props):
		"""
		This is the usual BCI2000 Preflight hook, which you
		would overshadow in your subclass in order to sanity-
		check parameter values and verify the availability of
		state variables after the "Set Config" button is
		pressed. You can also use it to specify the dimensions
		of your module's output signal packets, if these are
		different from the input.
		
		The input argument in_signal_props (as well as the
		instances attribute self.in_signal_props) is a dict
		containing details analogous to the SignalProperties
		object in C++ (in order to see it, either implement a
		Preflight hook containing only a "print in_signal_props"
		statement, or use the shell to examine
		self.in_signal_props after pressing "Set Config").
		If a value is returned (or, if you prefer, assigned in
		self.out_signal_props), then it should be a
		dict of the same form.
		
		Alternatively the function may return a two-element,
		sequence specifying the output signal dimensions:
		
			return (nOutputChannels, nOutputSamples)
		
		The attribute self.in_signal_dim contains this
		simplified information on input.

		For the application module, there is one further twist:
		this is the place to perform initial configuration of
		VisionEgg.config settings, and/or the window position
		using self.set_ve_window_pos().  The Initialize() hook
		will only be called after the VisionEgg window has been
		opened.
		"""###
		pass
		
	#############################################################

	def Initialize(self, in_signal_dim, out_signal_dim):
		"""
		This is the usual BCI2000 Initialize hook, called
		following Preflight when the "Set Config" button is
		pressed. Your subclass implementation of Initialize is
		the place to pre-allocate any objects you might need.
		Attach them as new attributes of self:
		
		    self.foo = FooObject()
		
		so that you'll know where to find them later on. Just
		be somewhat careful not to overwrite anything put there
		by the framework (in the shell, type "self." and then
		press tab to see what's there).

		The in_signal_dim and out_signal_dim input arguments
		are tuples specifying the shape of the Process hook's
		input and output signal packets, in the format
		(nChannels,nSamples). The same information is available
		in the self.in_signal_dim and self.out_signal_dim
		attributes.		

		For the application module, this is the best place to
		set up stimuli (indeed, for visual stimuli, it is the
		only place to do so). VisionEgg stimuli are created
		here and then registered using calls to self.stimulus().
		It's not necessary to attach them as attributes of self
		as well, since they will be available in a dict called 
		self.stimuli, but it you may also find it useful to
		(redundantly) do so. Note that the initial configuration
		of VisionEgg.Config parameters and the (x,y) position
		of the VisionEgg window *cannot* be done here: that must
		be done in Preflight, since Initialize is called after
		the window has been opened.
		"""###
		if not hasattr(self, '_warned'): self._warned = False
		if not self._warned:
			self._warned = True
			print
			print
			print "Hello, this is the BciGenericApplication superclass."
			print "This message appears because you have not overshadowed"
			print "Initialize(self, sig) inside a BciApplication subclass,"
			print "or perhaps not defined a BciApplication subclass in the"
			print "first place. Either way, it looks as if the application"
			print "module is going to do nothing."
			print
		
	#############################################################

	def StartRun(self):
		"""
		This is the usual BCI2000 start-of-run hook, which you
		can overshadow in your subclass implementation. If you
		have not defined a 'start' phase using an appropriate call
		to self.design() in the Phase hook, then you can also kick-
		start the experiment by issuing a call to
		self.change_phase('whatever') here in StartRun.
		"""###
		pass

	
	#############################################################

	def Phases(self):
		"""
		This hook, which you should implement in your BciApplication
		class, exists only for the application module. It is called in
		order to define the "phase machine", i.e. the state-machine
		that governs transitions between stimulus presentation
		phases. In fact this hook is re-called at every transition,
		so dynamic changes in the phase-machine architecture are
		possible. As a simple example, it is meaningful to supply
		random numbers as stimulus durations if this is the desired
		behaviour---they will indeed be different each time the
		phase machine queries them.
		
		Use self.phase() and self.design() to specify the phase
		machine's structure.
		"""###
		pass
		
	#############################################################

	def Transition(self, phasename):
		"""
		This hook, which you should implement in your BciApplication
		class, exists only for the application module. It is called
		at every phase transition. The name of the presentation phase
		to which the machine is changing is supplied as a string
		argument.  This hook will probably do most of the work in
		presenting (non-continuous) stimuli, and in recording (by
		setting appropriately-named state variables) what is going on.
		"""###
		pass

	#############################################################

	def Frame(self, phasename):
		"""
		This hook, which you may or may not wish to implement in your
		BciApplication class, exists only for the application module.
		It is called on every video frame. The name of the current
		presentation phase is supplied as a string argument. One thing
		you might want to do is self.animate() some .parameter of
		a stimulus object.
		"""###
		pass

	#############################################################

	def Event(self, phasename, event):
		"""
		This hook, which you may or may not wish to implement in your
		BciApplication class, exists only for the application module.
		It is called for keyboard and mouse events. For each element
		of pygame.event.get(), this hook is called with the result,
		together with the name of the current presentation phase
		as a string. Compare event.type against various constants
		defined in pygame.locals in order to handle it
		appropriately.
		"""###
		if (event.type == pygame.locals.QUIT) or (event.type == pygame.locals.KEYDOWN and event.key == pygame.locals.K_ESCAPE):
			self.add_callback('StopRun', self._threads['vision egg'].post, ('stop',))
			self.states['Running'] = False

#################################################################
#################################################################

class BciStimulus(object):

	#############################################################

	def __init__(self, bci, name, z):		
		self.__dict__['_bci'] = bci
		self.__dict__['_maker'] = None
		self.__dict__['Stimulus'] = None
		self.__dict__['_name'] = name
		self.__dict__['z'] = z
		
	#############################################################

	def set(self, **kwargs):
		z = kwargs.pop('z')
		if z != None: self.z = float(z)
		if len(kwargs) == 0: return
		v = self.__dict__.get('Stimulus')
		if v == None: raise AttributeError, "a VisionEgg Stimulus has not yet been instantiated inside this object"
		v.set(**kwargs)
		
	#############################################################

	def __setattr__(self, key, value):
		if key in self.__dict__:
			self.__dict__[key] = value
			if key == 'z':
				b = self.__dict__['_bci']
				if not hasattr(b, '_stimq'): b._stimq = []
				if not self in b._stimq: b._stimq.append(self)
		else:
			v = self.__dict__.get('Stimulus')
			p = getattr(v, 'parameters', None)
			if p != None and hasattr(p, key): setattr(p, key, value)
			elif v != None and hasattr(v, key): setattr(v, key, value)
			elif v == None: raise AttributeError, "a VisionEgg Stimulus has not yet been instantiated inside this object"
			else: raise AttributeError, "'%s' object has no attribute or stimulus parameter '%s'"%(self.__class__.__name__, key)

	#############################################################

	def __getattr__(self, key):
		if key in self.__dict__: return self.__dict__[key]
		v = self.__dict__.get('Stimulus')
		p = getattr(v, 'parameters', None)
		if p != None and hasattr(p, key): return getattr(p, key)
		if v != None and hasattr(v, key): return getattr(v, key)
		if v == None: raise AttributeError, "a VisionEgg Stimulus has not yet been instantiated inside this object"
		raise AttributeError, "'%s' object has no attribute or stimulus parameter '%s'"%(self.__class__.__name__, key)

	#############################################################

	def _getAttributeNames(self):
		p = getattr(self.__dict__.get('Stimulus'), 'parameters', None)
		if p == None: return ()
		return p.__dict__.keys()

	#############################################################

	def __repr__(self):
		v = self.__dict__.get('Stimulus')
		if v == None: s = ''
		else: s = ' containing %s.%s' % (v.__class__.__module__,v.__class__.__name__)
		s = '<%s "%s" at 0x%08X%s, z=%g>' % (self.__class__.__name__,str(self._name),id(self),s,float(self.z))
		return s

	#############################################################

	def leave(self):
		v = self.__dict__.get('Stimulus')
		b = self.__dict__.get('_bci')
		if hasattr(b, '_stimlist') and v in b._stimlist:
			ind = b._stimlist.index(v)
			if len(getattr(b, '_stimz', [])) == len(b._stimlist): b._stimz.pop(ind)
			b._stimlist.pop(ind)
		if hasattr(b, '_stimq') and self in b._stimq:
			b._stimq.remove(self)
		if hasattr(b, 'stimuli') and (self._name, self) in b.stimuli.items():
			b.stimuli.pop(self._name)
			
	#############################################################

	def enter(self):
		b = self.__dict__.get('_bci')
		if isinstance(b._stimq, list):
			if not self in b._stimq:
				b._stimq.append(self)
		else:
			b._update_stimlist(self)
		b.stimuli[self._name] = self
		
	#############################################################

	def __del__(self):
		self.leave()
		
#################################################################
#################################################################
