# - Use Module for QT4
# Sets up C and C++ to use Qt 4.  It is assumed that FindQt.cmake
# has already been loaded.  See FindQt.cmake for information on
# how to load Qt 4 into your CMake project.

# CMake - Cross Platform Makefile Generator
# Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# 
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# 
# * Neither the names of Kitware, Inc., the Insight Software Consortium,
#   nor the names of their contributors may be used to endorse or promote
#   products derived from this software without specific prior written
#   permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# ------------------------------------------------------------------------------
# 
# The above copyright and license notice applies to distributions of
# CMake in source and binary form.  Some source files contain additional
# notices of original copyright by their contributors; see each source
# for details.  Third-party software packages supplied with CMake under
# compatible licenses provide their own copyright notices documented in
# corresponding subdirectories.
# 
# ------------------------------------------------------------------------------
# 
# CMake was initially developed by Kitware with the following sponsorship:
# 
#  * National Library of Medicine at the National Institutes of Health
#    as part of the Insight Segmentation and Registration Toolkit (ITK).
# 
#  * US National Labs (Los Alamos, Livermore, Sandia) ASC Parallel
#    Visualization Initiative.
# 
#  * National Alliance for Medical Image Computing (NAMIC) is funded by the
#    National Institutes of Health through the NIH Roadmap for Medical Research,
#    Grant U54 EB005149.
# 
#  * Kitware, Inc.

ADD_DEFINITIONS(${QT_DEFINITIONS})
SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG QT_DEBUG)
SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_RELEASE QT_NO_DEBUG)
SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_RELWITHDEBINFO QT_NO_DEBUG)
SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_MINSIZEREL QT_NO_DEBUG)

INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})
IF(Q_WS_MAC AND QT_USE_FRAMEWORKS)
  INCLUDE_DIRECTORIES(${QT_QTCORE_LIBRARY})
ENDIF(Q_WS_MAC AND QT_USE_FRAMEWORKS)

SET(QT_LIBRARIES "")

IF (QT_USE_QTMAIN)
  IF (WIN32)
    SET(QT_LIBRARIES ${QT_LIBRARIES} ${QT_QTMAIN_LIBRARY})
  ENDIF (WIN32)
ENDIF (QT_USE_QTMAIN)

IF(QT_DONT_USE_QTGUI)
  SET(QT_USE_QTGUI 0)
ELSE(QT_DONT_USE_QTGUI)
  SET(QT_USE_QTGUI 1)
ENDIF(QT_DONT_USE_QTGUI)

IF(QT_DONT_USE_QTCORE)
  SET(QT_USE_QTCORE 0)
ELSE(QT_DONT_USE_QTCORE)
  SET(QT_USE_QTCORE 1)
ENDIF(QT_DONT_USE_QTCORE)

IF (QT_USE_QT3SUPPORT)
  ADD_DEFINITIONS(-DQT3_SUPPORT)
ENDIF (QT_USE_QT3SUPPORT)

# list dependent modules, so dependent libraries are added
SET(QT_QT3SUPPORT_MODULE_DEPENDS QTGUI QTSQL QTXML QTNETWORK QTCORE)
SET(QT_QTSVG_MODULE_DEPENDS QTGUI QTXML QTCORE)
SET(QT_QTUITOOLS_MODULE_DEPENDS QTGUI QTXML QTCORE)
SET(QT_QTHELP_MODULE_DEPENDS QTGUI QTSQL QTXML QTNETWORK QTCORE)
IF(QT_QTDBUS_FOUND)
  SET(QT_PHONON_MODULE_DEPENDS QTGUI QTDBUS QTCORE)
ELSE(QT_QTDBUS_FOUND)
  SET(QT_PHONON_MODULE_DEPENDS QTGUI QTCORE)
ENDIF(QT_QTDBUS_FOUND)
SET(QT_QTDBUS_MODULE_DEPENDS QTXML QTCORE)
SET(QT_QTXMLPATTERNS_MODULE_DEPENDS QTNETWORK QTCORE)
SET(QT_QAXCONTAINER_MODULE_DEPENDS QTGUI QTCORE)
SET(QT_QAXSERVER_MODULE_DEPENDS QTGUI QTCORE)
SET(QT_QTSCRIPTTOOLS_MODULE_DEPENDS QTGUI QTCORE)

# Qt modules  (in order of dependence)
FOREACH(module QT3SUPPORT QTOPENGL QTASSISTANT QTDESIGNER QTMOTIF QTNSPLUGIN
               QAXSERVER QAXCONTAINER QTSCRIPT QTSVG QTUITOOLS QTHELP 
               QTWEBKIT PHONON QTSCRIPTTOOLS QTGUI QTTEST QTDBUS QTXML QTSQL 
               QTXMLPATTERNS QTNETWORK QTCORE)

  IF (QT_USE_${module} OR QT_USE_${module}_DEPENDS)
    IF (QT_${module}_FOUND)
      IF(QT_USE_${module})
        STRING(REPLACE "QT" "" qt_module_def "${module}")
        ADD_DEFINITIONS(-DQT_${qt_module_def}_LIB)
        INCLUDE_DIRECTORIES(${QT_${module}_INCLUDE_DIR})
      ENDIF(QT_USE_${module})
      SET(QT_LIBRARIES ${QT_LIBRARIES} ${QT_${module}_LIBRARY} ${QT_${module}_LIB_DEPENDENCIES})
      FOREACH(depend_module ${QT_${module}_MODULE_DEPENDS})
        SET(QT_USE_${depend_module}_DEPENDS 1)
      ENDFOREACH(depend_module ${QT_${module}_MODULE_DEPENDS})
    ELSE (QT_${module}_FOUND)
      MESSAGE("Qt ${module} library not found.")
    ENDIF (QT_${module}_FOUND)
  ENDIF (QT_USE_${module} OR QT_USE_${module}_DEPENDS)
  
ENDFOREACH(module)

