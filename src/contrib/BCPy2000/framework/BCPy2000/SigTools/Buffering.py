#   $Id$
#   
#   This file is part of the BCPy2000 framework, a Python framework for
#   implementing modules that run on top of the BCI2000 <http://bci2000.org/>
#   platform, for the purpose of realtime biosignal processing.
# 
#   Copyright (C) 2007-9  Jeremy Hill, Thomas Schreiner,
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
__all__ = [
	'trap', 'ring',
]
import numpy

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
	accessible with t.read()
	
	Non-leaky traps, on the other hand, are most effective when triggered
	by a particular signal channel.  t.trigger_channel specifies the 0-
	based index of the channel to watch, and t.trigger_threshold specifies
	the threshold that will "spring" the trap when the value of the
	(processed) trigger signal exceeds it.  t.trigger_processing (defaulting
	to numpy.abs) specifies the function by which the trigger channel is
	processed before comparison against the threshold.
	
	Before the trap is sprung, no samples are collected---the signal will
	only start being collected from the sample at which the threshold is
	exceeded (although if no trigger channel was specified, the trap will
	spring as soon as anything is put into it).  Once a non-leaky trap is
	full, no further samples will be stored, until the trap is emptied and
	re-armed with t.flush()
	
	In either case, t.full() queries whether the target number of samples
	has yet been reached.		
	"""###
	def __init__(self, nsamples, nchannels, trigger_channel=None, trigger_threshold=0.0, trigger_processing=numpy.abs, leaky=False):
		"""
		Initializes self.nsamples, self.trigger_channel,
		self.trigger_threshold, self.trigger_processing and self.leaky with
		the specified values.
		"""###
		self.ring = ring(nsamples, nchannels)
		self.sprung = False
		self.trigger_channel = trigger_channel
		self.trigger_threshold = trigger_threshold
		self.trigger_processing = trigger_processing
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
		samples are discarded as they are accumulated.  If the trap is
		non-leaky and full, nothing is accumulated.
		
		The optional argument <arm> can be set to False in order to suppress
		the ability of the trap to spring on this packet. If the trap is
		already sprung, the signal is accumulated regardless of <arm>.
		"""###
		if not self.leaky and self.full(): return 0
		output = 0
		if not isinstance(x, numpy.ndarray):
			x = numpy.array(x)
			if len(x.shape)==1: x.shape += (1,)
		startx,stopx = 0,x.shape[1]
		if arm and not self.sprung:
			if self.trigger_channel == None:  # in this case, the trap springs
				self.sprung = True            # as soon as it is used
				output = 1
			else:
				tr = numpy.asarray(x[self.trigger_channel, :]).ravel()
				if self.trigger_processing != None: tr = self.trigger_processing(tr)
				tr = numpy.argwhere(tr > self.trigger_threshold)
				if len(tr):
					self.sprung = True
					startx = tr[0,0]
					output = tr[0,0] + 1
		if self.sprung:
			excess = self.collected() + (stopx-startx) - self.nsamples
			if excess > 0:
				if self.leaky: self.ring.forget(excess) # excess leaks out the bottom
				else: stopx -= excess  # excess spills over the top
			if stopx > startx: self.ring.write(x[:, startx:stopx])
		return output

	__call__ = process
	
	def read(self):
		rh = self.ring.readhead
		x = self.ring.read()
		self.ring.readhead = rh
		return x
		
	def collected(self):
		"""
		Returns the number of samples accumulated in the trap.
		"""###
		return self.ring.to_read()
	
	def full(self):
		"""
		Return a boolean indicating whether the trap has yet accumulated
		the requested number of samples.
		"""###
		return self.collected() >= self.nsamples
	
	def flush(self):
		"""
		Return the contents of the trap after removing it and re-arming
		(un-springing) the trap.
		"""###
		b = self.ring.read(self.nsamples)
		self.sprung = False
		return b

	def spring(self):
		"""
		Manually spring the trap instead of waiting for the trigger channel. 
		"""###
		self.sprung = True

class ring(object):
	"""
	An class implementing a classic ring-buffer for reading and writing
	signals in channels-by-samples packets.

	b = ring(nsamples, nchannels)  initializes the buffer
	b.to_read()  # returns the number of samples available but not yet read
	b.to_write() # returns the number of samples that can be written
	b.read(n)    # reads n samples from the buffer 
	b.write(x)   # writes a channels-by-samples numpy array to the buffer
	
	"""###
	
	def __init__(self, nsamp, nchan):
		self.writehead = 0
		self.readhead  = 0
		self.buf = numpy.zeros((nchan,nsamp+1), dtype=numpy.float64)
	
	def channels(self):
		return self.buf.shape[0]

	def samples(self):
		return self.buf.shape[1]
		
	def to_write(self):
		"""
		Returns the amount of free space currently in the buffer, i.e. the
		number of samples that can be written.
		"""###
		if self.readhead > self.writehead: return self.readhead - self.writehead - 1
		else: return self.readhead + self.samples() - self.writehead - 1

	def to_read(self):
		"""
		Returns the number of samples pending in the buffer, i.e. available
		but not yet read.
		"""###
		if self.writehead >= self.readhead: return self.writehead - self.readhead
		else: return self.writehead + self.samples() - self.readhead

	def write(self, x):
		"""
		Writes signal packet <x>, a channels-by-samples numpy array, to the
		buffer.
		"""###
		(nchan, nsamp) = x.shape
		if nchan != self.channels(): raise ValueError, "incoming data has the wrong number of channels"
		if nsamp > self.to_write(): raise RuntimeError, "ring buffer overflow"
		n = min([nsamp, self.samples() - self.writehead])
		self.buf[:, self.writehead:self.writehead+n] = x[:,:n]
		self.writehead = (self.writehead + n) % self.samples()
		m = max([0, nsamp - n])
		self.buf[:, :m] = x[:, n:n+m]
		self.writehead += m
	
	def read(self, nsamp=None):
		"""
		Reads <nsamp> samples from the buffer, returning a channels-by-samples
		numpy array.
		"""###
		available = self.to_read()
		if nsamp == None: nsamp = available
		if nsamp > available: raise RuntimeError, "ring buffer underflow"
		x = numpy.zeros((self.channels(), nsamp), dtype=self.buf.dtype)
		n = min([nsamp, self.samples() - self.readhead])
		x[:, :n] = self.buf[:, self.readhead:self.readhead+n]
		self.readhead = (self.readhead + n) % self.samples()
		m = max([0, nsamp - n])
		x[:, n:n+m] = self.buf[:, :m]
		self.readhead += m
		return x

	def forget(self, nsamp):
		nsamp = min([nsamp, self.to_read()])
		self.readhead = (self.readhead + n) % self.samples()
