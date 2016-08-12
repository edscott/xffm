#include <dirent.h>
#include "debug.h"

#include "xfdir_c.hpp"





GSList *
xfdir_c::read_items (const gchar *path, gint *heartbeat) {
    GSList *directory_list = NULL;

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
#endif
	directory_list = g_slist_prepend(directory_list, xd_p);
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
    if (directory_list) directory_list = g_slist_reverse(directory_list);

    // At least the ../ record should have been read. If this
    // is not so, then a read error occurred.
    // (not uncommon in bluetoothed obexfs)
    if (!directory_list) {
	NOOP("read_files_local(): Count failed! Directory not read!\n");
    }
    return (directory_list);
}

// FIXME: iconview cell area is too large.
// FIXME: composite icons for links to directories or files
GdkPixbuf *
xfdir_c::get_type_pixbuf(xd_t *xd_p){
    if (strcmp(xd_p->d_name, "..")==0)
	return get_pixbuf ("go-up", GTK_ICON_SIZE_DIALOG);
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    if (xd_p->d_type == DT_DIR) 
	return get_pixbuf ("folder", GTK_ICON_SIZE_DIALOG);
    if (xd_p->d_type == DT_LNK) 
	return get_pixbuf ("emblem-symbolic-link", GTK_ICON_SIZE_DIALOG);
    if (xd_p->d_type == DT_UNKNOWN) 
	return get_pixbuf ("dialog-question", GTK_ICON_SIZE_DIALOG);
#else
    // FIXME: do a stat here...
#endif
    return get_pixbuf ("text-x-generic", GTK_ICON_SIZE_DIALOG);
}
#if 10
GtkTreeModel *
xfdir_c::get_tree_model (const gchar *path)
{
    if (!g_file_test(path, G_FILE_TEST_EXISTS)) return NULL;
    GtkListStore *list_store;
    GdkPixbuf *p_file, *p_image, *p_dir;
    GtkTreeIter iter;

 
    list_store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, GDK_TYPE_PIXBUF);

    heartbeat = 0;
    GSList *directory_list = read_items (path, &heartbeat);
    GSList *l = directory_list;
    for (; l && l->data; l= l->next){
	xd_t *xd_p = (xd_t *)l->data;
        gtk_list_store_append (list_store, &iter);
	GdkPixbuf *p=p_file;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	if (xd_p->d_type == DT_DIR) p=p_dir;
#endif
        gtk_list_store_set (list_store, &iter, COL_DISPLAY_NAME, xd_p->d_name,
                        COL_PIXBUF, get_type_pixbuf(xd_p), -1);
    }
    // FIXME: free directory_list
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

