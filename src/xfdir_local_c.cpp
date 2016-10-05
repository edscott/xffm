#include <dirent.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdlib.h>

#include "xfdir_local_c.hpp"

#define MAX_AUTO_STAT 500

#define O_ALL(x) ((S_IROTH & x) && (S_IWOTH & x) &&  (S_IXOTH & x))
#define G_ALL(x) ((S_IRGRP & x) && (S_IWGRP & x) &&  (S_IXGRP & x))
#define U_ALL(x) ((S_IRUSR & x) && (S_IWUSR & x) &&  (S_IXUSR & x))
#define O_RX(x) ((S_IROTH & x) &&  (S_IXOTH & x))
#define G_RX(x) ((S_IRGRP & x) &&  (S_IXGRP & x))
#define U_RX(x) ((S_IRUSR & x) &&  (S_IXUSR & x))
#define O_RW(x) ((S_IROTH & x) && (S_IWOTH & x))
#define G_RW(x) ((S_IRGRP & x) && (S_IWGRP & x))
#define U_RW(x) ((S_IRUSR & x) && (S_IWUSR & x))
#define O_RX(x) ((S_IROTH & x) && (S_IXOTH & x))
#define G_RX(x) ((S_IRGRP & x) && (S_IXGRP & x))
#define U_RX(x) ((S_IRUSR & x) && (S_IXUSR & x))
#define O_R(x) (S_IROTH & x)
#define G_R(x) (S_IRGRP & x)
#define U_R(x) (S_IRUSR & x)
#define MY_FILE(x) (x == geteuid())
#define MY_GROUP(x) (x == getegid())


static gint compare_by_name (const void *, const void *);

xfdir_local_c::xfdir_local_c(const gchar *data, gtk_c *data_gtk_c): 
    xfdir_c(data, data_gtk_c), local_file_info_c(data_gtk_c)
{
    treemodel = mk_tree_model();
}

gchar *
xfdir_local_c::make_tooltip_text (GtkTreePath *tpath) {
    if (!tpath) return g_strdup("tpath is NULL\n");

    gchar *text = get_tooltip_text(tpath);
    if (text) return text;

    gchar *file_path = get_verbatim_name(tpath);
    if (!file_path) return g_strdup("file_path is NULL\n");
    
    gchar *mimetype;
    GtkTreeIter iter;
    gtk_tree_model_get_iter (treemodel, &iter, tpath);
    gtk_tree_model_get (treemodel, &iter, 
            COL_MIMETYPE, &mimetype, -1);

    text = get_path_info(file_path, mimetype);  
    set_tooltip_text(tpath, text);
    g_free(mimetype);
    g_free(file_path);
    return text;
}
    
GtkTreeModel *
xfdir_local_c::mk_tree_model (void)
{
    if (!path || !g_file_test(path, G_FILE_TEST_EXISTS)) {
        fprintf(stderr, "%s does not exist\n", path);
        return NULL;
    }
    if (chdir(path)<0){
        fprintf(stderr, "chdir(%s): %s\n", path, strerror(errno));
        return NULL;
    }
    path = g_get_current_dir();

    
    GtkListStore *list_store = gtk_list_store_new (NUM_COLS, 
	    GDK_TYPE_PIXBUF, // icon in display
	    GDK_TYPE_PIXBUF, // normal icon reference
	    GDK_TYPE_PIXBUF, // highlight icon reference
	    GDK_TYPE_PIXBUF, // preview, tooltip image (cache)
	    G_TYPE_STRING,   // name in display (UTF-8)
	    G_TYPE_STRING,   // name from filesystem (verbatim)
	    G_TYPE_STRING,   // tooltip text (cache)
	    G_TYPE_STRING,   // icon identifier (name or composite key)
	    G_TYPE_INT,      // mode (to identify directories)
	    G_TYPE_STRING,   // mimetype (further identification of files)
	    G_TYPE_POINTER,  // stat record or NULL
	    G_TYPE_STRING,   // Preview path
	    G_TYPE_INT,      // Preview time
	    GDK_TYPE_PIXBUF); // Preview pixbuf

    heartbeat = 0;
    GList *directory_list = read_items (&heartbeat);
    insert_list_into_model(directory_list, list_store);

    return GTK_TREE_MODEL (list_store);
}

