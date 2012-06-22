import os,sys,time,glob
import BCI2000Tools.FileReader as FileReader
import BCI2000Tools.DataFiles as DataFiles
import BCI2000Tools.Classification as Classification
import Hashing
import cPickle as pickle

# TODO
#     run method: applyweights

# -+- ORDINARY CLASSIFIER TRAINING
#  |   |
#  |   +- ORDINARY CV 
#  |   +- LEAVE-ONE-RUN-OUT 
#  |
#  +- AVERAGE OF PER-BLOCK WEIGHTS
#  |   |
#  |   +- LEAVE-ONE-RUN-OUT 

class Cacheable( object ):
	def __init__( self, cachedir, fields, prefix ):
		self.__cachedir = cachedir
		self.__cachefields = fields
		self.__cacheprefix = prefix

	def GetCacheDir( self ):
		return self.__cachedir
		
	def GetCacheFilePath( self, attrname ):
		return os.path.join( self.__cachedir, attrname + '.pk' )
		
	def SaveInfoToCache( self, keys=None ):
		if keys == None: keys = self.__cachefields
		if isinstance( keys, basestring ): keys = [ keys ]
		for k in keys:
			pickle.dump( getattr( self, self.__cacheprefix + k ), open( self.GetCacheFilePath( k ), 'wb' ), 2 )
	
	def LoadInfoFromCache( self, keys=None ):
		if keys == None: keys = self.__cachefields
		if isinstance( keys, basestring ): keys = [ keys ]
		for k in keys:
			fn = self.GetCacheFilePath( k )
			if os.path.exists( fn ):
				v = pickle.load( open( fn, 'rb' ) )
				setattr( self, self.__cacheprefix + k, v ) # hmmm

class CalibrationWeights( Cacheable ):
	
	def __init__( self, directory, stems, opts, cachedir ):
		self.__directory = directory
		self.__datestamp = None
		self.__cachedir = cachedir
		if opts == None: self.__opts = {}
		else: self.__opts = dict(opts)
	
		self.__stems = sorted( stems )
		self.__result = None
		self.__ntrials = None
		self.__cv = None
		self.__errmsg = None
		
		Cacheable.__init__( self, cachedir, [ 'result', 'cv', 'errmsg', 'ntrials' ], '_CalibrationWeights__' )
	
	def GetCacheFilePath( self, attrname ):
		h = Hashing.hash( self.stems )[ :10 ]
		return os.path.join( self.GetCacheDir(), h + '_' + attrname + '.pk' )
		
	@apply
	def stems():
		def fget( self ): return self.__stems
		return property( fget, doc="file stems of runs used to make these weights" )

	@apply
	def errmsg():
		def fget( self ): return self.__errmsg
		def fset( self, val ): self.__errmsg = val; self.SaveInfoToCache( 'errmsg' )
		return property( fget, fset, doc="error message from last attempt to CrossValidate or train on these runs" )

	@apply
	def cv():
		def fget( self ): return self.__cv
		def fset( self, val ): self.__cv = val; self.SaveInfoToCache( 'cv' )
		return property( fget, fset, doc="trained classifier object from last attempt to train on these runs" )

	@apply
	def result():
		def fget( self ): return self.__result
		def fset( self, val ): self.__result = val; self.SaveInfoToCache( 'result' )
		return property( fget, fset, doc="result from last attempt to train on these runs" )

	@apply
	def ntrials():
		def fget( self ): return self.__ntrials
		def fset( self, val ): self.__ntrials = val; self.SaveInfoToCache( 'ntrials' )
		return property( fget, fset, doc="total number of trials in each class" )
	
	def CrossValidate( self ):
		under_construction # TODO
		try:
			self.result,self.cv = Classification.ClassifyERPs( pkfile, **self.__opts )
		except Exception,e:
			self.result,self.cv = None,None
			self.errmsg = '%s: %s' % ( e.__class__.__name__, e.message )
			print self.errmsg
		else:
			self.errmsg = None
		self.ntrials = self.GetNumberOfTrials()
			
