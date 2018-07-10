
/* globber.c */
/* 
   Copyright 2000-2001 Edscott Wilson Garcia

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program;  */

/*****************************************************************/

#include "fgr-globber.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <glob.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef DMALLOC
# include "dmalloc.h"
// exit(1) taken care of below.
#endif

/** tripas **/
/* private */
#ifndef GLOB_TILDE
# define GLOB_TILDE 0x0
#endif
#ifndef GLOB_ONLYDIR
# define GLOB_ONLYDIR 0x0
#endif

#define MONTH_T 2628000
#define DAY_T 86400
#define HOUR_T 3600
#define MIN_T 60

typedef struct objeto_globber {
    int options;
    int type;
    int user;
    int group;
    off_t sizeG;
    off_t sizeL;
    long unsigned month_t;
    long unsigned day_t;
    long unsigned hour_t;
    long unsigned min_t;
/* private variables, not to be duplicated on recursion: */
    struct stat st;
    int pass;
    time_t tiempo;
    time_t actual;
    int dostat;
} objeto_globber;

static int
display (char *input) {
    printf ("%s\n", input);     /*fflush(stdout); */
    return 0;
}

/* public */
int
glob_clear_options (void *address) {
    if (!address) return 1;
    objeto_globber *objeto = address;
    memset(objeto, 0, sizeof(objeto_globber));
    objeto->user = -1;
    objeto->group = -1;
    objeto->sizeL = -1;
    objeto->sizeG = -1;
    objeto->min_t = -1;
    objeto->hour_t = -1;
    objeto->day_t = -1;
    objeto->month_t = -1;
    return 1;
}

void *
globber_create (void) {
    objeto_globber *objeto;
    objeto = (objeto_globber *) malloc (sizeof (objeto_globber));
    if (!objeto) {
	fprintf(stderr, "Unable to malloc basic structure.\n");
	exit(1);
    }
    glob_clear_options ((void *)objeto);
    return (void *)objeto;
}

void *
globber_destroy (void *address) {
    if(address)
        free (address);
    return NULL;
}

int
glob_set_options (void *address, int options) {
    if (!address){
	fprintf(stderr, "broken call to glob_set_options()\n");
	exit(0);
    }
    objeto_globber *objeto = address;
    objeto->options |= options;
    return 1;
}

int
glob_set_type (void *address, int type) {
    if (!address){
	fprintf(stderr, "broken call to glob_set_type()\n");
	exit(0);
    }
    objeto_globber *objeto = address;
    objeto->type = type;
    return 1;
}

int
glob_set_sizeG (void *address, off_t size) {
    if (!address){
	fprintf(stderr, "broken call to glob_set_sizeG()\n");
	exit(0);
    }
    objeto_globber *objeto = address;
    glob_set_options (objeto, GLOBBER_SIZE);
    objeto->sizeG = size;
    return 1;
}

int
glob_set_sizeL (void *address, off_t size) {
    if (!address){
	fprintf(stderr, "broken call to glob_set_sizeL()\n");
	exit(0);
    }
    objeto_globber *objeto = address;
    glob_set_options (objeto, GLOBBER_SIZE);
    objeto->sizeL = size;
    return 1;
}

int
glob_set_user (void *address, int user) {
    if (!address){
	fprintf(stderr, "broken call to glob_set_user()\n");
	exit(0);
    }
    objeto_globber *objeto = address;
    glob_set_options (objeto, GLOBBER_USER);
    objeto->user = user;
    return 1;
}

int
glob_set_group (void *address, int group) {
    if (!address){
	fprintf(stderr, "broken call to glob_set_group()\n");
	exit(0);
    }
    objeto_globber *objeto = address;
    ;
    glob_set_options (objeto, GLOBBER_GROUP);
    objeto->group = group;
    return 1;
}

int
glob_set_minutes (void *address, long unsigned min_t) {
    if (!address){
	fprintf(stderr, "broken call to glob_set_time()\n");
	exit(0);
    }
    objeto_globber *objeto = address;
    objeto->min_t = min_t;
    return 1;
}

