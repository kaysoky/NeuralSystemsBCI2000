import os,sys
import BCI2000.DataFiles as DataFiles
import BCI2000.Classification as Classification

class CalibrationRun( object ):
	
	def __init__( self, directory, filestem ):
		self.directory = directory
		self.filestem = filestem
		self.datfile = None
		self.pkfile = None
		self.date = None
		self.ntrials = None
		self.loto = None
		self.loro = None
		self.selected = False
		self.CheckFiles()
		
	def CheckFiles( self ):
		datfile = self.filestem + '.dat'
		if os.path.isfile( os.path.join( self.directory, datfile ) ): self.datfile = datfile
		else: self.datfile = ''

		pkfile = self.filestem + '.pk'
		if os.path.isfile( os.path.join( self.directory, pkfile ) ): self.pkfile = pkfile
		else: self.pkfile = ''
		
	def Describe( self, attr ):
		if attr == 'selected': return {True:'[x]', False:'[ ]'}.get( self.selected, '???' )
		if attr == 'datfile': return {True:'(no .dat file)'}.get( self.datfile=='', self.datfile ) 
		if attr == 'pkfile': return {True:'(no .pk file)'}.get( self.pkfile=='', self.pkfile ) 
		return '???'
		
	def __repr__( self ):
		return '<%s object at 0x%08X>: %s' % ( self.__class__.__name__, id( self ), str( self ) )	

	def __str__( self ):
		return '   '.join( self.report() )
	
	def report( self ):
		s = []
		s.append( self.Describe( 'selected' ) )
		s.append( self.Describe( 'datfile' ) )
		s.append( self.Describe( 'pkfile' ) )
		return s
		
class CalibrationManager( object ):
	
	def __init__( self, directory='.' ):
		self.directory = None
		self.runs = {}
		self.ScanDirectory( directory  )
		
	def ScanDirectory( self, directory=None ):
		if directory != None:
			newdir = os.path.abspath( directory )
			if newdir != self.directory:
				self.runs.clear()
				self.directory = newdir
				
		if self.directory == None: raise ValueError( 'no directory set' )
		stems = sorted( set( [os.path.splitext( x )[0] for x in os.listdir( self.directory ) if x.lower().endswith( ('.dat', '.pk') ) ] ) )
		for stem in stems:
			if stem not in self.runs: self.runs[stem] = CalibrationRun( self.directory, stem )
			else: self.runs[stem].CheckFiles()
	
	def GetRuns( self ):
		return [ v for k,v in sorted( self.runs.items() ) ]  # TODO: sort magically taking into account numbers in filenames
	
	def __repr__( self ):
		return '<%s object at 0x%08X>:\n  %s' % ( self.__class__.__name__, id( self ), str( self ).replace( '\n', '\n  ' ) )	

	def __str__( self ):
		s = []
		s.append( 'directory: %s' % self.directory )
		if len( self.runs ):
			rows = [ x.report() for x in self.GetRuns() ]
			lengths = [ 2+max([len(row[colIndex]) for row in rows]) for colIndex,blah in enumerate( rows[0] ) ]
			s += [ ''.join([ (x+' '*(width-len(x))) for width,x in zip( lengths,row ) ] ) for row in rows]				
		else:
			s.append( 'no files found' )
		return '\n'.join( s )

import Tkinter as tk

