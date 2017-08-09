#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <gio/gio.h>

#include "local_monitor_c.hpp"
void
monitor_f (GFileMonitor *, GFile *, GFile *, GFileMonitorEvent, gpointer);
static gboolean 
rm_func (GtkTreeModel *, GtkTreePath *, GtkTreeIter *, gpointer);
static gboolean 
stat_func (GtkTreeModel *, GtkTreePath *, GtkTreeIter *, gpointer);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

local_monitor_c::local_monitor_c( const gchar *data): xfdir_c(data), store(NULL){
    items_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    cancellable = g_cancellable_new ();
    gfile = g_file_new_for_path (data);
    monitor = NULL;
}

local_monitor_c::~local_monitor_c(void){
    NOOP("Destructor:~local_monitor_c()\n");
    stop_monitor();
    g_hash_table_destroy(items_hash);
    //g_cancellable_cancel (cancellable);
    //g_object_unref(cancellable);
    if (gfile) g_object_unref(gfile);
    if (monitor) g_object_unref(monitor);
}


GFile *
local_monitor_c::get_gfile(void){ return gfile;}

xd_t *
local_monitor_c::get_xd_p(struct dirent *d){
    xd_t *xd_p = (xd_t *)calloc(1,sizeof(xd_t));
    xd_p->d_name = g_strdup(d->d_name);
    xd_p->mimetype = NULL;
    xd_p->mimefile = NULL;
    xd_p->st = NULL;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    xd_p->d_type = d->d_type;
#else
    xd_p->d_type = 0;
#endif
    return xd_p;
}

xd_t *
local_monitor_c::get_xd_p(GFile *first){

    gchar *path = g_file_get_path(gfile);
    gchar *basename = g_file_get_basename(first);
    struct dirent *d; // static pointer
    NOOP("looking for %s info\n", basename);
    DIR *directory = opendir(path);
    xd_t *xd_p = NULL;
    if (directory) {
      while ((d = readdir(directory))  != NULL) {
        if(strcmp (d->d_name, basename)) continue;
        xd_p = get_xd_p(d);
        break;
      }
      closedir (directory);
    } else {
      g_warning("monitor_f(): opendir %s: %s\n", path, strerror(errno));
    }
    g_free(basename); 
    g_free(path); 
    return xd_p;
}

gboolean
local_monitor_c::add_new_item(GFile *file){
   xd_t *xd_p = get_xd_p(file);
    if (xd_p) {
        add_local_item(store, xd_p);
        free_xd_p(xd_p);
        return TRUE;
    } 
    return FALSE;
}

gboolean 
local_monitor_c::remove_item(GFile *file){
    // find the iter and remove item
    gchar *basename = g_file_get_basename(file);
    g_hash_table_remove(items_hash, basename); 
    gtk_tree_model_foreach (GTK_TREE_MODEL(store), rm_func, (gpointer) basename); 
    g_free(basename);
    return TRUE;
}

gboolean 
local_monitor_c::restat_item(GFile *src){
    gchar *basename = g_file_get_basename(src);
    if (!g_hash_table_lookup(items_hash, basename)) {
        g_free(basename);
        return FALSE; 
    }
    g_free(basename);
    gchar *fullpath = g_file_get_path(src);
    gtk_tree_model_foreach (GTK_TREE_MODEL(store), stat_func, (gpointer) fullpath); 
    g_free(fullpath);
    return TRUE;
}

void
local_monitor_c::free_xd_p(xd_t *xd_p){
    g_free(xd_p->mimefile);
    g_free(xd_p->mimetype);
    g_free(xd_p->d_name);
    g_free(xd_p);
}

GtkListStore*
local_monitor_c::get_liststore(void){ return store;}

void
local_monitor_c::start_monitor(const gchar *data, GtkTreeModel *data2){
    store = GTK_LIST_STORE(data2);
    fprintf(stderr, "*** start_monitor: %s\n", data);
    if (gfile) g_object_unref(gfile);
    gfile = g_file_new_for_path (data);
    GError *error=NULL;
    if (monitor) g_object_unref(monitor);
    monitor = g_file_monitor_directory (gfile, G_FILE_MONITOR_WATCH_MOVES, cancellable,&error);
    if (error){
        DBG("g_file_monitor_directory(%s) failed: %s\n",
                data, error->message);
        g_object_unref(gfile);
        gfile=NULL;
        return;
    }
    g_signal_connect (monitor, "changed", 
            G_CALLBACK (monitor_f), (void *)this);
}

