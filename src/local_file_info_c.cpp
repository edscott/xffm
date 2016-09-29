#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "local_file_info_c.hpp"

local_file_info_c::local_file_info_c(void){
    user_string_mutex=PTHREAD_MUTEX_INITIALIZER;
    group_string_mutex=PTHREAD_MUTEX_INITIALIZER;
    date_string_mutex=PTHREAD_MUTEX_INITIALIZER;
}

local_file_info_c::~local_file_info_c(void){
    DBG("local_file_info_c::~local_file_info_c...\n");
}

gchar *
local_file_info_c::get_path_info (const gchar *file_path, GtkTreePath *tpath) {
    struct stat st;
    if (!file_path) return g_strdup("file_path is NULL\n");
    gchar *g=NULL;
    if (lstat(file_path, &st) != 0) {
        gchar *u = utf_string(file_path);
        g = g_strdup_printf(_("Cannot stat \"%s\":\n%s\n"), u, strerror(errno));
        g_free(u);
        return g;
    }
    g = g_strdup("");
    if(S_ISDIR (st.st_mode)) {
        gint files = count_files (file_path);
        gint hidden = count_hidden_files (file_path);
        if(files) {
            gchar *files_string = g_strdup_printf (ngettext (" (containing %'d item)", " (containing %'d items)", files),files);
    
            gchar *plural_string = 
                g_strdup_printf(ngettext ("%'u item","%'u items",hidden), hidden);
            gchar *hidden_string = 
                g_strdup_printf ("%s: %s.",_("Hidden"), plural_string);
            g_free(plural_string);
            g_free(g);
            g = g_strdup_printf ("%s\n%s", files_string, hidden_string);
            g_free(hidden_string);
            g_free (files_string);
        } else {
            g = g_strdup_printf ("%s", _("The location is empty."));
        }
    }
    
    gchar *info = path_info (file_path, &st, g);
    g_free(g);
    g = info;
    
    return g;
}


/**
 * @path: directory path
 * Returns: file count within directory, including hidden files
 *
 * This function is non-recursive.
 **/
gint
local_file_info_c::count_files (const gchar * file_path) {
    if(!g_file_test (file_path, G_FILE_TEST_IS_DIR)) return 0;
    DIR *directory;
    directory = opendir (file_path);
    if(!directory) return 0;
    gint count = 0;
    struct dirent *d;
    while((d = readdir (directory)) != NULL) {
        if(strcmp (d->d_name, ".") == 0)
            continue;
        if(strcmp (d->d_name, "..") == 0)
            continue;
        count++;
    }
    closedir (directory);
    return count;
}

/**
 * @path: directory path
 * Returns: hidden file count within directory
 *
 * This function is non-recursive.
 **/
gint
local_file_info_c::count_hidden_files (const gchar * file_path) {
    if(!g_file_test (file_path, G_FILE_TEST_IS_DIR)) return 0;
    DIR *directory;
    directory = opendir (file_path);
    if(!directory) return 0;
    gint count = 0;
    struct dirent *d;
    while((d = readdir (directory)) != NULL) {
        if(strcmp (d->d_name, ".") == 0)
            continue;
        if(strcmp (d->d_name, "..") == 0)
            continue;
        if(d->d_name[0] != '.')
            continue;
        count++;
    }
    closedir (directory);
    return count;
}


