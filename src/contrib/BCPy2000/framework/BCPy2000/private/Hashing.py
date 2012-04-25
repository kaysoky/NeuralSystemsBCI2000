

import numpy, struct, md5

#sstructFieldOrderMatters=False
#classOfSequencesMatters=False
#dtypeOfNumpyArrayMatters=False
#classOfNumberMatters=False	
#classOfStringMatters=False


def hash( x, **opts ):
	h = md5.md5()
	UpdateHash( h, x, **opts )
	return h.hexdigest()

def hash2( x, **opts ):
	return md5.md5( serialize( x, **opts ) ).hexdigest() # should be the same as hash() but potentially uses much more memory

def serialize( x, **opts ):
	class serializer:
		def __init__( self ): self.str = ''
		def update( self, x ): self.str += x
	h = serializer()
	UpdateHash( h, x, **opts )
	return h.str

def UpdateHash( h, x, **opts ):
	cl = x.__class__.__module__ + '.' + x.__class__.__name__
	if hasattr( x, '_allitems' ):
		x = x._allitems();
		if opts.get( 'sstructFieldOrderMatters', False ) == False: x = sorted( x )
		h.update( cl + ' ' )
		UpdateHash( h, x, **opts )
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
		h.update( cl + ' ' )		
		h.update( '%d ' % len( x ) )
		h.update( x )
	h.update( ' ' )
	