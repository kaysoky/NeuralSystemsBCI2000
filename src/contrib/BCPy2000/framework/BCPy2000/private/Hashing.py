__all__ = ['hash', 'match']

import numpy, struct, md5

def hash( x, **opts ):
	"""
	Create an md5 hash out of a complex Python object that may consist of
	strings and numeric values arranged in tuples, lists, numpy.arrays, dicts
	and SigTools.NumTools.sstruct containers.  Hash only the "interesting"
	aspects of the content.
	
	Options and their defaults:
	   sstructFieldOrderMatters = False   if false: disregard the order of fields and subfields in a sstruct
	   sstructClassMatters      = False   if false: anything with a _allitems() method is considered a sstruct
	   classOfSequencesMatters  = False   if false: tuples, lists and numpy.arrays are considered equivalent
	   classOfNumpyArrayMatters = False   if false: the class (matrix, etc) of numpy.arrays is disregarded
	   dtypeOfNumpyArrayMatters = False   if false: the dtype of numpy.arrays is disregarded
	   classOfNumberMatters     = False   if false: bools, ints and floats of equal value are considered equivalent
	   classOfStringMatters     = False   if false: unicode and str with the same content are considered equivalent
	   caseOfStringMatters      = True    if false: upper and lower case characters are considered equivalent
	"""###
	h = md5.md5()
	UpdateHash( h, x, **opts )
	return h.hexdigest()

def match( a, b, **opts ):
	"""
	Determine whether two objects yield the same hash() with the given options.
	"""###
	return hash( a, **opts ) == hash( b, **opts )

def hash2( x, **opts ):
	"""
	Sanity-check: should be the same as hash() but potentially uses much more memory.
	"""###
	return md5.md5( serialize( x, **opts ) ).hexdigest()

def serialize( x, **opts ):
	"""
	For checking / debugging the serialization algorithm used by hash()
	"""###
	class serializer:
		def __init__( self ): self.str = ''
		def update( self, x ): self.str += x # quack
	h = serializer()
	UpdateHash( h, x, **opts )
	return h.str

def UpdateHash( h, x, **opts ):
	"""
	This bad boy does all the work for hash()
	"""###
	cl = x.__class__.__module__ + '.' + x.__class__.__name__
	if hasattr( x, '_allitems' ):
		x = x._allitems();
		if opts.get( 'sstructClassMatters', False ) == False: cl = 'sstruct-or-similar'
		if opts.get( 'sstructFieldOrderMatters', False ) == False: x = sorted( x )
		h.update( cl + ' ' )
		UpdateHash( h, x, **opts )
	elif isinstance( x, type(None) ):
		cl = 'None'
		h.update( cl + ' ' )
	elif isinstance( x, dict ):
		x = sorted( x.items() )
		h.update( cl + ' ' )
		UpdateHash( h, x, **opts )
	elif isinstance( x, ( tuple,list,numpy.ndarray ) ):
		isnumpy = isinstance( x, numpy.ndarray )
		if   opts.get( 'classOfSequenceMatters',  False ) == False: cl = 'tuple/list/numpy.ndarray'
		elif opts.get( 'classOfNumpyArrayMatters', False ) == False and isnumpy: cl = 'numpy.ndarray'
		if   opts.get( 'dtypeOfNumpyArrayMatters', False ) == True  and isnumpy: cl += '(' + x.dtype.str + ')' 
		h.update( cl + ' ' )
		h.update( '%d ' % len( x ) )
		for xi in x: UpdateHash( h, xi, **opts )
	elif isinstance( x, (bool,int,float) ):
		if opts.get( 'classOfNumberMatters', False ) == False: cl = 'bool/int/float'
		h.update( cl + ' ' )
		h.update( struct.pack( 'd', float( x ) ) )
	elif isinstance( x, basestring ):
		if opts.get( 'classOfStringMatters', False ) == False: cl = 'basestring'
		if opts.get( 'caseOfStringMatters', True ) == False: x = x.lower()
		h.update( cl + ' ' )		
		h.update( '%d ' % len( x ) )
		try: h.update( x )
		except UnicodeEncodeError: h.update( x.encode( 'utf8' ) )
	h.update( ' ' )
	