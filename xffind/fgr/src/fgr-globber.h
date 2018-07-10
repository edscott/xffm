
/* globber.h */
/* 
   Copyright 2000 Edscott Wilson Garcia

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program;   */

/*****************************************************************/

/* version 0.5.0 is object oriented and eliminates sharing of
*  global variables with other modules. */

/* globber in its own .o file and link it in later:*/

#define GLOBBER_VERSION 0.5.2

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#include  <locale.h>
#include "fgr-intl.h"

#define GLOBBER_MASK	     0xffff

#define GLOBBER_RECURSIVE    		0x01
#define GLOBBER_RECURSIVE_NO_HIDDEN    	0x02
#define GLOBBER_VERBOSE      		0x04
#define GLOBBER_ADD_DOT_FILTER		0x08

/* things that require a stat: */
#define GLOBBER_MTIME        0x040
#define GLOBBER_ATIME        0x080
#define GLOBBER_CTIME        0x100
#define GLOBBER_TIME         (GLOBBER_MTIME|GLOBBER_ATIME|GLOBBER_CTIME)
#define GLOBBER_XDEV         0x200
#define GLOBBER_SIZE         0x400
#define GLOBBER_PERM         0x800
#define GLOBBER_TYPE         0x1000
#define GLOBBER_USER         0x2000
#define GLOBBER_GROUP        0x4000
#define GLOBBER_STAT	     (GLOBBER_XDEV|GLOBBER_SIZE|GLOBBER_TIME|GLOBBER_PERM|GLOBBER_TYPE|GLOBBER_USER|GLOBBER_GROUP)
#define GLOBBER_RESULT_LIMIT        0x8000

#ifndef S_IFMT
# define S_IFMT     0170000
#endif
#ifndef S_IFSOCK
# define        S_IFSOCK   0140000
#endif
#ifndef S_IFLNK
# define       S_IFLNK    0120000
#endif
#ifndef S_IFREG
# define       S_IFREG    0100000
#endif
#ifndef S_IFBLK
# define      S_IFBLK    0060000
#endif
#ifndef S_IFDIR
# define       S_IFDIR    0040000
#endif
#ifndef S_IFCHR
# define       S_IFCHR    0020000
#endif
#ifndef S_IFIFO
# define       S_IFIFO    0010000
#endif
#ifndef S_ISVTX
# define      S_ISVTX    0001000
#endif

int globber (
    void *address,
    char *input,
    int (*operate) (char *),
    char *filter
);
int glob_clear_options (
    void *address
);
void *globber_create (
    void
);
void *globber_destroy (
    void *
);
int glob_set_options (
    void *address,
    int options
);
int glob_set_type (
    void *address,
    int type
);
int glob_set_sizeL (
    void *address,
    off_t size
);
int glob_set_sizeG (
    void *address,
    off_t size
);
int glob_set_user (
    void *address,
    int user
);
int glob_set_group (
    void *address,
    int group
);

int glob_set_minutes ( void *address, long unsigned min_t);

int glob_set_hours ( void *address, long unsigned hour_t);

int glob_set_days ( void *address, long unsigned day_t);

int glob_set_months ( void *address, long unsigned month_t);
