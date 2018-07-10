
/* glob.c file filter for grep.*/
/* 
   Copyright 2000-2010 Edscott Wilson Garcia

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

/** tripas */
#define GREP "grep"

static void *object = NULL;
static int initial;
static int terminated = 0;
static char *token;
static int options = 0,
    type = 0;
static long size = 0;
static long month_t = 0;
static long unsigned day_t = 0;
static long unsigned hour_t = 0;
static long unsigned min_t = 0;

/* two low order bytes are defined in globber.h */
#define GLOBRUN_PID     	0x10000
#define GLOBRUN_COUNT   	0x20000
#define GLOBRUN_FILTERED   	0x40000
#define GLOBRUN_IGNORE_CASE	0x80000
#define GLOBRUN_REG_EXP  	0x100000
#define GLOBRUN_INVERT       	0x200000
#define GLOBRUN_WORDS_ONLY   	0x400000
#define GLOBRUN_LINES_ONLY   	0x800000
#define GLOBRUN_ZERO_BYTE    	0x1000000
#define GLOBRUN_NOBINARIES   	0x2000000

#define MAX_ARG 25
#ifndef COPYRIGHT
# define COPYRIGHT "Copyright (c) 2002-2010 Edscott Wilson Garcia. GPL distribution licence."
#endif

static int
display (char *input) {
    if(terminated)
        return terminated;      /* die quietly and quickly */
    printf ("%s\n", input);
    if(time (NULL) - initial > 3) {
        fflush (NULL);
        initial = time (NULL);
    }
    return terminated;
}

static int
grep (char *file) {
    static char *arguments[MAX_ARG];
    int status = 0;
    if(terminated)
        return terminated;      /* die quietly and quickly */

    arguments[status++] = "grep";
#ifdef GNU_GREP
    //arguments[status++] = "-d";
    //arguments[status++] = "skip";
    //arguments[status++] = "-H";
    if(options & GLOBRUN_NOBINARIES)
        arguments[status++] = "-I";
    if(options & GLOBRUN_ZERO_BYTE)
        arguments[status++] = "-Z";
    if(options & GLOBRUN_INVERT) {
        arguments[status++] = "-L";
    } else {
        arguments[status++] = "-l";
    }
#else
    if(options & GLOBRUN_INVERT) {
        arguments[status++] = "-v";
    } else {
        arguments[status++] = "-l";
    }
#endif
    if(options & GLOBRUN_IGNORE_CASE)
        arguments[status++] = "-i";
    if(options & GLOBRUN_WORDS_ONLY)
        arguments[status++] = "-w";
    if(options & GLOBRUN_LINES_ONLY)
        arguments[status++] = "-x";

    if((options & GLOBRUN_COUNT)) {
        arguments[status++] = "-c";
    }
#ifdef GNU_GREP
    if(options & GLOBRUN_REG_EXP)
        arguments[status++] = "-E";
    else
#endif
        arguments[status++] = "-e";
    
    arguments[status++] = token;

    arguments[status++] = file;
    arguments[status++] = (char *)0;
#if 0
  //  if (options & GLOBBER_VERBOSE)
  {
    int i;
    for (i = 0; arguments[i] != NULL; i++)
      fprintf(stderr,"%s ", arguments[i]);
    fprintf(stderr,"\n");
  }
#endif
    if(fork () == 0) {
        execvp (GREP, arguments);
        fprintf(stderr, "%s: %s\n", strerror (ENOENT), GREP);
        exit (1);
    }
    wait (&status);

    return terminated;
}