class CalibrationRun( Cacheable ):
	
	def __init__( self, directory, datfile, pkfile, opts, cachedir ):
		self.__directory = directory
		self.__datfile = datfile
		self.__pkfile = pkfile
		self.__datestamp = None
		self.__cachedir = cachedir
		self.__bcistream = None
		if opts == None: self.__opts = {}
		else: self.__opts = dict(opts)
			
		self.__result = None
		self.__ntrials = None
		self.__cv = None
		self.__errmsg = None
		self.__selected = self.GetPkFile() != None
		
		Cacheable.__init__( self, cachedir, [ 'result', 'cv', 'errmsg', 'selected', 'ntrials' ], '_CalibrationRun__' )
		
		self.LoadInfoFromCache()
	
	@apply
	def selected():
		def fget( self ): return self.__selected
		def fset( self, val ): self.__selected = bool( val ); self.SaveInfoToCache( 'selected' )
		return property( fget, fset, doc="whether or not to include this run when generating weights" )

	@apply
	def errmsg():
		def fget( self ): return self.__errmsg
		def fset( self, val ): self.__errmsg = val; self.SaveInfoToCache( 'errmsg' )
		return property( fget, fset, doc="error message from last attempt to CrossValidate or train on this run on its own" )

	@apply
	def cv():
		def fget( self ): return self.__cv
		def fset( self, val ): self.__cv = val; self.SaveInfoToCache( 'cv' )
		return property( fget, fset, doc="trained classifier object from last attempt to train on this run on its own" )

	@apply
	def result():
		def fget( self ): return self.__result
		def fset( self, val ): self.__result = val; self.SaveInfoToCache( 'result' )
		return property( fget, fset, doc="result from last attempt to train on this run on its own" )

	@apply
	def ntrials():
		def fget( self ): return self.__ntrials
		def fset( self, val ): self.__ntrials = val; self.SaveInfoToCache( 'ntrials' )
		return property( fget, fset, doc="number of trials in each class" )


	def GetStem( self ):
		f = self.GetPkFile()
		if f != None: return os.path.splitext( os.path.basename( f ) )[0]
		f = self.GetDatFile()
		if f != None: return os.path.splitext( os.path.basename( f ) )[0]
		return None
		
	def GetPkFile( self ):
		f = self.__pkfile
		if f != None:
			if not os.path.isabs( f ): f = os.path.join( self.__directory, f )
			if os.path.isfile( f ): return f
		return None
	
	def GetDatFile( self ):
		f = self.__datfile
		if f != None:
			if not os.path.isabs( f ): f = os.path.join( self.__directory, f )
			if os.path.isfile( f ): return f
		return None
		
	def GetStream( self, reread=False ):
		if self.__bcistream == None or reread: self.__bcistream = FileReader.bcistream( self.GetDatFile() )
		self.__bcistream.close()
		return self.__bcistream
		
	def GetDate( self, reread=False ):
		if self.__datestamp == None or reread:
			if self.__datfile != None and self.GetStream( reread=reread ) != None:
				self.__datestamp = self.GetStream().date()
		return self.__datestamp
	
	def GetParameter( self, paramName, reread=False ):
		b = self.GetStream( reread=reread )
		if b == None: return None
		return b.params.get( paramName, None )

	def CrossValidate( self ):
		self.ntrials = self.GetNumberOfTrials()
		try:
			pkfile = self.GetPkFile()
			if pkfile == None: raise ValueError('cannot cross-validate without a .pk file')
			self.result,self.cv = Classification.ClassifyERPs( pkfile, **self.__opts )
		except Exception,e:
			self.result,self.cv = None,None
			self.errmsg = '%s: %s' % ( e.__class__.__name__, e.message )
			print self.errmsg
		else:
			self.errmsg = None

	def Describe( self, attr ):
		if attr == 'selected': return {True:'[x]', False:'[ ]'}.get( self.selected, '???' )
		if attr == 'ntrials':
			if self.ntrials == None: return ''
			return ' / '.join( [ '%s:%s' % ( k, v ) for k,v in sorted( self.ntrials.items() ) ] )
		if attr == 'datfile': f = self.GetDatFile(); return ( f == None ) and '(no .dat file)' or os.path.basename( f )
		if attr == 'pkfile':  f = self.GetPkFile();  return ( f == None ) and '(no .pk file)'  or os.path.basename( f )
		if attr == 'datestamp': d = self.GetDate();  return { None:'(unknown date)' }.get( d, d )
		if attr == 'informal_date':
			d = self.GetDate()
			try: then = time.strptime( d, '%Y-%m-%d %H:%M:%S' )
			except: return '-'
			now = time.localtime()
			seconds = time.mktime( now ) - time.mktime( then )
			day  = int( time.mktime( then ) / ( 60.0 * 60.0 * 24.0 ) ) 
			today = int( time.mktime( now ) / ( 60.0 * 60.0 * 24.0 ) )
			days = today - day
			if   seconds < 50.0: return '%d seconds ago' % round( seconds )
			elif seconds < 90.0: return 'about a minute ago'
			elif seconds < 50*60.0: return '%d minutes ago' % round( seconds / 60.0 )
			elif seconds < 90*60.0: return 'about an hour ago'
			elif days == 0: return '%d hours ago' % round( seconds / 3600.0 )
			elif days == 1: return 'yesterday'
			else: return '%d days ago' % days
		if attr == 'cv':
			if self.cv == None: return None
			return '%g%%' % round(100.0 - 100.0 * self.cv.loss.train) # TODO
		return '???'
	
	def GetNumberOfTrials( self, reread=False ):
		pkfile = self.GetPkFile()
		if pkfile == None: return None
		y = DataFiles.load( pkfile )[ 'y' ]
		count = {}
		for yi in y.flat: count[ int( yi ) ] = count.get( yi, 0 ) + 1
		return count
	
	def GetCacheFilePath( self, attrname ):
		return os.path.join( self.GetCacheDir(), self.GetStem() + '_' + attrname + '.pk' )
			
	def report( self ):
		s = []
		s.append( self.Describe( 'selected'  ) )
		s.append( self.Describe( 'datestamp' ) )
		s.append( self.Describe( 'datfile'   ) )
		s.append( self.Describe( 'pkfile'    ) )
		return s
		
	def __str__( self ):
		return '   '.join( self.report() )
		
	def __repr__( self ):
		return '<%s object at 0x%08X>: %s' % ( self.__class__.__name__, id( self ), str( self ) )