void 
local_monitor_c::stop_monitor(void){
    gchar *p = g_file_get_path(gfile);
    fprintf(stderr, "*** stop_monitor at: %s\n", p);
    g_free(p);
    g_file_monitor_cancel(monitor);
    while (gtk_events_pending())gtk_main_iteration();  
    g_hash_table_remove_all(items_hash);
    // hash table remains alive (but empty) until destructor.
}

void
local_monitor_c::set_show_hidden(gboolean state){shows_hidden = state;}

void
local_monitor_c::add_local_item(GtkListStore *list_store, xd_t *xd_p){
    // if it already exists, do nothing
    if (g_hash_table_lookup(items_hash, (void *)xd_p->d_name)){    
        DBG("local_monitor_c::not re-adding %s\n", xd_p->d_name);
        return;
    }

    if (!shows_hidden && xd_p->d_name[0] == '.'  && strcmp("..", xd_p->d_name)){
	return;
    }
    
    GtkTreeIter iter;
    gtk_list_store_append (list_store, &iter);
    gchar *utf_name = utf_string(xd_p->d_name);
    // plain extension mimetype fallback
    if (!xd_p->mimetype) xd_p->mimetype = mime_c::mime_type(xd_p->d_name); 
    gchar *icon_name = get_iconname(xd_p);
    
    // chop file extension (will now appear on the icon). (XXX only for big icons)
    gboolean is_dir;
    gboolean is_reg_not_link;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    is_dir = (xd_p->d_type == DT_DIR);
    is_reg_not_link = (xd_p->d_type == DT_REG && !(xd_p->d_type == DT_LNK));
#else 
    is_dir = (xd_p->st && S_ISDIR(xd_p->st->st_mode));
    is_reg_not_link = (xd_p->st && S_ISREG(xd_p->st->st_mode) && !S_ISLNK(xd_p->st->st_mode));
#endif
    if (is_reg_not_link) {
        gchar *t = g_strdup(xd_p->d_name);
        if (strchr(t, '.') && strrchr(t, '.') != t){
            *strrchr(t, '.') = 0;
            g_free(utf_name);
            utf_name = utf_string(t);
            g_free(t);
        }
    }
    gchar *highlight_name;
    if (is_dir){
        if (strcmp(xd_p->d_name, "..")==0) {
            highlight_name = g_strdup("go-up");
        } else highlight_name = g_strdup("document-open");
    } else {
        gchar *h_name = get_iconname(xd_p, FALSE);
        if (xd_p->st && U_RX(xd_p->st->st_mode)) {
            highlight_name = 
                g_strdup_printf("%s/NE/emblem-run/2.0/220", h_name);
        } else {
            highlight_name = 
                g_strdup_printf("%s/NE/document-open/2.0/220", h_name);
        }
        g_free(h_name);
    }
   
    GdkPixbuf *normal_pixbuf = pixbuf_c::get_pixbuf(icon_name,  get_icon_size(xd_p->d_name));
    //GdkPixbuf *highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);
    GdkPixbuf *highlight_pixbuf = pixbuf_c::get_pixbuf(highlight_name,  GTK_ICON_SIZE_DIALOG);
    gtk_list_store_set (list_store, &iter, 
            DISPLAY_NAME, utf_name,
            ACTUAL_NAME, xd_p->d_name,
            ICON_NAME, icon_name,
            DISPLAY_PIXBUF, normal_pixbuf, 
            NORMAL_PIXBUF, normal_pixbuf, 
            HIGHLIGHT_PIXBUF, highlight_pixbuf, 
            COL_TYPE,xd_p->d_type, 
            COL_STAT,xd_p->st, 
            COL_MIMETYPE, xd_p->mimetype,
            COL_MIMEFILE, xd_p->mimefile, // may be null here.
            -1);
    g_free(icon_name);
    g_free(highlight_name);
    g_free(utf_name);
    g_hash_table_replace(items_hash, g_strdup(xd_p->d_name), GINT_TO_POINTER(1));
}


gchar *
local_monitor_c::get_iconname(xd_t *xd_p){
    return get_iconname(xd_p, TRUE);
}

gchar *
local_monitor_c::get_iconname(xd_t *xd_p, gboolean use_lite){
    gchar *name = get_basic_iconname(xd_p);
    gchar *emblem = get_emblem_string(xd_p, use_lite);
    gchar *iconname = g_strconcat(name, emblem, NULL);
    g_free(name);
    g_free(emblem);
    return iconname;
}


