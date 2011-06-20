import os
import re
import random
import numpy
import pygame, pygame.locals
import thread
import socket
import AppTools.Displays
from AppTools.CurrentRenderer import VisualStimuli

import BCI2000Tools.SpellerTools
import LangTools.TextPrediction
from   LangTools.TextPrediction import * # for eval'ing LanguageModel parameter

##############################################################################################
##############################################################################################

class BciCodebook(object):
	def __init__(self, bci, rownumber):
		paramname = 'Codebooks'
		coldefs = BCI2000Tools.SpellerTools.CodebooksParamHeadings
		bci.assert_matrixlabels(paramname, zip(*coldefs)[0], 'column')
		row = bci.params[paramname][rownumber-1]
		desc = '%s parameter, row %d' % (paramname, rownumber)
		for label, type in coldefs:
			val = bci.params[paramname][rownumber-1, label]
			try: self.__dict__[label] = type(val)
			except: raise EndUserError('error in %s: could not convert "%s" to type %s' % (desc, val, type.__name__))
		
		if self.Name in [cb.Name for cb in bci.codebooks.values()]:
			raise EndUserError('error in %s parameter: codebook names are not unique' % paramname)
		
		self.Pages = self.Pages.split(' ')
		if self.Pages == ['']: self.Pages = []
		try: self.Pages = [int(p) for p in self.Pages]
		except: pass
		if True in [p == 0 for p in self.Pages]: raise EndUserError('0 is not a valid page number in "Pages" column of %s' % desc)
		try: bad = [p in bci.grid['Page'] for p in self.Pages].index(False)
		except ValueError: pass
		else:
			#print repr(self.Pages)
			if len(self.Pages): raise EndUserError('error in "Pages" column of %s: entry "%s" is not a valid page number as listed in the Grid parameter' % (desc, str(self.Pages[bad])))
		
		self.Matrix = self.Matrix.split(' ')
		if len(self.Matrix) < 5 or (self.Matrix[0], self.Matrix[1], self.Matrix[-1]) != ('{', 'matrix', '}'):
			raise EndUserError('error in "Matrix" column of %s: this should be a submatrix' % desc)
		mdesc = '%s matrix for codebook "%s"' % (paramname, self.Name)
		try: self.N, self.L = (int(self.Matrix[2]), int(self.Matrix[3]))
		except: raise EndUserError('error in %s: could not interpret matrix dimensions' % mdesc)
		size = self.N * self.L
		if size == 0: raise EndUserError('error in %s: matrix is empty' % mdesc)
		try: self.Matrix = [int(i) for i in self.Matrix[4:-1]]
		except: raise EndUserError('error in %s: could not interpret matrix content as integers' % mdesc)
		if len(self.Matrix) != size: raise EndUserError('error in %s: matrix has the wrong number of elements for its declared dimensions' % mdesc)
		self.Matrix = numpy.array(self.Matrix)
		self.Matrix.shape = (self.N, self.L)
		
		unique = set([tuple(x) for x in self.Matrix])
		if len(unique) != self.N: raise EndUserError('error in %s: rows are not unique' % mdesc)
		
		for p in self.Pages:
			ntargets = len(bci.pages[p])
			if self.N != ntargets:
				raise EndUserError('error in %s: for use on page %d, the matrix should have %d rows, but it has %d' % (mdesc, p, ntargets, self.N))
					

		try: self.RandomizeSpace = tuple([int(i) for i in self.RandomizeSpace.split(' ')])
		except: self.RandomizeSpace = tuple([i.strip() for i in self.RandomizeSpace if len(i.strip())])
		if self.RandomizeSpace == (1,): self.RandomizeSpace = (1,) * self.N
		elif self.RandomizeSpace == (0,): self.RandomizeSpace = None
		elif len(self.RandomizeSpace) == 1: self.RandomizeSpace = tuple(str(self.RandomizeSpace[0]))
		if self.RandomizeSpace != None and len(self.RandomizeSpace) != self.N:
			raise EndUserError('error in %s: if the RandomizeSpace entry is not 0 or 1 it should be a list of %d characters (= number of symbols in the codebook)' % (desc, self.N))

		try: self.RandomizeTime = tuple([int(i) for i in self.RandomizeTime.split(' ')])
		except: self.RandomizeTime = tuple([i.strip() for i in self.RandomizeTime if len(i.strip())])
		if self.RandomizeTime == (1,): self.RandomizeTime = (1,) * self.L
		elif self.RandomizeTime == (0,): self.RandomizeTime = None
		elif len(self.RandomizeTime) == 1: self.RandomizeTime = tuple(self.RandomizeTime[0])
		if self.RandomizeTime != None and len(self.RandomizeTime) != self.L:
			raise EndUserError('error in %s: if the RandomizeTime entry is not 0 or 1 it should be a list of characters the same length as the length of the codebook (%d)' % (desc, self.L))

		self.LastColumn = None
		self.ColumnOrder = []
		self.RowOrder = []

	def	step(self, reset = False):
		if len(self.ColumnOrder) == 0 or reset:
			self.ColumnOrder = range(1, 1+self.L)
			if self.RandomizeTime != None and self.L > 1:
				done = {}
				self.ColumnOrder = numpy.array(range(1, 1+self.L))
				for ident in self.RandomizeTime:
					if ident in done: continue
					done[ident] = 1
					mask = numpy.array([i==ident for i in self.RandomizeTime])
					if mask[0]:
						# This means that the first column is a candidate to be shuffled. Therefore, make sure that it is not equal to the LastColumn.
						new_mask = mask & (self.ColumnOrder != self.LastColumn) # Exclude LastColumn.
						subset = self.ColumnOrder[new_mask] # This subset does not contain the LastColumn.
						random_index = numpy.random.random_integers(len(subset))-1 # Randomly select a value to switch with the first column.
						if new_mask[0]:
							subset[0], subset[random_index] = subset[random_index], self.ColumnOrder[0] # Swap.
						else:
							self.ColumnOrder[0], subset[random_index] = subset[random_index], self.ColumnOrder[0] # Swap.
						# These two cases are necessary because depending on the mask, subset will either write over the first element of self.ColumnOrder, or it won't.
						self.ColumnOrder[new_mask] = subset # Write swapped values back to ColumnOrder, unmasking.
						mask[0] = False # Remove first column from mask.
					subset = self.ColumnOrder[mask]
					numpy.random.shuffle(subset)
					self.ColumnOrder[mask] = subset
				self.ColumnOrder = self.ColumnOrder.tolist()
					
			if self.RandomizeSpace != None and self.N > 1:
				# NB: spatial randomization breaks playbackability and entails loss of information
				# (TODO: to fix this, maybe record numpy's random seed as a state var and then set
				# it back from that state variable?)
				done = {}
				for ident in self.RandomizeSpace:
					if ident in done: continue
					done[ident] = 1
					mask = numpy.array([i==ident for i in self.RandomizeSpace])
					subset = self.Matrix[mask]
					numpy.random.shuffle(subset)
					self.Matrix[mask] = subset
		
		self.LastColumn = self.ColumnOrder.pop(0)
		return self.LastColumn
				
