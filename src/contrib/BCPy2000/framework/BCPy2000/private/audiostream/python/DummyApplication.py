from AppTools.StateMonitors import addstatemonitor, addphasemonitor

#################################################################
#################################################################

class BciApplication(BciGenericApplication):
				
	#############################################################

	def Preflight(self, sigprops):
		self.screen.setup(frameless_window=False)
				
	#############################################################

	def Initialize(self, indims, outdims):
		addstatemonitor(self, 'Stream1')
		addstatemonitor(self, 'Stream2')		
		addstatemonitor(self, 'CurrentTrial')
		addstatemonitor(self, 'TargetStream')
		
	#############################################################
		
#################################################################
#################################################################
