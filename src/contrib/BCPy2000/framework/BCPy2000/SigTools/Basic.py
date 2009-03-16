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
	'getfs', 'msec2samples', 'samples2msec', 
	'ampmod', 'wavegen', 'sinewave',
	'squarewave', 'trianglewave',
	'fft2ap', 'ap2fft',
	'toy', 'reconstruct',
]
import numpy
from scipy.signal import fft,ifft,hanning
import scipy.signal.waveforms
from NumTools import isnumpyarray,project,trimtrailingdims

class ArgConflictError(Exception): pass

def getfs(obj, defaultVal=None):
	"""
	Infer the sampling frequency from <obj>. <obj> may simply be the numerical
	sampling-frequency value, or it may be an object in which the sampling
	frequency is stored in obj.samplingfreq_hz (like SigTools.causalfilter
	objects),  obj.fs (like WavTools.wav objects) or obj.params['SamplingRate']
	(like BCPy2000 objects).
	"""###
	fs = None
	if isinstance(obj, float) or isinstance(obj, int): fs = obj
	elif hasattr(obj, 'samplingfreq_hz'): fs = obj.samplingfreq_hz
	elif hasattr(obj, 'fs'): fs = obj.fs
	elif hasattr(obj, 'params'): fs = obj.params
	if isinstance(fs, dict) and fs.has_key('SamplingRate'): fs = fs['SamplingRate']
	if isinstance(fs, str) and fs.lower().endswith('hz'): fs = fs[:-2]
	if fs == None: return defaultVal
	return float(fs)
	
def msec2samples(msec, samplingfreq_hz):
	"""
	Converts milliseconds to the nearest integer number of samples given
	the specified sampling frequency.
	"""###
	fs = getfs(samplingfreq_hz)
	if msec==None or fs==None: return None
	if isinstance(msec, numpy.ndarray): msec = msec.astype(numpy.float64)
	else: msec = float(msec)
	return numpy.round(float(fs) * msec / 1000.0)

def samples2msec(samples, samplingfreq_hz):
	"""
	Converts samples to milliseconds given the specified sampling frequency.
	"""###
	fs = getfs(samplingfreq_hz)
	if samples==None or fs==None: return None
	if isinstance(samples, numpy.ndarray): samples = samples.astype(numpy.float64)
	else: samples = float(samples)
	return 1000.0 * samples / float(fs)

def ampmod(w, freq_hz=1.0,phase_rad=None,phase_deg=None,amplitude=0.5,dc=0.5,samplingfreq_hz=None,duration_msec=None,duration_samples=None,axis=None,waveform=numpy.sin):
	"""
	Return a copy of <w> (a numpy.ndarray or a WavTools.wav object) in which
	the amplitude is modulated sinusoidally along the specified time <axis>.
	
	Default phase is such that amplitude is 0 at time 0, which corresponds to
	phase_deg=90 if <waveform> follows sine phase. To change this, specify
	either <phase_rad> or <phase_deg>.
	
	Uses wavegen()
	"""###
	if isnumpyarray(w): y = w
	elif hasattr(w, 'y'): y = w.y
	else: raise TypeError, "don't know how to handle this kind of carrier object"

	if samplingfreq_hz==None: samplingfreq_hz = getfs(w)	
	if phase_rad==None and phase_deg==None: phase_deg = 90.0
	if duration_samples==None and duration_msec==None: duration_samples = project(y,0).shape[0]
	envelope = wavegen(freq_hz=freq_hz,phase_rad=phase_rad,phase_deg=phase_deg,amplitude=amplitude,dc=dc,samplingfreq_hz=samplingfreq_hz,duration_msec=duration_msec,duration_samples=duration_samples,axis=axis,waveform=waveform)
	envelope = project(envelope, len(y.shape)-1)
	y = y * envelope
	if isnumpyarray(w): w = y
	else: w.y = y
	return w
	
