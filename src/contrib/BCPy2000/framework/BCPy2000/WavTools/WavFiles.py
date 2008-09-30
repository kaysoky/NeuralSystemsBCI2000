#   $Id$
#   
#   This file is part of the BCPy2000 framework, a Python framework for
#   implementing modules that run on top of the BCI2000 <http://bci2000.org/>
#   platform, for the purpose of realtime biosignal processing.
# 
#   Copyright (C) 2007-8  Thomas Schreiner, Jeremy Hill
#                         Christian Puzicha, Jason Farquhar
#   
#   bcpy2000@bci2000.org
#   
#   The BCPy2000 framework is free software: you can redistribute it
#   and/or modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation, either version 3 of
#   the License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
__all__ = []

import Base
import wave
import os.path

def read(self, filename):
	wf = wave.open(filename, 'rb')
	nbytes = wf.getsampwidth()
	nchan = wf.getnchannels()
	nsamp = wf.getnframes()
	fs = wf.getframerate()
	comptype = (wf.getcomptype(),wf.getcompname())
	strdat = wf.readframes(nsamp)
	wf.close()
	self.set_bitdepth(nbytes*8)
	self.fs = fs
	self.filename = os.path.abspath(filename)
	self.comptype = comptype
	self.y = self.str2dat(strdat, nsamp, nchan)
	if strdat != self.dat2str():
		print "warning: data mismatch in",self
Base.wav.read = read

def write(self, filename=None):
	if filename==None: filename = self.filename
	if filename==None: raise TypeError,'no filename supplied'		
	wf = wave.open(filename, 'wb')
	wf.setsampwidth(self.nbytes)
	wf.setnchannels(self.channels())
	wf.setnframes(self.samples())
	wf.setframerate(self.fs)
	wf.setcomptype(*self.comptype)
	wf.writeframes(self.dat2str())
	wf.close()
	self.filename = filename
Base.wav.write = write