gchar *
local_monitor_c::get_basic_iconname(xd_t *xd_p){

    // Directories:
    if (strcmp(xd_p->d_name, "..")==0) return  g_strdup("go-up");
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    if (xd_p->d_type == DT_DIR) {
        gchar *path = g_file_get_path(gfile);
	if (strcmp(path, g_get_home_dir())==0) {
            g_free(path);
            return get_home_iconname(xd_p->d_name);
            
	}
        g_free(path);
	return  g_strdup("folder");
    }

    // Symlinks:
/*    if (xd_p->d_type == xd_p->d_type == DT_LNK) {
	return  g_strdup("text-x-generic-template/SW/emblem-symbolic-link/2.0/220");
    }
*/
    // Character device:
    if (xd_p->d_type == DT_CHR ) {
	return  g_strdup("text-x-generic-template/SW/emblem-chardevice/2.0/220");
    }
    // Named pipe (FIFO):
    if (xd_p->d_type == DT_FIFO ) {
	return  g_strdup("text-x-generic-template/SW/emblem-fifo/2.0/220");
    }
    // UNIX domain socket:
    if (xd_p->d_type == DT_SOCK ) {
	return  g_strdup("text-x-generic-template/SW/emblem-socket/2.0/220");
    }
    // Block device
    if (xd_p->d_type == DT_BLK ) {
	return  g_strdup("text-x-generic-template/SW/emblem-blockdevice/2.0/220");
    }
    // Regular file:

    if (xd_p->d_type == DT_REG ) {
        const gchar *basic = get_mime_iconname(xd_p);
        return g_strdup(basic);
    }

    // Unknown:
    if (xd_p->d_type == DT_UNKNOWN) {
	return  g_strdup("dialog-question");
    }
#else
    if ((xd_p->st && S_ISDIR(xd_p->st->st_mode))) {
        gchar *path = g_file_get_path(gfile);
	if (strcmp(path, g_get_home_dir())==0) {
            g_free(path);
            return get_home_iconname(xd_p->d_name);
	}
        g_free(path);
	return  g_strdup("folder");
    }

    // Symlinks:
/*    if (xd_p->st && xd_p->d_type == xd_p->d_type == DT_LNK) {
	return  g_strdup("text-x-generic-template/SW/emblem-symbolic-link/2.0/220");
    }
*/
    // Character device:
    if ((xd_p->st && S_ISCHR(xd_p->st->st_mode))) {
	return  g_strdup("text-x-generic-template/SW/emblem-chardevice/2.0/220");
    }
    // Named pipe (FIFO):
    if ((xd_p->st && S_ISFIFO(xd_p->st->st_mode))) {
	return  g_strdup("text-x-generic-template/SW/emblem-fifo/2.0/220");
    }
    // UNIX domain socket:
    if ((xd_p->st && S_ISSOCK(xd_p->st->st_mode))) {
	return  g_strdup("text-x-generic-template/SW/emblem-socket/2.0/220");
    }
    // Block device
    if ((xd_p->st && S_ISBLK(xd_p->st->st_mode))) {
	return  g_strdup("text-x-generic-template/SW/emblem-blockdevice/2.0/220");
    }
    // Regular file:

    if ((xd_p->st && S_ISREG(xd_p->st->st_mode))) {
        const gchar *basic = get_mime_iconname(xd_p);
        return g_strdup(basic);
    }
#endif
    return  g_strdup("text-x-generic");
}


gchar *
local_monitor_c::get_emblem_string(xd_t *xd_p){
    return get_emblem_string(xd_p, TRUE);
}

