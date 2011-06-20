__all__ = ['ringin', 'examples', 'numbers', 'netzbrumm_fs250']

import os.path
import WavTools

def ringin():
	return WavTools.player('C:/WINDOWS/Media/ringin.wav')

def examples():
	#grwavdir = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../bci2000/gr/imagination/python/'))
	grwavdir = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../../../../../../gr/imagination/python/'))
	return (
		WavTools.player(os.path.join(grwavdir, 'attempt2-strings.wav')),
		WavTools.player(os.path.join(grwavdir, 'attempt2-piano.wav')),
	)
	
def numbers(n=5):
	grdir = '/agbs/share/bcigroup/gr'
	if not os.path.exists(grdir): grdir = '\\\\steinlach\\bcigroup\\gr'
	numstimdir = os.path.join(grdir, 'stimuli', 'audiospeller', 'thomas', 'row-column-numbers')
	w = []
	for i in range(n): w.append(WavTools.wav('%s/%02d.wav' % (numstimdir, i)).left())
	return w

def netzbrumm_fs250():
	return WavTools.wav(os.path.join(os.path.dirname(__file__), 'netzbrumm_fs250.wav'))
