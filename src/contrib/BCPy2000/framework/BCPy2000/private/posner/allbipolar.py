def allbipolar(s):
	s = [x.lower().split('x') for x in s.replace(',','+').replace(' ','').split('+')]
	ch = 1
	pairs = []
	for c,r in s:
		c,r = int(c),int(r)
		for i in range(r):
			for j in range(c-1): pairs.append((ch+j,ch+j+1))
			if i < r-1:
				for j in range(c): pairs.append((ch+j,ch+j+c))
			ch += c
	ch -= c
	s = """
Filtering:SpatialFilter                 int       SpatialFilterType=             2             // spatial filter type 0: none, 1: full matrix, 2: sparse matrix, 3: common average reference (CAR) (enumeration)
Filtering:SpatialFilter                 matrix    SpatialFilter=              % 3d { In Out Wt }
""" % (len(pairs)*2)
	for pos,neg in pairs:
		s += '    CH%03d CH%03d-CH%03d 1   CH%03d CH%03d-CH%03d -1' % (pos,pos,neg,neg,pos,neg)
	return s+'\n'
			
