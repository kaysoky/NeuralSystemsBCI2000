# -*- coding: utf-8 -*-
# 
#   $Id$
#   
#   This file is part of the BCPy2000 framework, a Python framework for
#   implementing modules that run on top of the BCI2000 <http://bci2000.org/>
#   platform, for the purpose of realtime biosignal processing.
# 
#   Copyright (C) 2007-11  Jeremy Hill, Thomas Schreiner,
#                          Christian Puzicha, Jason Farquhar
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
import sys,time

class progress:
	def __init__(self, todo=1.0, msg='progress', stream=None):
		if stream==None: stream = sys.stdout
		self.stream = stream
		self.ttymode = hasattr(self.stream, 'isatty') and self.stream.isatty()
		self.msg = msg
		self.period = 2.0
		self.wrap = 80
		self.silent = True
		self.start(todo)
		
	def start(self, todo=1.0):
		self.done()
		self.todo = float(todo)
		self.amount_done = 0.0
		self.started = time.time()
		self.finished = None
		self.silent = True
		self.linelength = 0
		self.nextreport = self.started + self.period
	
	def update(self, amount_done=None, extra=''):
		if amount_done == None: amount_done = self.amount_done + 1
		self.amount_done = float(amount_done)
		t = time.time()
		if self.amount_done == self.todo:
			if self.silent: return
		elif t < self.nextreport:
			return
		if self.silent:
			self.stream.write(self.msg)
			if self.ttymode: self.stream.write(': ')
			else: self.stream.write(' - percent done:\n')
		percent = 100.0 * self.amount_done / self.todo
		if self.ttymode:
			report = '%4d%% %s' % (round(percent), extra)
			self.stream.write('\x08'*self.linelength + report)
			self.linelength = len(report)
		else:
			report = '%4d ' % (round(percent),)
			if self.linelength + len(report) > self.wrap:
				self.stream.write('\n')
				self.linelength = 0
			self.stream.write(report)
			self.linelength += len(report)
		self.stream.flush()
		self.silent = False
		self.nextreport = t + self.period
		if self.amount_done == self.todo: self.done()
	
	def done(self):
		if not self.silent:
			self.stream.write('\n')
			self.stream.flush()
		self.finished = time.time()
		self.silent = True
		
	def __del__(self):
		self.done()