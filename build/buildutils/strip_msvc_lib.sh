#!/bin/sh

# $Id$
# Description: Build tool that strips debugging information from static MSVC libraries
#   using GNU binutils.
# Author: juergen.mellinger@uni-tuebingen.de

function strip_()
{
  f=$1
  echo Fixing up object file names ...
  fixup_msvc_lib "$(cygpath -a -w $f)" || exit -1
  echo Clearing .drectve debug flag ...
  coff_set_debug -v -s.drectve -0 "$(cygpath -a -w $f)" || exit -1
  echo Running objcopy -g ...
  objcopy -g $f || exit -1
  echo Enabling .drectve debug flag ...
  coff_set_debug -v -s.drectve -1 "$(cygpath -a -w $f)" || exit -1
}

if [ ! $1 ]; then
  echo "Expected names of libraries to strip." 2>&1
  exit -1
fi

res_=0
for i in $*; do
  echo "Stripping $i:"
  strip_ $i && echo "Done." && echo || ( echo "Failed." && set res_=-1 )
done

exit $res_
