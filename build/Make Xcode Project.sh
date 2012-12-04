#! /bin/sh
echo "NOTE:"
echo "XCode builds are not officially supported."
echo "If you are experiencing problems with building BCI2000 using XCode,"
echo "switch to building BCI2000 from the command line as described"
echo "on the BCI2000 wiki."
echo
echo "Press enter to proceed, or ctrl-C to abort."
read
echo "Running CMake ..."
CMake -DUSE_SSE2:BOOL=TRUE -DBUILD_TOOLS:BOOL=TRUE -G "Xcode"