gchar *
local_monitor_c::get_emblem_string(xd_t *xd_p, gboolean use_lite){
    gchar *emblem = g_strdup("");
    // No emblem for go up
    if (strcmp(xd_p->d_name, "..")==0) return emblem;
    gchar *g;
    gboolean is_dir;
    gboolean is_lnk;
    gboolean is_reg;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    is_dir = (xd_p->d_type == DT_DIR);
    is_lnk = (xd_p->d_type == DT_LNK);
    is_reg = (xd_p->d_type == DT_REG);
#else 
    is_dir = (xd_p->st && S_ISDIR(xd_p->st->st_mode));
    is_reg = (xd_p->st && S_ISREG(xd_p->st->st_mode));
    is_lnk = (xd_p->st && S_ISLNK(xd_p->st->st_mode));
#endif
    
    // Symlinks:
    if (is_lnk) {
        if (xd_p->d_name[0] == '.') {
            g = g_strconcat(emblem, "#888888", NULL); 
            g_free(emblem); 
            emblem = g;
        }
        g = g_strconcat(emblem, "/SW/emblem-symbolic-link/2.0/220", NULL);
        g_free(emblem);
        emblem = g;
    }
    if (is_dir && xd_p->d_name[0] == '.') {
	g = g_strconcat(emblem, "#888888", NULL); 
	g_free(emblem); 
	emblem = g;
    }
    if (is_dir){
	if (!xd_p->st){
	    g = g_strdup(emblem);
	}
        // all access:
        else if (xd_p->st && O_ALL(xd_p->st->st_mode)){
            g = g_strconcat(emblem, "/C/face-surprise/2.0/180", NULL);
        }
        else if ((MY_GROUP(xd_p->st->st_gid) && G_ALL(xd_p->st->st_mode)) 
                || (MY_FILE(xd_p->st->st_uid) && U_ALL(xd_p->st->st_mode))){
            g = g_strdup(emblem);
	}
        // read only:
        else if (O_RX(xd_p->st->st_mode) 
                || (MY_GROUP(xd_p->st->st_gid) && G_RX(xd_p->st->st_mode)) 
                || (MY_FILE(xd_p->st->st_uid) && U_RX(xd_p->st->st_mode))){
            g = g_strconcat(emblem, "/C/emblem-readonly/3.0/180", NULL);
        }
        else {
            // no access:
            g = g_strconcat(emblem, "/C/face-angry/3.0/180", NULL);
        }
        g_free(emblem); 
        emblem = g;
    }
    
    else if (is_reg){
        guchar red;
        guchar green;
        guchar blue;
        gchar *colors = g_strdup("");
        if (xd_p->d_name[0] == '.') {
            g = g_strconcat(emblem, "#888888", NULL); 
            g_free(emblem); 
            emblem = g;
        } else if (lite_c::get_lite_colors(xd_p->mimetype, &red, &green, &blue)){
            g_free(colors);
            colors = g_strdup_printf("#%02x%02x%02x", red, green, blue);
        }
        gchar *extension = g_strdup("");
        if (strrchr(xd_p->d_name, '.') && strrchr(xd_p->d_name, '.') != xd_p->d_name) {
            extension = g_strconcat("*", strrchr(xd_p->d_name, '.')+1, NULL) ;
        }
	if (!xd_p->st) {
            g = g_strdup_printf("%s%s", 
                    extension, colors);
        }
        // all access:
	else if (O_ALL(xd_p->st->st_mode) || O_RW(xd_p->st->st_mode)){
                g = g_strdup_printf("%s%s%s/C/face-surprise/2.0/180/NW/emblem-exec/3.0/180",
                        extension, colors, emblem);
	// read/write/exec
        } else if((MY_GROUP(xd_p->st->st_gid) && G_ALL(xd_p->st->st_mode)) 
                || (MY_FILE(xd_p->st->st_uid) && U_ALL(xd_p->st->st_mode))){
                g = g_strdup_printf("%s%s%s/NW/emblem-exec/3.0/180", 
                        extension, colors, emblem);
	// read/exec
        } else if (O_RX(xd_p->st->st_mode)
		||(MY_GROUP(xd_p->st->st_gid) && G_RX(xd_p->st->st_mode)) 
                || (MY_FILE(xd_p->st->st_uid) && U_RX(xd_p->st->st_mode))){
                g = g_strdup_printf("%s%s%s/NW/emblem-exec/3.0/180", 
                        extension, colors, emblem);

	// read/write
        } else if ((MY_GROUP(xd_p->st->st_gid) && G_RW(xd_p->st->st_mode))
                || (MY_FILE(xd_p->st->st_uid) && U_RW(xd_p->st->st_mode))) {
                g = g_strdup_printf("%s%s%s", 
                        extension, colors, emblem);

        // read only:
        } else if (O_R(xd_p->st->st_mode) 
                || (MY_GROUP(xd_p->st->st_gid) && G_R(xd_p->st->st_mode)) 
                || (MY_FILE(xd_p->st->st_uid) && U_R(xd_p->st->st_mode))){
                g = g_strdup_printf("%s%s%s/NW/emblem-readonly/3.0/130", 
                        extension, colors, emblem);
        } else if (S_ISREG(xd_p->st->st_mode)) {
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
            const gchar *lite_emblem = lite_c::get_lite_emblem(xd_p->mimetype);
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
local_monitor_c::get_home_iconname(const gchar *data){
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
local_monitor_c::get_mime_iconname(xd_t *xd_p){
    const gchar *basic = "text-x-generic";

    if (xd_p->mimetype) {
        // here we should get generic-icon from mime-module.xml!
        const gchar *basic = mime_c::get_mimetype_iconname(xd_p->mimetype);
        //DBG("xfdir_local_c::get_mime_iconname(%s) -> %s\n", xd_p->mimetype, basic);
        if (basic) {
            // check if the pixbuf is actually available
            GdkPixbuf *pixbuf = pixbuf_c::get_pixbuf(basic,  GTK_ICON_SIZE_DIALOG);
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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


static gboolean stat_func (GtkTreeModel *model,
                            GtkTreePath *path,
                            GtkTreeIter *iter,
                            gpointer data){
    struct stat *st=NULL;
    gchar *text;
    gchar *basename = g_path_get_basename((gchar *)data);
    gtk_tree_model_get (model, iter, 
            COL_ACTUAL_NAME, &text, 
            COL_STAT, &st, 
            -1);  
    
    if (strcmp(basename, text)){
        g_free(text);
        g_free(basename);
        return FALSE;
    }
    g_free(text);
    g_free(basename);
    g_free(st);

    GtkListStore *store = GTK_LIST_STORE(model);
    st = (struct stat *)calloc(1, sizeof(struct stat));
    if (stat((gchar *)data, st) != 0){
        fprintf(stderr, "stat: %s\n", strerror(errno));
        return FALSE;
    }

    gtk_list_store_set (store, iter, 
            COL_STAT,st, 
            -1);

    return TRUE;
}

static gboolean rm_func (GtkTreeModel *model,
                            GtkTreePath *path,
                            GtkTreeIter *iter,
                            gpointer data){
    gchar *text;
    struct stat *st_p=NULL;
    gtk_tree_model_get (model, iter, 
            COL_STAT, &st_p, 
            COL_ACTUAL_NAME, &text, 
            -1);  
    
    if (strcmp(text, (gchar *)data)){
        g_free(text);
        return FALSE;
    }
    DBG("removing %s from treemodel.\n", text);
    GtkListStore *store = GTK_LIST_STORE(model);

//  free stat record, if any
    g_free(st_p);

    gtk_list_store_remove(store, iter);
    g_free(text);
    return TRUE;
}


void
monitor_f (GFileMonitor      *mon,
          GFile             *first,
          GFile             *second,
          GFileMonitorEvent  event,
          gpointer           data)
{
    gchar *f= first? g_file_get_basename (first):g_strdup("--");
    gchar *s= second? g_file_get_basename (second):g_strdup("--");
   

    fprintf(stderr, "*** monitor_f call...\n");
    local_monitor_c *p = (local_monitor_c *)data;

    switch (event){
        case G_FILE_MONITOR_EVENT_DELETED:
        case G_FILE_MONITOR_EVENT_MOVED_OUT:
            fprintf(stderr,"Received DELETED  (%d): \"%s\", \"%s\"\n", event, f, s);
            p->remove_item(first);
            break;
        case G_FILE_MONITOR_EVENT_CREATED:
        case G_FILE_MONITOR_EVENT_MOVED_IN:
            fprintf(stderr,"Received  CREATED (%d): \"%s\", \"%s\"\n", event, f, s);
            p->add_new_item(first);
            break;
        case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
           fprintf(stderr,"Received  CHANGES_DONE_HINT (%d): \"%s\", \"%s\"\n", event, f, s);
            p->restat_item(first);
            // if image, then reload the pixbuf
            break;
        case G_FILE_MONITOR_EVENT_CHANGED:
            fprintf(stderr,"Received  CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
            break;
        case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
            fprintf(stderr,"Received  ATTRIBUTE_CHANGED (%d): \"%s\", \"%s\"\n", event, f, s);
            p->restat_item(first);
            break;
        case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
            fprintf(stderr,"Received  PRE_UNMOUNT (%d): \"%s\", \"%s\"\n", event, f, s);
            break;
        case G_FILE_MONITOR_EVENT_UNMOUNTED:
            fprintf(stderr,"Received  UNMOUNTED (%d): \"%s\", \"%s\"\n", event, f, s);
            break;
        case G_FILE_MONITOR_EVENT_MOVED:
        case G_FILE_MONITOR_EVENT_RENAMED:
            fprintf(stderr,"Received  MOVED (%d): \"%s\", \"%s\"\n", event, f, s);
            p->remove_item(first);
            p->add_new_item(second);
            break;
    }
    g_free(f);
    g_free(s);
}