def wavegen(freq_hz=1.0,phase_rad=None,phase_deg=None,amplitude=1.0,dc=0.0,samplingfreq_hz=None,duration_msec=None,duration_samples=None,axis=None,waveform=numpy.cos,container=None):
	"""
	Create a signal (or multiple signals, if the input arguments are arrays)
	which is a sine function of time (time being defined along the specified
	<axis>).
	
	Default phase is 0, but may be changed by either <phase_deg> or <phase_rad>
	(or both, as long as the values are consistent).
	
	Default duration is 1000 msec, but may be changed by either <duration_samples>
	or <duration_msec> (or both, as long as the values are consistent).
	
	A <container> object may be supplied: if so, it should be a WavTools.wav
	object. container.fs is then used as a fallback in case <samplingfreq_hz> is
	unspecified, the <axis> is set to 0, and the signal is put into container.y
	
	If <duration_samples> is specified and <samplingfreq_hz> is not, then the
	sampling frequency is chosen such that the duration is 1 second, so <freq_hz>
	can be interpreted as cycles per signal.

	The default <waveform> function is numpy.cos which means that amplitude, phase
	and frequency arguments can be taken straight from the kind of dictionary
	returned by fft2ap() for an accurate reconstruction.
	"""###
	fs = getfs(samplingfreq_hz)
	if fs == None and container != None: fs = getfs(container)
	for j in range(0,2):
		for i in range(0,2):
			if duration_msec==None:
				duration_msec = samples2msec(duration_samples, fs)
			if duration_samples==None:
				duration_samples = msec2samples(duration_msec, fs)
				if duration_samples != None:
					duration_msec = samples2msec(duration_samples, fs)
			if fs==None and duration_samples!=None and duration_msec!=None: fs = 1000.0 * float(duration_samples) / float(duration_msec)
			if fs==None and duration_samples!=None: fs = float(duration_samples)
			if fs==None and duration_msec!=None: fs = float(duration_msec)
		if duration_msec==None: duration_msec = 1000.0
	duration_sec = duration_msec / 1000.0
	duration_samples = float(round(duration_samples))
	if duration_msec != samples2msec(duration_samples,fs) or duration_samples != msec2samples(duration_msec,fs):
		raise ArgConflictError, "conflicting duration_samples and duration_msec arguments"
	x = numpy.arange(0.0,duration_samples) * (2.0 * numpy.pi / duration_samples)
	freq_hz = trimtrailingdims(numpy.array(freq_hz,dtype='float'))
	if phase_rad == None and phase_deg == None: phase_rad = [0.0]
	if phase_rad != None:
		if not isnumpyarray(phase_rad) or phase_rad.dtype != 'float': phase_rad = numpy.array(phase_rad,dtype='float')
		phase_rad = trimtrailingdims(phase_rad)
	if phase_deg != None:
		if not isnumpyarray(phase_deg) or phase_deg.dtype != 'float': phase_deg = numpy.array(phase_deg,dtype='float')
		phase_deg = trimtrailingdims(phase_deg)
	if phase_rad != None and phase_deg != None:
		if phase_rad.shape != phase_deg.shape: raise ArgConflictError, "conflicting phase_rad and phase_deg arguments"
		if numpy.max(numpy.abs(phase_rad * (180.0/numpy.pi) - phase_deg) > 1e-10): raise ArgConflictError, "conflicting phase_rad and phase_deg arguments"
	if phase_rad == None:
		phase_rad = phase_deg
		phase_rad *= (numpy.pi/180.0)

	amplitude = trimtrailingdims(numpy.array(amplitude,dtype='float'))
	dc = trimtrailingdims(numpy.array(dc,dtype='float'))
	maxaxis = max(len(freq_hz.shape), len(phase_rad.shape), len(amplitude.shape), len(dc.shape)) - 1
	if axis==None:
		if project(freq_hz,0).shape[0]==1 and project(phase_rad,0).shape[0]==1 and project(amplitude,0).shape[0]==1 and project(dc,0).shape[0]==1:
			axis=0
		else:
			axis = maxaxis + 1
	maxaxis = max(axis, maxaxis)
		
	x = project(x,maxaxis).swapaxes(0,axis)
	x = x * (project(freq_hz,maxaxis) * duration_sec) # *= won't work for broadcasting here
	# if you get an error here, try setting axis=1 and transposing the return value ;-)
	x = x - (project(phase_rad,maxaxis))              # += won't work for broadcasting here
	x = waveform(x)
	x = x * project(amplitude,maxaxis)                # *= won't work for broadcasting here
	if numpy.any(dc.flatten()):
		x = x + project(dc,maxaxis)                   # += won't work for broadcasting here
	if container != None: container.y = project(x,1); container.fs = int(round(fs)); x = container
	return x
	
def fft2ap(X, samplingfreq_hz=2.0, axis=0):
	"""
	Given discrete Fourier transform(s) <X> (with time along the
	specified <axis>), return a dict containing a properly scaled
	amplitude spectrum, a phase spectrum in degrees and in radians,
	and a frequency axis (coping with all the fiddly edge conditions).
	
	The inverse of   d=fft2ap(X)  is  X = ap2fft(*d)
	"""###
	fs = getfs(samplingfreq_hz)	
	nsamp = int(X.shape[axis])
	biggest_pos_freq = float(nsamp/2)       # floor(nsamp/2)
	biggest_neg_freq = -float((nsamp-1)/2)  # -floor((nsamp-1)/2)
	posfreq = numpy.arange(0.0, biggest_pos_freq+1.0) * (float(fs) / float(nsamp))
	negfreq = numpy.arange(biggest_neg_freq, 0.0) * (float(fs) / float(nsamp))
	fullfreq = numpy.concatenate((posfreq,negfreq))
	sub = [slice(None)] * max(axis+1, len(X.shape))
	sub[axis] = slice(0,len(posfreq))
	X = project(X, axis)[sub]
	ph = -numpy.angle(X)
	amp = numpy.abs(X) * (2.0 / float(nsamp))
	if nsamp%2 == 0:
		sub[axis] = -1
		amp[sub] /= 2.0
	return {'amplitude':amp, 'phase_rad':ph, 'phase_deg':ph*(180.0/numpy.pi), 'freq_hz':posfreq, 'fullfreq_hz':fullfreq, 'samplingfreq_hz':fs, 'axis':axis}

