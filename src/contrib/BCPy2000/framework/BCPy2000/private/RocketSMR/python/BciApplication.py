#import PygameRenderer
import sys, numpy, time, random, copy, warnings, math
import pygame
import AppTools.Displays
import WavTools


# Implementation of the SpaceGame including:
# - initial Calibration session, where cues are presented
# - the actual Game:
#		- fueling phase: cloud moves at top of screen releasing drops
#                        which need to be caught with a cart at the bottom of the screen
#					     (phase change after specified amount of drops was caught)
#		- flying phase: user navigates a spaceship at the bottom of the screen, avoiding
#						rockets which approach from the top of the screen, till planet appears
#						(phase change after planet was reached (adjusting phase)
#						 or after hit by rocket (back to fueling phase))
#		- adjusting phase: drop catching as in fueling phase, but size of cart and game speed changes
#						   according to user ability
#						   (phase change after certain number of reversals)

	

class BciApplication(BciGenericApplication):
	
	# functions used to interpret PaddleExpression (self.params) typed in by the user
	
	def Signal(self, ch, el):
		return self.in_signal[ch-1,el-1]
			
	def Diff(self, val, key):   #  Diff( (expr),  label ) also works provided the label is unique to every different occurence of Diff()
		self.prev = getattr(self, 'prev', {})
		prev = self.prev.get(key, val)
		self.prev[key] = val
		return (val - prev) * self.nominal.PacketsPerSecond
		
	def PythonizeExpressionSubstring(self, x):
		if x in self.states: return 'float(self.states.%s)' % x
		if hasattr(self, x): return 'self.' + x
		if hasattr(numpy, x): return 'numpy.' + x
		raise EndUserError("failed to interpret '%s' in expression" % x)
		
	def PythonizeExpression(self, x):
		import re
		x = x.replace('^', '**')
		x = re.sub('[A-Za-z_][0-9A-Za-z_]*', lambda m:self.PythonizeExpressionSubstring(m.group()), x)
		return x
	
	#############################################################
	
	def Description(self):
		return "game $Revision$ - basic catching raindrops, bonus fly spaceship avoiding rockets"
	
	#############################################################
	
	def Construct(self):
		self.define_param(
			"PythonApp:Screen	int		ScreenID=			-1	   -1     %     % // monitor id (0,1,2..., or -1 for last)",
			"PythonApp:Screen	float	WindowSize=         0.8   1.0   0.0   1.0 // size of the stimulus window, proportional to the screen",
			
			"Application:Calibration matrix  Prompts= 2  { TargetCode String }  1  <<Left    2 Right>>  % % % // calibration prompt strings and corresponding TargetCode values",
			"Application:Calibration intlist PromptSequence= 6  1 2 1 2 1 2  % 1 % // sequence of stimulus codes to present as prompts for calibration",
			"Application:Calibration float   PromptDuration= 4  % 0 % // duration of prompts in s",
			"Application:Calibration float	 InterStimInterval= 2.5 % 0 % // duration between prompts in s",
			"Application:Calibration float   FeedbackStateDelay= 0.5  % 0 % // delay between setting TargetCode state and Feedback (for normalizer) state in s",
			"Application:Calibration int     FeedbackVisibleAfter= 4  4 0 % // number of trials to perform without visible feedback",

			"PythonApp:ScoreDisplay	float	ScoreDisplayWidth=	0.15  %		0.1	  0.2 // proportion of stimulus window used to display Scores",
			"PythonApp:ScoreDisplay	floatlist	ScoreDisplayBgColor= 3  0.2 0.2 0.2  %  0.0  1.0 // background color of score display (proportions of r,g,b color values)",
			"PythonApp:ScoreDisplay int		ScoreFontSize=		30  %  15  40 // size of Text displaying the Scores",
			"PythonApp:ScoreDisplay floatlist ScoreFontColor= 3	 1.0 1.0 1.0 % 0.0 1.0 // color of Text displaying the Scores",
			
			"PythonApp:Settings	string		PaddleExpression= (Signal(2,1)-Signal(1,1))*2 % % %// Signal to use for paddle speed",
			"PythonApp:Settings	int		NormalizerMode= 0 % 0 1 // 0 no normalization after calibration, 1 adaptive normalization (enumeration)",
			# Update Trigger of Normalizer needs to be set to "Feedback == 0"
			# if adaptive normalization is used, the Normalizer variables are updated each time an input sequence
			# into the normalizer stops, yet if an single input sequence lasts longer than the NormalizerUpdate value specified
			# Normalizer variables are also updated
			"PythonApp:Settings float	NormalizerUpdate= 5.0 % % % // period in s after which normalizer variables are updated by default if adaptive normalization is employed",
			"PythonApp:Settings int		UseDirectWiimoteControl= 0 % % % // use direct control via Wiimotes (boolean)",
			# necessary due to the variance in the signal wiimotes produce even if not in motion
			"PythonApp:Settings float   DirectControlZeroThreshold= 1 % % % // only for direct control mode",
			# necessary due to the variance in the signal wiimotes produce during motion
			"PythonApp:Settings int		DirectControlSmoothingLength= 4 % % % // how many past values to consider for smoothing",
			
			"PythonApp:Objects	floatlist	InitPaddleSize=	{width height} 0.2	0.07    %	  0.0	1.0 // width and height of paddle proportional to gaming window size",
			"PythonApp:Objects	floatlist	CloudSize=		{width height}	0.3	0.15	%     0.0	1.0 // width and height of cloud proportional to gaming window size",
			"PythonApp:Objects	floatlist	DropSize=	{width height}	0.03  0.12	%	  0.0	0.2//  width and height of drops proportional to gaming window size",
			"PythonApp:Objects	floatlist	InitSpaceshipSize=	{width height} 0.2 0.15   %	  0.0	1.0 // width and height of spaceship proportional to gaming window size",
			"PythonApp:Objects	floatlist	RocketSize=	{width height}	0.03 0.12	%	  0.0	0.2 // width and height of rocket proportional to gaming window size",
			"PythonApp:Objects	stringlist 	Images=		{paddle cloud drop spaceship rocket planet} cart_empty.png cloud.png raindrop.png spaceship.png rocket.png planet.png  %  % % // images to be used for objects (spaceship, rocket(angle upright))",
			"PythonApp:Objects	stringlist	Sounds=		{hitDrop missDrop hitRocket} hitDrop.wav missDrop.wav explosion.wav % % % // Ingame sounds",
			"PythonApp:Objects  float		BottomOfWater= 0.23 % % % //proportion of cart height were water starts to fill up cart",
			
			"PythonApp:BasicRound float	DropFrequency= 2 % 0 % // determines distance between different Drops falling from sky (DrFreq*PaddleHeight)",
			"PythonApp:BasicRound float InitGravity= 1 % 0.1 % // speed with which drops fall down (movement per frame)",
			"PythonApp:BasicRound int	CloudMode= 0 % 0 1 // 0 cloud moves according to CloudPath and CloudSpeed, 1 cloud moves away from paddle (enumerate)",
			# floats values from 0.0(left) to 1.0(right) give the fraction of the screen the cloud is supposed to move to next
			# values from 10 - 100 define the length of a period were the cloud is supposed to stay at its current spot
			#	the real length (i.e. seconds) defined varies according to game speed and frame rate
			# the values in a single row repeat itself during a single phase
			# upon entering a new phase (either adjusting or fueling) the next row is considered
			"PythonApp:BasicRound matrix	CloudPathes= 3 5	1 0.2 30 0.4 0.8	0.2 0.5 50 0.2 0.8	1 60 0.4 0.2 0 	% % % //path which cloud follows",
			"PythonApp:BasicRound floatlist CloudSpeed= 5    0.5 1.5 0.8 2.3 2.0  % % %// List of Cloud Speeds to be used",
			"PythonApp:BasicRound int		DropsTillBonus= 10 % 1 32 // number of drops that have to be collected to enter Bonus Round",
			
			"PythonApp:BonusRound matrix	RocketPattern= 6 {veryLeft Left Center Right veryRight} 	1 0 0 0 0	0 1 0 0 0	0 1 1 0 0  0 0 0 1 1  0 1 0 0 1  0 1 0 1 0 % 0 1 // Falling pattern of rockets (0 no rocket, 1 rocket)",
			"PythonApp:BonusRound float		RocketFrequency= 2  %  0  % //determines distance between different RocketRows (RoFreq*SpaceshipHeight)",
			
			"PythonApp:PaddleSizeAdjusting int	  AdjustingFrequency=	5 % % % //Minutes after which the paddle size is adjusted again, even if no planet was reached",	
			"PythonApp:PaddleSizeAdjusting float  TargetAccuracy= 0.75 % 0.0 1.0 //HitRate to aim for in adjusting paddle size",
			"PythonApp:PaddleSizeAdjusting float  InitGameDifficulty= 1.0 % % % // GameDifficulty",
			"PythonApp:PaddleSizeAdjusting float  PaddleWidthHitFactor= 0.9 % 0.01 0.99 //determines the amount the paddle grows and shrinks with each hit or miss",
			"PythonApp:PaddleSizeAdjusting float  MaximumPaddleWidth= 0.5 % % % //maximum width paddle can grow to proprotional to gaming window width",
			"PythonApp:PaddleSizeAdjusting float  MinimumPaddleWidth= 0.05 % % % //minimum width paddle can grow to proprotional to gaming window width",
			"PythonApp:PaddleSizeAdjusting float  GravityHitFactor= 0.1 % 0.01 0.99 //determines the amount the speed increases or decreases with each hit or miss",
			"PythonApp:PaddleSizeAdjusting float  MinimalGravity= 0.1 % 0.02 % //determines the minimal gravity",
			"PythonApp:PaddleSizeAdjusting int	  ReversalsToDiscard= 2 % % % // number of reversals in the beginning to be discarded before computing the new paddle size",
			"PythonApp:PaddleSizeAdjusting int	  ReversalsToCompute= 6 % % % // number of reversals to be taken into account for new paddle size computation",
		)
		
		self.define_state(
			"TargetCode    2 0 0 0", # defines the required movement direction 1(left) 2(right)
			"Feedback      1 0 0 0", # defines wether feedback is send to the signal processing module (i.e. normalizer)
									 # Update Trigger of Normalizer should be "Feedback == 0"
			"GameMode 2 0 0 0", # 1:fueling phase 2:flying phase 3:adjusting paddle size
			"DistanceTimes10 14 0 0 0", # distance till the next planet is reached
			"Hits 8 0 0 0",
			"Misses	8 0 0 0",
			"JustHit 1 0 0 0",
			"JustMissed 1 0 0 0",
			"HitsInRow 14 0 0 0",
			"Score	20 0 0 0",
			"Highscore 20 0 0 0",
			"AvoidedSingle 14 0 0 0", # how many single rockets have been avoided
			"AvoidedRows 10 0 0 0", # how many rows of rockets have been avoided
			"Planets 7 0 0 0",
			"NumberOfGame	7 0 0 0", # one game lasts from playing (adjusting phase) till planet was reached (or adjusting phase entered by default)
			
			"PaddleWidthTimes10000	14 0 0 0",
			"PaddlePosXTimes10000	14 0 0 0",
			"PaddlePosYTimes10000	14 0 0 0",
			"CloudSpeedTimes10	14 0 0 0",
			"CloudSpeedSign 1 0 0 0",
			"CloudPosXTimes10000	14 0 0 0",
			"CloudPosYTimes10000	14 0 0 0",
			"TargetDropPosXTimes10000 14 0 0 0",
			"TargetDropPosYTimes10000 14 0 0 0",
			"DropWidthTimes10000 14 0 0 0",
			"GravityTimes100 14 0 0 0",
			
			"SpaceshipWidthTimes10000	14 0 0 0", # constant in current implementation
			"SpaceshipPosXTimes10000	14 0 0 0",
			"SpaceshipPosYTimes10000	14 0 0 0",
			"Rocket0PosXTimes10000	14 0 0 0", # gives positions of each rocket in the lowest rocket row
			"Rocket1PosXTimes10000	14 0 0 0", # if it consits of only one rocket only Rocket0PosX.. will
			"Rocket2PosXTimes10000	14 0 0 0", # have a value, others will be 0
			"Rocket3PosXTimes10000	14 0 0 0", # if three rockets are present Rocket0Pos.. - Rocket2Pos..
			"Rocket4PosXTimes10000	14 0 0 0", # will have value, others are 0
			"Rocket5PosXTimes10000	14 0 0 0",
			"Rocket6PosXTimes10000	14 0 0 0",
			"Rocket7PosXTimes10000	14 0 0 0",
			"Rocket8PosXTimes10000	14 0 0 0",
			"Rocket9PosXTimes10000	14 0 0 0",
			"LowestRocketRowPosYTimes10000 14 0 0 0",
		)
	
	#############################################################
	
	def Preflight(self, in_signal_props):
		#~ called as SetConfig Button is pressed (also on Startup)
		#~ purpose: sanity-check parameter values and verify the availability of state variables
		#
		#~ Builds Stimulus Window of specified Size (self.params['WindowSize']) on specified
		#~ 		Monitor (self.params['ScreenID'])
		
		monitor = int(self.params['ScreenID'])
		windowsize = float(self.params['WindowSize'])
		if AppTools.Displays.number_of_monitors() > 1 and monitor == -1: windowsize = 1.0
		AppTools.Displays.fullscreen(id=monitor, scale=windowsize)
		self.screen.setup(frameless_window=(windowsize==1.0))
		
