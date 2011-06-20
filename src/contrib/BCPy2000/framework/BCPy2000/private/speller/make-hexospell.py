import sys,os
import SigTools; from SigTools import *
pylab = SigTools.Plotting.load_pylab()
import matplotlib,pylab,numpy,os,sys

grid = """
	Display        Scale   Row Column Page  Action
	hexes/page2.png 3.7   1.00  2.50    1   goto(2)
	hexes/page3.png 3.7   1.75  3.80    1   goto(3)
	hexes/page4.png 3.7   3.25  3.80    1   goto(4)
	hexes/page5.png 3.7   4.00  2.50    1   goto(5)
	hexes/page6.png 3.7   3.25  1.20    1   goto(6)
	hexes/page7.png 3.7   1.75  1.20    1   goto(7)
	hexes/space.png 3.7   1.00  2.50    2   write('%20');goto(1)
	hexes/A.png     3.7   1.75  3.80    2   write('A');goto(1)
	hexes/B.png     3.7   3.25  3.80    2   write('B');goto(1)
	hexes/C.png     3.7   4.00  2.50    2   write('C');goto(1)
	hexes/D.png     3.7   3.25  1.20    2   write('D');goto(1)
	hexes/hex.png   3.7   1.75  1.20    2   goto(1)
	hexes/E.png     3.7   1.00  2.50    3   write('E');goto(1)
	hexes/F.png     3.7   1.75  3.80    3   write('F');goto(1)
	hexes/G.png     3.7   3.25  3.80    3   write('G');goto(1)
	hexes/H.png     3.7   4.00  2.50    3   write('H');goto(1)
	hexes/I.png     3.7   3.25  1.20    3   write('I');goto(1)
	hexes/hex.png   3.7   1.75  1.20    3   goto(1)
	hexes/J.png     3.7   1.00  2.50    4   write('J');goto(1)
	hexes/K.png     3.7   1.75  3.80    4   write('K');goto(1)
	hexes/L.png     3.7   3.25  3.80    4   write('L');goto(1)
	hexes/M.png     3.7   4.00  2.50    4   write('M');goto(1)
	hexes/N.png     3.7   3.25  1.20    4   write('N');goto(1)
	hexes/hex.png   3.7   1.75  1.20    4   goto(1)
	hexes/O.png     3.7   1.00  2.50    5   write('O');goto(1)
	hexes/P.png     3.7   1.75  3.80    5   write('P');goto(1)
	hexes/Q.png     3.7   3.25  3.80    5   write('Q');goto(1)
	hexes/R.png     3.7   4.00  2.50    5   write('R');goto(1)
	hexes/S.png     3.7   3.25  1.20    5   write('S');goto(1)
	hexes/hex.png   3.7   1.75  1.20    5   goto(1)
	hexes/T.png     3.7   1.00  2.50    6   write('T');goto(1)
	hexes/U.png     3.7   1.75  3.80    6   write('U');goto(1)
	hexes/V.png     3.7   3.25  3.80    6   write('V');goto(1)
	hexes/W.png     3.7   4.00  2.50    6   write('W');goto(1)
	hexes/X.png     3.7   3.25  1.20    6   write('X');goto(1)
	hexes/hex.png   3.7   1.75  1.20    6   goto(1)
	hexes/Y.png     3.7   1.00  2.50    7   write('Y');goto(1)
	hexes/Z.png     3.7   1.75  3.80    7   write('Z');goto(1)
	hexes/dot.png   3.7   3.25  3.80    7   write('.');goto(1)
	hexes/comma.png 3.7   4.00  2.50    7   write(',');goto(1)
	hexes/qmark.png 3.7   3.25  1.20    7   write('?');goto(1)
	hexes/hex.png   3.7   1.75  1.20    7   goto(1)
"""###
grid = grid.strip().split('\n')
headers = grid.pop(0)
for i,row in enumerate(grid): grid[i] = [x.replace('%20', ' ').replace('%','') for x in row.strip().split()]
if 0:
	from BCI2000Tools.Parameters import Param
	gp = Param(grid, clabels=headers.split(), tab='PythonApp', section='Stimulus', name='Grid')
	gp.appendto(os.path.join('..', 'parms','hex.prm'))


if 1:
	filenames = sorted(set([os.path.join(*os.path.split(row[0])) for row in grid]))
	nsides = 6
	fig = pylab.figure(figsize=(6,6))
	ax = pylab.gca()
	p = matplotlib.patches.RegularPolygon((0.0,0.0),nsides,radius=1.0,orientation=numpy.pi/nsides,edgecolor=(1,1,1),facecolor=(1,1,1))
	ax.add_patch(p)
	ax.set(xlim=(-1,1),ylim=(-1,1),xticks=[],yticks=[],frame_on=False,aspect='equal',position=(0.05,0.05,0.9,0.9))
	central = pylab.Text(x=0,y=0,text='blah',color='k',verticalalignment='center',horizontalalignment='center',fontname='monospaced',fontweight='bold',size=80)
	ax.add_artist(central)

	a = numpy.linspace(90, 90-360, nsides, endpoint=False) * numpy.pi/180
	a = numpy.c_[numpy.cos(a),numpy.sin(a)] * 0.7
	
	sides = []
	for x,y in a: 
		t = pylab.Text(x=x,y=y,text=' ',color='k',verticalalignment='center',horizontalalignment='center',fontname='monospaced',fontweight='bold',size=40)
		ax.add_artist(t)
		sides.append(t)
	
	for filename in filenames:
		content = os.path.splitext(os.path.basename(filename))[0]
		tr = {'qmark':'?', 'dot':'.', 'comma':',', 'hex':' '}
		trs = {'dot':160, 'comma':160, 'space':60}
		if content.startswith('page'):
			pageno = int(content[4:])
			kids = [os.path.splitext(os.path.basename(row[0]))[0] for row in grid if int(row[4]) == pageno]
			kids = [tr.get(kid,kid) for kid in kids]
			for content,handle in zip(kids,sides):
				content = tr.get(content,content)
				size=trs.get(content,80) * 0.7
				handle.set(text=content,size=size)
			central.set(text='')
		else:
			size=trs.get(content,80)
			content = tr.get(content,content)
			for handle in sides: handle.set(text='')
			central.set(text=content,size=size)
		pylab.savefig(filename, transparent=True); print filename
	pylab.draw()
	


	