#!/bin/sh

# $Id$
# Description: Build tool that strips debugging information from static MSVC libraries
#   using GNU binutils.
# Author: juergen.mellinger@uni-tuebingen.de

dir=$(dirname $0)

function strip_()
{
  f=$1
  echo Fixing up object file names ...
  ${dir}/fixup_msvc_lib "$(cygpath -a -w $f)" || exit -1
  echo Clearing .drectve debug flag ...
  ${dir}/coff_set_section_flags -v -s.drectve -debug "$(cygpath -a -w $f)" || exit -1
  echo Running objcopy -g ...
  objcopy -g $f || exit -1
  echo Enabling .drectve debug flag ...
  ${dir}/coff_set_section_flags -v -s.drectve +debug "$(cygpath -a -w $f)" || exit -1
}

function strip64_()
{ # in x64 lib files, it seems we cannot remove sections without breaking things
  f=$1
  echo Disabling debugging info ...
  sed -b -i 's/\.debug\$./\.ignored/g' $f || exit -1
}

if [ ! $1 ]; then
  echo "Expected names of libraries to strip." 2>&1
  exit -1
fi

res_=0
if [ "$1"=="-64" ]; then
  is64=1
  shift
  for i in $*; do
    echo "Stripping $i:"
    strip64_ $i && echo "Done." && echo || ( echo "Failed." && set res_=-1 )
  done
else
  for i in $*; do
    echo "Stripping $i:"
    strip_ $i && echo "Done." && echo || ( echo "Failed." && set res_=-1 )
  done
fi

exit $res_