def ap2fft(amplitude,phase_rad=None,phase_deg=None,samplingfreq_hz=2.0,axis=0,freq_hz=None,fullfreq_hz=None,nsamp=None):
	"""
	Keyword arguments match the fields of the dict
	output by that fft2ap() .

	The inverse of   d=fft2ap(X)  is  X = ap2fft(*d)
	"""###
	fs = getfs(samplingfreq_hz)	
	if nsamp==None:
		if fullfreq_hz != None: nsamp = len(fullfreq_hz)
		elif freq_hz != None:   nsamp = len(freq_hz) * 2 - 2
		else: nsamp = amplitude.shape[axis] * 2 - 2
	
	amplitude = project(numpy.array(amplitude,dtype='float'), axis)
	if phase_rad == None and phase_deg == None: phase_rad = numpy.zeros(shape=amplitude.shape,dtype='float')
	if phase_rad != None:
		if not isnumpyarray(phase_rad) or phase_rad.dtype != 'float': phase_rad = numpy.array(phase_rad,dtype='float')
		phase_rad = project(phase_rad, axis)
	if phase_deg != None:
		if not isnumpyarray(phase_deg) or phase_deg.dtype != 'float': phase_deg = numpy.array(phase_deg,dtype='float')
		phase_deg = project(phase_deg, axis)
	if phase_rad != None and phase_deg != None:
		if phase_rad.shape != phase_deg.shape: raise ArgConflictError, "conflicting phase_rad and phase_deg arguments"
		if numpy.max(numpy.abs(phase_rad * (180.0/numpy.pi) - phase_deg) > 1e-10): raise ArgConflictError, "conflicting phase_rad and phase_deg arguments"
	if phase_rad == None:
		phase_rad = phase_deg
		phase_rad *= (numpy.pi/180.0)

	f = phase_rad * 1j
	f = numpy.exp(f)
	f = f * amplitude
	f *= float(nsamp)/2.0
	sub = [slice(None)] * max(axis+1, len(f.shape))
	if nsamp%2 == 0:
		sub[axis] = -1
		f[sub] *= 2.0
	sub[axis] = slice((nsamp%2)-2, 0, -1)
	f = numpy.concatenate((f, numpy.conj(f[sub])), axis=axis)
	return f

def squarewave(theta, tol=1e-8):
	"""
	A square wave with its peaks and troughs in sine phase
	"""###
	x = numpy.sin(theta)
	if tol:
		f = x.flat
		f[numpy.where(numpy.abs(f)<tol)] = 0
	return numpy.sign(x)

def trianglewave(theta):
	"""
	A triangle wave with its peaks and troughs in sine phase
	"""###
	return scipy.signal.waveforms.sawtooth(theta+numpy.pi/2, width=0.5)

def toy(n=11,f=None,a=[1.0,0.1],p=0):
	"""
	Toy sinusoidal signals for testing fft2ap() and ap2fft().
	Check both odd and even <n>.
	"""###
	if f==None: f = [1.0,int(n/2)]
	return wavegen(duration_samples=n,samplingfreq_hz=n,freq_hz=f,phase_deg=p,amplitude=a,axis=1).transpose()
	
def reconstruct(ap,**kwargs):
	"""
	Check the accuracy of fft2ap() and wavegen() by reconstructing
	a signal as the sum of cosine waves with amplitudes and phases
	specified in dict ap, which is of the form output by fft2ap.
	"""###
	ap = dict(ap) # makes a copy, at least of the container dict
	ap['duration_samples'] = len(ap.pop('fullfreq_hz'))
	ap.update(kwargs)
	axis=ap.pop('axis', -1)
	extra_axis = axis+1
	for v in ap.values(): extra_axis = max([extra_axis, len(getattr(v, 'shape', []))])
	ap['freq_hz'] = project(ap['freq_hz'], extra_axis).swapaxes(axis,0)
	ap['axis'] = extra_axis
	r = wavegen(**ap)
	r = r.swapaxes(extra_axis, axis)
	r = r.sum(axis=extra_axis)
	return r

sinewave = wavegen
sinewave.__doc__ = \
	"""
	sinewave() is a deprecated synonym for wavegen().
	Meanwhile, however, other functions, such as trianglewave() and squarewave()
	can be used as the <waveform> argument, so this function does not strictly
	always return sine waves. Note also that the default <waveform> is and always
	has been numpy.cos rather than numpy.sin, so by default, a wave with a phase
	of 0 starts at its maximum.
	"""###