gchar *
local_file_info_c::path_info (const gchar *file_path, struct stat *st, const gchar *pretext) {
    gchar *s1 = NULL, *s2 = NULL;
    gchar *info = NULL;
    if(!file_path) return NULL;
    if(S_ISLNK (st->st_mode)) {
	NOOP(stderr, "local lnk  type...\n");
        gchar lpath[_POSIX_PATH_MAX + 1];
        memset (lpath, 0, _POSIX_PATH_MAX + 1);
        if(readlink (file_path, lpath, _POSIX_PATH_MAX) > 0) {
            gchar *v = utf_string(lpath);
            gchar *escaped_markup = g_markup_escape_text(v, -1);
            g_free(v);
            gchar *q = utf_string (escaped_markup);
            g_free(escaped_markup);
            gchar *linkto=g_strdup_printf (_("Link to %s"), q);
            s1 = g_strdup_printf ("%s\n<i>%s</i>\n\n", linkto, pretext);
            g_free(linkto);
            g_free (q);
        }
    } 
    gchar *p = g_strdup_printf ("<i>%s</i>\n\n", pretext);
    s1 = p;
    gchar *s12 = NULL;
	
#if 0
    // overkill    
    rfm_set_mime_dtype(en);
    if (!en->mimetype) {
	NOOP(stderr, "getting mimetype: %s\n", en->path);
	en->mimetype = MIME_type(en->path, st); 
    }
    
    if (IS_LOCAL_TYPE(en->type)){
	if (!en->mimemagic || strcmp(en->mimemagic, _("unknown"))==0) {
	    gchar *old = en->mimemagic;
	    NOOP(stderr, "getting magic type: %s\n", en->path);
	    en->mimemagic = rfm_rational(RFM_MODULE_DIR, "mime", en, "mime_magic", "mime_function"); 
	    g_free(old);
	    
	    if (!en->mimemagic) en->mimemagic = g_strdup(_("unknown"));
	}
	if (!en->filetype || strcmp(en->filetype, _("unknown"))==0) {
	    NOOP(stderr, "getting file type: %s\n", en->path);
	    gchar *old = en->filetype;
	    en->filetype = rfm_rational(RFM_MODULE_DIR, "mime", en, "mime_file", "mime_function"); 
	    g_free(old);
	    if (!en->filetype) en->filetype = g_strdup(_("unknown"));
	}
	if (!en->encoding || strcmp(en->encoding, _("unknown"))==0) {
	    gchar *old = en->encoding;
	    NOOP(stderr, "getting file encoding: %s\n", en->path);
	    en->encoding = rfm_rational(RFM_MODULE_DIR, "mime", en, "mime_encoding", "mime_function"); 
	    g_free(old);
	    if (!en->encoding) en->encoding = g_strdup(_("unknown"));
	}

    } 
    else {
	NOOP(stderr, "Not a local type: %s\n", en->path);
    }

    if ((en->mimetype && strstr(en->mimetype, "x-trash")) || 
	en->path[strlen(en->path)-1] =='~' ||
	en->path[strlen(en->path)-1] =='%' ) {
	g_free(en->filetype);
	en->filetype = g_strdup(_("Backup file"));
    }
    s12 = g_strdup_printf("<b>%s</b>: %s\n<b>%s</b> (freedesktop): %s\n<b>%s</b> (libmagic): %s\n<b>%s</b>: %s\n\n",
	    _("File Type"), en->filetype,
	    _("MIME Type"), (en->mimetype)?en->mimetype:_("unknown"),
	    _("MIME Type"), en->mimemagic,
	    _("Encoding"), en->encoding);

#endif
    gchar *grupo=group_string(st);
    gchar *owner=user_string(st);
    gchar *tag = sizetag ((off_t) st->st_size, -1);

    //    gchar *ss= rfm_time_to_string(st->st_mtime);   

    gchar *t = g_path_get_dirname (file_path);
    gchar *v = utf_string(t);
    gchar *escaped_markup = g_markup_escape_text(v, -1);
    g_free(v);
    gchar *dirname = utf_string (escaped_markup);
    g_free(t);
    g_free(escaped_markup);
    gchar *mode_string_s=mode_string (st->st_mode);
    s2 = g_strdup_printf (
            "<b>%s/%s</b>: %s/%s\n<b>%s</b>: %s\n<b>%s</b>: %s\n\n<b>%s</b>: %s",
             _("Owner"),_("Group"), owner, grupo,
            _("Permissions"), mode_string_s,
            _("Folder"), dirname, 
            _("Size"),  tag);

    g_free (owner);
    g_free (grupo);
    g_free (tag);
    g_free (dirname);
    g_free (mode_string_s);

    gchar buf[1024];

    gchar *date_string_s=date_string(st->st_ctime);

    sprintf (buf, "<b>%s :</b> %s", _("Status Change"), date_string_s);
    g_free(date_string_s);

    gchar *s3 = g_strconcat (s2, "\n", buf, NULL);
    g_free (s2);
    s2 = s3;

    date_string_s=date_string(st->st_mtime);
    sprintf (buf, "<b>%s</b> %s", _("Modification Time :"), date_string_s);
    g_free(date_string_s);


    s3 = g_strconcat (s2, "\n", buf, NULL);
    g_free (s2);
    s2 = s3;

    date_string_s=date_string(st->st_atime);
    sprintf (buf, "<b>%s</b> %s", _("Access Time :"), date_string_s);
    g_free(date_string_s);

    s3 = g_strconcat (s2, "\n", buf, NULL);
    g_free (s2);
    s2 = s3;

    gchar *hard_links = g_strconcat(_("Links")," (", _("hard"), ")", NULL);
    s3 = g_strdup_printf ("%s\n\n<b>%s</b>: %ld\n<b>%s</b>: %ld",
            s2, hard_links,
            (long)st->st_nlink, _("Inode"), (long)st->st_ino);
    g_free(hard_links);
            
    g_free (s2);
    s2 = s3;

    if(!s1) s1 = g_strdup ("");
    if(!s12) s12 = g_strdup ("");
    if(!s2) s2 = g_strdup ("");
    fprintf(stderr, "%s\n" , s2);
    info = g_strconcat (s1, s12, s2, NULL);
    g_free (s1);
    g_free (s2);
    g_free (s12);
   return info;
}




