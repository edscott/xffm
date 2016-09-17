#include <dirent.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdlib.h>
#include "xfdir_local_c.hpp"

static gint compare_type (const void *, const void *);

xfdir_local_c::xfdir_local_c(const gchar *data, gtk_c *data_gtk_c): 
    xfdir_c(data, data_gtk_c)
{
    treemodel = mk_tree_model();
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
	    GDK_TYPE_PIXBUF, // icon
	    G_TYPE_INT,      // mode
	    G_TYPE_STRING,   // name (UTF-8)
	    G_TYPE_STRING,   // name (verbatim)
	    G_TYPE_STRING);   // icon_name

    heartbeat = 0;
    GList *directory_list = read_items (&heartbeat);
    insert_list_into_model(directory_list, list_store);

    return GTK_TREE_MODEL (list_store);
}

GList *
xfdir_local_c::read_items (gint *heartbeat) {
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

    // At least the ../ record should have been read. If this
    // is not so, then a read error occurred.
    // (not uncommon in bluetoothed obexfs)
    if (!directory_list) {
	NOOP("read_files_local(): Count failed! Directory not read!\n");
    }
    directory_list = g_list_sort (directory_list,compare_type);
    return (directory_list);
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
	const gchar *icon_name;
        if (dir_count > 500) icon_name = get_type_pixbuf(xd_p);
        else icon_name = get_stat_pixbuf(xd_p, TRUE);
        gtk_list_store_set (list_store, &iter, 
		COL_DISPLAY_NAME, utf_name,
		COL_ACTUAL_NAME, xd_p->d_name,
		COL_ICON_NAME, icon_name,
		COL_MODE,xd_p->st.st_mode, 
                COL_PIXBUF, gtk_p->get_pixbuf(icon_name,  get_icon_size(xd_p->d_name)), 
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
const gchar *
xfdir_local_c::get_stat_pixbuf(xd_t *xd_p, gboolean restat){
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
#define O_RX(x) ((S_IROTH & x) && (S_IXOTH & x))
#define G_RX(x) ((S_IRGRP & x) && (S_IXGRP & x))
#define U_RX(x) ((S_IRUSR & x) && (S_IXUSR & x))
#define O_R(x) (S_IROTH & x)
#define G_R(x) (S_IRGRP & x)
#define U_R(x) (S_IRUSR & x)
#define MY_FILE(x) (x == geteuid())
#define MY_GROUP(x) (x == getegid())



    if (S_ISDIR(xd_p->st.st_mode)){
        // all access:
        if (O_ALL(xd_p->st.st_mode))
                return "folder/C/face-surprise/2.0/180";
        if ((MY_GROUP(xd_p->st.st_gid) && G_ALL(xd_p->st.st_mode)) 
                || (MY_FILE(xd_p->st.st_uid) && U_ALL(xd_p->st.st_mode))){
	    if (strcmp(path, g_get_home_dir())==0) return get_home_iconname(xd_p->d_name);
                return "folder";
	}
        // read only:
        if (O_RX(xd_p->st.st_mode) 
                || (MY_GROUP(xd_p->st.st_gid) && G_RX(xd_p->st.st_mode)) 
                || (MY_FILE(xd_p->st.st_uid) && U_RX(xd_p->st.st_mode)))
                return "folder/SW/emblem-readonly/3.0/180";
        // no access:
        return "folder/SW/emblem-unreadable/3.0/180/C/face-angry/2.0/180";
    }
    if (S_ISLNK(xd_p->st.st_mode)){
        struct stat st;
        stat(xd_p->d_name, &st);
        if (S_ISDIR(st.st_mode)){
	    // all access:
	    if (O_ALL(st.st_mode))
		    return "folder/C/face-surprise/2.0/180/NE/emblem-symbolic-link/2.5/180";
	    if ((MY_GROUP(st.st_gid) && G_ALL(st.st_mode)) 
		    || (MY_FILE(st.st_uid) && U_ALL(st.st_mode)))
		    return "folder";
	    // read only:
	    if (O_RX(st.st_mode) 
		    || (MY_GROUP(st.st_gid) && G_RX(st.st_mode)) 
		    || (MY_FILE(st.st_uid) && U_RX(st.st_mode)))
		    return "folder/SW/emblem-readonly/3.0/180/NE/emblem-symbolic-link/2.5/180";
	    // no access:
	    return "folder/SW/emblem-unreadable/3.0/180/C/face-angry/2.0/180/NE/emblem-symbolic-link/2.5/180";
	}
        if (S_ISREG(st.st_mode)){
	   // all access:
	    if (O_ALL(st.st_mode) || O_RW(st.st_mode))
		    return "text-x-generic/C/face-surprise/2.0/180/SW/emblem-exec/3.0/180/NE/emblem-symbolic-link/2.5/180";
	    // read/write/exec
	    if ((MY_GROUP(st.st_gid) && G_ALL(st.st_mode)) 
		    || (MY_FILE(st.st_uid) && U_ALL(st.st_mode)))
		    return "text-x-generic/SW/emblem-exec/3.0/180/NE/emblem-symbolic-link/2.5/180";
	    // read/exec
	    if (O_RX(st.st_mode)
		    ||(MY_GROUP(st.st_gid) && G_RX(st.st_mode)) 
		    || (MY_FILE(st.st_uid) && U_RX(st.st_mode)))
		    return "text-x-generic/SW/emblem-exec/3.0/180/NE/emblem-symbolic-link/2.5/180";

	    // read/write
	    if ((MY_GROUP(st.st_gid) && G_RW(st.st_mode))
		    || (MY_FILE(st.st_uid) && U_RW(st.st_mode)))
		    return "text-x-generic/NE/emblem-symbolic-link/2.5/180";

	    // read only:
	    if (O_R(st.st_mode) 
		    || (MY_GROUP(st.st_gid) && G_R(st.st_mode)) 
		    || (MY_FILE(st.st_uid) && U_R(st.st_mode)))
		    return "text-x-generic/SW/emblem-readonly/3.0/130/NE/emblem-symbolic-link/2.5/180";
	    // no access:
	    return "text-x-generic/SW/emblem-unreadable/3.0/180/C/face-angry/2.0/180/NE/emblem-symbolic-link/2.5/180";
	}
    }
#if 0
        if (S_ISDIR(st.st_mode)){
            if (!(O_RX(st.st_mode))
                    && !(MY_GROUP(st.st_gid) && G_RX(st.st_mode)) 
                    && !(MY_FILE(st.st_gid) && U_RX(st.st_mode)))
                return "folder/NE/emblem-symbolic-link/2.5/180/SW/emblem-unreadable/2.5/180";
            return "folder/NE/emblem-symbolic-link/2.5/180";
        }
        if (S_ISREG(st.st_mode)){
            if (O_RW(st.st_mode))
                return "text-x-generic/C/face-surprise/2.0/180/NE/emblem-symbolic-link/2.5/180";
            if (!(MY_GROUP(st.st_gid) && G_RW(st.st_mode)) 
                    && !(MY_FILE(st.st_gid) && U_RW(st.st_mode)))
                return "text-x-generic/SW/emblem-readonly/2.0/130/NE/emblem-symbolic-link/2.5/180";

            return "text-x-generic/NE/emblem-symbolic-link/2.5/180";
        }
	return  "emblem-symbolic-link";
    }
#endif

    if (S_ISREG(xd_p->st.st_mode)){
        // all access:
        if (O_ALL(xd_p->st.st_mode) || O_RW(xd_p->st.st_mode))
                return "text-x-generic/C/face-surprise/2.0/180/SW/emblem-exec/3.0/180";
	// read/write/exec
        if ((MY_GROUP(xd_p->st.st_gid) && G_ALL(xd_p->st.st_mode)) 
                || (MY_FILE(xd_p->st.st_uid) && U_ALL(xd_p->st.st_mode)))
                return "text-x-generic/SW/emblem-exec/3.0/180";
	// read/exec
        if (O_RX(xd_p->st.st_mode)
		||(MY_GROUP(xd_p->st.st_gid) && G_RX(xd_p->st.st_mode)) 
                || (MY_FILE(xd_p->st.st_uid) && U_RX(xd_p->st.st_mode)))
                return "text-x-generic/SW/emblem-exec/3.0/180";

	// read/write
        if ((MY_GROUP(xd_p->st.st_gid) && G_RW(xd_p->st.st_mode))
                || (MY_FILE(xd_p->st.st_uid) && U_RW(xd_p->st.st_mode)))
                return "text-x-generic";

        // read only:
        if (O_R(xd_p->st.st_mode) 
                || (MY_GROUP(xd_p->st.st_gid) && G_R(xd_p->st.st_mode)) 
                || (MY_FILE(xd_p->st.st_uid) && U_R(xd_p->st.st_mode)))
                return "text-x-generic/SW/emblem-readonly/3.0/130";
        // no access:
        return "text-x-generic/SW/emblem-unreadable/3.0/180/C/face-angry/2.0/180";
    }
    return "emblem-application";

}


const gchar *
xfdir_local_c::get_type_pixbuf(xd_t *xd_p){
    if (strcmp(xd_p->d_name, "..")==0)
	return  "go-up";
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    if (xd_p->d_type == DT_DIR){ 
	if (strcmp(path, g_get_home_dir())==0) {
	    return get_home_iconname(xd_p->d_name);
	}
	return  "folder";
    }
    if (xd_p->d_type == DT_LNK) 
	return  "emblem-symbolic-link";
    if (xd_p->d_type == DT_UNKNOWN) 
	return  "dialog-question";
#else
    return get_stat_pixbuf(xd_p, TRUE);
#endif
    return "text-x-generic";
}


const gchar *
xfdir_local_c::get_home_iconname(const gchar *data){
    if (!data) return "user-home";
    const gchar *dir[]={N_("Documents"), N_("Downloads"),N_("Music"),N_("Pictures"),
	        N_("Templates"),N_("Videos"),N_("Desktop"),N_("Bookmarks"),
		N_(".Trash"),NULL};
    const gchar *icon[]={"folder-documents", "folder-download","folder-music","folder-pictures",
	          "folder-templates","folder-videos","user-desktop","user-bookmarks",
		  "user-trash",NULL};
    const gchar **p, **i;
    for (p=dir, i=icon; p && *p ; p++, i++){
	if (strcasecmp(*p, data) == 0) {
	    return *i;
	}
    }
    return "folder";
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
}


