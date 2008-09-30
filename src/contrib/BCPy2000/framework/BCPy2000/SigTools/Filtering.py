#   $Id$
#   
#   This file is part of the BCPy2000 framework, a Python framework for
#   implementing modules that run on top of the BCI2000 <http://bci2000.org/>
#   platform, for the purpose of realtime biosignal processing.
# 
#   Copyright (C) 2007-8  Thomas Schreiner, Jeremy Hill
#                         Christian Puzicha, Jason Farquhar
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
__all__ = [
	'applyfilter', 'causalfilter', 'fader', 'trap',
]

from Basic import getfs
from NumTools import project,isnumpyarray
from scipy.signal import lfilter
import scipy.signal.filter_design
import numpy

def applyfilter(x,b=1.0,a=1.0,axis=-1,zi=None):
	"""
	Apply a causal filter (b,a)  to a signal x along a specified axis,
	optionally given the specified initial filter state zi.
	
	Return (y,zf):  the filtered signal, and the final filter state.
	
	The causalfilter object wraps this function more conveniently.
	"""###
	matrixformat = isinstance(x, numpy.matrix)
	x = numpy.asarray(x, dtype=numpy.float64)
	b = numpy.asarray(b, dtype=numpy.float64)
	a = numpy.asarray(a, dtype=numpy.float64)
	shape = list(x.shape)
	if axis < 0: axis = len(x.shape) + axis
	if zi==None:
		shape[axis]=max(len(b),len(a))-1
		zi=numpy.zeros(shape=shape,dtype='complex')	
	zi = project(numpy.array(zi, dtype='complex'), axis)
	b = b.flatten()
	a = a.flatten()	
	shape[axis] = 1
	y = numpy.zeros(shape=x.shape)
	zf = numpy.array(zi)
	colon = slice(None)
	for sub in numpy.ndindex(tuple(shape)): # have to do it this way because filtering of non-vector signals is still bugged in scipy (non-deterministic output, intermittent crashes)
		sub = list(sub)
		sub[axis] = colon
		v,zf[sub].flat[:]=lfilter(b=b,a=a,x=x[sub].flatten(),zi=zi[sub].flatten())
		y[sub].flat[:]=v.real
	if matrixformat: y = numpy.matrix(y, copy=False)
	return y,zf
	
class causalfilter(object):
	def __init__(self, freq_hz, samplingfreq_hz, order=10, type='bandpass', method=scipy.signal.filter_design.butter, **kwargs):
		"""
		Returns a new online causal filter object f with coefficient arrays
		f.b and f.a computed by the specified <method>, given the <order> and
		<freq_hz> parameters, the latter being interpreted in the context of
		the specified <samplingfreq_hz>. Any additional keyword arguments are
		passed through to the <method>.
		
		For example, a 50 Hz notch filter for 250Hz data:
		
		  f = causalfilter([48,52], 250, order=10, type='bandstop')
		
		  x1_filtered = f.apply(x1)  # x1 and x2 are channels-by-samples
		  x2_filtered = f.apply(x2)  # and are consecutive packets: x1
		                             # is "remembered" when filtering x2
		
		"""###
		self.type = scipy.signal.filter_design.band_dict[type]
		self.order = int(order)
		if not isinstance(freq_hz,list) and not isinstance(freq_hz,tuple): freq_hz = [freq_hz]
		self.freq_hz = map(float, freq_hz)
		self.samplingfreq_hz = getfs(samplingfreq_hz)
		self.method = method
		self.kwargs = kwargs
		lims = map((lambda x: x / self.samplingfreq_hz * 2.0), self.freq_hz);
		self.b, self.a = method(N=self.order/2, Wn=tuple(lims), btype=self.type, output='ba', **kwargs)
		self.state = None
		self.samples_filtered = 0
		
	def __repr__(self):
		basestr = "<%s.%s instance at 0x%08X>" % (self.__class__.__module__,self.__class__.__name__,id(self))
		bstr = ', '.join(["%.3g"%x for x in self.b])
		astr = ', '.join(["%.3g"%x for x in self.a])
		fstr = ', '.join(["%.3g"%x for x in self.freq_hz])
		if len(self.freq_hz) != 1: fstr = "[%s] "%fstr
		s = "%s\n    %s filter of order %d made with %s.%s:\n    freq = %sHz for data sampled at %gHz\n       b = [%s]\n       a = [%s]" % (basestr, self.type, self.order, self.method.__module__, self.method.__name__, fstr, self.samplingfreq_hz, bstr, astr)
		return s
		
	def filter(self, x, axis=None, reset=False):
		"""
		Filter the signal or signal packet <x> along the specified <axis>.
		By default, <axis> is the highest dimension of <x>, so a channels-
		by-samples array is the easiest representation.
		
		Optionally, if <reset> is set to True, then the filter object will
		be reset() before filtering <x>.
		
		Return the filtered signal, of the same dimensions as the input.
		"""###
		if hasattr(x, 'y'): # special support for WavTools.Base.wav objects
			obj, x  =  x.copy(), x.y;
			if axis == None: axis = 0
		else:
			obj = None;
			if axis == None: axis = -1

		if reset: self.reset()
		
		y, self.state = applyfilter(x=x, b=self.b, a=self.a, axis=axis, zi=self.state)
		self.samples_filtered += x.shape[axis]
		if obj != None: obj.y = y; y = obj
		return y

	apply = filter
	__call__ = filter
		
	def reset(self):
		"""
		Forget previous samples filtered.  The next signal packet to be
		filtered will behave as if it follows a flat 0 signal.
		"""###
		self.samples_filtered = 0
		self.state = None
		
		