gchar *
local_file_info_c::mode_string (mode_t mode) {
    gchar *str=(gchar *)malloc(13);
    if (!str) g_error("malloc: %s", strerror(errno));
    str[0] = ftypelet (mode);
    str[1] = mode & S_IRUSR ? 'r' : '-';
    str[2] = mode & S_IWUSR ? 'w' : '-';
    str[3] = mode & S_IXUSR ? 'x' : '-';
    str[4] = mode & S_IRGRP ? 'r' : '-';
    str[5] = mode & S_IWGRP ? 'w' : '-';
    str[6] = mode & S_IXGRP ? 'x' : '-';
    str[7] = mode & S_IROTH ? 'r' : '-';
    str[8] = mode & S_IWOTH ? 'w' : '-';
    str[9] = mode & S_IXOTH ? 'x' : '-';
    if(mode & S_ISUID)
        str[3] = mode & S_IXUSR ? 's' : 'S';
    if(mode & S_ISGID)
        str[6] = mode & S_IXGRP ? 's' : 'S';
    if(mode & S_ISVTX)
        str[9] = mode & S_IXOTH ? 't' : 'T';
    str[10] = 0;
    return (str);
}
    

gchar *
local_file_info_c::user_string (struct stat *st) {
    pthread_mutex_lock(&user_string_mutex);
    struct passwd *p;
    gchar *user_string;
    if((p = getpwuid (st->st_uid)) != NULL)
            user_string = g_strdup(p->pw_name);
        else if((gint)st->st_uid < 0)
            user_string = g_strdup("");
        else
            user_string = g_strdup_printf("%d", (gint)st->st_uid);
    pthread_mutex_unlock(&user_string_mutex);
    return user_string;
}



gchar *
local_file_info_c::group_string (struct stat *st) {
    pthread_mutex_lock(&group_string_mutex);
    struct group *g;
    gchar *group_string;
    if((g =  getgrgid(st->st_gid)) != NULL)
            group_string = g_strdup(g->gr_name);
    else
        group_string = g_strdup_printf("%d", (gint)st->st_gid);
    pthread_mutex_unlock(&group_string_mutex);
    return group_string;
}

