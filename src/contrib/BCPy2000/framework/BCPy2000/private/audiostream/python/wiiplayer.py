__all__ = ['init', 'wiiplayer']

import numpy
import scipy
import WavTools
import time
import wiiuse
from scipy.fftpack import hilbert
	
wiimote_ptrs = None
number_of_wiimotes = None
def init(nmotes):
	global wiimote_ptrs, number_of_wiimotes
	wiimote_ptrs = wiiuse.init(nmotes)
	found = wiiuse.find(wiimote_ptrs, nmotes, 5)
	if found != nmotes: raise RuntimeError("expected %d wiimotes, found %d" % (nmotes, found))
	#connected = wiiuse.connect(wiimote_ptrs, nmotes)
	#if connected != nmotes: raise RuntimeError("expected %d wiimotes, connected %d" % (nmotes, connected))
	number_of_wiimotes = nmotes
	

class wiiplayer(WavTools.Background.ongoing):
	def __init__(self, w,  wiimote_index=0, threshold=0.6):
		#variables
		WavTools.Background.ongoing.__init__(self)
		if isinstance(w, basestring): w = WavTools.wav(w)
		self.wav = w
		self.threshold = threshold
		self.wiimote_index = wiimote_index
		if wiimote_ptrs == None: raise RuntimeError("call init() first")
		wiiuse.set_leds(wiimote_ptrs[wiimote_index], wiiuse.LED[wiimote_index])
		
		# sampling rate in Hz
		sampling_rate = self.wav.fs		
		# envelope of self.wav.y
		original_length = self.wav.y.shape[0]
		wave_zeros = numpy.r_[self.wav.y, numpy.zeros((10000,1))] 
		wave_zeros = numpy.array(wave_zeros + hilbert(wave_zeros)*1.j, dtype = complex)
		wave = abs(wave_zeros[0:original_length])
		# find crossing of threshold
		crossing = numpy.r_[[[0]],wave,[[0]]]
		above_threshold = (crossing>=self.threshold)
		crossing[above_threshold] = 1
		below_threshold = (crossing<self.threshold)
		crossing[below_threshold] = -1
		crossing = crossing[1:] - crossing[0:-1]
		indices = numpy.nonzero(crossing)
		timing = indices[0]/float(sampling_rate)
		# define points of onset and offset
		on_off = (crossing[indices]>0)
		self.timing = numpy.broadcast(timing,on_off.astype(int))
		#while True:
		#	try: print self.timing.next()
		#	except StopIteration:
		#		self.timing.reset()
		#		break
				
		
	def play(self, bg=True):
		self.playing = True
		self.go(bg=bg)
		
	def core(self):
		ptr = wiimote_ptrs[self.wiimote_index]
		start_time = time.time()
		while self.keepgoing:
			try:
				time_on = self.timing.next()
			except StopIteration:
				self.timing.reset()
				break
			else:
				while time.time() <= time_on[0] + start_time:
					time.sleep(0.001)
				wiiuse.rumble(ptr, time_on[1])
		wiiuse.rumble(ptr, 0)
		self.playing = False
