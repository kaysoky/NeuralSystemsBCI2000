#!/bin/sh
cd . && echo \
@goto cmd \
>/dev/null

# posix shell code
dir=$(dirname "$0")
if [ %1 ]; then
  cmake $* "$dir"
  echo
  echo Run $(basename "$0") to edit configuration options.
elif which ccmake; then
  ccmake "$dir"
elif which cmake-gui; then
  cmake-gui "$dir"
else
  cmake -i "$dir"
fi
exit

# windows cmd code
:cmd
@echo off
cls
if "%1"=="" (
  start "" cmake-gui "%~dp0" 
) else (
  cmake %* "%~dp0"
  echo.
  echo Run %~nx0 to edit configuration options.
  pause
)