class Table( tk.Frame ):
	'''
	A Tk Widget, subclassed from Tk.Frame, containing optional horizontal and vertical
	Scrollbars plus a Canvas which reacts to the Scrollbars and which in turn contains
	another Frame.
	
	This was made from a magic recipe given by Bryan Oakley 2010-06-23 in answer to
	http://stackoverflow.com/questions/3085696/adding-a-scrollbar-to-a-grid-of-widgets-in-tkinter
	namely: make a Canvas and a Scrollbar, put a Frame inside the Canvas, and configure some
	rather incomprehensible interrelationships between the three. I put the whole thing in
	an additional Frame (self) to bind the Scrollbar and Canvas together and make it a
	modular pack()able Widget. I also added the GetCell method (delivering yet more Frames).
	'''
	def __init__( self, parent, *pargs, **kwargs ):
		vscroll = kwargs.pop( 'vscroll', True )
		hscroll = kwargs.pop( 'hscroll', True )
		tk.Frame.__init__( self, parent, *pargs, **kwargs )
		
		self.__canvas = tk.Canvas( self, borderwidth=0, background="#ffffff" )
		
		if vscroll:
			self.__vsb = tk.Scrollbar( self, orient="vertical", command=self.__canvas.yview )
			self.__vsb.pack( side="right", fill="y" )
			self.__canvas.configure( yscrollcommand=self.__vsb.set )
			
		if hscroll:
			self.__hsb = tk.Scrollbar( self, orient="horizontal", command=self.__canvas.xview )
			self.__hsb.pack( side="bottom", fill="x" )
			self.__canvas.configure( xscrollcommand=self.__hsb.set )
				
		
		self.__frame = tk.Frame( self.__canvas, background="#ffffff" )
		self.__frame.bind( "<Configure>", self.OnFrameConfigure )
		self.__canvas.create_window( (4,4), window=self.__frame, anchor="nw", tags="self.__frame" )
		self.__canvas.pack( side="left", fill="both", expand=True )
		self.__cells = {}
		
	def OnFrameConfigure( self, event ):
		'''Reset the scroll region to encompass the inner frame'''
		self.__canvas.configure( scrollregion=self.__canvas.bbox("all") )
		
	def GetCell( self, row, column, **kwargs ):
		key = (row, column)
		cell = self.__cells.get( key, None )
		if cell == None:
			if 'borderwidth' not in kwargs: kwargs['borderwidth'] = '1'
			if 'relief' not in kwargs: kwargs['relief'] = 'ridge'
			cell = self.__cells[key] = tk.Frame( self.__frame, **kwargs )
			cell.grid( row=row, column=column )
		return cell
	
	def DestroyCells( self ):
		while len( self.__cells ): self.__cells.pop( self.__cells.keys()[0] ).destroy()

	def Populate( self ):
		'''Put in some fake data'''
		for row in range( 100 ):
			tk.Label( self.__frame, text=str(row), width=3, borderwidth="1", relief="solid" ).grid( row=row, column=0 )
			tk.Button( self.__frame, text="this is the second column for row %s"%row ).grid( row=row, column=1 )
			
	def Populate2( self ):
		'''Put in some fake data'''
		for row in range( 100 ):
			tk.Label(  self.GetCell( row, 0 ), text=str(row), width=3, borderwidth="1", relief="solid" ).pack()
			tk.Button( self.GetCell( row, 1 ), text="this is the 2nd column for row %s"%row ).pack()

class CalibrationTableRow( object ):
	def __init__( self, table, rowNumber, run ):
		"""
		Manages all the Tk Widgets in one row of the Table g.table belonging to a CalibrationGUI g.
		"""
		self.run = run
		self.datfileLabel = tk.Label( table.GetCell( rowNumber, 0 ), text=run.datfile ); self.datfileLabel.pack()
		self.pkfileLabel  = tk.Label( table.GetCell( rowNumber, 1 ), text=run.pkfile  ); self.pkfileLabel.pack()
		self.selectedCheckbutton = tk.Checkbutton( table.GetCell( rowNumber, 2 ) ); self.selectedCheckbutton.pack()
		self.selected = tk.IntVar( self.selectedCheckbutton, run.selected )
		self.selectedCheckbutton.config( variable = self.selected, command=self.CheckbuttonCallback ) # couldja make this mechanism *any* more awkward??
		
	def CheckbuttonCallback( self ):
		self.run.selected = bool( self.selected.get() )
	
	def destroy( self ):
		self.selected = None
		self.selectedCheckbutton.destroy()
		self.pkfileLabel.destroy()
		self.datfileLabel.destroy()
		self.run = None
		
	def __del__( self ):
		self.destroy()

class CalibrationGUI( tk.Tk ):
	"""
	Graphically displays the content of a CalibrationManager object and allows the
	user to interact with the CalibrationManager methods in clicky-clicky fashion.
	"""
	def __init__( self, manager=None, go=True ):
		tk.Tk.__init__( self )
		self.minsize( width=800, height=200 )
		
		self.__rows = []

		if manager == None: manager = '.'
		if isinstance( manager, basestring ): manager = CalibrationManager( manager )
		self.__manager = manager
		self.Render()
		
		if go: self.mainloop()
		
	def Render( self ):
	
		fr = tk.Frame( self ); fr.pack( fill='x' )
		self.directoryEntry = tk.Entry( fr, validatecommand=self.UpdateTable )
		self.directoryEntry.delete( 0, tk.END )
		self.directoryEntry.insert(0, self.__manager.directory )
		tk.Button( fr, text='Update', command=self.UpdateTable ).pack( side='right' )
		self.directoryEntry.pack( fill='x' )
		
		self.table = Table( self, vscroll=True, hscroll=True )
		self.table.pack( fill='both', expand=True )

		tk.Button( self, text='Exit', command=self.destroy ).pack()
		
		self.UpdateTable()
		
	
	def UpdateTable( self ):
		self.__manager.ScanDirectory( self.directoryEntry.get() )
		self.directoryEntry.delete( 0, tk.END ); self.directoryEntry.insert(0, self.__manager.directory )
		while len( self.__rows ): self.__rows.pop( 0 ).destroy()
		self.table.DestroyCells()
		for row,(stem,run) in enumerate( sorted( self.__manager.runs.items() ) ):
			self.__rows.append( CalibrationTableRow( self.table, row, run ) )
	
if __name__ == "__main__":
	m = CalibrationManager( directory='20111006_8525_A_002' )
	j = CalibrationGUI( manager=m )