GList *
xfdir_local_c::read_items (gint *heartbeat) {
    GList *directory_list = NULL;
    if (chdir(path)){
	g_warning("xfdir_local_c::read_items(): chdir %s: %s\n", path, strerror(errno));
        return NULL;
    }
    //fprintf(stderr, "readfiles: %s\n", path);
    DIR *directory = opendir(path);
    if (!directory) {
	g_warning("xfdir_local_c::read_items(): opendir %s: %s\n", path, strerror(errno));
	return NULL;
    }

// http://womble.decadent.org.uk/readdir_r-advisory.html

#if 0
// this crashes on gvfs mount points....
//        bug reported by Liviu
#if defined(HAVE_FPATHCONF) && defined(HAVE_DIRFD)
    size_t size = offsetof(struct dirent, d_name) + 
	fpathconf(dirfd(directory), _PC_NAME_MAX) + 1;
#else
    size_t size = offsetof(struct dirent, d_name) +
	pathconf(xfdir_p->en->path, _PC_NAME_MAX) + 1;
#endif
#else
    // this should be more than enough
    size_t size = 256*256;
#endif

    struct dirent *buffer = (struct dirent *)calloc(1,size);
    if (!buffer) {
        fprintf(stderr,"calloc: %s\n", strerror(errno));
        return NULL;
    }

    gint error;
    struct dirent *d;
    while ((error = readdir_r(directory, buffer, &d)) == 0 && d != NULL){
        if(strcmp (d->d_name, ".") == 0) continue;
	xd_t *xd_p = (xd_t *)calloc(1,sizeof(xd_t));
	xd_p->d_name = g_strdup(d->d_name);
        xd_p->mimetype = NULL;
        memset (&(xd_p->st), 0, sizeof(struct stat));
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	xd_p->d_type = d->d_type;
#else
	xd_p->d_type = 0;
#endif
	directory_list = g_list_prepend(directory_list, xd_p);
	if (heartbeat) {
	    (*heartbeat)++;
	    NOOP("incrementing heartbeat records to %d\n", *heartbeat);
	}
    }
    if (error) {
        fprintf(stderr, "read_files_local: %s\n", strerror(errno));
    }

    closedir (directory);

    g_free(buffer);

    // At least the ../ record should have been read. If this
    // is not so, then a read error occurred.
    // (not uncommon in bluetoothed obexfs)
    if (!directory_list) {
	NOOP("read_files_local(): Count failed! Directory not read!\n");
    }
    directory_list = sort_directory_list (directory_list);
    return (directory_list);
}

GList *
xfdir_local_c::sort_directory_list(GList *list){
    // FIXME: get sort order and type
    gboolean do_stat = (g_list_length(list) <= MAX_AUTO_STAT);

    if (do_stat){
        GList *l;
        for (l=list; l && l->data; l=l->next){
            xd_t *xd_p = (xd_t *)l->data;
            memset(&(xd_p->st), 0, sizeof(struct stat));
            if (stat(xd_p->d_name, &(xd_p->st))){
                DBG("xfdir_local_c::sort_directory_list: cannot stat %s (%s)\n", 
                        xd_p->d_name, strerror(errno));
                continue;
            } 
            xd_p->mimetype = gtk_p->mime_type(xd_p->d_name, &(xd_p->st)); // using stat obtained above
        }
    }
    // Default sort order:
    return g_list_sort (list,compare_by_name);
}

