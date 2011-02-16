#!/bin/sh
################################################################################
# $Id$
# Author: juergen.mellinger@uni-tuebingen.de
# Description: Script to update version information in shared/config/Version.h.
#   Execute this script from its parent directory.
#
# (C) 2000-2008, BCI2000 Project
# http://www.bci2000.org
################################################################################
OUTFILE="../shared/config/Version.h";
INFILE="$OUTFILE.in";

REVISION=`svnversion ../.. -n`;
BUILDDATE=`date`;

sed -e "s/\\\$WCRANGE\\\$/$REVISION/" \
    -e '/\$WCDATE\$/d' \
    -e "s/\\\$WCNOW\\\$/$BUILDDATE/" \
    < $INFILE \
    > $OUTFILE;