class fader(object):
	"""
	An object in which the time series of a temporal window can
	be queued up for online application:
	
	    f = fader(0.0)          # sets initial gain
	
	    packet_1 = f(packet_1)  # comes out zeroed
	
	    rise = scipy.signal.hanning(129)[:65]
	                            # a window which rises from 0.0 to 1.0
	
	    f += rise               # so now the next 65 samples will be
	                            # windowed
	
	    packet_2 = f(packet_2)
	    packet_3 = f(packet_3)
	    packet_4 = f(packet_4)  # at some point the 65 windowing samples
	                            # in the queue will be exhausted
	    
	    packet_5 = f(packet_5)  # ... after which packets come out
	                            # multiplied by the last-used gain value,
	                            # which was 1.0
	"""###
	def __init__(self, initial_gain=1.0):
		"""
		<initial_gain> may be a float, or it may be a sequence of floats,
		one per channel.
		"""###
		g = numpy.asarray(initial_gain, dtype=numpy.float64)
		g.shape += (1,) * (2 - len(g.shape))
		self.gain   = g[:,[-1]]
		self.queue = g[:,:0]
	
	def __iadd__(self, g):
		self.queue = numpy.concatenate((self.queue, numpy.array(g, ndmin=2) * numpy.ones(self.gain.shape)), axis=1)
		return self
	
	def apply(self, x):
		"""
		Return <x> multiplied by whatever is in the queue. The samples used to
		window x are "used up" and removed from the queue.  If insufficient
		samples are in the queue, the remainder of the signal is multiplied by
		a constant factor equal to the last gain value that was in the queue.
		In other words, the gain stays constant unless you change it by
		queueing up more windowing samples.
		"""###
		g = self.queue[:,:x.shape[1]]
		self.queue = self.queue[:,x.shape[1]:]
		if g.size: self.gain = g[:,[-1]].copy()
		extra = x.shape[1] - g.shape[1]
		if extra > 0: g = numpy.concatenate((g, self.gain.repeat(extra, axis=1)), axis=1)
		return numpy.multiply(x, g)
	
	__call__ = apply
	
	def pending(self):
		"""
		Return the number of samples currently in the queue.
		"""###
		return self.queue.shape[1]
	