#############################################################
	
	def Initialize(self, indims, outdims):
		# called directly following Preflight as SetConfig Button is pressed
		# purpose: pre-allocation of any objects needed (attached as new attributes of self)
		#
		#~1.1. Setting screen background and creating Score display
		#~1.2. Creating Objects of Score Display
		#~1.3. Creating Text Stimulus to display instructions
		#~2.1. Creating Paddle
		#~2.2. Creating Cloud
		#~2.3. Transform CloudPathes
		#~2.4. Calculate DropReleaseInterval and Create necessary Drops
		#~3.1. Creating Spaceship
		#~3.2. Transform RocketPattern
		#~3.3. Calculate RocketReleaseInterval and Create necessary Rockets
		#~3.4. Creating Planets
		#~
		#~
		#~ Resulting Objects:					Variables:
		#~										self.gameDifficulty			self.lowestGameDifficulty
		#~ self.stimuli['instructions1']		self.widthHistory			self.growthDirection
		#~ self.stimuli['instructions2']	    self.gamingWindowWidth		
		#~ self.stimuli['paddle'] 				self.paddleSpeed
		#~ self.stimuli['water']				self.fullWaterHeight
		#~ self.stimuli['cloud']				self.cloudSpeed
		#~										self.allCloudSpeeds
		#~ 										self.cloudPathes
		#~										self.currentCloudPath
		#~										self.nextCloudPath
		#~										self.cloudMoveEnd
		#~ self.stimuli['drop_0']				self.DropReleaseInterval
		#~ self.stimuli['drop_1']				self.DropReleaseCount
		#~ ...									self.activeDrops
		#~										self.inactiveDrops
		#~										self.targetDrop
		#~
		#~ self.stimuli['spaceship']			(uses self.paddleSpeed)
		#~ self.stimuli['rocket_0']				self.activeRockets
		#~ self.stimuli['rocket_1']				self.inactiveRockets
		#~ ...									self.RocketPattern
		#~										self.nextRocketPatternRow
		#~										self.RocketFallingPositions
		#~										self.RocketReleaseInterval
		#~										self.RocketReleaseCount
		#~
		#~ Score Display Objects:
		#~ self.stimuli['Highscore']			self.highscore
		#~ self.stimuli['Score']				self.score
		#~ self.stimuli['Planets']				self.startTime
		#~ self.stimuli['Trajectory']			self.distance
		#~										self.planetDistance
		#~ self.stimuli['ScoreLabel']			
		#~ self.stimuli['HighScoreLabel']
		#~ self.stimuli['MiniPlanet']
		#~ self.stimuli['MiniSpaceship']
		#~ self.stimuli['ScoreDiplay']
		#~ 
		
		from AppTools.CurrentRenderer import VisualStimuli
		import AppTools.Shapes as shapes
		
		self.prompts = copy.deepcopy(self.params['Prompts'])
		self.promptSequence = copy.deepcopy(self.params['PromptSequence'])
		self.endFeedback = False
		
		#Create Sound Stimuli
		try:
			self.hitDrop = WavTools.player(self.params['Sounds'][0])
		except:
			raise EndUserError, "First Soundfile could not be loaded"
		try:
			self.missDrop = WavTools.player(self.params['Sounds'][1])
		except:
			raise EndUserError, "Second Soundfile could not be loaded"
		try:
			self.explosion = WavTools.player(self.params['Sounds'][2])
		except:
			raise EndUserError, "Third Soundfile could not be loaded"
	
		# some necessary variables
		self.expr = self.PythonizeExpression(self.params['PaddleExpression'])
		self.smoothDirectControl = []
		self.startTime = None
		self.difficultyHistory = []
		self.growthDirection = None
		self.gameDifficulty = float(self.params['InitGameDifficulty'])
		# lowest and highest game difficulty for paddle growth
		self.lowestGameDifficulty = round(math.log((float(self.params['MaximumPaddleWidth'])/float(self.params['InitPaddleSize'][0])),float(self.params['PaddleWidthHitFactor'])))
		self.highestGameDifficulty = round(math.log((float(self.params['MinimumPaddleWidth'])/float(self.params['InitPaddleSize'][0])),float(self.params['PaddleWidthHitFactor'])))
		# lowest GameDifficutly because of Gravity
		BelowLowestGameDifficulty = ((float(self.params['MinimalGravity'])/float(self.params['InitGravity']))-1)*(-1)/float(self.params['GravityHitFactor'])
		self.lowestGravityDifficulty = (BelowLowestGameDifficulty-self.lowestGameDifficulty)*(-1)
		if self.gameDifficulty < self.lowestGravityDifficulty:
			self.gameDifficulty = self.lowestGravityDifficulty
		
#~ 1.1. Setting screen background to black
#~		Creating Score display of specified Width (self.params['ScoreDisplayWidth'])
#~ 			with background of specified Color (self.params['ScoreDisplayBgColor'])
#~		Calculating left over gaming window width (self.gamingWindowWidth), Screen without ScoreDisplay
		self.screen.bgcolor = ((0.0,0.0,0.0))
		scoreDisplay_width = round(float(self.params['ScoreDisplayWidth'])*self.screen.size[0])
		bgColor = self.params['ScoreDisplayBgColor']
		bgColor = (float(bgColor[0]), float(bgColor[1]), float(bgColor[2]))
		self.stimulus('scoreDisplay', shapes.PolygonTexture, vertices=[(0,0),(1,0),(1,1),(0,1)], color=bgColor, position=(self.screen.size[0]-scoreDisplay_width/2,self.screen.size[1]/2), size=(scoreDisplay_width,self.screen.size[1]), anchor='center')
		self.gamingWindowWidth = self.screen.size[0]-self.stimuli['scoreDisplay'].size[0]
		
