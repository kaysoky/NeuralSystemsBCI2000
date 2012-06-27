###########################################################################
## $Id$
## Authors: griffin.milsap@gmail.com
## Description: Setup Macros for adding projects into BCI2000

INCLUDE( ${BCI2000_CMAKE_DIR}/SignalProcessingMacro.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/SignalSourceMacro.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/ApplicationMacro.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/SetupExtlibDependencies.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/ToolsCmdlineMacro.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/ToolsGUIAppMacro.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/SetupGUIImports.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/IncludeExtensionMacro.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/SetOutputDirectoryMacro.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/AddRegistryMacro.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/ModuleInventory.cmake )
INCLUDE( ${BCI2000_CMAKE_DIR}/AddCoreMain.cmake )