class trap(object):
	"""
	A trap for collecting segments of signal as they come in packet by
	packet.
	
	A leaky trap is the simplest type:
	
	    t = trap(1000, leaky=True)
	    
	    t += packet_1         # these three syntaxes
	    t.process(packet_2)   # are all equivalent
	    t(packet_3)           # packets are channels-by-samples
	    
	This simply stores the last 1000 samples at any given time,
	accessible as t.buffer.
	
	Non-leaky traps, on the other hand, are most effective when triggered
	by a particular signal channel.  t.trigger_channel specifies the 0-
	based index of the channel to watch, and t.trigger_threshold specifies
	the threshold that will "spring" the trap when the absolute value of
	the trigger signal exceeds it.  Before the trap is sprung, no samples
	are collected---the signal will only start being collected from the
	sample at which the threshold is exceeded (although if no trigger
	channel was specified, the trap will spring as soon as anything is put
	into it).  Once a non-leaky trap is full, no further samples will be
	stored, until the trap is emptied and re-armed with t.flush()
	
	In either case, t.full() queries whether the target number of samples
	has yet been reached.		
	"""###
	def __init__(self, nsamples, trigger_channel=None, trigger_threshold=0.0, leaky=False):
		"""
		Initializes self.nsamples, self.trigger_channel,
		self.trigger_threshold and self.leaky with the specified values.
		"""###
		self.buffer = None
		self.sprung = False
		self.trigger_channel = trigger_channel
		self.trigger_threshold = trigger_threshold
		self.nsamples = nsamples
		self.leaky = leaky
	
	def __iadd__(self, x):
		self.process(x)
		return self

	def process(self, x, arm=True):
		"""
		Process the channels-by-samples signal array <x>. If the trap is
		not yet sprung, monitor the trigger channel and only accumulate the
		signal from the sample at which its absolute value exceeds the
		trigger threshold.
		
		When the trap springs, the method returns 1 + the offset of the
		sample at which the trap is sprung. At all other times the method
		returns 0.
		
		When accumulating: if the trap is leaky and already full, as many
		samples are discarded as are accumulated.  If the trap is non-leaky
		and full, nothing is accumulated.
		
		The optional argument <arm> can be set to False in order to suppress
		the ability of the trap to spring on this packet. If the trap is
		already sprung, the signal is accumulated regardless of <arm>.
		"""###
		if not self.leaky and self.full(): return 0
		output = 0
		if not isinstance(x, numpy.ndarray):
			x = numpy.array(x)
			if len(x.shape)==1: x.shape += (1,)
		if arm and not self.sprung:
			if self.trigger_channel == None:  # in this case, the trap springs
				self.sprung = True            # as soon as it is used
				output = 1
			else:
				tr = numpy.asarray(x[self.trigger_channel, :]).ravel()
				tr = numpy.abs(tr) > self.trigger_threshold
				tr = numpy.argwhere(tr)
				if len(tr):
					self.sprung = True
					x = x[:, tr[0,0]:]
					output = tr[0,0] + 1
		if self.sprung:
			if self.buffer == None:
				self.buffer = x[:,:self.nsamples].copy()
			else:
				excess = self.buffer.shape[1] + x.shape[1] - self.nsamples
				if excess > 0:
					if self.leaky: self.buffer = self.buffer[:, excess:] # excess leaks out the bottom
					else:          x = x[:, :-excess]                    # excess spills over the top
				self.buffer = numpy.concatenate((self.buffer, x), axis=1)
		return output

	__call__ = process
	
	def full(self):
		"""
		Return a boolean indicating whether the trap has yet accumulated
		the requested number of samples.
		"""###
		return self.buffer != None and self.buffer.shape[1] >= self.nsamples
	
	def flush(self):
		"""
		Return the contents of self.buffer after removing it and re-arming
		(un-springing) the trap.
		"""###
		b = self.buffer
		self.buffer = None
		self.sprung = False
		return b

	def spring(self):
		"""
		Manually spring the trap instead of waiting for the trigger channel. 
		"""###
		self.sprung = True
		