#~ 1.2. Creating Text Stimuli to display Score of specified fontSize (self.params['ScoreFontSize'])
#		Create small spaceship flying towards Planet
		color = (float(self.params['ScoreFontColor'][0]), float(self.params['ScoreFontColor'][1]), float(self.params['ScoreFontColor'][2]))
		fontsize = int(self.params['ScoreFontSize'])
		self.stimulus('ScoreLabel', VisualStimuli.Text, text='Score:', anchor='center', position=(self.stimuli['scoreDisplay'].position[0], self.screen.size[1]*0.85), font_size=fontsize, color=color)
		self.stimulus('Score', VisualStimuli.Text, text='0', anchor='center', position=(self.stimuli['scoreDisplay'].position[0], self.stimuli['ScoreLabel'].position[1]-self.stimuli['ScoreLabel'].size[1]*1.2), font_size=fontsize, color=color)
		self.score = 0
		self.stimulus('HighscoreLabel', VisualStimuli.Text, text='Highscore:', anchor='center', position=(self.stimuli['scoreDisplay'].position[0], self.screen.size[1]*0.75), font_size=fontsize, color=color)
		self.stimulus('Highscore', VisualStimuli.Text, text='0', anchor='center', position=(self.stimuli['scoreDisplay'].position[0], self.stimuli['HighscoreLabel'].position[1]-self.stimuli['HighscoreLabel'].size[1]*1.2), font_size=fontsize, color=color)
		self.highscore = 0
		self.stimulus('Trajectory', shapes.PolygonTexture, vertices=[(0,0),(1,0),(1,1),(0,1)], color=color, anchor='bottom', position=(self.stimuli['scoreDisplay'].position[0], self.screen.size[1]*0.25), size=(self.stimuli['scoreDisplay'].size[0]*0.07, self.screen.size[1]*0.34))
		miniPlanet_size = (self.stimuli['scoreDisplay'].size[0]*0.35, self.stimuli['scoreDisplay'].size[0]*0.35)
		try:
			self.stimulus('MiniPlanet', VisualStimuli.ImageStimulus, anchor='center', texture=self.params['Images'][5])
		except:
			warnings.warn("Imagefile for planet could not be loaded")
			self.stimulus('MiniPlanet', shapes.PolygonTexture, vertices=[(0,0),(1,0),(1,1),(0,1)], anchor='center', color=(0.0,0.6,1.0))
		self.stimuli['MiniPlanet'].size = miniPlanet_size
		self.stimuli['MiniPlanet'].position = (self.stimuli['Trajectory'].position[0], self.stimuli['Trajectory'].position[1]+self.stimuli['Trajectory'].size[1]+miniPlanet_size[1]*0.6)
		miniSpaceship_width = self.stimuli['scoreDisplay'].size[0]*0.4
		miniSpaceship_height = self.screen.size[1]*0.07
		try:
			self.stimulus('MiniSpaceship', VisualStimuli.ImageStimulus, texture=self.params['Images'][3])
		except:
			warnings.warn("Imagefile for spaceship could not be loaded")
			self.stimulus('MiniSpaceship', shapes.PolygonTexture, vertices=[(0,0),(1,0),(1,1),(0,1)], color=(0.6,0.6,1.0))
		self.stimuli['MiniSpaceship'].size = (miniSpaceship_width, miniSpaceship_height)
		self.stimuli['MiniSpaceship'].position = self.stimuli['Trajectory'].position
		self.stimuli['MiniSpaceship'].anchor = 'center'
		bigPlanet_size = (self.gamingWindowWidth*0.5, self.gamingWindowWidth*0.5)
		try:
			self.stimulus('bigPlanet', VisualStimuli.ImageStimulus, texture = self.params['Images'][5], on=False, anchor = 'center', size=bigPlanet_size, position=(self.gamingWindowWidth/2, self.screen.size[1]/2))			
		except:
			warnings.warn("Imagefile for planet could not be loaded")
			self.stimulus('bigPlanet', shapes.Disc, radius=0.5, anchor='center', color=(0.0,0.6,1.0), on=False, size=bigPlanet_size)

#~ 1.3. Creating TextStimuli to display instructions
		self.stimulus('instructions1', VisualStimuli.Text, text='Start the Game', z=0, anchor='center', font_size=40, color=(1.0,1.0,1.0))
		self.stimuli['instructions1'].position = position=(self.gamingWindowWidth/2, self.screen.size[1]/2+self.stimuli['instructions1'].size[1]*0.6)
		self.stimulus('instructions2', VisualStimuli.Text, text='Have Fun!',z=0, anchor='center', position=(self.gamingWindowWidth/2, self.screen.size[1]/2-self.stimuli['instructions1'].size[1]*0.6), font_size=40, color=(1.0,1.0,1.0))

#~ 2.1. Create Paddle of specified Size(self.params['InitPaddleSize'])
#~			from specified image(self.params['Images'][0]
#~		Position it in the center at the bottom of the gaming window
#~		Create water to fill up the cart
		if self.gameDifficulty < self.lowestGameDifficulty:
			newPaddleWidth = float(self.params['MaximumPaddleWidth'])
		elif self.gameDifficulty > self.highestGameDifficulty:
			newPaddleWidth = float(self.params['MinimumPaddleWidth'])
		else:
			newPaddleWidth = float(self.params['InitPaddleSize'][0])*math.pow(float(self.params['PaddleWidthHitFactor']),self.gameDifficulty)
		paddle_width = round(newPaddleWidth*self.gamingWindowWidth)
		paddle_height = round(float(self.params['InitPaddleSize'][1])*self.screen.size[1])
		water_bottom_height = round(paddle_height*float(self.params['BottomOfWater']))
		self.fullWaterHeight = paddle_height-water_bottom_height
		self.stimulus('water', shapes.PolygonTexture, vertices=[(1,1),(0,1), (0,0),(1,0)], on=False, color=(0.0,0.0,1.0), anchor='bottom', position=(self.gamingWindowWidth/2,water_bottom_height), size=(paddle_width,self.fullWaterHeight))
		try:
			self.stimulus('paddle', VisualStimuli.ImageStimulus, texture=self.params['Images'][0])
		except:
			warnings.warn("Imagefile for paddle could not be loaded")
			self.stimulus('paddle', shapes.PolygonTexture, vertices=[(0,0),(1,0),(1,1),(0,1)], color=(0.8,0.8,0.8))
		self.stimuli['paddle'].size = (paddle_width, paddle_height)
		self.stimuli['paddle'].anchor = 'center'
		self.stimuli['paddle'].on = False
		self.stimuli['paddle'].position = (self.gamingWindowWidth/2, self.stimuli['paddle'].size[1]/2)
		self.paddleSpeed = 0
		

#~ 2.2. Create Cloud of specified Size (self.params['CloudSize'])
#~			from specified image(self.params['Images'][1]
#~		Position it in the topleft of the gaming window
		cloud_width = round(float(self.params['CloudSize'][0])*self.gamingWindowWidth)
		cloud_height = round(float(self.params['CloudSize'][1])*self.screen.size[1])
		try:
			self.stimulus('cloud', VisualStimuli.ImageStimulus, texture=self.params['Images'][1])
		except:
			warnings.warn("Imagefile for cloud could not be loaded")
			self.stimulus('cloud', shapes.PolygonTexture, vertices=[(0,0),(1,0),(1,1),(0,1)], color=(1.0,1.0,1.0))
		self.stimuli['cloud'].size = (cloud_width, cloud_height)
		self.stimuli['cloud'].anchor = 'center'
		self.stimuli['cloud'].position = (cloud_width/2, self.screen.size[1]-cloud_height/2)
		self.stimuli['cloud'].on = False
		self.cloudSpeed = 0
		self.allCloudSpeeds = copy.deepcopy(self.params['CloudSpeed'])
		
#~ 2.3. Transform given CloudPathes (self.params['CloudPathes'] into list in list structure
		# unnecessary if CloudMode = 1
		if float(self.params['CloudMode']) == 0:
			pathes = numpy.array(self.params['CloudPathes'])
			list = []
			for i in range(pathes.shape[0]):
				path =[]
				for j in range(pathes.shape[1]):
					x = float(pathes[i,j])	
					if not ((x >= 0 and x<=1.0) or (x>=10 and x<=100)):
						raise EndUserError, "cloudpathes must hold values between 0.0-1.0 and 10-100 only"
					path.append(x)
				list.append(path)
			self.cloudPathes = list
			self.nextCloudPath = -1
			self.currentCloudPath = copy.deepcopy(self.cloudPathes[0])
			self.cloudMoveEnd = None
		