static char *message[] = {
    " ",
    N_("Options:")," ","[-vVPrMACaiIyLcwxZ] [-fpotkhsmudgeE]] ["N_("Path"),"]", "\n", " \n",
    "fgr: ", N_("Locate documents and folders on this computer by name or content"), "\n", " \n",
    N_("Filter:"), N_("Search for files"),"\n",
    //N_("-k | -h | -d | -m --->  -M | -C | -A"), "\n",
    N_("Contents:"), N_("File's contents filtered as plain text."), "\n", " \n",

    "-v", "\t\t",
    N_("Verbose output debugging mode."), "\n",
    "-V", "\t\t",
    N_("Show version information"), "\n"," \n",

    N_("Filter Details"),"\n"," \n",

    "-a", "\t\t",
    N_("Stay on single filesystem"),"\n",
    "-P", "\t\t",
    N_("Print...") ," ", N_("The unique Process ID that identifies this process."),"\n",
    "-D", "\t\t",
    N_("Find hidden files and directories"), "\n",
    "-f ", N_("Filter"),"\t"
    N_("To display files which pass the filter (stars and regexps will be processed)"),"\n",
    "-r", "\t\t",
    N_("Recursive"),"\n",
    "-s ", "+",N_("KByte"), "\t",
    N_("is greater than or equal to"), " (KB): ", N_("KByte"),"\n",
    "-s ", "-", N_("KByte"), "\t",
    N_("is less than or equal to"), " (KB): ", N_("KByte"),"\n",
    "-p ", "suid | exe", "\t",
    N_("Set UID"), "(suid) | ", N_("Executable Text Files"), " (exe)\n",
    "-o ", "octal", "\t",
    N_("The permissions of the file, in octal notation."),"\n",
    "-t ", N_("Type"), " \t",
    " any | reg | dir | sym | sock | blk | chr | fifo","\n",
    "\t\t",N_("Any")," (any) ", N_("Default type"),"\n",
    "\t\t",N_("Regular file")," (reg)\n",
    "\t\t",N_("Directory")," (dir)\n",
    "\t\t",N_("Symbolic Link")," (sym)\n",
    "\t\t",N_("UNIX Socket")," (sock)\n",
    "\t\t",N_("Block device")," (blk)\n",
    "\t\t",N_("Character device")," (chr)\n",
    "\t\t",N_("FIFO")," (fifo)\n",

    "-k ", N_("Minutes"),"\t",
    N_("Details"),": ", N_("TIME"),
      " (", N_("Previous"), "): " N_("Minutes"), " [--> -M | -C | -A]", "\n",
    "-h ", N_("Hours"), "\t",
    N_("Details"),": ", N_("TIME"),
      " (", N_("Previous"), "): " N_("Hours"), " [--> -M | -C | -A]", "\n",
    "-d ", N_("Days"), " \t",
    N_("Details"),": ", N_("TIME"),
      " (", N_("Previous"), "): " N_("Days"), " [--> -M | -C | -A]", "\n",
    "-m ", N_("Month"), "\t", 
    N_("Details"),": ", N_("TIME"),
      " (", N_("Previous"), "): " N_("Month(s)"), " [--> -M | -C | -A]", "\n",

    "-M ", N_("TIME"), "\t",
    N_("Modification Time :"),  N_("TIME"), " (", 
	"mknod, truncate, utime, write)", "\n", 

    "-A ", N_("TIME"), "\t",
    N_("Access Time :"),  N_("TIME"), " (",
	"exec, mknod, pipe, utime, read)", "\n", 

    "-C ", N_("TIME"), "\t",
    N_("Creation Time :"),  N_("TIME"), " (",
	"chown, chgrp, chmod, ln)", "\n", 

    "-u ", N_("uid"), "  \t",
    N_("User:"), " ", N_("uid"),"\n",

    "-g ", " ", N_("gid"), "  \t",
    N_("Group:"), " ",  N_("gid"),"\n",

    "-Z", "\t\t",
    N_("Output  a  zero  byte  (the  ASCII  NULL  character) instead of the character that  normally  follows  a file  name"),"\n \n",
    
    N_("Content View"), ": --------\n \n", 
    "-e ", N_("STRING"), "\t"
    N_("Regular expression"), ": ", N_("STRING"), "\n",
    "-E ", N_("STRING"), "\t",
    N_("Regular expression"), "(", N_("Extended"), "): ", N_("STRING"), "\n",
    "-i", "\t\t",
    N_("Ignore case"), "\n",
    "-I", "\t\t",
    N_("Text Files"), "\n", 
    "-L", "\t\t",
    N_("No match"), "\n", 
    "-c", "\t\t",
    N_("Line Count"), "\n",
    "-w", "\t\t",
    N_("Whole words only"), "\n",
    "-x", "\t\t",
    N_("Match whole lines only"), "\n \n ",
    "fgr-", VERSION, " ", COPYRIGHT, "\n",
    NULL
};

void
finish (int sig) {
    /*printf("\n****\nglob terminated by signal\n****\n"); */
    terminated = 1;
    fflush (NULL);
}

void
halt (int sig) {
    fflush (NULL);
    globber_destroy (object);
    exit (1);
}

