#!/usr/bin/env python

import sys,platform
if sys.argv[1:] == []:
	sys.argv += ['build']
	if platform.system().lower() == 'windows':
		sys.argv += ['-cmingw32']
	sys.argv += ['install']

import distutils
from distutils.core import setup, Extension

import numpy
numpy_include_dir = numpy.get_include()

import os
include_dirs = [
]
source_modules = [
	"SoundRecord.cpp",
	"DX9Sound.cpp",
	"lpt.c",
]

setup(name = "pydsound",
      version = "1.0",
      #packages = ["pydsound"],
      #package_dir = {"pydsound":package_dir},
      #ext_package = "pydsound",
      ext_modules = [
                      Extension(
                                name = "_pydsound",
                                sources = ["pydsound_pyd.i", "pydsound_pyd.cpp"] + source_modules,
                                include_dirs=[numpy_include_dir] + include_dirs,
                               )
                    ]
     )
