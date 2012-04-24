def hashval( x, **kwargs ):
	return hash( serialize( x, **kwargs ) )
	
def serialize( x, withClass=None, **opts ):
	#sstructFieldOrderMatters=False
	#classOfOneDimensionalSequencesMatters=False
	#dtypeOfNumpyArrayMatters=False
	#classOfNumericConstantMatters=False	
	#classOfStringMatters=False
	
	if withClass:
		pass
	