def cmprun( a, b ):
	ab = [ a.GetDate(), b.GetDate() ]
	if None in ab or ab[0] == ab[1]:
		ab = [a,b]
		width = 20
		import re
		for i,x in enumerate( ab ):
			if x.GetPkFile() != None: x = x.GetPkFile()
			else: x = x.GetDatFile()
			if x != None:
				x = os.path.splitext( os.path.basename( x ) )[0]
				x = re.sub('[0-9]+', lambda m: m.group().rjust(width,'0'), x)
			ab[i] = x
	return cmp( ab[0], ab[1] )
	

class CalibrationManager( object ):
	
	def __init__( self, directory='.', depth=0 ):
		"""
		<depth> may be an integer, or a range of integers.
		depth=0 means that files are expected to be directly in the specified <directory>.
		"""
		self.depth = None
		self.__directory = None
		self.__runs = {}
		self.__opts = { 'gamma':0.0 }  # TODO
		self.ScanDirectory( directory, depth=depth  )
		
	def GetDirectory( self ):
		return self.__directory
		
	def ScanDirectory( self, directory=None, depth=None ):
		if directory != None:
			newdir = os.path.abspath( directory )
			if newdir != self.__directory:
				self.__runs.clear()
				self.__directory = newdir
		if self.__directory == None: raise ValueError( 'no directory set' )
		if depth != None: self.depth = depth
		if self.depth == None: self.depth = 0
			
		def findfiles( directory, pattern, depth ):
			ff = []
			if not isinstance( depth, ( tuple, list ) ): depth = [ depth ]
			for d in depth:
				path = [ directory ] + [ '*' ] * d + [ pattern ]
				ff += glob.glob( os.path.join( *path ) )
			d = {}
			for f in ff:
				key = os.path.splitext( os.path.basename( f ) )[0]
				d[ key ] = d.get( key, [] ) + [ f ]
			dup = sorted( [ k for k,v in d.items() if len( v ) > 1 ] )
			if len( dup ) == 0: return [ ( k, v[0] ) for k,v in sorted( d.items() ) ]
			msg = 'duplicate filenames found for ' + dup[0]
			if len( dup ) == 1: msg += ' and ' + dup[1]
			if len( dup ) > 1:  msg += ' and %d others' % ( len( dup ) - 1 )
			raise ValueError( msg )
		
		pkfiles  = dict( findfiles( self.__directory, '*.pk',  depth=self.depth ) )
		datfiles = dict( findfiles( self.__directory, '*.dat', depth=self.depth ) )
		
		cachedir = self.GetCacheDirectory( mkdir=True )
		runopts = dict( self.__opts )
		runopts['folds'] = 'LOO'
		for stem in sorted( set( pkfiles.keys() + datfiles.keys() ) ):
			if stem not in self.__runs: self.__runs[stem] = CalibrationRun( self.__directory, datfile=datfiles.get( stem, None ), pkfile=pkfiles.get( stem, None ), opts=runopts, cachedir=cachedir )
	
	def GetCacheDirectory( self, mkdir=False ):
		parts = [ 'CalibrationManager', 'opts_' + Hashing.hash( self.__opts )[:10] ]
		if mkdir:
			for i,d in enumerate( parts ):
				d = os.path.join( self.__directory, *parts[:i+1] )
				if not os.path.isdir( d ): os.mkdir( d )
		d = os.path.join( self.__directory, *parts )
		return d
				
	def GetRuns( self ):
		return sorted( self.__runs.values(), cmp=cmprun )[::-1]
	
	def __repr__( self ):
		return '<%s object at 0x%08X>:\n  %s' % ( self.__class__.__name__, id( self ), str( self ).replace( '\n', '\n  ' ) )	

	def __str__( self ):
		s = []
		s.append( 'directory: %s' % self.__directory )
		if len( self.__runs ):
			rows = [ [ '[%d]' % i ] + x.report() for i,x in enumerate( self.GetRuns() ) ]
			lengths = [ 2+max([len(row[colIndex]) for row in rows]) for colIndex,blah in enumerate( rows[0] ) ]
			s += [ ''.join([ (x+' '*(width-len(x))) for width,x in zip( lengths,row ) ] ) for row in rows]				
		else:
			s.append( 'no files found' )
		return '\n'.join( s )
	
	def __getitem__( self, index ):
		return self.GetRuns()[index]