#~ 2.4. Calculate DropReleaseInterval 
#~		Calculate maximum number of necessary Drops
#~ 		Create Drops of specified Size (self.params['DropSize'])
#~ 			and store Drop objects in list of inactiveDrops
		self.activeDrops = []	# list of Rockets currently displayed on screen
		self.inactiveDrops = []
		drop_width = round(float(self.params['DropSize'][0])*self.gamingWindowWidth)
		drop_height = round(float(self.params['DropSize'][1])*self.screen.size[1])
		distance_between_releases = drop_height+self.stimuli['paddle'].size[1]*round(float(self.params['DropFrequency']),1)
		
		self.DropReleaseInterval = distance_between_releases
		self.DropReleaseCount = 0
		maxDrops_onScreen = (self.screen.size[1]-self.stimuli['cloud'].size[1])/distance_between_releases
		if round(maxDrops_onScreen) > maxDrops_onScreen:
			maxDrops_onScreen = round(maxDrops_onScreen)-1
		else:
			maxDrops_onScreen = round(maxDrops_onScreen)
		nDrops = maxDrops_onScreen
		Drops = range(nDrops)
		try:
			for d in range(nDrops):
				self.stimulus('_'.join(('Drop',str(d))), VisualStimuli.ImageStimulus, texture=self.params['Images'][2], size=(drop_width,drop_height), on=False, anchor='center')
				self.inactiveDrops.append(self.stimuli['_'.join(('Drop',str(d)))])
		except:
			warnings.warn("Imagefile for drops could not be loaded")
			for d in range(nDrops):
				self.stimulus('_'.join(('Drop',str(d))), shapes.PolygonTexture, vertices=[(0,0),(1,0),(1,1),(0,1)], color=(1.0,1.0,0.0), size=(drop_width,drop_height), on=False, anchor='center')
				self.inactiveDrops.append(self.stimuli['_'.join(('Drop',str(d)))])
		self.targetDrop = []
		paddle = self.stimuli['paddle']
		if paddle.size[0]*0.7 < round(self.gamingWindowWidth*float(self.params['DropSize'][0]),1):
			for d in self.activeDrops: d.size=(paddle.size[0]*0.7, d.size[1])
			for d in self.inactiveDrops: d.size=(paddle.size[0]*0.7, d.size[1])
		
#~ 3.1. Creating Spaceship of specified Size (self.params['InitSpaceshipSize']
#~			from specified image (self.params['Images'][3])
		spaceship_width = round(float(self.params['InitSpaceshipSize'][0])*self.gamingWindowWidth)
		spaceship_height = round(float(self.params['InitSpaceshipSize'][1])*self.screen.size[1])
		try:
			self.stimulus('spaceship', VisualStimuli.ImageStimulus, texture=self.params['Images'][3])
		except:
			warnings.warn("Imagefile for spaceship could not be loaded")
			self.stimulus('spaceship', shapes.PolygonTexture, vertices=[(0,0),(1,0),(1,1),(0,1)], color=(0.6,0.6,1.0))
		self.stimuli['spaceship'].size = (spaceship_width, spaceship_height)
		self.stimuli['spaceship'].anchor = 'center'
		self.stimuli['spaceship'].on = False
		
#~ 3.2. Transform given RocketPattern (self.params['RocketPattern']) into usable numpy matrix
#~		Calculate Positions of Rocket release
		pattern = numpy.array(self.params['RocketPattern'])
		columns = pattern.shape[1]
		if columns < 2 or columns > 10:
			raise EndUserError, "RocketPattern matrix must have at least 2 and a maximum of 8 columns"
		int_pattern = numpy.zeros((pattern.shape[0], pattern.shape[1]))
		for i in range(pattern.shape[0]):
			for j in range(pattern.shape[1]):
				if not (pattern[i,j]=='1' or pattern[i,j]=='0'):
					raise EndUserError, "RocketPattern matrix must hold values 0 and 1 only"
				int_pattern[i,j] = int(pattern[i,j])
		check = numpy.sum(int_pattern, axis=1)  # check if at least one drop is released in each row
		for c in check:
			if c <= 0:
				raise EndUserError, "RocketPattern contains at least one row with only zeros"
		self.RocketPattern = int_pattern
		self.nextRocketPatternRow = 0
		rocket_distance = round(self.gamingWindowWidth/columns)
		self.RocketFallingPositions = range(columns)
		for nPos in range(columns):
			self.RocketFallingPositions[nPos] = rocket_distance/2 + rocket_distance*nPos

#~ 3.3.	Calculate RocketReleaseInterval 
#~		Calculate maximum number of necessary Rockets
#~ 		Create Rockets of specified Width (self.params['RocketWidth']) and Height (self.params['RocketHeight']
#~ 			and store Rocket objects in list of inactiveRockets
		self.activeRockets = []	# list of Rockets currently displayed on screen
		self.inactiveRockets = []
		rocket_width = round(float(self.params['RocketSize'][0])*self.gamingWindowWidth)
		rocket_height = round(float(self.params['RocketSize'][1])*self.screen.size[1])
		distance_between_releases = rocket_height+self.stimuli['spaceship'].size[1]*round(float(self.params['RocketFrequency']),1)
		self.RocketReleaseInterval = distance_between_releases
		self.RocketReleaseCount = 0
		maxRocketRows_onScreen = self.screen.size[1]/distance_between_releases
		if round(maxRocketRows_onScreen) > maxRocketRows_onScreen:
			maxRocketRows_onScreen = round(maxRocketRows_onScreen)-1
		else:
			maxRocketRows_onScreen = round(maxRocketRows_onScreen)
		nRockets = maxRocketRows_onScreen*self.RocketPattern.shape[0]
		Rocket = range(nRockets)
		try:
			for r in range(nRockets):
				self.stimulus('_'.join(('Rocket',str(r))), VisualStimuli.ImageStimulus, texture=self.params['Images'][4], size=(rocket_width,rocket_height), on=False, anchor='center', angle=180)
				self.inactiveRockets.append(self.stimuli['_'.join(('Rocket',str(r)))])
		except:
			warnings.warn("Imagefile for drops could not be loaded")
			for d in range(nRockets):
				self.stimulus('_'.join(('Rocket',str(r))), shapes.PolygonTexture, vertices=[(0,0),(1,0),(1,1),(0,1)], color=(0.0,1.0,1.0), size=(rocket_width,rocket_height), on=False, anchor='center')
				self.inactiveRockets.append(self.stimuli['_'.join(('Rocket',str(r)))])
				
#~ 3.4.	Creating Planets
		planet_size = round(self.stimuli['scoreDisplay'].size[0]/3.5)
		self.planets = []
		try:
			for p in range(10):
				if p < 5:
					self.stimulus('_'.join(('Planet',str(p))), VisualStimuli.ImageStimulus, texture=self.params['Images'][5], on=False, anchor='center', size=(planet_size,planet_size), position=(self.stimuli['scoreDisplay'].position[0]-self.stimuli['scoreDisplay'].size[0]/2+(planet_size/2)*(p+1)+planet_size*0.2, planet_size*1.5))
				else:
					self.stimulus('_'.join(('Planet',str(p))), VisualStimuli.ImageStimulus, texture=self.params['Images'][5], on=False, anchor='center', size=(planet_size,planet_size), position=(self.stimuli['scoreDisplay'].position[0]-self.stimuli['scoreDisplay'].size[0]/2+(planet_size/2)*(p-4)+planet_size*0.2, planet_size*0.7))
				self.planets.append(self.stimuli['_'.join(('Planet',str(p)))])
		except:
			warnings.warn("Imagefile for planet could not be loaded")
			for p in range(10):
				if p < 5:
					self.stimulus('_'.join(('Planet',str(p))), shapes.Disc, radius=0.5, anchor='center', color=(0.0,0.6,1.0), on=False, size=(planet_size,planet_size), position=(self.stimuli['scoreDisplay'].position[0]-self.stimuli['scoreDisplay'].size[0]/2+(planet_size/2)*(p+1)+planet_size*0.2, planet_size*1.5))
				else:
					self.stimulus('_'.join(('Planet',str(p))), shapes.Disc, radius=0.5, anchor='center', color=(0.0,0.6,1.0), on=False, size=(planet_size,planet_size), position=(self.stimuli['scoreDisplay'].position[0]-self.stimuli['scoreDisplay'].size[0]/2+(planet_size/2)*(p-4)+planet_size*0.2, planet_size*0.7))
				self.planets.append(self.stimuli['_'.join(('Planet',str(p)))])
		self.penaltyDistance = round(self.RocketReleaseInterval*(maxRocketRows_onScreen+0.5))
		self.planetDistance = round(self.penaltyDistance+self.RocketReleaseInterval*self.RocketPattern.shape[0])
		self.distance = self.planetDistance
		self.stimulus('planets', VisualStimuli.Text, text='0', anchor='center',on=False, position=(self.stimuli['scoreDisplay'].position[0], self.stimuli['Planet_0'].position[1]+self.stimuli['Planet_0'].size[1]), font_size=fontsize, color=color)		