##############################################################################################
##############################################################################################

class BciApplication(BciGenericApplication):

	##########################################################################################
	
	def Description(self):
		return "Python implementation of an updated visual grid speller interface"
	
	##########################################################################################
	
	def Construct(self):

		self.require_version(17374)
		
		# import codebook definitions
		import mpiCodes

		if AppTools.Displays.number_of_monitors() == 1: defaultsize = 0.8
		else: defaultsize = 1.0

		# default (showcase) two-page grid
		grid = \
			BCI2000Tools.SpellerTools.GridParam("""\
			  A    B    C    D    E    F 
			                             
			  G    H    I    J    K    L 
			                             
			  M    N    O    P    Q    R 
			                             
			  S    T    U    V    W    X 
			                             
			  Y    Z    .    ,    ?    ! 
			                             
			  :    /    @   BKSP SPC  123
			""") + \
			BCI2000Tools.SpellerTools.GridParam("""\
			            %    #    $    / 
			                             
			            7    8    9    * 
			                             
			            4    5    6    - 
			                             
			            1    2    3    + 
			                             
			            0    .    =   ABC
			""") ###
		grid[0]['SPC'].command  = 'write(" ")'
		grid[0]['BKSP'].command = 'backspace'
		grid[0]['123'].command  = 'goto(2)'
		grid[1]['ABC'].command  = 'goto(1)'
		
		params = [
		
			str(grid),

			"PythonApp:Stimulus int     StartPage=            1     1     %   %   // ",
			"PythonApp:Stimulus int     StimulusEvent=        1     1     1   4   // stimulus event type: 1 flash, 2 horizontal-vertical rectangle flip, 3 h-v rect flip with change-blindness, 4 rainbow (enumeration)",
			"PythonApp:Stimulus matrix  Colors= { Background Low High } {Red Green Blue } 0.0 0.0 0.0   0.2 0.2 0.2    1.0 1.0 1.0    0.0 0.0 1.0 // colour scheme",
			"PythonApp:Stimulus matrix  RainbowColors= {Color1 Color2 Color3 Color4 Color5 Color6 Color7} {Name Red Green Blue}  Red 1.0 0.0 0.0   Orange 1.0 0.65 0.0   Yellow 1.0 1.0 0.0   Green 0.0 0.5 0.0   Blue 0.0 0.0 1.0  Indigo 0.29 0.0 0.51   Violet 0.93 0.51 0.93   0.0 0.0 1.0 // colour scheme for rainbow stimulus type",
			"PythonApp:Stimulus string  TargetFont=           %     %     %   %   // target font name (leave blank for default monospaced)",
			"PythonApp:Stimulus float   ScaleText=            1.0   1.0   0.0 %   // scaling factor for text stimuli on the grid",
			"PythonApp:Stimulus float   ScaleImages=          1.0   1.0   0.0 %   // scaling factor for image stimuli on the grid",
			"PythonApp:Stimulus int     FlashFrames=         10    10     1   %   // number of frames to stay highlighted during \"flash\" stimulus event",
			"PythonApp:Stimulus float   SOAMsec=            250   250     1   %   // stimulus onset asynchrony (period of stimulus events) in msec",
			"PythonApp:Stimulus int     PreTrialMsec=       500   500     1   %   // pre-trial (stimulus sequence) pause in msec",
			"PythonApp:Stimulus int     PostTrialMsec=     3500  3500     1   %   // post-trial (stimulus sequence) pause in msec",
			"PythonApp:Stimulus int     TransitionTimeLock=   0     0     0   2   // timelock presentation phases?: 0 no, 1 to packets, 2 to frames (enumeration)",
			"PythonApp:Stimulus float   SoundVolume=          1.0   1.0   0   1   // master sound volume from 0.0 to 1.0",
			#"PythonApp:Window   int     ScreenId=            -1    -1     %   %   // on which screen should the stimulus window be opened (zero-based: use -1 for last)",
			#"PythonApp:Window   float   WindowSize=           "+str(defaultsize)+"   1.0   0.0 1.0 // size of the stimulus window, proportional to the screen",
			"PythonApp:Window   float   WindowWidth=        800   800     1   %   // Width of the stimulus window",
			"PythonApp:Window   float   WindowHeight=       600   600     1   %   // Height of the stimulus window",
			"PythonApp:Window   float   WindowLeft=           0     0     0   %   // Left position of the stimulus window",
			"PythonApp:Window   float   WindowTop=            0     0     0   %   // Top position of the stimulus window",
			"PythonApp:Debug    int     SoundTest=            0     0     0   3   // use sound to test the pipeline?: 0 no, 1 mono, 2 stereo, 3 surround (enumeration)",
			"PythonApp:Debug    float   SpatialSensitivity=   0.5   0.5   0   %   // the extent to which neighbouring grid locations cause false alarms in the sound test",
			"PythonApp:Task     int     BlocksPerRun=         1     1     1   %   // number of times to repeat the exercise",
			"PythonApp:Task     int     TrialsPerBlock=    1000  1000     1   %   // maximum number of letters to spell",
			"PythonApp:Task     string  TextToSpell=          THE%20QUICK%20BROWN%20FOX%20JUMPS%20OVER%20THE%20LAZY%20DOG     %     %   %   // string to copy-spell (leave blank for free-spelling)",
			"PythonApp:Task     string  TextResult=           %     %     %   %   // initial content of the text result field",
			#"PythonApp:Task     int     ForReal=              0     0     0   1   // emit keystrokes? (boolean)",
			"PythonApp:Task     int     VisualCue=            0     0     0   1   // Display a visual cue in the matrix for the next letter to spell (boolean)",
			"PythonApp:Task     int     VisualCueDuration= 3000  3000    0 3000   // Time/ms to display visual cue",
			"PythonApp:Task     matrix  VisualCueColor= {High} {Red Green Blue} 1.0 1.0 0.0 // Color for visual cue in RGB values",
			"PythonApp:Task     string  DestinationAddress=   %     %     %   %   // network address for speller output in IP:port format",


			# default (showcase) sets of codebooks
			str(
				BCI2000Tools.SpellerTools.CodebooksParam(mpiCodes.codes36).set('Pages', 1) +
				BCI2000Tools.SpellerTools.CodebooksParam(mpiCodes.codes20).set('Pages', 2) +
				BCI2000Tools.SpellerTools.CodebooksParam(mpiCodes.codes72).set('Pages', []) +
				BCI2000Tools.SpellerTools.CodebooksParam(mpiCodes.codes4).set('Pages', []) +
				BCI2000Tools.SpellerTools.CodebooksParam(mpiCodes.codes9).set('Pages', [])
			) + "  % % %  // ",
				
			"EncDec:Encoding    int    MinEpochsPerTrial=    4     4     1   %   // minimum number of epochs per attempt to transmit/receive one letter",
			"EncDec:Encoding    int    MaxEpochsPerTrial=   72    72     1   %   // maximum number of epochs per attempt to transmit/receive one letter",
			"EncDec:Encoding    float  ProbabilityThreshold= 0.8   0.8   0.5 1.0 // ",

			"EncDec:Decoding    float  MinimumProbability=   0.0   0.0   0.0 1.0 // minimum probability in predictive distributions (if 0, a small value is computed automatically)",
			"EncDec:Decoding    float  SmoothingExponent=    1.0   1.0   0.0 1.0 // exponent for crudely post-smoothing predictive distributions",		
			"EncDec:Decoding    string LanguageModel=        %     %     %   %   // Python command for creating/loading language model object",
		]
	
		states = [
			"Repetition       5  0 0 0",
			"Epoch            8  0 0 0",
			"Codebook         5  0 0 0",
			"CodebookColumn   8  0 0 0",
			"Page             3  0 0 0",
			"TargetBitValue   2  0 0 0",
			"TargetCode       8  0 0 0",
			"ResultCode       8  0 0 0",
			"RainbowColorCode 8  0 0 0",
		]
		
		self.udp_socket = socket.socket(type=socket.SOCK_DGRAM)
			
		return params,states
		
	##########################################################################################
	
	def Preflight(self, in_signal_props):
		#print "Preflight"
		
		self.bci2000_dialog_encoding = 'latin-1'
		
		self.event_type = {'1':'donchin', '2':'hvrect', '3':'hvrect_cb', '4':'rainbow'}[self.params['StimulusEvent']]

		self.assert_matrixlabels('Colors', ('Red', 'Green', 'Blue'), 'column')
		if self.event_type == 'rainbow':
			if not self.params['Colors'].matrixlabels()[0][:2] == ['Background', 'Low']:
				raise EndUserError("Colors parameter must have at least 2 rows with headings 'Background' and 'Low'")
			self.assert_matrixlabels('RainbowColors', ('Name', 'Red', 'Green', 'Blue'), 'column')
			if len(self.params['RainbowColors']) < 1:
				raise EndUserError('At least one color in the RainbowColors matrix parameter must be defined for StimulusEvent type rainbow')
			self.rainbowcolors = {}
			for item in self.params['RainbowColors'].matrixlabels()[0]:
				cname = self.params['RainbowColors'][item][0]
				self.rainbowcolors[cname] = [max(0.0, min(1.0, float(x))) for x in self.params['RainbowColors'][item][1:]]
		else:
			self.assert_matrixlabels('Colors', ('Background', 'Low', 'High'), 'row')

		self.colors = {}
		for lab in self.params['Colors'].matrixlabels()[0]:
			self.colors[lab] = [max(0.0, min(1.0, float(x))) for x in self.params['Colors'][lab,:]]

		self.visual_cue = (int(self.params['VisualCue']) == 1)
		if self.visual_cue:
			self.assert_matrixlabels('VisualCueColor', ('High',), 'row')
			self.assert_matrixlabels('VisualCueColor', ('Red', 'Green', 'Blue'), 'column')
			self.visual_cue_color = [max(0.0, min(1.0, float(x))) for x in self.params['VisualCueColor']['High']]
			self.visual_cue_msec = int(self.params['VisualCueDuration'])

		self.flashframes = int(self.params['FlashFrames'])

		self.ntargets = len(self.params['Grid'])
		
		maxtargets = 2 ** min(self.bits['ResultCode'], self.bits['TargetCode']) - 1
		if self.ntargets > maxtargets:
			raise EndUserError('too many targets defined:  current TargetCode and ResultCode state variable definitions allow for a maximum of %d' % maxtargets)
		
		# make sure the Grid parameter has the correct number of columns and correct column headings in correct order
		coldefs = BCI2000Tools.SpellerTools.GridParamHeadings
		self.assert_matrixlabels('Grid', zip(*coldefs)[0], 'column')

		# read Grid parameter entries and make sure they are of the correct type
		self.grid = {}
		for label, type in coldefs:
			self.grid[label] = []
			for x in self.params['Grid'][:, label]:
				try: x = type(x)
				except: raise EndUserError("Illegal value in %s column of Grid parameter: cannot cast %s as %s" % (label, repr(x), type.__name__))
				self.grid[label].append(x)
		self.grid['Status'] = ['invisible'] * self.ntargets
		self.grid['Stimulus'] = [None] * self.ntargets
		self.grid['Text'] = [None] * self.ntargets
		
		# Check to see whether each 'Display' string is actually an image filename
		self.grid['Text'] = list(self.grid['Display'])
		for i in range(self.ntargets):
			txt = self.grid['Display'][i]
			dirname = os.path.dirname(txt)
			stem,extn = os.path.splitext(os.path.basename(txt))
			if len(stem) > 0 and (len(dirname) > 0 or len(extn) > 1):
				if not os.path.isfile(txt): raise EndUserError('error in Grid parameter: file "%s" not found' % txt)
				self.grid['Display'][i] = pygame.image.load(txt)
				self.grid['Text'][i] = stem
			elif not isinstance(txt, unicode):
				self.grid['Display'][i] = self.grid['Text'][i] = txt.decode(self.bci2000_dialog_encoding)
		# self.grid is now a dictionary whose keys are properties ('Display', 'Page', etc) and whose values are lists
		
		# parse and check 'Page' numbers and links
		pp = self.grid['Page']
		spp = set(pp)
		self.pages = {}
		maxpage = 2 ** self.bits['Page'] - 1
		for p in spp:
			if p < 0 or p > maxpage: raise EndUserError('illegal page number % d (given current state definitions, numbers in the "Page" column of the Grid parameter may not be less than 0 or greater than %d)' % (p, maxpage))
			self.pages[p] = [i for i in range(len(pp)) if pp[i]==p]
		# self.pages is now a dictionary whose keys are 1-based integer page numbers (entries in the "Page" column of the Grid parameter)
		# and whose values are lists of 0-based indices to targets

		self.startpage = int(self.params['StartPage'])
		if self.startpage not in spp:
			raise EndUserError('Illegal StartPage value (does not appear in "Page" column of Grid parameter)')
		self.history = []
		
		# parse and "compile" commands in the 'Actions' column
		actions = self.Actions()
		pattern = re.compile(r'(?P<command>[^\(\)]*)\s*(\(\s*(?P<argstr>.*?)\s*\))?')
		# NB: this parser is very crude. There will be trouble using a close-bracket or semicolon anywhere within each command
		# argument.  If necessary a close-bracket can be written as '\x29' and a semicolon as '\x3B'.
		for i in range(self.ntargets):
			a = [x.strip() for x in self.grid['Action'][i].split(';') if len(x.strip())]
			actionlist = self.grid['Action'][i] = []
			for s in a:
				m = pattern.match(s)
				if m == None or m.end() < len(s): raise EndUserError('failed to parse command in Grid parameter: %s' % s)
				command,argstr = m.group('command'), m.group('argstr')
				if argstr == None: argstr = ''
				dbstr = 'command "%s" in "Action" column of Grid parameter, row %d'%(command, i+1)
				try: command = actions[command]
				except KeyError: raise EndUserError('unrecognized ' + dbstr)
				args = command(argstr=argstr, dbstr=dbstr)
				actionlist.append((command,args))
			if len(actionlist) == 0:
				actionlist.append((self.DoWrite, {'text':self.grid['Text'][i]}))
			self.grid['Text'][i] = ''.join([a[1]['text'] for a in actionlist if a[0]==self.DoWrite])
			
		self.minEpochs = int(self.params['MinEpochsPerTrial'])
		self.maxEpochs = int(self.params['MaxEpochsPerTrial'])
		
		self.codebooks = {}
		for i in range(len(self.params['Codebooks'])): self.codebooks[i+1] = BciCodebook(self, i+1)
		# self.codebooks is now a dictionary whose keys are 1-based integers corresponding to row numbers in the Codebooks parameter
		# and whose values BciCodebook objects

		self.alternatives = {}
		for p in self.pages:
			self.alternatives[p] = [cbnum for cbnum,cb in sorted(self.codebooks.items()) if p in cb.Pages]
			if len(self.alternatives[p]) == 0: raise EndUserError('page %d does not have any codebooks assigned to it in the Codebooks parameter' % p)
		# self.alternatives is now a dictionary whose keys are 1-based integer page numbers (entries in the "Page" column of the Grid parameter)
		# and whose values are lists of 1-based integers corresponding to row numbers in the Codebooks parameter
	
		#siz = float(self.params['WindowSize'])
		#screenid = int(self.params['ScreenId'])  # ScreenId 0 is the first screen, 1 the second, -1 the last
		##AppTools.Displays.fullscreen(scale=siz, id=screenid, frameless_window=(siz==1), hide_mouse=not int(self.params['SoundTest']))
		## only use a borderless window if the window is set to fill the whole screen
		screenid = -1  # ScreenId 0 is the first screen, 1 the second, -1 the last
		m = AppTools.monitor(screenid)
		w_width  = int(self.params['WindowWidth'])
		w_height = int(self.params['WindowHeight'])
		w_left   = int(self.params['WindowLeft'])
		w_top    = int(self.params['WindowTop'])
		m.rect = (w_left, m.height-w_height-w_top, w_width+w_left, m.height-w_top)
		AppTools.init_screen(m, frameless_window=False, hide_mouse=not int(self.params['SoundTest']))

		self.soa_msec = float(self.params['SOAMsec'])
		maxpacketsize = int(0.4 * self.samplingrate() * self.soa_msec / 1000.0)
		if int(self.params['SampleBlockSize']) > maxpacketsize:
			raise EndUserError('For SOAMsec=%g and SamplingRate=%g, SampleBlockSize should not exceed %d' % (self.soa_msec, self.samplingrate(), maxpacketsize))
		self.pre_trial_msec  = int(self.params['PreTrialMsec'])
		self.post_trial_msec = int(self.params['PostTrialMsec'])

		self.copyspelling = len(self.params['TextToSpell']) > 0
		w = self.params.get('ERPClassifierWeights',[])
		self.weightless = len(w) == 0 or (w.val==0).all()
		if self.weightless and not self.copyspelling:
			raise EndUserError("either supply TextToSpell for copy-spelling, or a matrix of ERPClassifierWeights - cannot leave both blank")

		for ch in self.params['TextToSpell']:
			try: n = self.grid['Text'].index(ch)
			except ValueError: raise EndUserError('impossible TextToSpell: character "%s" is not on the grid' % ch)
			p = self.grid['Page'][n]
			#if p != self.startpage: raise EndUserError('all characters in TextToSpell should appear on the StartPage (%d) whereas "%s" is on page %d' % (self.startpage, ch, p))
		
		self.write_access('TextResult')
		
		transition_lock_mode = int(self.params['TransitionTimeLock'])
		if transition_lock_mode == 0: self.lock_transitions(None)
		if transition_lock_mode == 1: self.lock_transitions('packet')
		if transition_lock_mode == 2: self.lock_transitions('frame')

		self.langmod = None
		self.mappings = {}
		lm = self.params['LanguageModel'].strip()
		if len(lm):
			try: self.langmod = eval(lm)
			except Exception, e: raise EndUserError('Python error while evaluating LanguageModel constructor command: %s' % str(e))

			loweronly,upperonly,both = self.langmod.cases()
			case_sensitive = both>0 or (loweronly>0 and upperonly>0)
			direct = {}
			indirect = {}
			leadsto = [set() for targetid in range(self.ntargets)]
			for pgid in self.pages:
				subset = self.pages[pgid]
				if case_sensitive: direct[pgid] = set([self.grid['Text'][targetid] for targetid in subset])
				else:      direct[pgid] = set([self.grid['Text'][targetid].lower() for targetid in subset] + [self.grid['Text'][targetid].upper() for targetid in subset])
				indirect[pgid] = set()
			for i in range(10):
				for targetid in range(self.ntargets):
					thispage = self.grid['Page'][targetid]
					txt = self.grid['Text'][targetid]
					if txt != None and len(txt):
						if case_sensitive:
							leadsto[targetid].add(txt)
						else:
							leadsto[targetid].add(txt.lower())
							leadsto[targetid].add(txt.upper())
					for a in self.grid['Action'][targetid]:
						if a[0] == self.DoGoto:
							targetpage = a[-1]['page']
							#if i == 0: print "target #%d: goto %d" % (targetid+1, targetpage)
							indirect[thispage] = indirect[thispage].union(direct[targetpage]).union(indirect[targetpage]) - direct[thispage]
							leadsto[targetid] = leadsto[targetid].union(direct[targetpage]).union(indirect[targetpage]) - direct[thispage]
			
			for pgid in self.pages:
				subset = self.pages[pgid]
				self.mappings[pgid] = [leadsto[targetid] for targetid in subset]
		
		#if int(self.params.get('ForReal', False)): import SendKeys; self.SendKeys = SendKeys.SendKeys
		if self.params['DestinationAddress'].find(':') > 0:
			self.udp_host,self.udp_port = self.params['DestinationAddress'].split(':')
			self.udp_port = int(self.udp_port)
		else:
			self.udp_host,self.udp_port = ('',0)

		def setCaptionThread():
			import win32gui as w, time
			hwnd = None
			while not hwnd:
				try:
					hwnd=w.FindWindow('pygame', 'Vision Egg')
				except Exception, e:
					hwnd=None
					time.sleep(1)
			w.SetWindowText(hwnd, 'PySpeller')
		thread.start_new_thread(setCaptionThread,())

	##########################################################################################

	def Initialize(self, in_signal_dims, out_signal_dims):
		#print "Initialize"
		
		self.screen.color = self.colors['Background']
		
		disp   = self.grid['Display']
		scale  = self.grid['Scale']
		row    = self.grid['Row']
		col    = self.grid['Column']
		page   = self.grid['Page']
		action = self.grid['Action']
		
		nrows = max(row) - min(row) + 1.0
		ncols = max(col) - min(col) + 1.0
		aspect = ncols/nrows # to change the aspect ratio of the tiles and hence the whole grid, multiply this by something
		self.screen.coords.anchor = 'center'
		gridbounds = self.screen.coords.copy()
		ttsbounds = self.screen.coords.copy()
		ttsbounds.anchor = 'top'
		ttsbounds.height /= 8
		gridbounds.anchor = 'lower right'
		#gridbounds.anchor = 'bottom'
		gridbounds.height -= ttsbounds.height		
		if len(self.params.get('TriggerChannel','')) > 0:
			if gridbounds.width < 200:
				raise EndUserError('Window not wide enough to add optical sync patch; increase window width to >= 200 pixels')
			gridbounds.width -= 100
			#ttsbounds.width -= 100
			ttsbounds.left += 100
		if int(self.params['ShowSignalTime']):
			gridbounds.width  -= 200
			gridbounds.height -= 50
			gridbounds.bottom += 50
		#gridbounds.width  = min(gridbounds.width,  gridbounds.height * aspect)
		#gridbounds.height = min(gridbounds.height, gridbounds.width  / aspect)
		self.tilesize = (gridbounds.width / ncols, gridbounds.height / nrows)
		#gridbounds.anchor = 'center'
		#gridbounds.width  *= (ncols - 2.0) / ncols
		#gridbounds.height *= (nrows - 2.0) / nrows
		
		#row = numpy.array(row); row -= row.min(); row /= row.max()
		#col = numpy.array(col); col -= col.min(); col /= col.max()
		row = numpy.array(row); row -= row.min(); row /= (row.max() + 1)
		col = numpy.array(col); col -= col.min(); col /= (col.max() + 1)
		
		scale_images = float(self.params['ScaleImages'])
		scale_text = float(self.params['ScaleText'])
		defaultfont = self.screen.monofont
		gridfont = self.params['TargetFont']
		if gridfont:
			found = self.screen.findfont([gridfont])
			if found: gridfont = found
			else: print "WARNING: failed to find font %s" % gridfont
		else: gridfont = defaultfont
		
		self.basefontsize = self.tilesize[1] * 0.35 * scale_text
		
		tsz=tuple([int(i) for i in self.tilesize])
		tile_width=self.tilesize[0]
		tile_height=self.tilesize[1]
		for i in range(self.ntargets):
			key = 'target%03d' % (i+1)
			pos = (gridbounds.left + gridbounds.width * col[i] + tile_width/2, gridbounds.top - gridbounds.height * row[i] - tile_height/2)
			size = int(round(self.basefontsize*scale[i]))
			visible = page[i]==self.startpage
			
			content = disp[i]
			if isinstance(content, pygame.Surface):
				width,height = content.get_size()
				aspect = float(width)/float(height)
				width,height = self.tilesize
				width  = min(width,  height * aspect)
				height = min(height, width  / aspect)
				size = (width*0.3*scale[i]*scale_images, height*0.3*scale[i]*scale_images)
				#print "creating image stimulus, content=",content,"pos=",pos
				s = self.stimulus(key, VisualStimuli.ImageStimulus, texture=content, position=pos, size=size, anchor='center', color=self.colors['Low'], on=visible)
			else:
				textpos = list(pos)
				# move period and comma up 15% in its space
				if content in ('.', ','): textpos[1] += self.tilesize[1] * 0.15
				#print "creating text stimulus, content=",content,"pos=",textpos
				s = self.stimulus(key, VisualStimuli.Text, text=content, position=textpos, font_name=gridfont, font_size=size, anchor='center', color=self.colors['Low'], on=visible)
			self.grid['Stimulus'][i] = s
			if self.event_type in ['hvrect', 'hvrect_cb']:
				s.color = self.colors['High']
				size = min(self.tilesize) * 0.6
				size = (size, size / 1.618)
				angle = random.randint(0,1) * 90.0
				s = self.stimulus(key.replace('target', 'rect'), VisualStimuli.Block, z=-1, size=size, position=pos, anchor='center', color=self.colors['Low'], on=visible, orientation=angle)
				if 'Rectangle' not in self.grid: self.grid['Rectangle'] = [None] * self.ntargets
				self.grid['Rectangle'][i] = s
					
		#for i in range(nrows):
			#for j in range(ncols):
				#pos = (gridbounds.left + j * tile_width + tile_width/2, gridbounds.top - i * tile_height - tile_height/2)
				#c = (random.random()/10+0.1, random.random()/10+0.1, random.random()/10+0.1)
				#key = 'tile%02d%02d' %(i+1,j+1)
				#s = self.stimulus(key, VisualStimuli.Block, z=-1, size=tsz, position=pos, anchor='center', color=c, on=True)

		self.current_target = None
		self.selections = []
		self.abort = False
		
		ttsbounds.anchor='center'
		ttsbounds.height -= 1;  ttsbounds.width -= 1
		linespacing = ttsbounds.height / 2
		fontsize = int(round(linespacing * 0.8))
		pos = ttsbounds.top - linespacing
		if self.copyspelling:
			self.stimulus('TTSBackground', VisualStimuli.Block, position=(ttsbounds.left,pos), anchor='lowerleft', size=(ttsbounds.width,linespacing-2), color=self.colors['High'])
			self.stimulus('TTSForeground', VisualStimuli.Text,  position=(ttsbounds.left,pos), anchor='lowerleft', font_name=defaultfont, font_size=fontsize, color=self.colors['Low'], text=self.params['TextToSpell'])
			pos -= linespacing
		self.stimulus('OutputBackground',  VisualStimuli.Block, position=(ttsbounds.left,pos), anchor='lowerleft', size=(ttsbounds.width,linespacing-2), color=self.colors['High'])
		self.stimulus('OutputForeground',  VisualStimuli.Text,  position=(ttsbounds.left,pos), anchor='lowerleft', font_name=defaultfont, font_size=fontsize, color=self.colors['Low'], text='')

		if len(self.params.get('TriggerChannel','')) > 0:
			self.stimulus('SyncPatch', VisualStimuli.Block, size=(40,40), position=self.screen.coords.map((-1,-1)), anchor='lowerleft', on=True, color=(0,0,0))
			
		self.init_volume(float(self.params['SoundVolume']))
		soundmode = int(self.params['SoundTest'])
		if soundmode:
			import WavTools
			
			d = 0.75 * self.soa_msec / 1000.0
			d = min(d, 0.6)
			w = WavTools.rise(d/2, fs=44100, hanning=True) % WavTools.fall(d/2, fs=44100, hanning=True)
			w.y *= (numpy.random.rand(*w.y.shape) - 0.5)
			self.p3sound = w
			d = WavTools.msec2samples(10, w)
			self.triggersound = w * 0 + (numpy.random.rand(d,1) - 0.5)
			if soundmode == 2:
				self.p3sound.y      = self.p3sound.y      * [1,0]
				self.triggersound.y = self.triggersound.y * [0,1]
			if soundmode == 3:
				self.p3sound.y      = self.p3sound.y      * [1,0,1,0]
				self.triggersound.y = self.triggersound.y * [0,1,0,1]
			self.sound = []
			for i in range(4): self.sound.append(WavTools.player(w))
				
			dot = self.stimulus('MouseFocus', VisualStimuli.Disc, radius=10, on=False, color=(1,0,0))
			self.manual_dot = False
		else:
			self.sound = None
		
		self.smoothexp = float(self.params['SmoothingExponent'])
		self.minprob   = float(self.params['MinimumProbability'])
		if self.minprob == 0.0: self.minprob = None
		
		self.statemonitors = {1:1,2:2,3:3,4:4,5:5,6:6}
		if int(self.params['ShowSignalTime']):
			m = self.addphasemonitor()
			m = self.addstatemonitor('Page', color=self.colors['High'])
			m = self.addstatemonitor('Codebook', color=self.colors['High'])
			m = self.addstatemonitor('Codebook Name', color=self.colors['High'])
			m.func = lambda bci: '%s' % getattr(bci.codebooks.get(bci.states['Codebook']), 'Name', ''); m.pargs = (self,)
			m = self.addstatemonitor('Repetition', color=self.colors['High'])
			m = self.addstatemonitor('Epoch', color=self.colors['High'])
			m = self.addstatemonitor('CodebookColumn', color=self.colors['High'])
			m = self.addstatemonitor('TargetCode', color=self.colors['High'])
			m = self.addstatemonitor('TargetBitValue', color=self.colors['High'])
		
		self.pages[0] = self.pages[1] #TODO
		self.codebooks[0] = self.codebooks[1] #TODO
		print "TODO: get rid of this scary hack and find the reason why self.states.Page and self.states.Codebook go to 0 after a few packets please Collin"
		
		self.transient('ResultCode', manual=True)
		self.ResetOutput()

	##########################################################################################

	def glarp(self, filename): # &&&
		import SigTools
		m = self.decoder.model
		self.decoder.model = self.params.LanguageModel
		SigTools.pickle(self.decoder, filename)	
		self.decoder.model = m
	
	##########################################################################################
	
	def StartRun(self):
		#print "StartRun"
		#numpy.random.seed(1234) # &&&
		#print 'remember to remove all parts marked &&& in BciApplication.py and TextPrediction.py '
		
		self.states['Page'] = self.startpage
		self.states['Codebook'] = self.alternatives[self.startpage][0]
		self.states['CodebookColumn'] = 0
		self.ComputeGridStates()
		self.penultimate = False
		
		self.previous_trialsperblock = self.params['TrialsPerBlock']
		if self.copyspelling: nchars = len(self.params['TextToSpell']) - len(self.params['TextResult'])
		else:                 nchars = int(self.params['TrialsPerBlock']) - len(self.params['TextResult'])
		self.params['TrialsPerBlock'] = min(int(self.params['TrialsPerBlock']), nchars if nchars > 0 else 0)
		print "setting TrialsPerBlock to",self.params['TrialsPerBlock']
		
		self.decoder = None
		self.previous_result_arrived = 0.0
		self.nepochs = []

		self.ResetOutput()
		
	##########################################################################################
	
	def StopRun(self):
		#print "StopRun, params['TrialsPerBlock']=",self.params['TrialsPerBlock']
		self.params['TrialsPerBlock'] = self.previous_trialsperblock
		if not self.weightless: self.params['TextResult'] = self.output.encode(self.bci2000_dialog_encoding)
		
	##########################################################################################
	
	def Phases(self):
		#print "Phases"

		if self.penultimate or self.abort: next='grace'
		else: next = 'highlight'

		cue = True if (self.copyspelling and self.visual_cue) else False

		self.phase(name='pre',        duration=self.pre_trial_msec,   next='cue' if cue else 'highlight')
		if cue:
			self.phase(name='cue',      duration=self.visual_cue_msec,  next='post_cue')
			self.phase(name='post_cue', duration=500,                  next='highlight')
		self.phase(name='highlight',  duration=self.soa_msec,         next=next)
		self.phase(name='grace',      duration=500,                  next='post')
		self.phase(name='post',       duration=self.post_trial_msec,  next='pre')

		self.design(start='pre', new_trial='pre')

	##########################################################################################

	def Transition(self, phase):

		#print 'Transitioning to ',phase,'Page is', self.states.Page
		self.penultimate = False

		if self.changed('CurrentBlock') and self.states['CurrentBlock'] == 1 and self.states['CurrentTrial'] == 1:
			self.ResetOutput()


		if phase == 'highlight':
			pgid = self.states['Page']
			subset = self.pages[pgid]
			cbid = self.states['Codebook']
			cb = self.codebooks[cbid]

			self.states['Epoch'] += 1
			self.penultimate = self.states['Epoch'] >= self.maxEpochs - 1
			self.states['Repetition'] = (self.states['Epoch']-1) // cb.L + 1

			if self.states['Epoch'] == 1:
				reset_codebook = True
			else:
				reset_codebook = False
			self.states['CodebookColumn'] = cb.step(reset_codebook) # Get next column index from (possibly randomized) codebook. If the columns have been exhausted, a new (possibly randomized) codebook is generated.
			col = cb.Matrix[:, self.states['CodebookColumn']-1] # Get column from index.
			self.decoder.new_column(col)
			tc = self.states['TargetCode']
			if tc and (tc-1) in subset:
				tbv = col[subset.index(tc-1)]
				self.states['TargetBitValue'] = {0:2, 1:1}[tbv]
			else:
				self.states['TargetBitValue'] = 0

			self.ComputeGridStates()

			if self.event_type in ['hvrect', 'hvrect_cb']:
				for target in subset:
					if self.grid['Status'][target] == 'highlighted':
						stim = self.grid['Rectangle'][target]
						stim.orientation = 90 - stim.orientation
		
			if self.event_type in ['rainbow']:
				self.rainbowcolor_idx = int(random.random() * len(self.rainbowcolors))
				self.states['RainbowColorCode'] = self.rainbowcolor_idx
				
			if self.sound != None:
				dot = self.stimuli['MouseFocus']
				if dot.on:
					sensitivity_radius = min(self.tilesize)* float(self.params['SpatialSensitivity'])
					scale_factor = 3.0
					p = numpy.array(dot.position)
					d = numpy.array([numpy.sum((p - self.grid['Stimulus'][target].position)**2) for target in subset])
					d = numpy.exp(-0.5 * d / sensitivity_radius ** 2)
					d /= len(d)
					d = d * [int(self.grid['Status'][target] == 'highlighted') for target in subset]
					amp = d.sum() * scale_factor
				else:
					amp = 0
				amp *= 20
				self.sound[0].play(w=self.triggersound + amp * self.p3sound)
				self.sound.append(self.sound.pop(0))

		else:
			self.states['Epoch'] = 0
			self.states['CodebookColumn'] = 0
			self.states['Repetition'] = 0
			self.states['TargetBitValue'] = 0
			self.codebook = []
			self.ComputeGridStates()
		
		if phase == 'cue':
			if self.target != None:
				#print "giving visual cue for target %d ..."%(self.target)
				self.grid['Stimulus'][self.target].color=tuple([float(i) for i in self.visual_cue_color])
		
		if phase == 'pre':
			self.UpdateOutput()
			self.udp_queue = []
			self.abort = False
			self.previous_result_arrived = 0.0
			pgid = self.states['Page']
			if pgid == 0: return
			cbid = self.alternatives[pgid].pop(0)
			self.alternatives[pgid].append(cbid)
			self.states['Codebook'] = cbid
			
			context = self.output
			if self.langmod:
				context = self.langmod.prepend_context(context, '. ')
				context = self.langmod.match_case(context)
			labels = [self.grid['Display'][targetid] for targetid in self.pages[pgid]]
			self.decoder = LangTools.TextPrediction.Decoder(context=context, model=self.langmod,
				choices=self.pages[pgid], mapping=self.mappings.get(pgid,None), labels=labels,
				threshold=float(self.params['ProbabilityThreshold']), min_epochs=self.minEpochs, max_epochs=self.maxEpochs,
				minprob=self.minprob, exponent=self.smoothexp)

			if self.target == None:
				self.states['TargetCode'] = 0
			else:
				self.states['TargetCode'] = self.target + 1
				if self.sound != None and not self.manual_dot:
					dot = self.stimuli['MouseFocus']
					dot.position = self.grid['Stimulus'][self.target].position
					dot.on = True

		if phase == 'post':
			if self.states['ResultCode']:
				result = self.states['ResultCode'] - 1
				self.selections.append(result)
				for command,kwargs in self.grid['Action'][result]: command(**kwargs)
				self.acknowledge('ResultCode')
			self.UpdateOutput()
			self.ProcessUdpQueue()
			
		#print 'Transitioned to ',phase,'Page is', self.states.Page
		#print

	##########################################################################################
	
	def Process(self, sig):
		#print self.packet_count, self.current_presentation_phase, 'Page',self.states.Page
		if self.states.get('Ready', 0):
			p,msec = sig.flat
			if self.previous_result_arrived:
				gap = (msec - self.previous_result_arrived)
				rgap = gap / self.soa_msec
				if rgap < 0.5: raise RuntimeError('gap between results was too short, at %g * SOA = %g msec' % (rgap, gap))
				if rgap > 1.75: raise RuntimeError('gap between results was too long, at %g * SOA = %g msec' % (rgap, gap))
			self.previous_result_arrived = msec
			if self.decoder != None:
				result = self.decoder.new_transmission(p)
				if result != None:
					self.states['ResultCode'] = result+1 # lookup into appropriate page has already been done in decoder
					self.abort = True
					self.nepochs.append(self.decoder.L)
					
	##########################################################################################
	
	def Frame(self, phase):
	
		t = self.since('transition')

		for i in range(self.ntargets):
			stim = self.grid['Stimulus'][i]
			status = self.grid['Status'][i]
			
			stim.on = (status != 'invisible')
			if self.event_type in ['hvrect', 'hvrect_cb']:
				r = self.grid['Rectangle'][i]
				r.on = stim.on
				
				if self.event_type.endswith('_cb') and phase == 'highlight':
					if t['frames'] <= self.flashframes: self.screen.color, r.on = self.colors['High'], False
					else:                               self.screen.color, r.on = self.colors['Background'], True
					#if t['frames'] <= self.flashframes: self.screen.color = self.colors['Low']
					#else:                               self.screen.color = self.colors['Background']
					#if t['frames'] <= self.flashframes: r.size = (r.size[0], r.size[0])
					#else:                               r.size = (r.size[0], r.size[0] / 1.618)
			
			elif self.event_type in ['donchin', 'rainbow'] and phase != 'cue':
				if status == 'highlighted' and t['frames'] <= self.flashframes:
					if self.event_type == 'donchin':
						stim.color = self.colors['High']
					else:
						stim.color = self.rainbowcolors.values()[self.rainbowcolor_idx]
				else:
					stim.color = self.colors['Low']

		sync = self.stimuli.get('SyncPatch')
		if sync != None:
			if phase=='highlight' and t['frames'] <= 3: sync.color = (1,1,1)
			else: sync.color = (0,0,0)
		
	##########################################################################################
	
	def Event(self, phase, event):

		if event.type == pygame.locals.MOUSEBUTTONDOWN:
			if self.sound != None:
				p = (event.pos[0], self.screen.size[1]-event.pos[1])
				dot = self.stimuli['MouseFocus']
				r = numpy.array(p) - numpy.array(dot.position)
				self.manual_dot = True
				if dot.on == True and numpy.inner(r,r) <= dot.radius**2:
					dot.on = False
					if event.button > 1: self.manual_dot = False
				else:
					dot.on = True
					dot.position = p
				
	##########################################################################################
	
	def ComputeGridStates(self):
	
		pgid = self.states['Page']
		if pgid == 0: pgid = self.states['Page'] = self.startpage
		subset = self.pages[pgid]

		cbid = self.states['Codebook']
		if cbid == 0: cbid = self.states['Codebook'] = self.alternatives[pgid][0]
		cb = self.codebooks[cbid]
		
		for targetid in range(self.ntargets):
			self.grid['Status'][targetid] = 'invisible'

		col = self.states['CodebookColumn'] - 1
		for row in range(len(subset)):
			targetid = subset[row]
			if 0 <= col < cb.L and cb.Matrix[row, col]:
				self.grid['Status'][targetid] = 'highlighted'
			else:
				self.grid['Status'][targetid] = 'normal'
		
	##########################################################################################

	def ResetOutput(self):
		self.output = self.params['TextResult']
		if not isinstance(self.output, unicode): self.output = self.output.decode(self.bci2000_dialog_encoding)
		self.UpdateOutput()
		
	##########################################################################################
	
	def UpdateOutput(self):
						
		self.ComputeGridStates()
		outstim = self.stimuli['OutputForeground']
		
		if len(outstim.text): pixelsPerCharacter =  float(outstim.size[0]) / float(len(outstim.text))
		else: pixelsPerCharacter = 1 # TODO
		pixels2characters = lambda x: int(x / pixelsPerCharacter)
		characters2pixels = lambda x: int(round(x * pixelsPerCharacter))

		available = self.stimuli['OutputBackground'].size[0]
		if self.weightless: self.output = ' ' * (self.states['CurrentTrial'] - 1)
		outstim.text = self.output.split('\n')[-1]
		used = characters2pixels(len(outstim.text))
		if float(used)/float(available) > 0.9:
			outstim.text = outstim.text[-pixels2characters(available * 0.75):]
		
		self.target = None
		if self.copyspelling:
			tts = self.params['TextToSpell']
			if len(self.output) < len(tts):
				target = self.grid['Text'].index(tts[len(self.output)])
				if self.grid['Page'][target] == self.states['Page']: self.target = target
				else:
					gotos = [(a[-1][0], a[-1][-1].get('page', None)) for a in self.grid['Action']]
					match = (self.DoGoto,self.grid['Page'][target])
					if match in gotos: self.target = gotos.index(match)
					else: self.target = None
			
			chopped = len(self.output) - len(outstim.text)
			instim = self.stimuli['TTSForeground']
			instim.text = tts[chopped:]
			if self.weightless:
				outstim.text += instim.text[len(outstim.text)].replace(' ', '_')
				outstim.color = (1,0,0)
				outstim.position = instim.position

		if not self.weightless:
			outstim.text += '_'
			
	##########################################################################################
	
	def Actions(self):
		return {
			'write': self.DoWrite,
			'backspace': self.DoBackspace,
			'goto': self.DoGoto,
		}
		
	##########################################################################################
	
	def DoWrite(self, text=None, argstr=None, dbstr='speller command'):
		if argstr != None:
			try:
				text = eval(argstr)
				if isinstance(text, int): text = chr(text)
				if not isinstance(text, basestring): text = str(text)
				if not isinstance(text, unicode): text = text.decode(self.bci2000_dialog_encoding)
			except Exception, e: raise EndUserError('error in %s: python exception occurred while evaluating argument "%s": %s' % (dbstr, argstr, str(e)))
			return {'text':text}

		self.output += text
		#sk = getattr(self, 'SendKeys', None)
		#if sk != None: sk(str(text))
		#self.UdpSend(text)
		self.udp_queue.append(text)
		
	##########################################################################################
	
	def DoBackspace(self, argstr=None, dbstr='speller command'):
		if argstr != None:
			if len(argstr): raise EndUserError('error in %s: command takes no input arguments' % dbstr)
			return {}

		self.output = self.output[:-1]
		self.UdpSend('{BS}')
				
	##########################################################################################
	
	def DoGoto(self, page=None, argstr=None, dbstr='speller command'):
		if argstr != None:
			try: page = int(argstr)
			except: raise EndUserError('error in %s: argument should be a single integer' % dbstr)
			if page not in self.grid['Page']: raise EndUserError('error in %s:  there is no page %d' % (dbstr,page)) 
			return {'page':page}

		self.history.append(self.states['Page'])
		self.states['Page'] = page

	##########################################################################################
	
	def ProcessUdpQueue(self):
		if len(self.udp_queue) > 0:
			self.UdpSend(self.udp_queue)
			self.udp_queue = []

	def UdpSend(self, data):
		if self.udp_port:
			for item in data:
				d = "P3Speller_Output "+item
				self.udp_socket.sendto(d,(self.udp_host,self.udp_port))
				print "UdpSend to (%s,%d): %s"%(self.udp_host,self.udp_port,d)

##############################################################################################
##############################################################################################

