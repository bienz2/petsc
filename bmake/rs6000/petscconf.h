/* $Id: petscconf.h,v 1.5 1998/04/16 16:07:59 bsmith Exp bsmith $ */

/*
    Defines the configuration for this machine
*/
#if !defined(INCLUDED_PETSCCONF_H)
#define INCLUDED_PETSCCONF_H

#define HAVE_STROPTS_H 
#define HAVE_SEARCH_H 
#define HAVE_PWD_H 
#define HAVE_STDLIB_H
#define HAVE_STRING_H 
#define HAVE_STRINGS_H 
#define HAVE_MALLOC_H 
#define _POSIX_SOURCE
#define HAVE_DRAND48  
#define HAVE_GETDOMAINNAME  
#define _XOPEN_SOURCE 
#define HAVE_X11
#define HAVE_UNISTD_H 
#define HAVE_SYS_TIME_H 
#define HAVE_UNAME 
#define _XOPEN_SOURCE_EXTENDED 1
#define _ALL_SOURCE
#define HAVE_DOUBLE_ALIGN_MALLOC

#if !defined(HAVE_XLF90)
#define HAVE_XLF90
#endif

#define PREFER_BZERO

#define HAVE_READLINK
#define HAVE_MEMMOVE

#endif