gchar *
local_file_info_c::date_string (time_t the_time) {
    pthread_mutex_lock(&date_string_mutex);

#ifdef HAVE_LOCALTIME_R
        struct tm t_r;
#endif
        struct tm *t;

#ifdef HAVE_LOCALTIME_R
        t = localtime_r (&the_time, &t_r);
#else
        t = localtime (&the_time);
#endif
        gchar *date_string=
	    g_strdup_printf ("%04d/%02d/%02d  %02d:%02d", t->tm_year + 1900,
                 t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);
    pthread_mutex_unlock(&date_string_mutex);

    return date_string;
}


gchar *
local_file_info_c::sizetag (off_t tama, gint count) {
    gchar *tag = _("bytes");
    gchar *buf = NULL;
    double utama = tama;

    buf = NULL;
    if(utama > 0) {
        if(utama >= (off_t)1000 * 1000 * 1000) {
            utama /= ((off_t)1000 * 1000 * 1000);
            tag = _("Gigabytes");
        } else if(utama >= 1000 * 1000) {
            utama /= (1000 * 1000);
            tag = _("Megabytes");
        } else if(utama >= 1000) {
            utama /= 1000;
            tag = _("Kilobytes");
        }
        if(count <= 0) {
            /* format for size column of regular files */
            buf = g_strdup_printf ("%.2lf %s", utama, tag);
        } else {
            gchar *plural_text=
                g_strdup_printf (ngettext ("%'u item", "%'u items", 
                            count),count);
	    if (tama < 1000) {
		buf = g_strdup_printf ("%s: %.0lf %s.", plural_text,
                    utama, tag);
	    } else {
		buf = g_strdup_printf ("%s: %.2lf %s.", plural_text,
                    utama, tag);
	    }
            g_free(plural_text);
    
        }
    } else {
        if(count <=0) {
            buf = g_strdup_printf (_("The location is empty."));
        } else {
            buf=
                g_strdup_printf (ngettext ("%'u item", "%'u items", count),
                        count);
        }
    }
    return buf;
}


/* Return a character indicating the type of file described by
   file mode BITS:
   'd' for directories
   'D' for doors
   'b' for block special files
   'c' for character special files
   'n' for network special files
   'm' for multiplexor files
   'M' for an off-line (regular) file
   'l' for symbolic links
   's' for sockets
   'p' for fifos
   'C' for contigous data files
   '-' for regular files
   '?' for any other file type.  */


gchar
local_file_info_c::ftypelet (mode_t bits) {
#ifdef S_ISBLK
    if(S_ISBLK (bits)) return 'b';
#endif
    if(S_ISCHR (bits)) return 'c';
    if(S_ISDIR (bits)) return 'd';
    if(S_ISREG (bits)) return '-';
#ifdef S_ISFIFO
    if(S_ISFIFO (bits)) return 'p';
#endif
#ifdef S_ISLNK
    if(S_ISLNK (bits)) return 'l';
#endif
#ifdef S_ISSOCK
    if(S_ISSOCK (bits)) return 's';
#endif
#ifdef S_ISMPC
    if(S_ISMPC (bits)) return 'm';
#endif
#ifdef S_ISNWK
    if(S_ISNWK (bits)) return 'n';
#endif
#ifdef S_ISDOOR
    if(S_ISDOOR (bits)) return 'D';
#endif
#ifdef S_ISCTG
    if(S_ISCTG (bits)) return 'C';
#endif

    /* The following two tests are for Cray DMF (Data Migration
       Facility), which is a HSM file system.  A migrated file has a
       `st_dm_mode' that is different from the normal `st_mode', so any
       tests for migrated files should use the former.  */

#ifdef S_ISOFD
        /* off line, with data  */
    if(S_ISOFD (bits)) return 'M';
#endif
#ifdef S_ISOFL
    /* off line, with no data  */
    if(S_ISOFL (bits)) return 'M';
#endif
    return '?';
}