int
glob_set_hours (void *address, long unsigned hour_t) {
    if (!address){
	fprintf(stderr, "broken call to glob_set_time()\n");
	exit(0);
    }
    objeto_globber *objeto = address;
    objeto->hour_t = hour_t;
    return 1;
}

int
glob_set_days (void *address,long unsigned day_t) {
    if (!address){
	fprintf(stderr, "broken call to glob_set_time()\n");
	exit(0);
    }
    objeto_globber *objeto = address;
    objeto->day_t = day_t;
    return 1;
}

int
glob_set_months (void *address, long unsigned month_t) {
    if (!address){
	fprintf(stderr, "broken call to glob_set_time()\n");
	exit(0);
    }
    objeto_globber *objeto = address;
    objeto->month_t = month_t;
    return 1;
}

/* if the user defined "operate" function returns TRUE, Globber will exit 
 * and return to calling module with the same return value  */

int
globber (void *address, char *path, int (*operate) (char *), char *filter) {
    /* these variables must be kept on the heap */
    glob_t dirlist;
    int i;
    char *globstring,
     *dot_filter = NULL,
        *actual_filter = NULL;
    objeto_globber *object;
    struct stat path_st;
    errno=0;
  /*  if (!path || !strlen(path)) {
        fprintf(stderr, "invalid path: %s\n", path?"\"\"":"NULL");
        return -1;
    }*/
    if(!address)
        object = (objeto_globber *) globber_create ();
    else
        object = (objeto_globber *) address;

// this is debug rather than verbose...
// if (object->options & GLOBBER_VERBOSE) fprintf(stderr, "---> %s\n", path);
    if(object->options & GLOBBER_TIME) {
        if(object->options & GLOBBER_MTIME)
            object->options &= ((GLOBBER_CTIME | GLOBBER_ATIME) ^ 0xffffffff);
        else if(object->options & GLOBBER_CTIME)
            object->options &= (GLOBBER_ATIME ^ 0xffffffff);
    }

    dirlist.gl_offs = 2;
    if(!operate)
        operate = display;
    actual_filter = filter;
  filter_repeat:

    if(actual_filter) {
        globstring = (char *)malloc (strlen (path) + strlen (actual_filter) + 2);
	if (!globstring){fprintf(stderr,"malloc: %s", strerror(errno)); exit(1);}
        strcpy (globstring, path);
        if(path[strlen (path) - 1] != '/')
            strcat (globstring, "/");
        strcat (globstring, actual_filter);
    } else
        globstring = path;

    // If file cannot be stat(), assume it is not there (whatever).
    // coverity[fs_check_call : FALSE]
    if(stat (path, &path_st) < 0) {
        fprintf(stderr, "%s: %s\n", path, strerror (errno));
        int pass = object->pass;
        if (!address) free(object);
        if (actual_filter) free(globstring);
        return (pass);
    }
    if(glob (globstring, GLOB_ERR | GLOB_TILDE, NULL, &dirlist) != 0) {
// this is debug rather than verbose...
//    if (object->options & GLOBBER_VERBOSE)
//      fprintf(stderr, "%s: %s\n", globstring,strerror(ENOENT));
    } else {
        for(i = 0; i < dirlist.gl_pathc; i++) {
            if((object->options & GLOBBER_STAT) && lstat (dirlist.gl_pathv[i], &(object->st))>=0) {
                if(object->options & GLOBBER_USER) {
                    if(object->user != object->st.st_uid)
                        continue;
                }
                if(object->options & GLOBBER_GROUP) {
                    if(object->group != object->st.st_gid)
                        continue;
                }
                if(object->options & GLOBBER_TIME) {
                    object->actual = time (NULL);
                    if(object->options & GLOBBER_MTIME)
                        object->tiempo = object->st.st_mtime;
                    if(object->options & GLOBBER_ATIME)
                        object->tiempo = object->st.st_atime;
                    if(object->options & GLOBBER_CTIME)
                        object->tiempo = object->st.st_ctime;

                    if((object->min_t > 0) && ((object->actual - object->tiempo) / MIN_T > object->min_t))
                        continue;
                    if((object->hour_t > 0) && ((object->actual - object->tiempo) / HOUR_T > object->hour_t))
                        continue;
                    if((object->day_t > 0) && ((object->actual - object->tiempo) / DAY_T > object->day_t))
                        continue;

                    if((object->month_t > 0) && ((object->actual - object->tiempo) / MONTH_T > object->month_t))
                        continue;


                }
                if(object->options & GLOBBER_SIZE) {
                    if((object->sizeL >= 0) && (object->st.st_size > object->sizeL))
                        continue;
                    if((object->sizeG > 0) && object->st.st_size < object->sizeG)
                        continue;
                }
                if(object->options & GLOBBER_PERM) {
                    if((object->st.st_mode & 07777) & (object->type & 07777)) ;
                    else {
                        if((object->st.st_mode & 07777) == (object->type & 07777)) ;
                        else
                            continue;
                    }
                }

                if(object->options & GLOBBER_TYPE) {
                    if((object->st.st_mode & S_IFMT) != (object->type & S_IFMT))
                        continue;
                }
            }
            /* done lstat'ing */
            if((object->pass = (*(operate)) (dirlist.gl_pathv[i])) != 0)
                break;
        }
    }                           /* initial glob is done */
    if(actual_filter) free (globstring);
    globfree (&dirlist);
    if(!dot_filter && object->options & GLOBBER_ADD_DOT_FILTER) {
        dot_filter = (char *)malloc (strlen (filter) + 2);
	if (!dot_filter){fprintf(stderr,"malloc: %s", strerror(errno)); exit(1);}
        strcpy (dot_filter, ".");
        strcat (dot_filter, filter);
        actual_filter = dot_filter;
        goto filter_repeat;
    }
    if(dot_filter) {
        free (dot_filter);
        dot_filter = NULL;
        actual_filter = NULL;
    }
    if(object->pass) {
        int pass = object->pass;
        if (!address) free(object);
        return (pass);  /* error returned from function */
    }

    if(object->options & GLOBBER_RECURSIVE) {
        DIR *directory;
        struct dirent *d;
        directory = opendir (path);
        if(directory)
            while((d = readdir (directory)) != NULL) {
                char *fullpath;
                if(strcmp (d->d_name, ".") == 0 || strcmp (d->d_name, "..") == 0)
                    continue;
                fullpath = (char *)malloc (strlen (path) + strlen (d->d_name) + 2);
	if (!fullpath){fprintf(stderr,"malloc: %s", strerror(errno)); exit(1);}
                sprintf (fullpath, "%s/%s", path, d->d_name);

                if(lstat (fullpath, &(object->st)) < 0) {
                    free (fullpath);
                    continue;
                }

                if(!S_ISDIR (object->st.st_mode) || (object->st.st_mode & S_IFMT) == S_IFLNK) {
                    free (fullpath);
                    continue;   /* dont follow symlinks */
                }
                if(S_ISDIR (object->st.st_mode) && *(d->d_name) == '.' && (object->options & GLOBBER_RECURSIVE_NO_HIDDEN)) {
                    /*printf("object->options=0x%x\n",(unsigned)object->options); */
                    free (fullpath);
                    continue;   /* dont recurse into hidden directories */
                }
                if(object->options & GLOBBER_XDEV && object->st.st_dev != path_st.st_dev) {
                    free (fullpath);
                    continue;   /* dont leave device */
                }
// this is debug rather than verbose...
//      if (object->options & GLOBBER_VERBOSE) fprintf(stderr, "%s: --->\n",fullpath);

                object->pass = globber (address, fullpath, operate, filter);
                if(object->pass) {
                    free (fullpath);
                    break;
                }
                free (fullpath);
            }
        closedir (directory);

    }
    int pass = object->pass;
    if (!address) free(object);
    return (pass);
}
