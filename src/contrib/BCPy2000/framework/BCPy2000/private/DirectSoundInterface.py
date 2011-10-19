__all__ = []
import WavTools
import numpy
import time

# where to find development version of extensions...
import os, sys, distutils.util
platlib = "lib.%s-%s" % (distutils.util.get_platform(), sys.version[0:3])
# modelled on distutils.command.build.build.finalize_options which is the source of the name used by setuptools.command.build_ext
extdir = os.path.realpath(os.path.join(os.path.dirname(__file__), platlib))
if os.path.isdir(extdir) and not extdir in sys.path: sys.path.append(extdir)

loaded = False

try:
	import _pydsound as ds
except ImportError:
	print "WARNING:  failed to load DirectSound interface"
else:
	loaded = True

	def ds_open(self, w=None, dev=None, timecritical=False, data=None):
		
		if self.playing and not timecritical: return
		# allow the call from __init__, or those from the command-line while not playing
		# allow the call from core (new parameter timecritical respected)
		# ignore the call from play
		
		if isinstance(w,str): w = WavTools.wav(w)
		if w == None: w = self.wav
		self.wav = w
		if self.wav == None: return
		if data==None: data = self.wav.y
		oldstamp = getattr(self, 'wavstamp', None)
		self.wavstamp = [id(self.wav), id(data), self.wav.revision]
		self.dsptr = getattr(self, 'dsptr', None)
		if self.dsptr == None or self.wavstamp != oldstamp or not data is self.wav.y:
			if timecritical: print "warning: sound data seem to have changed; re-transferring (and hence losing time-critical performance)"
			self.close()
			data = numpy.asarray(data, dtype=numpy.float64) # doesn't make a copy if already float64
			self.dsptr = ds.NewSound(data, self.wav.fs, self.wav.bits)
		if hasattr(self, 'lpt_preplay'):
			port = self.lpt_preplay.get('port',0)
			ds.PreplayLPT(self.dsptr, self.lpt_preplay.get('val',-1), getattr(port, 'port', port))
		if hasattr(self, 'lpt_postplay'):
			self.lpt_postplay.get('port',0)
			ds.PostplayLPT(self.dsptr, self.lpt_postplay.get('val',-1), getattr(port, 'port', port))
	
		
	def ds_close(self):
		dsptr = getattr(self, 'dsptr', None)
		if dsptr != None: ds.DeleteSound(dsptr)
		self.dsptr = None
		
		
	def ds_core(self, repeats=1, w=None, data=None):
					
		self.timestamps['core'] = WavTools.Background.prectime()
		sleep_msec = 1.0
		preplay_done = False
	
		self.open(w=w, data=data, timecritical=True)		
		if self.wav == None:
			self.playing = False
			return
		
		speed = pan = vol = None		
		dsrepeats = min(1, repeats)		
		while repeats != 0:
			pressPlay = True
			while True:
				if not self.keepgoing or self.timedout(): self.keepgoing=False; break				
	
				# turn self.vol and self.pan into scalar volume and pan
				# what?  well, self.pan can be a vector you see
				v = float(self.vol) * WavTools.Base.panhelper(self.pan, nchan=self.wav.y, norm=self.norm)
				v = numpy.clip(numpy.asarray(v).flatten(), 0.0, 1.0)[:2]
				if len(v) == 1: v = numpy.concatenate((v,v))
				oldvol, vol = vol, max(v)
				if vol: v /= vol
				else: v[:] = 1.0
				oldpan, pan = pan, numpy.sign(v[1]-v[0])*(2.0-sum(v))
				oldspeed,speed = speed,float(self.speed)				
							
				if speed != oldspeed: ds.SetSpeed(self.dsptr, speed)
				if pan != oldpan:     ds.SetPan(self.dsptr, pan)
				if vol != oldvol:     ds.SetVolume(self.dsptr, vol)				
	
				if not preplay_done and self.preplay != None and self.preplay['func'] != None:
					self.preplay['func'](*self.preplay['pargs'], **self.preplay['kwargs'])
					preplay_done = True
				if pressPlay:
					ds.PlaySound(self.dsptr, dsrepeats)
					pressPlay = False
				time.sleep(sleep_msec/1000.0)
				if not ds.Check(self.dsptr): break
			if not self.keepgoing: break
			repeats -= 1
		ds.StopSound(self.dsptr)
		if self.postplay != None and self.postplay['func'] != None:
			self.postplay['func'](*self.postplay['pargs'], **self.postplay['kwargs'])
		self.playing = False
		
	def ds_set_preplay_hook(self, func, *pargs, **kwargs):
		inst = getattr(func, 'im_self', func)
		if str(inst.__class__) == 'AppTools.ParallelPort.lpt' and len(kwargs)+len(pargs) == 1:
			self.lpt_preplay = {'port':inst, 'val':(kwargs.values()+list(pargs))[0]}
			func,pargs,kwargs = None,('preplay lpt setting has been rerouted to the pydsound binary',),self.lpt_preplay
		else:
			self.lpt_preplay = {'port':0, 'val':-1} # NB: assumes only one hook at a time (watch out if this model changes in PyAudioInterface)
		self.preplay = {'func':func,'pargs':pargs,'kwargs':kwargs}
	
	def ds_set_postplay_hook(self, func, *pargs, **kwargs):
		inst = getattr(func, 'im_self', func)
		if str(inst.__class__) == 'AppTools.ParallelPort.lpt' and len(kwargs)+len(pargs) == 1:
			self.lpt_postplay = {'port':inst, 'val':(kwargs.values()+list(pargs))[0]}
			func,pargs,kwargs = None,('postplay lpt setting has been rerouted to the pydsound binary',),self.lpt_postplay
		else:
			self.lpt_postplay = {'port':0, 'val':-1} # NB: assumes only one hook at a time (watch out if this model changes in PyAudioInterface)
		self.postplay = {'func':func,'pargs':pargs,'kwargs':kwargs}
	
	WavTools.player.open  = ds_open;  del ds_open
	WavTools.player.close = ds_close; del ds_close
	WavTools.player.core  = ds_core;  del ds_core
	WavTools.player.set_preplay_hook   = ds_set_preplay_hook;   del ds_set_preplay_hook
	WavTools.player.set_postplay_hook  = ds_set_postplay_hook;  del ds_set_postplay_hook
	delattr(WavTools.player, 'openstream')
	delattr(WavTools.player, 'closestream')
	delattr(WavTools.player, 'ready')
