#include <dirent.h>
#include "debug.h"

#include "xfdir_c.hpp"
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdlib.h>



xfdir_c::xfdir_c(const gchar *data){
    path = g_strdup(data);
    treemodel = mk_tree_model();
}

xfdir_c::~xfdir_c(void){
    g_free(path);
    g_object_unref(treemodel);
}

static gint
compare_type (const void *a, const void *b) {
    const xd_t *xd_a = (const xd_t *)a;
    const xd_t *xd_b = (const xd_t *)b;

    if (strcmp(xd_a->d_name, "..")==0) return -1;
    if (strcmp(xd_b->d_name, "..")==0) return 1;

    gboolean a_cond = (xd_a->d_type == DT_DIR);
    gboolean b_cond = (xd_b->d_type == DT_DIR);
    if (a_cond && !b_cond) return -1; 
    if (!a_cond && b_cond) return 1;
    return strcasecmp(xd_a->d_name, xd_b->d_name);
    //return strcmp(xd_a->d_name, xd_b->d_name);
}

void 
xfdir_c::reload(const gchar *data){
    if (!data || !g_file_test(data, G_FILE_TEST_EXISTS)) {
        fprintf(stderr, "%s does not exist\n", data);
        return;
    }
    chdir(data);
    g_free(path);
    path = g_get_current_dir();
    gtk_list_store_clear (GTK_LIST_STORE(treemodel));

    
    heartbeat = 0;
    GList *directory_list = read_items (&heartbeat);
    insert_list_into_model(directory_list, GTK_LIST_STORE(treemodel));

    
}

