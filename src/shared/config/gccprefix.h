/*******************************************************************************
/* $Id$
/* Author: juergen.mellinger@uni-tuebingen.de
/* Description: Compiler specific definitions for the GNU gcc compiler.
/*   When compiling under gcc, this file should be specified as "prefix file".
/*
/* (C) 2000-2009, BCI2000 Project
/* http://www.bci2000.org
/*******************************************************************************/
#ifndef GCC_PREFIX_H
#define GCC_PREFIX_H

#ifndef __GNUC__
# error GCC prefix header file included for a compiler that is not GCC.
#endif

#ifndef NO_PCHINCLUDES
# define NO_PCHINCLUDES
#endif

#ifndef __FUNC__
# define __FUNC__        __PRETTY_FUNCTION__
#endif

#ifndef stricmp
# define stricmp         strcasecmp
#endif

#endif /* GCC_PREFIX_H */
