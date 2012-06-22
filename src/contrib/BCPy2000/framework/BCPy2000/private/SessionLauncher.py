import os,sys
import Tkinter as tk


class SessionGUI( tk.Tk ):

	def __init__( self, go=True ):
		tk.Tk.__init__( self )
		self.minsize( width=800, height=200 )
		self.title( 'Session Launcher' )
		
		self.__settings = {
			'Subject':   'TestSubject',
			'Condition': '031',
			'Mode':      'CALIB',            '_Mode':    [ 'CALIB', 'FREE' ],
			'Source':    'gUSBampSource',    '_Source':  [ 'gUSBampSource' ], #, 'SignalGenerator' ],
			'Montage':   'B',                '_Montage': [ 'B', '16' ],
		}

		self.__directory = ''
		self.__inputs = {}
		self.__widgets = {}
		
		self.Render()
		
		if go: self.mainloop()
		else: self.update()
		
	def Render( self ):
	
		row  = 0
		fr = tk.Frame( self ); fr.pack( fill='x' )
		
		for row, varName in enumerate( 'Subject Condition Mode Source Montage'.split( ) ):
			
			w = self.__widgets[ varName + 'Label' ] = tk.Label( fr, text=varName + ': ' )
			w.grid( row=row, column=0, sticky='e' )
			if '_' + varName in self.__settings:
				v = self.__inputs[ varName ] = tk.StringVar( value=self.__settings[ varName ] )
				w = self.__widgets[ varName + 'Input' ] = tk.OptionMenu( fr, v, *self.__settings[ '_' + varName ], command=self.UpdateVars )
			else:
				w = self.__widgets[ varName + 'Input' ] = self.__inputs[ varName ] = tk.Entry( fr, validatecommand=self.UpdateVars )
				w.delete( 0, tk.END )
				w.insert( 0, self.__settings[ varName ] )
			w.grid( row=row, column=1, sticky='w' )
		
		fr = tk.Frame( self ); fr.pack( fill='x' )

		self.dirLabel = tk.Label( fr, text='data directory: ')
		self.dirLabel.pack( fill='x' )

		
		tk.Button( self, text='Launch BCI2000', command=self.LaunchBCI2000 ).pack( side='left' )
		tk.Button( self, text='Quit BCI2000', command=self.QuitBCI2000 ).pack( side='left' )
		tk.Button( self, text='Reload ChosenWeights.prm', command=self.ReloadChosenWeights ).pack( side='left' )
		tk.Button( self, text='Exit', command=self.destroy ).pack( side='left' )
		
		self.UpdateVars()
		for w in self.__widgets.values( ):
			if isinstance( w, tk.Entry ): w.configure( validate='focusout' ) # 'all' would be nice but it always seems to lag one keystroke behind :-<
		
	
	def LaunchBCI2000( self ):
		self.UpdateVars() # because of the lag
		print self.__directory
	
	def QuitBCI2000( self ):
		self.UpdateVars() # because of the lag
	
	def ReloadChosenWeights( self ):
		self.UpdateVars() # because of the lag
		
			
	def UpdateVars( self, *pargs ):
		for k in self.__settings:
			if k in self.__inputs: self.__settings[ k ] = self.__inputs[ k ].get( )
		self.__directory = self.__settings[ 'Subject' ] + self.__settings[ 'Mode' ] + self.__settings[ 'Condition' ]
		self.dirLabel.configure( text = 'directory: ' + self.__directory )
		return True
		
	def destroy( self ):
		self.UpdateVars() # because of the lag
		# other cleanup
		tk.Tk.destroy( self )