void 
xfdir_local_c::reload(const gchar *data){
    if (!data || !g_file_test(data, G_FILE_TEST_EXISTS)) {
        fprintf(stderr, "%s does not exist\n", data);
        return;
    }
    if (chdir(data)<0){
        fprintf(stderr, "chdir(%s): %s\n", data, strerror(errno));
        return;
    }
    g_free(path);
    path = g_get_current_dir();
    DBG("current dir is %s\n", path);
    gtk_list_store_clear (GTK_LIST_STORE(treemodel));

    
    heartbeat = 0;
    GList *directory_list = read_items (&heartbeat);
    insert_list_into_model(directory_list, GTK_LIST_STORE(treemodel));

    
}
   

void
xfdir_local_c::insert_list_into_model(GList *data, GtkListStore *list_store){
    GdkPixbuf *p_file, *p_image, *p_dir;
    GtkTreeIter iter;

    GList *directory_list = (GList *)data;
    dir_count = g_list_length(directory_list);
    GList *l = directory_list;
    for (; l && l->data; l= l->next){
	xd_t *xd_p = (xd_t *)l->data;
        gtk_list_store_append (list_store, &iter);
	GdkPixbuf *p=p_file;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	if (xd_p->d_type == DT_DIR) p=p_dir;
#endif
	gchar *utf_name = utf_string(xd_p->d_name);
	gchar *icon_name = get_iconname(xd_p);
        gchar *mimetype = (xd_p->mimetype)?
            g_strdup(xd_p->mimetype):
            gtk_p->mime_type(xd_p->d_name); // plain extension mimetype
        
        // chop file extension (will now appear on the icon). (XXX only for big icons)
        if (S_ISREG(xd_p->st.st_mode) && !S_ISLNK(xd_p->st.st_mode)) {
            gchar *t = g_strdup(xd_p->d_name);
            if (strchr(t, '.') && strrchr(t, '.') != t){
                *strrchr(t, '.') = 0;
                g_free(utf_name);
                utf_name = utf_string(t);
                g_free(t);
            }
        }
	gchar *highlight_name;
        if (S_ISDIR(xd_p->st.st_mode)){
            if (strcmp(xd_p->d_name, "..")==0) {
                highlight_name = g_strdup("go-up");
            } else highlight_name = g_strdup("document-open");
        } else {
            gchar *h_name = get_iconname(xd_p, FALSE);
            if (U_RX(xd_p->st.st_mode)) {
                highlight_name = 
                    g_strdup_printf("%s/NE/emblem-run/2.0/220", h_name);
            } else {
                highlight_name = 
                    g_strdup_printf("%s/NE/document-open/2.0/220", h_name);
            }
            g_free(h_name);
        }
       
        GdkPixbuf *normal_pixbuf = gtk_p->get_pixbuf(icon_name,  get_icon_size(xd_p->d_name));
        //GdkPixbuf *highlight_pixbuf = gtk_p->get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);
        GdkPixbuf *highlight_pixbuf = gtk_p->get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);
        gtk_list_store_set (list_store, &iter, 
		DISPLAY_NAME, utf_name,
		ACTUAL_NAME, xd_p->d_name,
		ICON_NAME, icon_name,
                DISPLAY_PIXBUF, normal_pixbuf, 
                NORMAL_PIXBUF, normal_pixbuf, 
                HIGHLIGHT_PIXBUF, highlight_pixbuf, 
		COL_MODE,xd_p->st.st_mode, 
                COL_MIMETYPE, mimetype,
		-1);
	g_free(icon_name);
	g_free(highlight_name);
	g_free(utf_name);
        g_free(mimetype);
    }
    GList *p = directory_list;
    for (;p && p->data; p=p->next){
	xd_t *xd_p = (xd_t *)p->data;
        g_free(xd_p->d_name);
        g_free(xd_p);
    }
    g_list_free(directory_list);
}

gchar *
xfdir_local_c::get_emblem_string(xd_t *xd_p){
    return get_emblem_string(xd_p, TRUE);
}