#############################################################
	
	def StartRun(self):
		#~ called as Start Button is pressed
		#
		#~ Assigning initial values to some State Variables:
		self.states['PaddleWidthTimes10000'] = round((self.stimuli['paddle'].size[0]/self.gamingWindowWidth)*10000)
		# determine initial Gravity
		if self.gameDifficulty > 1.0:
			newGravity = (float(self.params['InitGravity'])*(1+(float(self.params['GravityHitFactor'])*self.gameDifficulty)))
		elif self.gameDifficulty < self.lowestGameDifficulty:
			negativeSpeedDiff = self.lowestGameDifficulty - self.gameDifficulty
			newGravity = (float(self.params['InitGravity'])*(1-(float(self.params['GravityHitFactor'])*negativeSpeedDiff)))
		else:
			newGravity = float(self.params['InitGravity'])
		newGravity = round(newGravity,2)
		self.states['GravityTimes100'] = newGravity*100
		self.states['SpaceshipWidthTimes10000'] = round((self.stimuli['spaceship'].size[0]/self.gamingWindowWidth)*10000)
		self.states['DropWidthTimes10000'] = round((self.stimuli['Drop_0'].size[0]/self.gamingWindowWidth)*10000)
		self.states['Hits'] = 0
		self.states['Misses'] = 0
		self.states['JustHit'] = 0
		self.states['JustMissed'] = 0
		self.states['Score'] = 0
		self.states['AvoidedSingle'] = 0
		self.states['AvoidedRows'] = 0
		self.states['DistanceTimes10'] = 10000
		self.states['Planets'] = 0
		self.states['NumberOfGame'] = 1
		self.states['GameMode'] = 0
		self.states['CloudSpeedSign'] = 0
	#############################################################
	
	def Phases(self):
		#~ setting up the 'phase-machine' to define different phases of stimulus presentation
		self.phase(name = 'inter_Stimuli', duration = float(self.params['InterStimInterval'])*1000, next='pre_Feedback')
		self.phase(name = 'pre_Feedback', duration = float(self.params['FeedbackStateDelay'])*1000, next='Feedback')
		self.phase(name = 'Feedback', duration = float(self.params['PromptDuration'])*1000, next = 'inter_Stimuli')
		
		self.phase(name = 'preplay_text', duration = 5000, next='play')
		self.phase(name = 'play', duration = None, next= 'prebonus_text')
		self.phase(name = 'prebonus_text', duration = 5000, next='bonus')
		self.phase(name = 'bonus', duration = None, next= 'preplay_text')
		
		self.phase(name = 'pre_determinePaddleSize', duration=None, next='determinePaddleSize')
		self.phase(name = 'determinePaddleSize', duration = None, next='past_determinePaddleSize')
		self.phase(name = 'past_determinePaddleSize', duration = None, next='preplay_text')
		
		self.design(start='inter_Stimuli', new_trial='preplay_text')
		
	#############################################################
		
	def Transition(self, phase):
		#~ called at each phase transition
		
	#~ deactivate Instructions
		self.stimuli['instructions1'].color = (1.0,1.0,1.0)
		self.stimuli['instructions2'].color = (1.0,1.0,1.0)
		self.stimuli['instructions1'].on = False
		self.stimuli['instructions2'].on = False
		self.stimuli['bigPlanet'].on = False		
	#~ Reset Hits and Misses	
		self.states['Hits'] = 0
		self.states['Misses'] = 0
		self.states['JustHit'] = 0
		self.states['JustMissed'] = 0
		self.HitsInRow = 0
		self.states['HitsInRow'] = 0
		self.states['AvoidedRows'] = 0
		self.states['AvoidedSingle'] = 0
		self.states['GameMode'] = 0
	#~ Reset activeDrops list
		if self.activeDrops:
			for d in self.activeDrops:
				d.on = False
				self.inactiveDrops.append(d)
			del self.activeDrops[:]
		self.targetDrop = []
		self.DropReleaseCount = 0
		self.stimuli['cloud'].on = False
		self.states['CloudPosXTimes10000'] = 0
		self.states['CloudPosYTimes10000'] = 0
		self.states['CloudSpeedTimes10'] = 0
		self.states['PaddlePosXTimes10000'] = 0
		self.states['PaddlePosYTimes10000'] = 0
		self.states['TargetDropPosXTimes10000'] = 0
		self.states['TargetDropPosYTimes10000'] = 0
	#~ Reset activeRocket list
		if self.activeRockets:
			for row in self.activeRockets:
				for r in row:
					r.on = False
					self.inactiveRockets.append(r)
			del self.activeRockets[:]
		self.RocketReleaseCount = 0
		self.nextRocketPatternRow = 0
	#~ Deactivate and reset position of Spaceship
		self.stimuli['spaceship'].on = False	
		self.states['SpaceshipPosXTimes10000'] = 0
		self.states['SpaceshipPosYTimes10000'] = 0
		self.states['Rocket0PosXTimes10000'] = 0
		self.states['Rocket1PosXTimes10000'] = 0
		self.states['Rocket2PosXTimes10000'] = 0
		self.states['Rocket3PosXTimes10000'] = 0
		self.states['Rocket4PosXTimes10000'] = 0
		self.states['Rocket5PosXTimes10000'] = 0
		self.states['Rocket6PosXTimes10000'] = 0
		self.states['Rocket7PosXTimes10000'] = 0
		self.states['Rocket8PosXTimes10000'] = 0
		self.states['Rocket9PosXTimes10000'] = 0
		self.states['LowestRocketRowPosYTimes10000'] = 0
	# set position for miniSpaceship		
		normalized = 1-(self.distance/self.planetDistance)
		self.stimuli['MiniSpaceship'].position = (self.stimuli['MiniSpaceship'].position[0], self.stimuli['Trajectory'].position[1]+self.stimuli['Trajectory'].size[1]*normalized)
		self.states['DistanceTimes10'] = round((1-normalized)*10000)	

		if phase in ['inter_Stimuli']:
			self.states['TargetCode'] = 0
		
		if phase in ['pre_Feedback']:
			if len(self.promptSequence) == len(self.params['PromptSequence'])-int(self.params['FeedbackVisibleAfter']):
				self.stimuli['paddle'].position = (self.gamingWindowWidth/2, self.stimuli['paddle'].size[1]/2)
				self.stimuli['paddle'].on = True
				self.stimuli['water'].position = (self.gamingWindowWidth/2, self.stimuli['paddle'].size[1]-self.fullWaterHeight)
				self.stimuli['water'].size = (self.stimuli['paddle'].size[0], self.fullWaterHeight)
				self.stimuli['water'].on = True
			if self.promptSequence == [] or self.promptSequence == '':
				self.endFeedback = True
			else:				
				targetCode = int(self.promptSequence.pop(0))
				self.states['TargetCode'] = targetCode
				for pair in self.prompts:
					if int(pair[0]) == targetCode:
						self.stimuli['instructions1'].text = pair[1]
						break;
				self.stimuli['instructions1'].on = True
			
		if phase in ['Feedback']:
			self.stimuli['instructions1'].on = True
			self.states['Feedback'] = 1
		else:
			self.states['Feedback'] = 0
		
	#~ Display Paddle and Water in necessary phases	
		if phase in ['preplay_text','pre_determinePaddleSize','play','determinePaddleSize']:
			self.stimuli['paddle'].position = (self.gamingWindowWidth/2, self.stimuli['paddle'].size[1]/2)
			self.stimuli['paddle'].on = True
			self.stimuli['water'].position = (self.gamingWindowWidth/2, self.stimuli['paddle'].size[1]-self.fullWaterHeight)
			self.stimuli['water'].size = (self.stimuli['paddle'].size[0], 0)
			self.stimuli['water'].on = True
		elif phase not in ['pre_Feedback','Feedback', 'inter_Stimuli']:
			self.stimuli['paddle'].on = False
			self.stimuli['water'].on = False
		
	#~ Display Cloud if necessary
		if phase in ['play', 'determinePaddleSize']:
			if not self.startTime:		# start timer
				self.startTime = time.time()
			self.stimuli['cloud'].on = True
			if self.params['CloudMode'] == 0:
				self.nextCloudPath = self.nextCloudPath+1
				if self.nextCloudPath >= len(self.cloudPathes):
					self.nextCloudPath = 0
				self.currentCloudPath = copy.deepcopy(self.cloudPathes[self.nextCloudPath])
	
	#~ Set instructions	
		if phase in ['preplay_text']:
			if not self.startTime:		# start timer
				self.startTime = time.time()
			if self.stimuli['instructions1'].text == 'Avoid the rockets,':
				self.stimuli['instructions1'].text = 'OH NO: You were hit by a Rocket!'
				self.stimuli['instructions1'].on = True
				self.stimuli['instructions2'].text = 'Please refuel your spaceship'
				self.stimuli['instructions2'].on = True
			else:
				text = 'Catch %s drops' %self.params['DropsTillBonus']				
				self.stimuli['instructions1'].text = text
				self.stimuli['instructions1'].on = True
				self.stimuli['instructions2'].text = 'to fuel your spaceship'
				self.stimuli['instructions2'].on = True
		if phase in ['pre_determinePaddleSize']:
			self.stimuli['instructions1'].text = 'Adjust the Difficulty of the Game'
			self.stimuli['instructions1'].on = True
			self.stimuli['instructions2'].text = '(Press any KEY to continue)'
			self.stimuli['instructions2'].on = True
		if phase in ['past_determinePaddleSize']:
			text = 'New Difficulty is: %s' %(str(self.gameDifficulty))
			self.stimuli['instructions1'].text = text
			self.stimuli['instructions1'].on = True
			self.stimuli['instructions2'].text = '(Press any KEY to start next Round)'
			self.stimuli['instructions2'].on = True
		if phase in ['prebonus_text']:
			self.stimuli['spaceship'].position = (self.gamingWindowWidth/2, self.stimuli['spaceship'].size[1]/2)
			self.stimuli['spaceship'].on = True
			self.stimuli['instructions1'].text = 'Avoid the rockets,'
			self.stimuli['instructions1'].on = True
			self.stimuli['instructions2'].text = 'and try to make it to the Planet!'
			self.stimuli['instructions2'].on = True
			self.stimuli['CurrentPhase'] = 0
		
	#~	Set State Variable CurrentPhase	
		if phase in ['play']:
			self.states['GameMode'] = 1
		if phase in ['bonus']:
			self.stimuli['spaceship'].on = True
			self.states['GameMode'] = 2
		if phase in ['determinePaddleSize']:
			self.states['GameMode'] = 3			
	
	#~ Reset Score etc.
		if phase in ['pre_determinePaddleSize']:
			self.difficultyHistory = []
			self.growthDirection = None
			self.states['NumberOfGame'] = self.states['NumberOfGame']+1
			self.startTime = None
			self.distance = self.planetDistance
			self.states['DistanceTimes10'] = 10000
			self.stimuli['MiniSpaceship'].position = self.stimuli['Trajectory'].position
			color = (float(self.params['ScoreFontColor'][0]), float(self.params['ScoreFontColor'][1]), float(self.params['ScoreFontColor'][2]))
			self.stimuli['Highscore'].color = color
			self.stimuli['HighscoreLabel'].color = color
			self.stimuli['Score'].text = '0'
			self.states['Score'] = 0
			self.score = 0
		
