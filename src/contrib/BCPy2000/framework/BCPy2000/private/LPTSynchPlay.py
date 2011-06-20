__all__ = ['dll', 'setLPT', 'preload', 'play']
import WavTools.Base as Base
import ctypes,numpy

import os
dllpath = os.path.abspath(os.path.join(os.path.abspath(__file__), '..\\..\\..\\..\\..\\..\\..\\..\\..\\sound\\matlab\\lptsynchplay.dll'))
print 'loading',dllpath
dll = ctypes.CDLL(dllpath)
(dll.snd, dll.addsound, dll.setLPT, dll.cleanup)

def setLPT(port):
	dll.setLPT(ctypes.c_int(int(port)))

def preload(filename):
	filename = ctypes.create_string_buffer(filename)
	rawdoubles = ctypes.c_voidp(0)
	nchan = ctypes.c_int(0)
	nsamp = ctypes.c_int(0)
	fs = ctypes.c_int(0)
	return dll.addsound(filename,rawdoubles,nchan,nsamp,fs)

def play(soundid, startstate=-1, endstate=-1, when=0.0, synchronous=True):
	soundid = ctypes.c_int(soundid)
	filename = ctypes.c_voidp(0)
	rawdoubles = ctypes.c_voidp(0)
	nchan = ctypes.c_int(0)
	nsamp = ctypes.c_int(0)
	fs = ctypes.c_int(0)
	startstate = ctypes.c_int(startstate)	
	endstate = ctypes.c_int(endstate)
	when = ctypes.c_double(when)
	synchronous = ctypes.c_int(int(synchronous))
	timestamp = ctypes.c_double(0)
	res = dll.snd(soundid,filename,rawdoubles,nchan,nsamp,fs,startstate,endstate,when,synchronous,ctypes.addressof(timestamp))
	return timestamp.value
	
def lptsynchplay(self, startstate=None, endstate=None, when=0.0, synchronous=True):
	if hasattr(self, 'lptsynchplay_params'):
		if startstate == None: startstate = self.lptsynchplay_params.get('startstate', -1)
		if endstate == None:   endstate   = self.lptsynchplay_params.get('endstate',   -1)
		preload = self.lptsynchplay_params.get('preload_id', 0)
		if preload: return play(preload, startstate, endstate, when, synchronous)
	if startstate == None: startstate = -1
	if endstate   == None: endstate   = -1
	soundid = ctypes.c_int(0)
	filename = ctypes.c_voidp(0)
	rawdoubles = numpy.array(self.y,dtype='float64').tostring(order='F')
	rawdoubles = ctypes.create_string_buffer(rawdoubles)
	nchan = ctypes.c_int(self.channels())
	nsamp = ctypes.c_int(self.samples())
	fs = ctypes.c_int(int(self.fs))
	startstate = ctypes.c_int(startstate)	
	endstate = ctypes.c_int(endstate)
	when = ctypes.c_double(when)
	synchronous = ctypes.c_int(int(synchronous))
	timestamp = ctypes.c_double(0)
	res = dll.snd(soundid,filename,rawdoubles,nchan,nsamp,fs,startstate,endstate,when,synchronous,ctypes.addressof(timestamp))
	return timestamp.value
Base.wav.lptsynchplay = lptsynchplay; del lptsynchplay

def lptsynchplay_preload(self):
	filename = ctypes.c_voidp(0)
	rawdoubles = numpy.array(self.y,dtype='float64').tostring(order='F')
	rawdoubles = ctypes.create_string_buffer(rawdoubles)
	nchan = ctypes.c_int(self.channels())
	nsamp = ctypes.c_int(self.samples())
	fs = ctypes.c_int(int(self.fs))
	soundid = dll.addsound(filename,rawdoubles,nchan,nsamp,fs)
	self.lptsynchplay_set(preload_id=soundid);
	return soundid
Base.wav.lptsynchplay_preload = lptsynchplay_preload; del lptsynchplay_preload

def lptsynchplay_set(self, **kwargs):
	if not hasattr(self, 'lptsynchplay_params'): self.__dict__['lptsynchplay_params'] = {};
	self.lptsynchplay_params.update(kwargs)

Base.wav.lptsynchplay_set = lptsynchplay_set; del lptsynchplay_set

def test():
	import example_wavs
	w = example_wavs.ringin().wav
	ww = (w % 0.5) & (0.5 % Base.reverse(w))
	a = preload(w.filename)
	b = ww.lptsynchplay_preload()
	return(w,ww)

from AppTools.ParallelPort import lpt
setLPT(lpt().port) # in order to take advantage of lookup_lpt_port

import WavTools.PyAudioInterface as PyAudioInterface
def newcore(self, repeats=1, w=None, data=None):
	if w==None: w = self.wav
	self.timestamps['core']=w.lptsynchplay()
	self.playing = False
PyAudioInterface.player.core = newcore; del newcore