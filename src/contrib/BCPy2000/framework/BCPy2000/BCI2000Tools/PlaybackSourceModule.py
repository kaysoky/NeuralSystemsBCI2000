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
import numpy,os
from BCI2000Tools.FileReader import bcistream

class PlaybackError(EndUserError): pass

#################################################################
#################################################################

class BciSource(BciGenericSource):

	#############################################################

	def Description(self):
		return 'plays back a pre-recorded .dat file in "slave" mode'
	
	#############################################################

	def Construct(self):
		parameters = [
			"Source:Playback string PlaybackFileName= % % % % // play back the named BCI2000 file (inputfile)",
			"Source:Playback string PlaybackStart= 00:00:00.000 % % % // offset at which to start",
		]
		states = [
			"SignalStopRun 1 0 0 0",
		]
				
		return (parameters, states)
		
	#############################################################
	def Preflight(self, inprop):
		fn = self.params['PlaybackFileName']
		fn = fn.replace('$DATA'+os.path.sep, os.path.realpath(os.path.join(self.data_dir, '..'))+os.path.sep)
		try: self.stream = bcistream(fn)
		except Exception, e: raise PlaybackError(str(e))
		self.blocksize = int(self.params['SampleBlockSize'])
		self.master = int(self.params['EnslavePython']) != 0
		nch = int(self.params['SourceCh'])
		pbnch = self.stream.channels()
		if nch != pbnch:
			raise PlaybackError, 'mismatch between number of channels in SourceCh parameter (%d) and playback file (%d)' % (nch,pbnch)
		fs = self.samplingrate()
		pbfs = self.stream.samplingrate()
		if fs != pbfs:
			raise PlaybackError, 'mismatch between sampling rate in SamplingRate parameter (%gHz) and playback file (%gHz)' % (fs,pbfs)
		
		self.out_signal_props['Type'] = self.stream.headline.get('DataFormat', 'float32')
		# default data format is actually int16, but float32 is safe to cast the other formats into 
		
	#############################################################

	def StartRun(self):
		self.stream.seek(self.params['PlaybackStart'])
		self.states['SignalStopRun'] = 0
		print "\nplaying back:"
		print self.stream
		
	#############################################################

	def Process(self, sig):
		if int(self.states['Running']) == 0:
			return sig * 0
		
		if self.states['SignalStopRun']:
			self.states['Running'] = 0
			self.states['SignalStopRun'] = 0
		
		newsig,states = self.stream.decode(self.blocksize)
		if newsig.shape[1] < self.blocksize:
			self.states['SignalStopRun'] = 1
			return sig * 0
		if self.stream.tell() >= self.stream.samples():
			self.states['SignalStopRun'] = 1
		
		if self.master:
			for k in self.states.keys():
				if not k in ('Running', 'Recording', 'AppStartTime', 'StimulusTime', 'SourceTime') and states.has_key(k):
					self.states[k] = int(numpy.asarray(states[k]).flat[-1])
		
		return newsig
		
#################################################################
#################################################################