GList *
xfdir_c::read_items (gint *heartbeat) {
    GList *directory_list = NULL;
    //fprintf(stderr, "readfiles: %s\n", path);
    DIR *directory = opendir(path);
    if (!directory) {
	fprintf(stderr, "read_files_local(): Cannot open %s\n", path);
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
        // if(strcmp (d->d_name, "..") == 0 && strcmp (path, "/") == 0) continue;
	xd_t *xd_p = (xd_t *)calloc(1,sizeof(xd_t));
	xd_p->d_name = g_strdup(d->d_name);
        memset (&(xd_p->st), 0, sizeof(struct stat));
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	xd_p->d_type = d->d_type;
#else
FIXME set d_type from a stat or other method
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
    //if (directory_list) directory_list = g_list_reverse(directory_list);

    // At least the ../ record should have been read. If this
    // is not so, then a read error occurred.
    // (not uncommon in bluetoothed obexfs)
    if (!directory_list) {
	NOOP("read_files_local(): Count failed! Directory not read!\n");
    }
    directory_list = g_list_sort (directory_list,compare_type);
    return (directory_list);
}

const gchar *
xfdir_c::get_stat_pixbuf(xd_t *xd_p, gboolean restat){
    if (strcmp(xd_p->d_name, "..")==0) return  "go-up";

    if (restat){
        lstat(xd_p->d_name, &(xd_p->st));
    }
#define O_ALL(x) ((S_IROTH & x) && (S_IWOTH & x) &&  (S_IXOTH & x))
#define G_ALL(x) ((S_IRGRP & x) && (S_IWGRP & x) &&  (S_IXGRP & x))
#define U_ALL(x) ((S_IRUSR & x) && (S_IWUSR & x) &&  (S_IXUSR & x))
#define O_RX(x) ((S_IROTH & x) &&  (S_IXOTH & x))
#define G_RX(x) ((S_IRGRP & x) &&  (S_IXGRP & x))
#define U_RX(x) ((S_IRUSR & x) &&  (S_IXUSR & x))
#define O_RW(x) ((S_IROTH & x) && (S_IWOTH & x))
#define G_RW(x) ((S_IRGRP & x) && (S_IWGRP & x))
#define U_RW(x) ((S_IRUSR & x) && (S_IWUSR & x))
#define O_R(x) (S_IROTH & x)
#define G_R(x) (S_IRGRP & x)
#define U_R(x) (S_IRUSR & x)
#define MY_FILE(x) (x == geteuid())
#define MY_GROUP(x) (x == getegid())



    if (S_ISDIR(xd_p->st.st_mode)){
        gboolean my_file = (xd_p->st.st_uid == geteuid());
        gboolean my_group = (xd_p->st.st_gid == getgid());
        // all access:
        if (O_ALL(xd_p->st.st_mode))
                return "folder/C/face-surprise/2.0/180/SE/emblem-write-ok/2.0/180";
        if ((MY_GROUP(xd_p->st.st_gid) && G_ALL(xd_p->st.st_mode)) 
                || (MY_FILE(xd_p->st.st_uid) && U_ALL(xd_p->st.st_mode)))
                return "folder/SE/emblem-write-ok/2.0/180";
        // read only:
        if (O_RX(xd_p->st.st_mode) 
                || (MY_GROUP(xd_p->st.st_gid) && G_RX(xd_p->st.st_mode)) 
                || (MY_FILE(xd_p->st.st_uid) && U_RX(xd_p->st.st_mode)))
                return "folder/SE/emblem-write-ok/2.0/250/SE/emblem-readonly/2.0/130";
        // no access:
        return "folder/SE/emblem-unreadable/2.5/180";
    }
    if (S_ISLNK(xd_p->st.st_mode)){
        struct stat st;
        stat(xd_p->d_name, &st);
        if (S_ISDIR(st.st_mode)){
            if (!(O_RX(st.st_mode))
                    && !(MY_GROUP(st.st_gid) && G_RX(st.st_mode)) 
                    && !(MY_FILE(st.st_gid) && U_RX(st.st_mode)))
                return "folder/NE/emblem-symbolic-link/2.5/180/SE/emblem-unreadable/2.5/180";
            return "folder/NE/emblem-symbolic-link/2.5/180";
        }
        if (S_ISREG(st.st_mode)){
            if (O_RW(st.st_mode))
                return "text-x-generic/C/face-surprise/2.0/180/NE/emblem-symbolic-link/2.5/180";
            if (!(MY_GROUP(st.st_gid) && G_RW(st.st_mode)) 
                    && !(MY_FILE(st.st_gid) && U_RW(st.st_mode)))
                return "text-x-generic/SE/emblem-readonly/2.0/130/NE/emblem-symbolic-link/2.5/180";

            return "text-x-generic/NE/emblem-symbolic-link/2.5/180";
        }
	return  "emblem-symbolic-link";
    }

    if (S_ISREG(xd_p->st.st_mode)){

        // XXX do the read/write emblem here
        return "text-x-generic";
    }
    return "emblem-application";

}


const gchar *
xfdir_c::get_type_pixbuf(xd_t *xd_p){
    if (strcmp(xd_p->d_name, "..")==0)
	return  "go-up";
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    if (xd_p->d_type == DT_DIR) 
	return  "folder/E/emblem-bsd/2.5/180/W/emblem-gentoo/2.5/180";
	//return  "folder";
    if (xd_p->d_type == DT_LNK) 
	return  "emblem-symbolic-link";
    if (xd_p->d_type == DT_UNKNOWN) 
	return  "dialog-question";
#else
    return get_stat_pixbuf(xd_p, TRUE);
#endif
    return "text-x-generic";
}
#if 10
GtkTreeModel *
xfdir_c::get_tree_model (void){return treemodel;}

GtkTreeModel *
xfdir_c::mk_tree_model (void)
{
    if (!path || !g_file_test(path, G_FILE_TEST_EXISTS)) {
        fprintf(stderr, "%s does not exist\n", path);
        return NULL;
    }
    chdir(path);
    path = g_get_current_dir();

    
    GtkListStore *list_store = gtk_list_store_new (NUM_COLS, 
	    GDK_TYPE_PIXBUF,
	    G_TYPE_STRING, 
	    G_TYPE_STRING,
	    G_TYPE_STRING);

    heartbeat = 0;
    GList *directory_list = read_items (&heartbeat);
    insert_list_into_model(directory_list, list_store);

    return GTK_TREE_MODEL (list_store);
}

gint 
xfdir_c::get_icon_size(const gchar *name){
    if (strcmp(name, "..")==0) return GTK_ICON_SIZE_DND;
    return GTK_ICON_SIZE_DIALOG;
}

gint 
xfdir_c::get_icon_highlight_size(const gchar *name){
    return GTK_ICON_SIZE_DIALOG;
}

void
xfdir_c::insert_list_into_model(GList *data, GtkListStore *list_store){
    GdkPixbuf *p_file, *p_image, *p_dir;
    GtkTreeIter iter;

    GList *directory_list = (GList *)data;
    gint dir_count = g_list_length(directory_list);
    GList *l = directory_list;
    for (; l && l->data; l= l->next){
	xd_t *xd_p = (xd_t *)l->data;
        gtk_list_store_append (list_store, &iter);
	GdkPixbuf *p=p_file;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	if (xd_p->d_type == DT_DIR) p=p_dir;
#endif
	gchar *utf_name = utf_string(xd_p->d_name);
	const gchar *icon_name;
        if (dir_count > 500) icon_name = get_type_pixbuf(xd_p);
        else icon_name = get_stat_pixbuf(xd_p, TRUE);
        gtk_list_store_set (list_store, &iter, 
		COL_DISPLAY_NAME, utf_name,
		COL_ACTUAL_NAME, g_strdup(xd_p->d_name),
		COL_ICON_NAME, icon_name,
                COL_PIXBUF, get_pixbuf(icon_name,  get_icon_size(xd_p->d_name)), 
		-1);
	g_free(utf_name);
    }
    GList *p = directory_list;
    for (;p && p->data; p=p->next){
	xd_t *xd_p = (xd_t *)p->data;
        g_free(xd_p->d_name);
        g_free(xd_p);
    }
    g_list_free(directory_list);
}


#else

GtkTreeModel *
xfdir_c::get_tree_model (const gchar *path)

{
  GtkListStore *list_store;
  GdkPixbuf *p1, *p2;
  GtkTreeIter iter;
  GError *err = NULL;
  int i = 0;

  p1 = gdk_pixbuf_new_from_file ("image1.png", &err);
                            /* No error checking is done here */
  p2 = gdk_pixbuf_new_from_file ("image2.png", &err);
   
  list_store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, GDK_TYPE_PIXBUF);

  do {
    gtk_list_store_append (list_store, &iter);
    gtk_list_store_set (list_store, &iter, COL_DISPLAY_NAME, "Image1",
                        COL_PIXBUF, p1, -1);
    gtk_list_store_append (list_store, &iter);
    gtk_list_store_set (list_store, &iter, COL_DISPLAY_NAME, "Image2",
                        COL_PIXBUF, p2, -1);
  } while (i++ < 100);

  return GTK_TREE_MODEL (list_store);
}
#endif

