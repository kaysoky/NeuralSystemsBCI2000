###########################################################################
## $Id$
## Authors: juergen.mellinger@uni-tuebingen.de
## Description: Defines an inventory of modules.
##
##   To add a module to the inventory, use
##   BCI2000_ADD_TO_INVENTORY( SignalSource SignalGenerator )
##
##   To use the inventory in a program, call
##   BCI2000_USE_INVENTORY( SOURCES HEADERS )
##   from its CMakeLists.txt file. SOURCES and HEADERS are the names
##   of output variables to which source and header file are added.
##
##   To access the inventory from code, create an object of type
##   Inventory declared in src/shared/config/ModuleInventory.h.
##   There, allowed values for the "KIND" argument to BCI_ADD_TO_INVENTORY
##   are declared in an enumeration.
##
##   The module inventory file lists all registered modules in the form
##     MODULE( SignalSource, SignalGenerator )
##     MODULE( SignalProcessing, ARSignalProcessing )
##     MODULE( Application, CursorTask )
##     MODULE( Operator, Operator )
##     MODULE( Tool, BCI2000Launcher )

# Definition of inventory file:
SET( INVENTORY_INCLUDES ${CMAKE_CURRENT_BINARY_DIR} )
SET( INVENTORY_INC "${CMAKE_CURRENT_BINARY_DIR}/Inventory.inc" )
FILE( WRITE ${INVENTORY_INC} "// File contents created by BCI2000_ADD_TO_INVENTORY -- re-run CMake to update this file\n" )

# Macro definitions
MACRO( BCI2000_ADD_TO_INVENTORY KIND NAME )
  FILE( APPEND ${INVENTORY_INC} "MODULE( ${KIND}, ${NAME} )\n" )
ENDMACRO( BCI2000_ADD_TO_INVENTORY KIND NAME )  


MACRO( BCI2000_USE_INVENTORY SOURCES HEADERS )
  INCLUDE_DIRECTORIES(
    ${INVENTORY_INCLUDES}
  )
  SET( INVENTORY_H
    ${BCI2000_SRC_DIR}/shared/config/ModuleInventory.h
  )
  SET( INVENTORY_CPP
    ${BCI2000_SRC_DIR}/shared/config/ModuleInventory.cpp
  )
  SET_PROPERTY( # Make sure the inventory cpp file is re-compiled each time the inventory has changed.
    SOURCE ${INVENTORY_CPP}
    APPEND PROPERTY OBJECT_DEPENDS ${INVENTORY_INC}
  )
  SET_PROPERTY(
    SOURCE ${INVENTORY_INC}
    APPEND PROPERTY GENERATED TRUE
  )
  SET( ${SOURCES}
    ${${SOURCES}}
    ${INVENTORY_INC}
    ${INVENTORY_CPP}
  )
  SET( ${HEADERS}
    ${${HEADERS}}
    ${INVENTORY_H}
  )
  SOURCE_GROUP( Source\\BCI2000_Framework\\shared\\config FILES ${INVENTORY_CPP} )
  SOURCE_GROUP( Headers\\BCI2000_Framework\\shared\\config FILES ${INVENTORY_H} )
  SOURCE_GROUP( Generated FILES ${INVENTORY_INC} )
ENDMACRO()
