#!/usr/bin/env python
import os,sys,shutil
import Tkinter as tk

class SessionGUI( tk.Tk ):

	def __init__( self, settingsfile=None, go=True ):
		tk.Tk.__init__( self )
		self.minsize( width=800, height=200 )
		
		if settingsfile == None: settingsfile = 'SessionGUISettings.txt'
		self.__settingsfile = settingsfile
		self.__optionprefix = '_PossibleValuesFor'
		
		self.__settings = {
			'Subject':    'TestSubject',
			'Condition':  '031',
			'Mode':       'CALIB',            self.__optionprefix + 'Mode':    [ 'CALIB', 'FREE' ],
			'Source':     'gUSBampSource',    self.__optionprefix + 'Source':  [ 'gUSBampSource' ], #, 'SignalGenerator' ],
			'Montage':    'B',                self.__optionprefix + 'Montage': [ 'B', '16' ],
			
			'_Arguments':               'Subject Condition Mode Source Montage',
			'_ChosenWeightsFileName':   'ChosenWeights.prm',
			'_DataDir':                 '../data',
			'_ProgDir':                 '../prog',
			'_ScriptFile':              '../batch3-real/core.bat',
			'_Title':                   'Launch auditory streaming experiment',
		}

		self.__inputs = {}
		self.__widgets = {}
		self.__masterframe = None
		self.LoadSettings()
		self.Render()
		
		if go: self.mainloop()
		else: self.update()
				
	def LoadSettings( self ):
		if not os.path.isfile( self.__settingsfile ): self.SaveSettings()
		self.__settings.update( eval( '\n'.join( open( self.__settingsfile, 'rt' ).readlines() ) ) )
		
	def ReportSettings( self, file=None ):
		if file == None: file = sys.stdout
		if isinstance( file, basestring ): file = open( file, 'wt' )
		s = dict( self.__settings )
		args = s[ '_Arguments' ].split()
		args.append( None )
		args += [ k for k in s.keys() if k.startswith( self.__optionprefix ) ]
		args.append( None )
		keys = args + sorted( set( s.keys() ) - set( args ) )
		maxlen = max( [ len(k) for k in keys if k != None ] )
		file.write( '{\n' )
		for k in keys:
			if k == None: file.write( '\n' )
			else: file.write( ' ' * ( maxlen - len( k ) + 4 ) + repr(k) + ':   ' + repr( s[ k ] ) + ',\n' )
		file.write( '}\n' )
			
	def SaveSettings( self ):
		self.ReportSettings( file=self.__settingsfile )
	
	def Render( self ):
	
		self.title( self.__settings[ '_Title' ] )
		
		mf = self.__masterframe
		if mf != None: mf.destroy()
		self.__masterframe = mf = tk.Frame( self ); mf.pack( fill='both' )
		
		left = tk.Frame( mf ); left.pack( side='left' )
		fr = left
		#fr = tk.Frame( mf ); #fr.pack( fill='x' )
		
		row  = 0
		
		for row, varName in enumerate( self.__settings[ '_Arguments' ].split() ):
			
			w = self.__widgets[ varName + 'Label' ] = tk.Label( fr, text=varName + ': ' )
			w.grid( row=row, column=0, sticky='e' )
			optionsKey = self.__optionprefix + varName
			if optionsKey in self.__settings:
				v = self.__inputs[ varName ] = tk.StringVar( value=self.__settings[ varName ] )
				w = self.__widgets[ varName + 'Input' ] = tk.OptionMenu( fr, v, *self.__settings[ optionsKey ], command=self.UpdateVars )
			else:
				w = self.__widgets[ varName + 'Input' ] = self.__inputs[ varName ] = tk.Entry( fr, validatecommand=self.UpdateVars )
				w.delete( 0, tk.END )
				w.insert( 0, self.__settings[ varName ] )
			w.grid( row=row, column=1, sticky='w' )
		
		right = tk.Frame( mf, background='#FFFFFF' ); right.pack( side='left', padx=50 )
		fr = right

		self.dirLabels = {}
		self.dirLabels[ 'CALIB' ] = w = tk.Label( fr, text='(calibration directory info)'); w.pack( fill='x' )
		self.dirLabels[ 'FREE' ]  = w = tk.Label( fr, text='(free-choice directory info)'); w.pack( fill='x' )

		#footer = tk.Frame( mf ); footer.pack( fill='x' )
		#fr = footer

		self.transferWeightsButton = w = tk.Button( fr, text='Transfer weights CALIB->FREE', command=self.TransferChosenWeights ); w.pack( side='left' )
		self.launchBCI2000Button = w = tk.Button( self, text='Launch BCI2000', command=self.LaunchBCI2000 ); w.pack( side='left' )
		self.quitBCI2000Button = w = tk.Button( self, text='Quit BCI2000', command=self.QuitBCI2000 ); w.pack( side='left' )
		self.reloadWeightsButton = w = tk.Button( self, text='Reload weights', command=self.ReloadChosenWeights ); w.pack( side='left' )
		self.exitButton = w = tk.Button( self, text='Exit', command=self.destroy ); w.pack( side='left' )

		self.UpdateVars()
		for w in self.__widgets.values():
			if isinstance( w, tk.Entry ): w.configure( validate='focusout' ) # 'all' would be nice but it always seems to lag one keystroke behind :-<
			
	def UpdateVars( self, *pargs ):
		for k in self.__inputs: self.__settings[ k ] = self.__inputs[ k ].get()
		self.SaveSettings()
		self.UpdateDisplay()
		return True
		
	def UpdateDisplay( self ):
		for mode in [ 'CALIB', 'FREE' ]:
			d = self.GetDataDirectory( mode=mode )
			s = 'directory ' + self.GetSubjectDirName( mode=mode )
			if os.path.isdir( d ):
				s += ' : '
				f = [ x for x in os.listdir( d ) if x.lower().endswith( '.dat' ) ]
				if len( f ) == 0: s += 'no data files'
				elif len( f ) == 1: s += '1 data file'
				else: s += '%d data files' % len( f )
				w = self.GetChosenWeightsPath( mode=mode )
				if os.path.isfile( w ): s += ', weights chosen'
				else: s += ', no weights'
			else:
				s += ' does not yet exist'
			s += '\n'
			if self.__settings[ 'Mode' ] == mode: bg = '#FFFF00'
			else: bg = '#FFFFFF'
			self.dirLabels[ mode ].configure( text=s, justify='left', background=bg )

		if os.path.isfile( self.GetChosenWeightsPath() ): self.reloadWeightsButton.configure( state='normal' )
		else: self.reloadWeightsButton.configure( state='disabled' )

		if os.path.isfile( self.GetChosenWeightsPath( mode='CALIB' ) ): self.transferWeightsButton.configure( state='normal' )
		else: self.transferWeightsButton.configure( state='disabled' )
		
		try: self.after_cancel( self.__afterid )
		except: pass
		self.__afterid = self.after( 500, self.UpdateDisplay )

	def GetShellExecutable( self ):
		return os.path.realpath( os.path.join( self.__settings[ '_ProgDir' ], 'BCI2000Shell' ) )
	
	def GetDataDirectory( self, **kwargs ):
		return os.path.realpath( os.path.join( self.__settings[ '_DataDir' ], self.GetSubjectDirName( **kwargs ) ) )
	
	def GetSubjectDirName( self, subject=None, condition=None, mode=None ):
		if subject == None:   subject   = self.__settings.get( 'Subject', '' )
		if condition == None: condition = self.__settings.get( 'Condition', '' )
		if mode == None:      mode      = self.__settings.get( 'Mode', '' )
		return subject + mode.upper() + condition
	
	def GetChosenWeightsPath( self, **kwargs ):
		return os.path.join( self.GetDataDirectory( **kwargs ), self.__settings['_ChosenWeightsFileName'] )
	
	def GetScriptFile( self ):
		return os.path.realpath( self.__settings[ '_ScriptFile' ] )
		
	def GetScriptDir( self ):
		return os.path.split( self.GetScriptFile() )[ 0 ]
	
	def GetLaunchCommand( self ):
		args = [ self.GetShellExecutable(), self.GetScriptFile() ]
		args += [ self.__settings[ k ] for k in self.__settings[ '_Arguments' ].split() ]
		args.append( '#!' )
		return ' '.join( args )
	
	def ExecuteInShell( self, cmd ):
		return os.system( self.GetShellExecutable() + ' -c ' + cmd )
			
	def LaunchBCI2000( self ):
		self.UpdateVars() # because of the lag
		os.system( self.GetLaunchCommand() )
		self.UpdateDisplay()
		
	def QuitBCI2000( self ):
		self.UpdateVars() # because of the lag
		self.ExecuteInShell( 'quit' )
		self.UpdateDisplay()
		
	def ReloadChosenWeights( self ):
		self.UpdateVars() # because of the lag
		self.ExecuteInShell( 'load parameterfile "' + self.GetChosenWeightsPath() + '"' )
		self.ExecuteInShell( 'setconfig' )
		self.UpdateDisplay()

	def TransferChosenWeights( self ):
		self.UpdateVars() # because of the lag
		srcfile = self.GetChosenWeightsPath( mode='CALIB' )
		freedir = self.GetDataDirectory( mode='FREE' )
		if not os.path.isdir( freedir ): os.mkdir( freedir )
		dstfile = self.GetChosenWeightsPath( mode='FREE' )
		shutil.copyfile( srcfile, dstfile )
		self.UpdateDisplay()

	def destroy( self ):
		self.UpdateVars() # because of the lag
		# other cleanup
		tk.Tk.destroy( self )


if __name__ == '__main__':
	argv = getattr( sys, 'argv', [] )
	if len( argv ) >= 2: settingsfile = argv[ 1 ]
	else: settingsfile = None
	g = SessionGUI( settingsfile=settingsfile )


