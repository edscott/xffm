#include <dirent.h>
#include "debug.h"

#include "xfdir_c.hpp"
#include <unistd.h>
#include <strings.h>



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


GList *
xfdir_c::read_items (gint *heartbeat) {
    GList *directory_list = NULL;
fprintf(stderr, "readfiles: %s\n", path);
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
	xd_t *xd_p = (xd_t *)calloc(1,sizeof(xd_t));
	xd_p->d_name = g_strdup(d->d_name);
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

// FIXME: composite icons for links to directories or files
const gchar *
xfdir_c::get_type_pixbuf(xd_t *xd_p){
    if (strcmp(xd_p->d_name, "..")==0)
	return  "go-up";
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    if (xd_p->d_type == DT_DIR) 
	return  "folder";
    if (xd_p->d_type == DT_LNK) 
	return  "emblem-symbolic-link";
    if (xd_p->d_type == DT_UNKNOWN) 
	return  "dialog-question";
#else
    // FIXME: do a stat here...
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

    
    GtkListStore *list_store;
    GdkPixbuf *p_file, *p_image, *p_dir;
    GtkTreeIter iter;

 
    list_store = gtk_list_store_new (NUM_COLS, 
	    GDK_TYPE_PIXBUF,
	    G_TYPE_STRING, 
	    G_TYPE_STRING,
	    G_TYPE_STRING);

    heartbeat = 0;
    GList *directory_list = read_items (&heartbeat);
    GList *l = directory_list;
    for (; l && l->data; l= l->next){
	xd_t *xd_p = (xd_t *)l->data;
        gtk_list_store_append (list_store, &iter);
	GdkPixbuf *p=p_file;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	if (xd_p->d_type == DT_DIR) p=p_dir;
#endif
	gchar *utf_name = utf_string(xd_p->d_name);
	const gchar *icon_name = get_type_pixbuf(xd_p);
        gtk_list_store_set (list_store, &iter, 
		COL_DISPLAY_NAME, utf_name,
		COL_ACTUAL_NAME, g_strdup(xd_p->d_name),
		COL_ICON_NAME, icon_name,
                COL_PIXBUF, get_pixbuf(icon_name, GTK_ICON_SIZE_DIALOG ), 
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
    return GTK_TREE_MODEL (list_store);
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