gchar *
xfdir_local_c::get_emblem_string(xd_t *xd_p, gboolean use_lite){
    gchar *emblem = g_strdup("");
    // No emblem for go up
    if (strcmp(xd_p->d_name, "..")==0) return emblem;
    gchar *g;
    // Symlinks:
    if (S_ISLNK(xd_p->st.st_mode) || xd_p->d_type == DT_LNK) {
        if (xd_p->d_name[0] == '.') {
            g = g_strconcat(emblem, "#888888", NULL); 
            g_free(emblem); 
            emblem = g;
        }
        g = g_strconcat(emblem, "/SW/emblem-symbolic-link/2.0/220", NULL);
        g_free(emblem);
        emblem = g;
    }

    if (S_ISDIR(xd_p->st.st_mode) || xd_p->d_type == DT_DIR){
        if (xd_p->d_name[0] == '.') {
            g = g_strconcat(emblem, "#888888", NULL); 
            g_free(emblem); 
            emblem = g;
        }
        // all access:
        if (O_ALL(xd_p->st.st_mode)){
            g = g_strconcat(emblem, "/C/face-surprise/2.0/180", NULL);
        }
        else if ((MY_GROUP(xd_p->st.st_gid) && G_ALL(xd_p->st.st_mode)) 
                || (MY_FILE(xd_p->st.st_uid) && U_ALL(xd_p->st.st_mode))){
            g = g_strdup(emblem);
	}
        // read only:
        else if (O_RX(xd_p->st.st_mode) 
                || (MY_GROUP(xd_p->st.st_gid) && G_RX(xd_p->st.st_mode)) 
                || (MY_FILE(xd_p->st.st_uid) && U_RX(xd_p->st.st_mode))){
            g = g_strconcat(emblem, "/C/emblem-readonly/3.0/180", NULL);
        }
        else {
            // no access:
            g = g_strconcat(emblem, "/C/face-angry/3.0/180", NULL);
        }
        g_free(emblem); 
        emblem = g;
    }
    

    else if (S_ISREG(xd_p->st.st_mode) || xd_p->d_type == DT_REG){
        guchar red;
        guchar green;
        guchar blue;
        gchar *colors = g_strdup("");
        if (xd_p->d_name[0] == '.') {
            g = g_strconcat(emblem, "#888888", NULL); 
            g_free(emblem); 
            emblem = g;
        } else if (gtk_p->get_lite_colors(xd_p->mimetype, &red, &green, &blue)){
            g_free(colors);
            colors = g_strdup_printf("#%02x%02x%02x", red, green, blue);
        }
        gchar *extension = g_strdup("");
        if (strchr(xd_p->d_name, '.') && strchr(xd_p->d_name, '.') != xd_p->d_name) {
            extension = g_strconcat("*", strrchr(xd_p->d_name, '.')+1, NULL) ;
        }
        // all access:
        if (O_ALL(xd_p->st.st_mode) || O_RW(xd_p->st.st_mode)){
                g = g_strdup_printf("%s%s%s/C/face-surprise/2.0/180/NW/emblem-exec/3.0/180",
                        extension, colors, emblem);
	// read/write/exec
        } else if((MY_GROUP(xd_p->st.st_gid) && G_ALL(xd_p->st.st_mode)) 
                || (MY_FILE(xd_p->st.st_uid) && U_ALL(xd_p->st.st_mode))){
                g = g_strdup_printf("%s%s%s/NW/emblem-exec/3.0/180", 
                        extension, colors, emblem);
	// read/exec
        } else if (O_RX(xd_p->st.st_mode)
		||(MY_GROUP(xd_p->st.st_gid) && G_RX(xd_p->st.st_mode)) 
                || (MY_FILE(xd_p->st.st_uid) && U_RX(xd_p->st.st_mode))){
                g = g_strdup_printf("%s%s%s/NW/emblem-exec/3.0/180", 
                        extension, colors, emblem);

	// read/write
        } else if ((MY_GROUP(xd_p->st.st_gid) && G_RW(xd_p->st.st_mode))
                || (MY_FILE(xd_p->st.st_uid) && U_RW(xd_p->st.st_mode))) {
                g = g_strdup_printf("%s%s%s", 
                        extension, colors, emblem);

        // read only:
        } else if (O_R(xd_p->st.st_mode) 
                || (MY_GROUP(xd_p->st.st_gid) && G_R(xd_p->st.st_mode)) 
                || (MY_FILE(xd_p->st.st_uid) && U_R(xd_p->st.st_mode))){
                g = g_strdup_printf("%s%s%s/NW/emblem-readonly/3.0/130", 
                        extension, colors, emblem);
        } else if (S_ISREG(xd_p->st.st_mode)) {
            // no access: (must be have stat info to get this emblem)
            g = g_strdup_printf("%s%s%s/NW/emblem-unreadable/3.0/180/C/face-angry/2.0/180", 
                    extension, colors, emblem);
        } else {
            g = g_strdup_printf("%s%s", 
                    extension, colors);
        }
        g_free(extension);
        g_free(emblem); 
        emblem = g;
        if (use_lite) {
            const gchar *lite_emblem = gtk_p->get_lite_emblem(xd_p->mimetype);
            if (lite_emblem){
                g = g_strconcat(emblem, "/NE/", lite_emblem, "/1.8/200", NULL); 
                g_free(emblem); 
                emblem = g;
            }
        } 
    }
    return emblem;
}


