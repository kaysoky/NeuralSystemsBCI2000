from BCI2000PythonApplication import *

import numpy
from SigTools import msec2samples,samples2msec
class tester(BciGenericApplication):
	# y is samples, x is seconds
	# y = fs * (x-x0) + offset
	def __init__(self, fs=500.4, offset_msec=10, jitter_msec=2, packetlen_msec=40, duration_sec=20):
		self.nominal = {}
		self.nominal['SamplesPerPacket'] = round(float(fs) * float(packetlen_msec)/1000.0)
		self.nominal['SamplesPerSecond'] = round(float(fs))
		self.fs = float(fs)
		self.offset_msec = float(offset_msec)
		self.jitter_msec = float(jitter_msec)
		npackets = int(1000.0 * float(duration_sec) / float(packetlen_msec))
		y = numpy.arange(npackets)*self.nominal['SamplesPerPacket']
		self.t = samples2msec(y, self) - self.offset_msec
		self.t += numpy.random.randn(*self.t.shape) * self.jitter_msec

		for ti in self.t: self.reg(ti)
		
		x = self.t / 1000.0
		ym = y.mean();
		xm = x.mean();
		xc = x - xm
		a = sum(xc * y) / sum(xc * xc)
		self.batch = {}
		self.batch['OffsetSamples'] = ym - a * xm
		self.batch['SamplesPerSecond'] = a
		
	def predict(self, t):
		return {
			'true': msec2samples(t, self),
			'online_est': self._regfs['OffsetSamples'] + self._regfs['SamplesPerSecond'] * elapsed_msec/1000.0,
			'batch_est':  self.batch['OffsetSamples']  + self.batch['SamplesPerSecond']  * elapsed_msec/1000.0,
		}
	def __repr__(self):
		return repr(self.__dict__)		
			
tester.reg = BciGenericApplication._regress_sampling_rate
