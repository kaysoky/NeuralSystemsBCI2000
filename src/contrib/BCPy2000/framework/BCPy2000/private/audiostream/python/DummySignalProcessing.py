import numpy
import SigTools

#################################################################
#################################################################

class BciSignalProcessing(BciGenericSignalProcessing):	

	#############################################################

	def Construct(self):
		parameters = [
		]
		states = [
		]
		return (parameters, states)

	#############################################################
	def Preflight(self, sigprops):
		pass
				
	#############################################################

	def Initialize(self, indim, outdim):
		self.filt = SigTools.causalfilter(freq_hz=0.5, type='highpass', samplingfreq_hz=self.samplingrate())
				
	#############################################################

	def Process(self, sig):
		return sig #self.filt(sig)
	
#################################################################
#################################################################