gchar *
xfdir_local_c::get_basic_iconname(xd_t *xd_p){

    // Directories:
    if (strcmp(xd_p->d_name, "..")==0) return  g_strdup("go-up");
    if (S_ISDIR(xd_p->st.st_mode) || xd_p->d_type == DT_DIR) {
	if (strcmp(path, g_get_home_dir())==0) {
            return get_home_iconname(xd_p->d_name);
	}
    /*    if (xd_p->d_type == DT_LNK) {
	    return  g_strdup("folder/SW/emblem-symbolic-link/2.0/220");
        } else {
            struct stat lst;
            if (lstat(xd_p->d_name, &lst)==0){
                if (S_ISLNK(lst.st_mode)){
	            return  g_strdup("folder/SW/emblem-symbolic-link/2.0/220");
                }
            }
        }*/
	return  g_strdup("folder");
    }

    // Symlinks:
/*    if (S_ISLNK(xd_p->st.st_mode)|| xd_p->d_type == xd_p->d_type == DT_LNK) {
	return  g_strdup("text-x-generic-template/SW/emblem-symbolic-link/2.0/220");
    }
*/
    // Character device:
    if (S_ISCHR(xd_p->st.st_mode) || xd_p->d_type == DT_CHR) {
	return  g_strdup("text-x-generic-template/SW/emblem-chardevice/2.0/220");
    }
    // Named pipe (FIFO):
    if (S_ISFIFO(xd_p->st.st_mode) || xd_p->d_type == DT_FIFO) {
	return  g_strdup("text-x-generic-template/SW/emblem-fifo/2.0/220");
    }
    // UNIX domain socket:
    if (S_ISSOCK(xd_p->st.st_mode) || xd_p->d_type == DT_SOCK) {
	return  g_strdup("text-x-generic-template/SW/emblem-socket/2.0/220");
    }
    // Block device
    if (S_ISBLK(xd_p->st.st_mode) || xd_p->d_type == DT_BLK) {
	return  g_strdup("text-x-generic-template/SW/emblem-blockdevice/2.0/220");
    }
    // Regular file:

    if (S_ISREG(xd_p->st.st_mode) || xd_p->d_type == DT_REG) {
        const gchar *basic = get_mime_iconname(xd_p);
        return g_strdup(basic);
    }

    // Unknown:
    if (xd_p->d_type == DT_UNKNOWN) {
	return  g_strdup("dialog-question");
    }
    return  g_strdup("text-x-generic");
}