#############################################################

	def Frame(self,phasename):
		# called on every video Frame
		#
		#~ "Implementation of the Game"
		#~ i.e. Movement of Objects, Behaviour during Collision of Objects etc.
		#~ 
		#~ 1. Play Phase: catching drops
		#~ 2. Bonus Phase: avoiding rockets
		
		self.states['JustHit'] = 0
		self.states['JustMissed'] = 0
		
		# Measuring Time
		if phasename in ['play', 'bonus']:
			elapsedtime = time.time()-self.startTime
			if elapsedtime >= int(self.params['AdjustingFrequency'])*60:
				self.change_phase(phasename='pre_determinePaddleSize')
		
		if phasename in ['Feedback','pre_Feedback','inter_Stimuli']:
			# move paddle
			paddle = self.stimuli['paddle']
			paddle.position = (paddle.position[0]+self.paddleSpeed, paddle.position[1])
			if paddle.position[0] < paddle.size[0]/2:
				paddle.position = (self.gamingWindowWidth-paddle.size[0]+paddle.size[0]/4, paddle.position[1])
			if paddle.position[0] > self.gamingWindowWidth-paddle.size[0]/2:
				paddle.position = (paddle.size[0]-paddle.size[0]/4, paddle.position[1])
			self.stimuli['water'].position = (paddle.position[0], paddle.size[1]-self.fullWaterHeight)
		
		if phasename in ['play', 'determinePaddleSize']:
			#~ 1.1. Paddle Movement
			#~ 1.2.	Cloud Movement
			#~ 1.3. Drop release
			#~ 1.4.	Moving Drops down and checking for Catch by paddle
			#~ 1.5. Training Normalizer
		
#~ 1.1. Move Paddle at current speed (self.paddleSpeed assigned in Process)
#~		Check that it remains in the gaming window
			paddle = self.stimuli['paddle']
			paddle.position = (paddle.position[0]+self.paddleSpeed, paddle.position[1])
			if paddle.position[0] < paddle.size[0]/2:
				paddle.position = (paddle.size[0]/2, paddle.position[1])
			if paddle.position[0] > self.gamingWindowWidth-paddle.size[0]/2:
				paddle.position = (self.gamingWindowWidth-paddle.size[0]/2, paddle.position[1])
			self.stimuli['water'].position = (paddle.position[0], paddle.size[1]-self.fullWaterHeight)
			self.states['PaddlePosXTimes10000'] = round((paddle.position[0]/self.gamingWindowWidth)*10000)	
			self.states['PaddlePosYTimes10000'] = round((paddle.position[1]/self.screen.size[1])*10000)			
		
#~ 1.2. Move Cloud according to self.cloudPathes at given Speed(self.params['CloudSpeed'])
			if float(self.params['CloudMode']) == 0:
				cloud = self.stimuli['cloud']
				if self.cloudMoveEnd == None:
					if not self.currentCloudPath: # if path end reached start path from beginning
						self.currentCloudPath = copy.deepcopy(self.cloudPathes[self.nextCloudPath])
					self.cloudMoveEnd = self.currentCloudPath.pop(0)
					if self.cloudMoveEnd > 1: # cloud supposed to stop
						self.cloudSpeed = 0
						self.states['CloudSpeedTimes10'] = 0
					else:	# cloud supposed to move
						if self.allCloudSpeeds==[]:
							self.allCloudSpeeds = copy.deepcopy(self.params['CloudSpeed'])
						self.cloudSpeed = round(float(self.allCloudSpeeds.pop()),1)
						self.states['CloudSpeedTimes10']=self.cloudSpeed*10
						self.cloudMoveEnd = round((self.gamingWindowWidth-cloud.size[0])*self.cloudMoveEnd+(cloud.size[0]/2),1)
				if self.cloudSpeed == 0:		# cloud is currently stopping
					self.cloudMoveEnd = self.cloudMoveEnd - (0.1*(float(self.states['GravityTimes100'])/100))
					if self.cloudMoveEnd <= 0:
						self.cloudMoveEnd = None
				else:		# cloud is moving
					speed = self.cloudSpeed
					if cloud.position[0] > self.cloudMoveEnd: speed = speed*(-1)					
					cloud.position = (cloud.position[0]+speed, cloud.position[1])
					if cloud.position[0] < cloud.size[0]/2:	#screen end left reached
						cloud.position = (cloud.size[0]/2, cloud.position[1])
						self.cloudMoveEnd = None
					if cloud.position[0] > self.gamingWindowWidth-cloud.size[0]/2: #screen end right reached
						cloud.position = (self.gamingWindowWidth-cloud.size[0]/2, cloud.position[1])
						self.cloudMoveEnd = None
					if self.cloudMoveEnd:
						if speed > 0:	# path end reached
							if cloud.position[0] >= self.cloudMoveEnd:
								self.cloudMoveEnd = None
						else:
							if cloud.position[0] <= self.cloudMoveEnd:
								self.cloudMoveEnd = None
#~ 1.2. Move Cloud away from Paddle
			else:
				cloud = self.stimuli['cloud']
				spaceLeftOfPaddle = paddle.position[0]-paddle.size[0]/2
				spaceRightOfPaddle = self.gamingWindowWidth-(paddle.position[0]+paddle.size[0]/2)
				if spaceLeftOfPaddle > spaceRightOfPaddle:
					Force = -0.2
				else:
					Force = 0.2
				CloudLocation = cloud.position[0]/self.gamingWindowWidth
				PaddleLocation = paddle.position[0]/self.gamingWindowWidth
				ForceImpact = 1 - abs(CloudLocation-PaddleLocation)			
				Force = Force*ForceImpact
				Force = (round(Force*10)/10)
				if Force <= 0.1 and Force >= -0.1:
					Force = 0
				speedChange = float(random.randint(0,6))/10-0.3+Force
				currentCloudSpeed = float(self.states['CloudSpeedTimes10'])/10
				if self.states['CloudSpeedSign'] == 1:
					currentCloudSpeed = currentCloudSpeed*(-1);
				newCloudSpeed = currentCloudSpeed+speedChange
				if (newCloudSpeed < 0) and (cloud.position[0]<=cloud.size[0]/2):
					newCloudSpeed = 1
				elif (newCloudSpeed > 0) and (cloud.position[0]>=self.gamingWindowWidth-cloud.size[0]/2):
					newCloudSpeed = -1
				if newCloudSpeed < -3:
					newCloudSpeed = -3
				if newCloudSpeed > 3:
					newCloudSpeed = 3
				if newCloudSpeed < 0:
					self.states['CloudSpeedSign'] = 1
				else:
					self.states['CloudSpeedSign'] = 0
				self.states['CloudSpeedTimes10'] = abs(newCloudSpeed)*10
				
				cloudSpeed = float(self.states['CloudSpeedTimes10'])/10
				if self.states['CloudSpeedSign'] == 1:
					cloudSpeed = cloudSpeed*(-1)
				cloud.position = (cloud.position[0]+cloudSpeed,cloud.position[1])
				
				self.states['CloudPosXTimes10000'] = round((cloud.position[0]/self.gamingWindowWidth)*10000)
				self.states['CloudPosYTimes10000'] = round((cloud.position[1]/self.screen.size[1])*10000)				
					
#~ 1.3.	Release Drops
			if self.DropReleaseCount <= 0:
				drop = self.inactiveDrops.pop()
				drop.position = (cloud.position[0], cloud.position[1]-cloud.size[1]/2-drop.size[1]/2)
				drop.on = True
				self.activeDrops.append(drop)
				self.DropReleaseCount = self.DropReleaseInterval
				if not self.targetDrop:
					self.targetDrop = drop
			else: self.DropReleaseCount = self.DropReleaseCount-(float(self.states['GravityTimes100'])/100)		