#define CHECK_ARG if (argc <= i) goto error;
int
main (int argc, char **argv) {
    int i,
      timetype = 0;
    char *filter = "*";
    int (*operate) (char *) = display;
    char *path = ".";
    initial = time (NULL);

#ifdef ENABLE_NLS
    setlocale (LC_MESSAGES, "");
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
    textdomain (PACKAGE);
#endif
    /* initializations  */
    signal (SIGHUP, halt);
    signal (SIGSEGV, finish);
    signal (SIGKILL, finish);
    signal (SIGTERM, finish);

    if(argc < 2) {
      error:
        fprintf (stdout, _("Usage : %s [OPTIONS]\n"), argv[0]);
        i = 0;
        while(message[i])
            fprintf (stdout, "%s", _(message[i++]));
        exit (1);
    }
    object = globber_create ();
    for(i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            /* options for the globber : **************** */
            if(strstr (argv[i], "v") != NULL) {
                glob_set_options (object, GLOBBER_VERBOSE);
                options |= GLOBBER_VERBOSE;
                continue;
            }
            if(strstr (argv[i], "M") != NULL) {
                timetype = 1;
                glob_set_options (object, GLOBBER_MTIME);
                continue;
            }
            if(strstr (argv[i], "A") != NULL) {
                timetype = 1;
                glob_set_options (object, GLOBBER_ATIME);
                continue;
            }
            if(strstr (argv[i], "C") != NULL) {
                timetype = 1;
                glob_set_options (object, GLOBBER_CTIME);
                continue;
            }
            if(strstr (argv[i], "a") != NULL) {
                glob_set_options (object, GLOBBER_XDEV);
                options |= GLOBBER_XDEV;
                continue;
            }
            if(strstr (argv[i], "v") != NULL) {
                glob_set_options (object, GLOBBER_VERBOSE);
                options |= GLOBBER_VERBOSE;
                continue;
            }
            if(strstr (argv[i], "r") != NULL) {
                glob_set_options (object, GLOBBER_RECURSIVE);
                options |= GLOBBER_RECURSIVE;
                continue;
            }
            if(strstr (argv[i], "R") != NULL) {
                glob_set_options (object, GLOBBER_RECURSIVE);
                glob_set_options (object, GLOBBER_RECURSIVE_NO_HIDDEN);
                options |= GLOBBER_RECURSIVE;
                options |= GLOBBER_RECURSIVE_NO_HIDDEN;
                continue;
            }
            if(strstr (argv[i], "u") != NULL) {
                i++;
                CHECK_ARG;
                glob_set_user (object, atol (argv[i]));
                continue;
            }
            if(strstr (argv[i], "g") != NULL) {
                i++;
                CHECK_ARG;
                glob_set_group (object, atol (argv[i]));
                continue;
            }

            if(strstr (argv[i], "t") != NULL) {
                i++;
                type &= 07777;
                CHECK_ARG;
                /*if (strcmp (argv[i], "any") == 0) type &= 07777; */
                if(strcmp (argv[i], "reg") == 0)
                    type |= S_IFREG;
                if(strcmp (argv[i], "dir") == 0)
                    type |= S_IFDIR;
                if(strcmp (argv[i], "sym") == 0)
                    type |= S_IFLNK;
                if(strcmp (argv[i], "sock") == 0)
                    type |= S_IFSOCK;
                if(strcmp (argv[i], "blk") == 0)
                    type |= S_IFBLK;
                if(strcmp (argv[i], "chr") == 0)
                    type |= S_IFCHR;
                if(strcmp (argv[i], "fifo") == 0)
                    type |= S_IFIFO;
                if(strcmp (argv[i], "any") != 0) {
                    glob_set_options (object, GLOBBER_TYPE);
                    glob_set_type (object, type);
                }
                continue;
            }
            if(strstr (argv[i], "p") != NULL) {
                i++;
                /*type &= S_IFMT; */
                CHECK_ARG;
                if(strcmp (argv[i], "suid") == 0)
                    type |= S_ISUID;
                if(strcmp (argv[i], "exe") == 0)
                    type |= S_IXUSR;
                glob_set_options (object, GLOBBER_PERM);
                glob_set_type (object, type);
                continue;
            }
            if(strstr (argv[i], "o") != NULL) {
                int valor;
                i++;
                type &= S_IFMT;
                CHECK_ARG;
                sscanf (argv[i], "%o", &valor);
                type |= (07777 & valor);
                glob_set_options (object, GLOBBER_PERM);
                glob_set_type (object, type);
                continue;
            }

            if(strstr (argv[i], "s") != NULL) {
                i++;
                CHECK_ARG;
                size = atol (argv[i]);
                if(size < 0){
                    glob_set_sizeL (object, (-1) * size * 1000);
		} else {
                    glob_set_sizeG (object, size * 1000);
		}
                continue;
            }

            if(strstr (argv[i], "D") != NULL) {
                glob_set_options (object, GLOBBER_ADD_DOT_FILTER);
                continue;
            }
	    // time
	    // minutes
	    if(strstr (argv[i], "k") != NULL) {
                i++;
                CHECK_ARG;
                glob_set_options (object, GLOBBER_TIME);
                min_t = atol (argv[i]);
                glob_set_minutes (object, min_t);
                continue;
            }
	    // hours
            if(strstr (argv[i], "h") != NULL) {
                i++;
                CHECK_ARG;
                glob_set_options (object, GLOBBER_TIME);
                hour_t = atol (argv[i]);
                glob_set_hours (object, hour_t);
                continue;
            }
	    // days
            if(strstr (argv[i], "d") != NULL) {
                i++;
                CHECK_ARG;
                glob_set_options (object, GLOBBER_TIME);
                day_t = atol (argv[i]);
                glob_set_days (object, day_t);
                continue;
            }
	    // months
            if(strstr (argv[i], "m") != NULL) {
                i++;
                CHECK_ARG;
                glob_set_options (object, GLOBBER_TIME);
                month_t = atol (argv[i]);
                glob_set_months (object, month_t);
                continue;
            }

            if(strstr (argv[i], "f") != NULL) {
                options |= GLOBRUN_FILTERED;
                i++;
                CHECK_ARG;
                filter = argv[i];
                /*if (options & GLOBBER_VERBOSE) 
                 *  fprintf(stderr, "filtering %s\n", filter);*/
                continue;
            }
            /* options for grep : ****************** */
            if(strstr (argv[i], "I") != NULL) {
                options |= GLOBRUN_NOBINARIES;
                continue;
            }
            if((strstr (argv[i], "i") != NULL) || (strstr (argv[i], "y") != NULL)) {
                options |= GLOBRUN_IGNORE_CASE;
                continue;
            }
            if(strstr (argv[i], "L") != NULL) {
                options |= GLOBRUN_INVERT;
                continue;
            }
            if(strstr (argv[i], "c") != NULL) {
                options |= GLOBRUN_COUNT;
                continue;
            }
            if(strstr (argv[i], "w") != NULL) {
                options |= GLOBRUN_WORDS_ONLY;
                continue;
            }
            if(strstr (argv[i], "x") != NULL) {
                options |= GLOBRUN_LINES_ONLY;
                continue;
            }
            if(strstr (argv[i], "Z") != NULL) {
                options |= GLOBRUN_ZERO_BYTE;
                continue;
            }
            if(strstr (argv[i], "P") != NULL) {
                options |= GLOBRUN_PID;
                printf ("PID=%d\n", (int)getpid ());
                fflush (NULL);
                continue;
            }
            if(strstr (argv[i], "E") != NULL) {
                i++;
                CHECK_ARG;
                token = argv[i];
                operate = grep;
                options |= GLOBRUN_REG_EXP;
                continue;
            }
            if(strstr (argv[i], "e") != NULL) {
                i++;
                CHECK_ARG;
                token = argv[i];
                operate = grep;
                options |= GLOBRUN_REG_EXP;
                options ^= GLOBRUN_REG_EXP;     /* turn off extended regexp */
                continue;
            }

            if(strstr (argv[i], "V") != NULL) {
                printf ("This is %s %s (find<-->grep)\nForward any bug reports to %s\n%s\n", "Rodent fgr", PACKAGE_VERSION,
                        PACKAGE_BUGREPORT, COPYRIGHT);
                return 0;
            }
            fprintf (stdout, "Oops... %s: %s\n", strerror (EINVAL), argv[i]);
            exit (1);
        }
        if(((min_t) || (hour_t) || (day_t) || (month_t)) && !timetype)
            glob_set_options (object, GLOBBER_MTIME);
        if(argv[i] != NULL)
            path = argv[i];
    }                           /* end of argument processing */

    terminated = globber (object, path, operate, filter);
/*	if (terminated) printf("glob run was terminated.\n");*/
    if(!terminated) {           /* die quietly and quickly */
        if(options & GLOBRUN_PID)
            printf ("GLOB DONE=%d\n", (int)getpid ());
    }
    if(options & GLOBBER_VERBOSE) {
        fprintf (stdout, "fgr search complete!\n");
    }
    fflush (NULL);
    globber_destroy (object);
    exit (0);
}
