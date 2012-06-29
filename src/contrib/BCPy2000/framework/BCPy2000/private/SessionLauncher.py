#!/usr/bin/env python
import os,sys,shutil
import Tkinter as tk

class SessionGUI( tk.Tk ):

	def __init__( self, settingsfile=None, go=True ):
		tk.Tk.__init__( self )
		self.minsize( width=800, height=200 )
		self.option_add("*Font", "40")
		
		if settingsfile == None: settingsfile = 'SessionGUISettings.txt'
		self.__settingsfile = settingsfile
		self.__optionprefix = '_PossibleValuesFor'
		self.__bg = '#DDDDDD'
		self.__host = 'localhost'
		self.__port = 3999
		self.__valid = False
		
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
		if not os.path.isfile( self.__settingsfile ): self.SaveSettings( self.ReportSettings() )
		self.__saved_settings_content = '\n'.join( open( self.__settingsfile, 'rt' ).readlines() )
		self.__settings.update( eval( self.__saved_settings_content ) )
		
	def ReportSettings( self ):
		t = ''
		s = dict( self.__settings )
		args = s[ '_Arguments' ].split()
		args.append( None )
		args += [ k for k in s.keys() if k.startswith( self.__optionprefix ) ]
		args.append( None )
		keys = args + sorted( set( s.keys() ) - set( args ) )
		maxlen = max( [ len(k) for k in keys if k != None ] )
		t += '{\n'
		for k in keys:
			if k == None: t += '\n'
			else: t += ' ' * ( maxlen - len( k ) + 4 ) + repr(k) + ':   ' + repr( s[ k ] ) + ',\n'
		t += '}\n'
		return t
			
	def SaveSettings( self, txt ):
		open( self.__settingsfile, 'wt' ).write( txt )
		self.__saved_settings_content = txt
	
	def Render( self ):
		title = self.__settings[ '_Title' ]
		try: import win32gui
		except ImportError: pass
		else:
			h = win32gui.FindWindow( 0, title )
			if h != 0:
				tk.Label( self, text='Another window called "%s" is already open.\nClose this one, and use the original.' % title ).pack()
				return
			
		self.title( title )
		self.configure( background=self.__bg )

		mf = self.__masterframe
		if mf != None: mf.destroy()
		self.__masterframe = mf = tk.Frame( self, background=self.__bg ); mf.pack( fill='both' )
		
		left = tk.Frame( mf, background=self.__bg ); left.pack( side='left' )
		fr = left
		#fr = tk.Frame( mf, background=self.__bg ); #fr.pack( fill='x' )
		
		row  = 0
		
		for row, varName in enumerate( self.__settings[ '_Arguments' ].split() ):
			
			w = self.__widgets[ varName + 'Label' ] = tk.Label( fr, text=varName + ': ', background=self.__bg )
			w.grid( row=row, column=0, sticky='e' )
			optionsKey = self.__optionprefix + varName
			if optionsKey in self.__settings:
				v = self.__inputs[ varName ] = tk.StringVar( value=self.__settings[ varName ] )
				w = self.__widgets[ varName + 'Input' ] = tk.OptionMenu( fr, v, *self.__settings[ optionsKey ] )
				#w.configure( command=self.UpdateSettings )
			else:
				w = self.__widgets[ varName + 'Input' ] = self.__inputs[ varName ] = tk.Entry( fr, validatecommand=self.UpdateSettings )
				w.delete( 0, tk.END )
				w.insert( 0, self.__settings[ varName ] )
			w.grid( row=row, column=1, sticky='w' )
		
		right = tk.Frame( mf, background=self.__bg ); right.pack( side='left', padx=50 )
		fr = right

		self.dirLabels = {}
		modes = self.GetPossibleModes()
		for row,mode in enumerate(modes): 
			self.dirLabels[ mode ] = w = tk.Label( fr, text='(%s directory info)' % mode, anchor='w' )
			w.pack( anchor='w' )
			#w.grid( row=row, column=0, sticky='w' )
		if 'CALIB' in modes and 'FREE' in modes:
			self.transferWeightsButton = w = tk.Button( fr, text='Transfer weights CALIB->FREE', command=self.TransferChosenWeights ); w.pack( side='left' )
		else:
			self.transferWeightsButton = None
			
		#footer = tk.Frame( mf, background=self.__bg ); footer.pack( fill='x' )
		#fr = footer

		self.launchBCI2000Button = w = tk.Button( self, text='Launch BCI2000', command=self.LaunchBCI2000 ); w.pack( side='left', padx=20 )
		self.quitBCI2000Button = w = tk.Button( self, text='Quit BCI2000', command=self.QuitBCI2000 ); w.pack( side='left', padx=20 )
		self.reloadWeightsButton = w = tk.Button( self, text='Reload weights', command=self.ReloadChosenWeights ); w.pack( side='left', padx=20 )
		self.exitButton = w = tk.Button( self, text='Exit', command=self.destroy ); w.pack( side='left', padx=20 )

		self.__valid = True
		self.UpdateSettings()
		for w in self.__widgets.values():
			if isinstance( w, tk.Entry ): w.configure( validate='focusout' ) # 'all' would be nice but it always seems to lag one keystroke behind :-<
		
	def GetPossibleModes( self ):
		return self.__settings.get( self.__optionprefix + 'Mode', [ 'CALIB', 'FREE' ]  )
		
		
	def UpdateSettings( self ):
		if not self.__valid: return False
		for k in self.__inputs: self.__settings[ k ] = self.__inputs[ k ].get()
		sc = self.ReportSettings()
		if sc != self.__saved_settings_content: self.SaveSettings( sc )
			
		for mode in self.GetPossibleModes():
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
			else: bg = self.__bg
			self.dirLabels[ mode ].configure( text=s, justify='left', background=bg )

		if os.path.isfile( self.GetChosenWeightsPath() ): self.reloadWeightsButton.configure( state='normal' )
		else: self.reloadWeightsButton.configure( state='disabled' )

		if self.transferWeightsButton != None:
			if os.path.isfile( self.GetChosenWeightsPath( mode='CALIB' ) ): self.transferWeightsButton.configure( state='normal' )
			else: self.transferWeightsButton.configure( state='disabled' )
		
		try: self.after_cancel( self.__afterid )
		except: pass
		self.__afterid = self.after( 500, self.UpdateSettings )
		#import time; print time.time()
		return True

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
		self.UpdateSettings() # because of the lag
		os.system( self.GetLaunchCommand() )
		self.UpdateSettings()
		
	def QuitBCI2000( self ):
		self.UpdateSettings() # because of the lag
		self.ExecuteInShell( 'quit' )
		self.UpdateSettings()
	
	def ReloadChosenWeights( self ):
		if not self.BCI2000Running(): return self.LaunchBCI2000()
		self.UpdateSettings() # because of the lag
		self.ExecuteInShell( 'load parameterfile "' + self.GetChosenWeightsPath() + '"' )
		self.ExecuteInShell( 'setconfig' )
		self.UpdateSettings()

	def TransferChosenWeights( self ):
		self.UpdateSettings() # because of the lag
		srcfile = self.GetChosenWeightsPath( mode='CALIB' )
		freedir = self.GetDataDirectory( mode='FREE' )
		if not os.path.isdir( freedir ): os.mkdir( freedir )
		dstfile = self.GetChosenWeightsPath( mode='FREE' )
		shutil.copyfile( srcfile, dstfile )
		self.UpdateSettings()

	def destroy( self ):
		self.UpdateSettings() # because of the lag
		# other cleanup
		tk.Tk.destroy( self )

	def BCI2000Running( self ):
		import telnetlib
		try: t = telnetlib.Telnet( host=self.__host, port=self.__port )
		except: return False
		else: t.close()
		return True

if __name__ == '__main__':
	argv = getattr( sys, 'argv', [] )
	if len( argv ) >= 2: settingsfile = argv[ 1 ]
	else: settingsfile = None
	g = SessionGUI( settingsfile=settingsfile )