import Tkinter as tk

if sys.platform.startswith('win'):
	import Tix; import Tix as tk
else:
	Tix = None

class Table( tk.Frame ):
	'''
	A Tk Widget, subclassed from Tk.Frame, containing optional horizontal and vertical
	Scrollbars plus a Canvas which reacts to the Scrollbars and which in turn contains
	another Frame.
	
	This was adapted from a magic recipe given by Bryan Oakley 2010-06-23 in answer to
	http://stackoverflow.com/questions/3085696/adding-a-scrollbar-to-a-grid-of-widgets-in-tkinter
	namely: make a Canvas and a Scrollbar, put a Frame inside the Canvas, and configure some
	rather incomprehensible interrelationships between the three. I put the whole thing in
	an additional Frame (self) to bind the Scrollbar and Canvas together and make it a
	modular pack()able Widget. I also added the GetCell method (delivering yet more Frames).
	
	Tix appears to have a ScrolledGrid object but I could not find any doc on it, and
	the one example I found on the web failed to run, in non-obvious ways, with the Tix
	version packaged with Python 2.5.4
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
			cell.grid( row=row, column=column, sticky='NSEW' )
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

class FakeWidget( ):
	def destroy( self ): pass
	
def ToolTip( parentWidget, msg ):
	if Tix == None: return FakeWidget()
	w = Tix.Balloon( parentWidget )
	w.bind_widget( parentWidget, balloonmsg=msg )
	return w

class CalibrationTableRow( object ):
	def __init__( self, table, rowNumber, run ):
		"""
		Manages all the Tk Widgets in one row of the Table g.table belonging to a CalibrationGUI g.
		"""
		self.run = run
		self.__widgets = {}
		self.widget( 'selectedCheckbutton',   tk.Checkbutton( table.GetCell( rowNumber, 0 ), command=self.CheckbuttonCallback     ) ).pack()
		self.widget( 'informaldateLabel',     tk.Label(       table.GetCell( rowNumber, 1 ), text=run.Describe( 'informal_date' ) ) ).pack()
		self.widget( 'datestampLabel',        tk.Label(       table.GetCell( rowNumber, 2 ), text=run.Describe( 'datestamp' )     ) ).pack()
		self.widget( 'ntrialsLabel',          tk.Label(       table.GetCell( rowNumber, 3 ), text=run.Describe( 'ntrials' )       ) ).pack()
		self.widget( 'cvLabel',               tk.Label(       table.GetCell( rowNumber, 4 ), text=run.Describe( 'cv' )            ) ).pack( side='left' )
		self.widget( 'cvButton',              tk.Button(      table.GetCell( rowNumber, 4 ), text='Assess', command=self.Assess   ) ).pack( side='right', fill='x' )
		self.widget( 'pkfileLabel',           tk.Label(       table.GetCell( rowNumber, 5 ), text=run.Describe( 'pkfile' )        ) ).pack()
		self.widget( 'datfileLabel',          tk.Label(       table.GetCell( rowNumber, 6 ), text=run.Describe( 'datfile' )       ) ).pack()
		
		
		w = self.widget( 'selectedCheckbutton' )
		self.widget( 'tooltip', ToolTip( w, 'check to include this run when generating weights' ) )
		
		self.UpdateAssessmentDisplay()
		
		self.selected = tk.IntVar( self.widget( 'selectedCheckbutton' ), run.selected )
		self.widget( 'selectedCheckbutton' ).config( variable = self.selected ) # couldja make this mechanism *any* more awkward??
		if run.GetPkFile() == None:
			self.widget( 'selectedCheckbutton' ).config( state='disabled' )
			self.widget( 'cvButton' ).config( state='disabled' )
	
	def UpdateAssessmentDisplay( self ):
		self.widget( 'ntrialsLabel' ).config( text=self.run.Describe( 'ntrials' ) )
		if self.run.cv == None:
			if self.run.errmsg != None:
				self.widget( 'explanation', ToolTip( self.widget( 'cvLabel' ), self.run.errmsg ) )
				self.widget( 'cvLabel' ).config( text='error', bg='#FF0000' )
		else:
			q = self.run.cv.loss.train
			goodness = 1 - 2 * min( 0.5, q )
			badness = 2 * max( q - 0.5, 0.0 )
			goodness = goodness**2
			badness = badness**2
			neutral = 0.8
			somethingness = max( goodness, badness )
			red = badness + (1-somethingness)*neutral
			green = goodness + (1-somethingness)*neutral			
			blue = (1-somethingness)*neutral
			self.widget( 'cvLabel' ).config( text=self.run.Describe( 'cv' ), bg='#%02x%02x%02x'%(red*255.0,green*255.0,blue*255.0) )
			self.widget( 'cvButton' ).config( text='Re-Run' )
			
	def Assess( self ):
		self.widget( 'cvLabel' ).config( text='...' )
		self.widget( 'cvLabel' ).update()
		self.run.CrossValidate( )
		self.UpdateAssessmentDisplay()
		
	def CheckbuttonCallback( self ):
		self.run.selected = bool( self.selected.get() )
		
	def widget( self, name, w=None ):
		if w == None: w = self.__widgets[name]
		self.__widgets[name] = w
		return w
	
	def destroy( self ):
		self.selected = None
		for v in self.__widgets.values(): v.destroy()
		self.__widgets.clear()
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
		self.title( 'Calibration Manager' )
		
		self.__rows = []

		if manager == None: manager = '.'
		if isinstance( manager, basestring ): manager = CalibrationManager( manager )
		self.__manager = manager
		self.Render()
		
		if go: self.mainloop()
		else: self.update()
		
	def Render( self ):
	
		fr = tk.Frame( self ); fr.pack( fill='x' )
		self.directoryEntry = tk.Entry( fr, validatecommand=self.UpdateTable, validate='focusout' )
		self.directoryEntry.delete( 0, tk.END )
		self.directoryEntry.insert(0, self.__manager.GetDirectory() )
		tk.Button( fr, text='Refresh / update', command=self.UpdateTable ).pack( side='right' )
		self.directoryEntry.pack( fill='x' )
		
		self.table = Table( self, vscroll=True, hscroll=True )
		self.table.pack( fill='both', expand=True )

		tk.Button( self, text='Exit', command=self.destroy ).pack()
		
		self.UpdateTable()
		
	
	def UpdateTable( self ):
		self.__manager.ScanDirectory( self.directoryEntry.get() )
		self.directoryEntry.delete( 0, tk.END ); self.directoryEntry.insert(0, self.__manager.GetDirectory() )
		while len( self.__rows ): self.__rows.pop( 0 ).destroy()
		self.table.DestroyCells()
		for row,run in enumerate( self.__manager.GetRuns() ):
			self.__rows.append( CalibrationTableRow( self.table, rowNumber=row, run=run ) )
	
	def GetManager( self ):
		return self.__manager
	
	def destroy( self ):
		while len( self.__rows ): self.__rows.pop( 0 ).destroy()
		tk.Tk.destroy( self )
	
def test():
	m = CalibrationManager( directory='20111006_8525_A_002' )
	j = CalibrationGUI( manager=m, go=False )
	return m,j
	
if __name__ == "__main__": m,j = test()