#~ 1.4.	Moving Drops down and checking for Catch by paddle
			for d in self.activeDrops:
				if d.position[1] <= d.size[1]/2:
					self.missDrop.play()
					d.on = False
					self.activeDrops.remove(d)
					self.inactiveDrops.append(d)
				else:
					d.position = (d.position[0], d.position[1]-(float(self.states['GravityTimes100'])/100))					
			if self.targetDrop:
				target = self.targetDrop
				if target.position[1] <= paddle.size[1]+target.size[1]/2:
					if target.position[0] >= paddle.position[0]-paddle.size[0]/2+target.size[0]/2 and target.position[0] <= paddle.position[0]+paddle.size[0]/2-target.size[0]/2:
						self.states['Hits'] = self.states['Hits']+1
						self.states['JustHit'] = 1
						if phasename in ['play']:
							self.states['HitsInRow'] = self.states['HitsInRow']+1 #reward catches in row and low distance to center of paddle
							DistanceFactor_PaddleCenter_Drop = 1-round((abs(paddle.position[0]-target.position[0])/(paddle.size[0]/2)),1)
							DifficultyFactor = 1 + self.gameDifficulty-self.lowestGameDifficulty
							if DifficultyFactor < 0: DifficultyFactor = 0
							score = 5*DistanceFactor_PaddleCenter_Drop*self.states['HitsInRow']
							if score < 1: score = 1
							self.score = self.score + score + score * DifficultyFactor/10
							self.states['Score'] = self.score
							self.stimuli['Score'].text = str(int(self.score))
							if self.score >= self.highscore:
								self.highscore = self.score
								self.states['Highscore'] = self.highscore
								self.stimuli['Highscore'].text = str(int(self.highscore))
								self.stimuli['Highscore'].color = (0.5, 0.8, 0.5)
								self.stimuli['HighscoreLabel'].color = (0.5, 0.8, 0.5)
							self.stimuli['water'].size = (paddle.size[0], self.fullWaterHeight*(float(int(self.states['Hits']) % int(self.params['DropsTillBonus']))/(int(self.params['DropsTillBonus']))))
							# check if enough drops where caught
							if (int(self.states['Hits']) % int(self.params['DropsTillBonus'])) == 0:
								self.change_phase()
						else:
							# adjusting paddle size
							if self.growthDirection == -1:  #reversal occured
								self.difficultyHistory.append(self.gameDifficulty)
							self.growthDirection = 1
							self.gameDifficulty = self.gameDifficulty + 1
							if self.gameDifficulty < self.lowestGameDifficulty:
								newPaddleWidth = float(self.params['MaximumPaddleWidth'])
							elif self.gameDifficulty > self.highestGameDifficulty:
								newPaddleWidth = float(self.params['MinimumPaddleWidth'])
							else:
								newPaddleWidth = float(self.params['InitPaddleSize'][0])*math.pow(float(self.params['PaddleWidthHitFactor']),self.gameDifficulty)
							# determine Gravity
							if self.gameDifficulty > 1.0:
								newGravity = (float(self.params['InitGravity'])*(1+(float(self.params['GravityHitFactor'])*self.gameDifficulty)))
							elif self.gameDifficulty < self.lowestGameDifficulty:
								if self.gameDifficulty < self.lowestGravityDifficulty:
									newGravity = float(self.params['MinimalGravity'])
								else:
									negativeSpeedDiff = self.lowestGameDifficulty - self.gameDifficulty
									newGravity = (float(self.params['InitGravity'])*(1-(float(self.params['GravityHitFactor'])*negativeSpeedDiff)))
							else:
								newGravity = float(self.params['InitGravity'])
							self.states['GravityTimes100'] = newGravity*100
							paddle_width = round(newPaddleWidth*self.gamingWindowWidth)
							paddle.size = (paddle_width, paddle.size[1])
							if paddle_width*0.7 < round(self.gamingWindowWidth*float(self.params['DropSize'][0]),1):
								for d in self.activeDrops: d.size=(paddle.size[0]*0.7, d.size[1])
								for d in self.inactiveDrops: d.size=(paddle.size[0]*0.7, d.size[1])
							self.states['DropWidthTimes10000'] = round((self.stimuli['Drop_0'].size[0]/self.gamingWindowWidth)*10000)
							new_water_height = self.stimuli['water'].size[1]+1
							if new_water_height > self.fullWaterHeight: new_water_height = self.fullWaterHeight
							self.stimuli['water'].size = (paddle.size[0], new_water_height)
							self.states['PaddleWidthTimes10000'] = round((paddle_width/self.gamingWindowWidth)*10000)
						self.hitDrop.play()
						target.on = False
						index = self.activeDrops.index(target)
						if len(self.activeDrops) > index+1:
							self.targetDrop = self.activeDrops[index+1]
						else: self.targetDrop =[]
						self.activeDrops.remove(target)
						self.inactiveDrops.append(target)	
					else:
						self.states['Misses'] = self.states['Misses']+1
						self.states['JustMissed'] = 1
						self.states['HitsInRow'] = 0
						if phasename in ['determinePaddleSize']: 
							# adjusting paddle size
							if self.growthDirection == 1:  #reversal occured
								self.difficultyHistory.append(self.gameDifficulty)
							self.growthDirection = -1
							self.gameDifficulty = self.gameDifficulty + (-1*float(self.params['TargetAccuracy'])/(1-float(self.params['TargetAccuracy'])))
							if self.gameDifficulty < self.lowestGameDifficulty:
								newPaddleWidth = float(self.params['MaximumPaddleWidth'])
							elif self.gameDifficulty > self.highestGameDifficulty:
								newPaddleWidth = float(self.params['MinimumPaddleWidth'])
							else:
								newPaddleWidth = float(self.params['InitPaddleSize'][0])*math.pow(float(self.params['PaddleWidthHitFactor']),self.gameDifficulty)# determine Gravity
							if self.gameDifficulty > 1.0:
								newGravity = (float(self.params['InitGravity'])*(1+(float(self.params['GravityHitFactor'])*self.gameDifficulty)))
							elif self.gameDifficulty < self.lowestGameDifficulty:
								if self.gameDifficulty < self.lowestGravityDifficulty:
									newGravity = float(self.params['MinimalGravity'])
								else:
									negativeSpeedDiff = self.lowestGameDifficulty - self.gameDifficulty
									newGravity = (float(self.params['InitGravity'])*(1-(float(self.params['GravityHitFactor'])*negativeSpeedDiff)))
							else:
								newGravity = float(self.params['InitGravity'])
							self.states['GravityTimes100'] = newGravity*100
							paddle_width = round(newPaddleWidth*self.gamingWindowWidth)
							paddle.size = (paddle_width, paddle.size[1])
							if paddle_width*0.7 < round(self.gamingWindowWidth*float(self.params['DropSize'][0]),1):
								for d in self.activeDrops: d.size=(paddle.size[0]*0.7, d.size[1])
								for d in self.inactiveDrops: d.size=(paddle.size[0]*0.7, d.size[1])
							else:
								for d in self.activeDrops: d.size=(round(self.gamingWindowWidth*float(self.params['DropSize'][0]),1), d.size[1])
								for d in self.inactiveDrops: d.size=(round(self.gamingWindowWidth*float(self.params['DropSize'][0]),1), d.size[1])
							self.states['DropWidthTimes10000'] = round((self.stimuli['Drop_0'].size[0]/self.gamingWindowWidth)*10000)
							self.stimuli['water'].size = (paddle.size[0], self.stimuli['water'].size[1])
							self.states['PaddleWidthTimes10000'] = round((paddle_width/self.gamingWindowWidth)*10000)
						index = self.activeDrops.index(target)
						if len(self.activeDrops) > index+1:
							self.targetDrop = self.activeDrops[index+1]
						else: self.targetDrop =[]
			if self.targetDrop:		
				self.states['TargetDropPosXTimes10000'] = round((self.targetDrop.position[0]/self.gamingWindowWidth)*10000)
				self.states['TargetDropPosYTimes10000'] = round((self.targetDrop.position[1]/self.screen.size[1])*10000)	
			else:
				self.states['TargetDropPosXTimes10000'] = 0
				self.states['TargetDropPosYTimes10000'] = 0
			# check if adjusting period can be quit
			if phasename=='determinePaddleSize' and len(self.difficultyHistory)==(int(self.params['ReversalsToDiscard'])+int(self.params['ReversalsToCompute'])):
				#find median of gameDifficutly at reversals
				while len(self.difficultyHistory)>2:
					self.difficultyHistory.remove(max(self.difficultyHistory))
					self.difficultyHistory.remove(min(self.difficultyHistory))
				self.gameDifficulty = round((sum(self.difficultyHistory)/2),2)
				# if below lowest speed set to lowest speed
				if self.gameDifficulty < self.lowestGravityDifficulty:
					self.gameDifficulty = self.lowestGravityDifficulty
				# determine PaddleSize
				if self.gameDifficulty < self.lowestGameDifficulty:
					newPaddleWidth = float(self.params['MaximumPaddleWidth'])
				elif self.gameDifficulty > self.highestGameDifficulty:
					newPaddleWidth = float(self.params['MinimumPaddleWidth'])
				else:
					newPaddleWidth = float(self.params['InitPaddleSize'][0])*math.pow(float(self.params['PaddleWidthHitFactor']),self.gameDifficulty)# determine Gravity
				paddle_width = round(newPaddleWidth*self.gamingWindowWidth)
				paddle.size = (paddle_width, paddle.size[1])				
				self.stimuli['water'].size = (paddle.size[0], self.stimuli['water'].size[1])
				self.states['PaddleWidthTimes10000'] = round((paddle_width/self.gamingWindowWidth)*10000)
				if paddle_width*0.7 < round(self.gamingWindowWidth*float(self.params['DropSize'][0]),1):
					for d in self.activeDrops: d.size=(paddle.size[0]*0.7, d.size[1])
					for d in self.inactiveDrops: d.size=(paddle.size[0]*0.7, d.size[1])
				else:
					for d in self.activeDrops: d.size=(round(self.gamingWindowWidth*float(self.params['DropSize'][0]),1), d.size[1])
					for d in self.inactiveDrops: d.size=(round(self.gamingWindowWidth*float(self.params['DropSize'][0]),1), d.size[1])		
				# determine Gravity
				if self.gameDifficulty > 1.0:
					newGravity = (float(self.params['InitGravity'])*(1+(float(self.params['GravityHitFactor'])*self.gameDifficulty)))
				elif self.gameDifficulty < self.lowestGameDifficulty:
					negativeSpeedDiff = self.lowestGameDifficulty - self.gameDifficulty
					newGravity = (float(self.params['InitGravity'])*(1-(float(self.params['GravityHitFactor'])*negativeSpeedDiff)))
				else:
					newGravity = float(self.params['InitGravity'])
				self.states['GravityTimes100'] = newGravity*100
				self.change_phase()