const gchar *
xfdir_local_c::get_mime_iconname(xd_t *xd_p){
    const gchar *basic = "text-x-generic";

    if (xd_p->mimetype) {
        // here we should get generic-icon from mime-module.xml!
        const gchar *basic = gtk_p->get_mimetype_iconname(xd_p->mimetype);
        //DBG("xfdir_local_c::get_mime_iconname(%s) -> %s\n", xd_p->mimetype, basic);
        if (basic) {
            // check if the pixbuf is actually available
            GdkPixbuf *pixbuf = gtk_p->get_pixbuf(basic,  GTK_ICON_SIZE_DIALOG);
            if (pixbuf) return basic;
        } else {
	    if (strstr(xd_p->mimetype, "text/html")){
		return "text-html";
	    }
	}

/*
         "image-x-generic";
         "audio-x-generic";
         "font-x-generic";
         "package-x-generic";
         "video-x-generic";
         "x-office-address-book";
         "x-office-calendar";
         "x-office-document";
         "x-office-document-template";
         "x-office-drawing";
         "x-office-drawing-template";
         "x-office-presentation";
         "x-office-presentation-template";
         "x-office-spreadsheet";
         "x-office-spreadsheet-template";
         "text-html";
         "text-x-preview";
         "text-x-script";
         "application-x-executable";
         "application-certificate";
         "application-x-addon";
         "application-x-firmware";
         "x-package-repository";
*/
    }
    return  "text-x-generic";
}

gchar *
xfdir_local_c::get_iconname(xd_t *xd_p){
    return get_iconname(xd_p, TRUE);
}

gchar *
xfdir_local_c::get_iconname(xd_t *xd_p, gboolean use_lite){
    gchar *name = get_basic_iconname(xd_p);
    gchar *emblem = get_emblem_string(xd_p, use_lite);
    gchar *iconname = g_strconcat(name, emblem, NULL);
    g_free(name);
    g_free(emblem);
    return iconname;
}




gchar *
xfdir_local_c::get_home_iconname(const gchar *data){
    if (!data) return g_strdup("user-home");
    const gchar *dir[]={N_("Documents"), N_("Downloads"),N_("Music"),N_("Pictures"),
	        N_("Templates"),N_("Videos"),N_("Desktop"),N_("Bookmarks"),
		N_(".Trash"),NULL};
    const gchar *icon[]={"folder-documents", "folder-download","folder-music","folder-pictures",
	          "folder-templates","folder-videos","user-desktop","user-bookmarks",
		  "user-trash",NULL};
    const gchar **p, **i;
    for (p=dir, i=icon; p && *p ; p++, i++){
	if (strcasecmp(*p, data) == 0) {
	    return g_strdup(*i);
	}
    }
    return g_strdup("folder");
}

const gchar *
xfdir_local_c::get_xfdir_iconname(void){
    if (strcmp(path, g_get_home_dir())==0) {
	return "user-home";
    }
    gchar *d = g_path_get_dirname(path);
    if (strcmp(d, g_get_home_dir())==0) {
	g_free(d);
	gchar *b = g_path_get_basename(path);
	const gchar *iconname = get_home_iconname(b);
	g_free(b);
	return iconname;
    }
    g_free(d);
    return "folder";
}
/////////////////////////////////////////////////////////////////////

static gint
compare_by_name (const void *a, const void *b) {
    // compare by name, directories or symlinks to directories on top
    const xd_t *xd_a = (const xd_t *)a;
    const xd_t *xd_b = (const xd_t *)b;

    if (strcmp(xd_a->d_name, "..")==0) return -1;
    if (strcmp(xd_b->d_name, "..")==0) return 1;

    gboolean a_cond;
    gboolean b_cond;

    if (S_ISDIR(xd_a->st.st_mode) || S_ISDIR(xd_b->st.st_mode)) {
        a_cond = (S_ISDIR(xd_a->st.st_mode));
        b_cond = (S_ISDIR(xd_b->st.st_mode));
    } 
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    else {
        a_cond = (xd_a->d_type == DT_DIR);
        b_cond = (xd_b->d_type == DT_DIR);
    }
#endif

    if (a_cond && !b_cond) return -1; 
    if (!a_cond && b_cond) return 1;
    return strcasecmp(xd_a->d_name, xd_b->d_name);
}