#~ 1.5.	Training Normalizer (Calibration during Game)
			if float(self.params['NormalizerMode']) == 1:
				if self.activeDrops:
					allLeft = True
					allRight = True
					for d in self.activeDrops:
						if not d.position[0]+d.size[0] < paddle.position[0]-paddle.size[0]/2:
							allLeft = False
						if not d.position[0]-d.size[0] > paddle.position[0]+paddle.size[0]/2:
							allRight = False
					if allLeft or allRight:  					
						if self.states['Feedback'] == 0:
							self.remember('Feedback_Change')						
						self.states['Feedback'] = 1
						if allLeft: self.states['TargetCode'] = 1
						else: self.states['TargetCode'] = 2
					else:
						if self.states['Feedback'] == 1:
							self.remember('Feedback_Change')
						self.states['Feedback'] = 0
						self.states['TargetCode'] = 0	
				if self.states['Feedback'] == 1 and (self.since('Feedback_Change')['msec'] >= (float(self.params['NormalizerUpdate'])*1000)):
					self.states['Feedback'] = 0

				
		if phasename in ['bonus']:
			#~ 2.1. Spaceship Movement
			#~ 2.2. Rocket release
			#~ 2.3. Moving Rockets down and checking for Collision with Spaceship (TODO:Score)

#~ Measuring Distance
			self.distance = self.distance - (float(self.states['GravityTimes100'])/100)
			if self.distance < 0: self.distance = 0
			normalized = 1-(self.distance/self.planetDistance)
			self.stimuli['MiniSpaceship'].position = (self.stimuli['MiniSpaceship'].position[0], self.stimuli['Trajectory'].position[1]+self.stimuli['Trajectory'].size[1]*normalized)
			self.states['DistanceTimes10'] = round((1-normalized)*10000)

#~ 2.1. Move Shaceship at current speed (self.spaceshipSpeed assigned in Process)
#~		Check that it remains in the gaming window
			spaceship = self.stimuli['spaceship']
			spaceship.position = (spaceship.position[0]+self.paddleSpeed, spaceship.position[1])
			if spaceship.position[0] < spaceship.size[0]/2:
				spaceship.position = (spaceship.size[0]/2, spaceship.position[1])
			if spaceship.position[0] > self.gamingWindowWidth-spaceship.size[0]/2:
				spaceship.position = (self.gamingWindowWidth-spaceship.size[0]/2, spaceship.position[1])
			self.states['SpaceshipPosXTimes10000'] = round((spaceship.position[0]/self.gamingWindowWidth)*10000)	
			self.states['SpaceshipPosYTimes10000'] = round((spaceship.position[1]/self.screen.size[1])*10000)
			
#~ 2.2. Release next row of Rockets according to structure of RocketPattern
#~		For activeRockets use list in list pattern to identify different rows of rockets
			if self.RocketReleaseCount <= 0:
				if self.nextRocketPatternRow < self.RocketPattern.shape[0] and self.distance > self.penaltyDistance:
					pattern = self.RocketPattern[self.nextRocketPatternRow]
					releaseColumns = numpy.nonzero(pattern>0)[0]
					newRocketRow = []
					for c in releaseColumns:
						rocket = self.inactiveRockets.pop()
						rocket.position = (self.RocketFallingPositions[c], self.screen.size[1]-rocket.size[1]/2)
						rocket.on = True
						newRocketRow.append(rocket)
					if not self.activeRockets:
						states = ['Rocket0PosXTimes10000', 'Rocket1PosXTimes10000', 'Rocket2PosXTimes10000', 'Rocket3PosXTimes10000', 'Rocket4PosXTimes10000',
								'Rocket5PosXTimes10000', 'Rocket6PosXTimes10000', 'Rocket7PosXTimes10000', 'Rocket8PosXTimes10000', 'Rocket9PosXTimes10000']
						for r, s in zip(newRocketRow, states[0:len(newRocketRow)]):
							self.states[s] = round((r.position[0]/self.gamingWindowWidth)*10000)
					self.activeRockets.append(newRocketRow)
					self.RocketReleaseCount = self.RocketReleaseInterval
					self.nextRocketPatternRow = self.nextRocketPatternRow + 1
				else:	# approaching planet
					self.stimuli['bigPlanet'].position = (self.gamingWindowWidth/2, self.screen.size[1]+self.stimuli['bigPlanet'].size[1]/2)
					self.stimuli['bigPlanet'].on = True
					self.RocketReleaseCount = self.RocketReleaseInterval*10
			else:			
				self.RocketReleaseCount = self.RocketReleaseCount-(float(self.states['GravityTimes100'])/100)
			#Moving big planet down if necessary
			if self.stimuli['bigPlanet'].on == True and self.stimuli['instructions1'].on == False:
				self.stimuli['bigPlanet'].position = (self.stimuli['bigPlanet'].position[0], self.stimuli['bigPlanet'].position[1]-(float(self.states['GravityTimes100'])/100))
			if self.stimuli['instructions1'].on == True:
				time.sleep(3)
				self.change_phase(phasename='pre_determinePaddleSize')
			if self.stimuli['bigPlanet'].position[1] < self.screen.size[1]/2:
				self.stimuli['bigPlanet'].position = (self.gamingWindowWidth/2, self.screen.size[1]/2)
				self.stimuli['instructions1'].text = 'Congratulations!'
				self.stimuli['instructions1'].color = (0.0,0.0,0.0)
				self.stimuli['instructions1'].on = True
				self.stimuli['instructions2'].text = 'You reached the Planet!'
				self.stimuli['instructions2'].color = (0.0,0.0,0.0)
				self.stimuli['instructions2'].on = True				
				if self.states['Planets']>0 and self.states['Planets']%10 == 0:
					for p in self.planets: p.on = False
					text = '%d +' %((self.states['Planets']/10)*10)
					self.stimuli['planets'].text = text
					self.stimuli['planets'].on = True
				self.planets[(self.states['Planets']%10)].on = True
				self.states['Planets'] = self.states['Planets']+1
					
			
#~ 2.3.	Moving all rockets down and check if first row hit the ground
#~		For Rockets of first row check for Collision with spaceship --> if there is one change phase
			for row in self.activeRockets:
				for r in row:
					r.position = (r.position[0], r.position[1]-(float(self.states['GravityTimes100'])/100))
			if self.activeRockets:
				firstrow = self.activeRockets[0]
				rocket = firstrow[0]
				if rocket.position[1] < rocket.size[1]/2:	#first row hit ground
					for r in firstrow:
						r.on = False
						self.inactiveRockets.append(r)
						self.states['AvoidedSingle'] = self.states['AvoidedSingle']+1
						self.score = self.score + 10*(int(self.states['AvoidedRows'])+1)
					del self.activeRockets[0]
					self.states['Score'] = self.score
					self.stimuli['Score'].text = str(int(self.score))
					if self.score >= self.highscore:
								self.highscore = self.score
								self.states['Highscore'] = self.highscore
								self.stimuli['Highscore'].text = str(int(self.highscore))
								self.stimuli['Highscore'].color = (0.5, 0.8, 0.5)
								self.stimuli['HighscoreLabel'].color = (0.5, 0.8, 0.5)
					self.states['AvoidedRows'] = self.states['AvoidedRows']+1
					states = ['Rocket0PosXTimes10000', 'Rocket1PosXTimes10000', 'Rocket2PosXTimes10000', 'Rocket3PosXTimes10000', 'Rocket4PosXTimes10000',
							'Rocket5PosXTimes10000', 'Rocket6PosXTimes10000', 'Rocket7PosXTimes10000', 'Rocket8PosXTimes10000', 'Rocket9PosXTimes10000']
					if self.activeRockets:					#new first row has already been released
						for r, s in zip(self.activeRockets[0], states[0:len(self.activeRockets[0])]):
							self.states[s] = round((r.position[0]/self.gamingWindowWidth)*10000)
						for s in states[len(self.activeRockets[0]):]:
							self.states[s] = 0
					else:
						for s in states:
							self.states[s] = 0
			if self.activeRockets:		#if first row is available, check for collision
				spaceship = self.stimuli['spaceship']
				if self.activeRockets[0][0].position[1] <= spaceship.size[1]+self.activeRockets[0][0].size[1]/2:
					for r in self.activeRockets[0]:
						if (r.position[0]+r.size[0]/2 > spaceship.position[0]-spaceship.size[0]/2) and (r.position[0]-r.size[0]/2 < spaceship.position[0]+spaceship.size[0]/2):
							self.explosion.play()
							self.distance = self.distance+self.penaltyDistance
							if self.distance > self.planetDistance:
								self.distance = self.planetDistance
							self.change_phase()
				self.states['LowestRocketRowPosYTimes10000'] = round((self.activeRockets[0][0].position[1]/self.screen.size[1])*10000)

#############################################################			
			
	def Event(self, phasename, event):
		# to control paddle with keyboard
		if phasename in ['play','bonus', 'determinePaddleSize']:
			if event.type == pygame.KEYDOWN:
				if event.key == pygame.K_RIGHT:
					self.paddleSpeed = 3
				if event.key == pygame.K_LEFT:
					self.paddleSpeed = -3
			if event.type == pygame.KEYUP:
				if event.key in [pygame.K_RIGHT, pygame.K_LEFT]:
					self.paddleSpeed = 0
		
		# allow subjects to take a break before paddle adjustment		
		if phasename in ['pre_determinePaddleSize', 'past_determinePaddleSize']:
			if event.type == pygame.KEYDOWN:
				self.change_phase()				
					
	#############################################################		

	def Process(self, sig):
		# control paddle with signal defined in PaddleExpression
		val = eval(self.expr)
		
		if int(self.params['UseDirectWiimoteControl'])==1:
			if val < float(self.params['DirectControlZeroThreshold']) and val > -float(self.params['DirectControlZeroThreshold']): val = 0.0
			self.smoothDirectControl.append(val)
			del self.smoothDirectControl[:-int(self.params['DirectControlSmoothingLength'])]
			val = sum(self.smoothDirectControl)/len(self.smoothDirectControl)
		
		self.paddleSpeed = val
	
		if self.endFeedback == True:
			self.states['TargetCode']=0
			self.endFeedback = False
			self.change_phase(phasename='preplay_text')
	
		
		
		