#ifndef BOOKMARKS_HH
#define BOOKMARKS_HH
#define USER_DIR                 g_get_home_dir()
#define BOOKMARKS_DIR                "gtk-3.0"
#define BOOKMARKS_FILE                "bookmarks"

namespace xf {

enum transport_e {
    LOCAL_TRANSPORT,
    NFS_TRANSPORT,
    CIFS_TRANSPORT,
    OBEX_TRANSPORT,
    SSH_TRANSPORT,
    FTP_TRANSPORT,
    EFS_TRANSPORT
};

typedef struct bookmarkItem_t {
    gchar *uri;
    gchar *path;
    gchar *hostname;
    enum transport_e transport_type;
}bookmarkItem_t;

static GSList *bookmarks=NULL;

//template <class Type>
class Bookmarks {
    
public:
    static void setBookmarkIcon(GFileInfo *info, const char *path){
      int size = Settings::getInteger("xfterm", "iconsize", 24);
      auto gIcon = g_file_info_get_icon(info);
      
      const char *ball = EMBLEM_FAVOURITE;
      auto paintable = Texture<bool>::addEmblem(gIcon, ball, size, size);
      g_file_info_set_attribute_object(info, "xffm:paintable", G_OBJECT(paintable));  
      return;  
    }    

    static void
    initBookmarks(void) {
        if (bookmarks) return;
        bookmarks = readBookmarkFile(bookmarks);
    }

    static GSList *bookmarksList(void){
        return bookmarks;
    }

    static gboolean
    isBookmarked(const gchar *path){
       for (auto l=bookmarks; l && l->data; l=l->next){
            // Bookmarks in settings.ini
            // local bookmarks:
            auto p = (bookmarkItem_t *)l->data;
            if (!p->path) continue;
            if (strcmp(p->path, path)==0) return TRUE;
       }
       return FALSE;
    }
    
    static gboolean
    addBookmark(const gchar *path){
      initBookmarks();
        if (!path || !strlen(path)) {
            ERROR_("AddBookmark() path is NULL or strlen==0");
            return FALSE;
        }
        TRACE("Bookmarking %s\n", path);
        auto p = bookmarkItemNew(path);

        bookmarks = g_slist_prepend(bookmarks, p);
        saveBookmarkFile(bookmarks);
        return TRUE;
    }


    static gboolean
    removeBookmark(const gchar *path){
      initBookmarks();
        if (!path || !strlen(path)) {
            ERROR_("removeBookmark(%s) path is NULL or strlen==0", path);
            return FALSE;
        }
        TRACE("removing Bookmark  %s\n", path);
        gboolean retval = FALSE;
        for (auto l=bookmarks; l && l->data; l=l->next){
             auto p = (bookmarkItem_t *)l->data;
             if (p->path == NULL) continue;
             if (strcmp(p->path, path)==0){
                 TRACE("removeBookmark() gotcha %s\n", path);
                 bookmarks = g_slist_remove(bookmarks, p);
                 bookmarkItemFree(p);
                 retval = TRUE;
                 break;
             }
         }
         if (retval) saveBookmarkFile(bookmarks);
         return TRUE;
    }

private:

    static gchar *
    getBookmarksFilename(void){
        auto configDir = g_get_user_config_dir();
        auto dir = g_build_filename(configDir, BOOKMARKS_DIR, NULL);
        if (!g_file_test(dir, G_FILE_TEST_IS_DIR)){
            if (g_mkdir_with_parents(dir, 0755)<0){
                ERROR_("Cannot create %s: %s\n", dir, strerror(errno));
                g_free(dir);
                return NULL;
            }
        }
        auto name = g_build_filename(configDir, BOOKMARKS_DIR, BOOKMARKS_FILE, NULL);
        if (!g_file_test(name, G_FILE_TEST_EXISTS)){
            fclose(fopen(name, "w"));
        }
        g_free(dir);
        return name;
    }

    static bookmarkItem_t *
    bookmarkItemNew(void){
        bookmarkItem_t *p = (bookmarkItem_t *)calloc(1,sizeof(bookmarkItem_t));
        if (!p) {
            g_error("calloc: %s\n", strerror(errno));
        }
        return p;
    }

    static bookmarkItem_t *
    bookmarkItemNew(const gchar *path){
        bookmarkItem_t *p = bookmarkItemNew();
        p->path = g_strdup(path);
        GError *error = NULL;
        p->uri = g_filename_to_uri(path, NULL, &error);
        if (error){
            ERROR_("bookmarkItemNew(%s): %s\n", path, error->message);
            g_error_free(error);
            bookmarkItemFree(p);
            return NULL;
        }
        return p;
    }


    static void
    bookmarkItemFree( bookmarkItem_t *p){
        if (!p) return;
        g_free(p->uri);
        g_free(p->path);
        g_free(p->hostname);
        g_free(p);
        return;
    }

    static GSList *
    clearBookmarksList(GSList *list){
        for (auto l=list; l && l->data; l=l->next){
            bookmarkItemFree((bookmarkItem_t *)(l->data));
        }
        g_slist_free(list);
        return NULL;
    }

    static FILE *
    openBookmarkFile(void){
        auto filename = getBookmarksFilename();
        auto f=fopen(filename, "r");
        if (!f) {
            ERROR_("fopen(%s, \"r\") failed: %s\n", filename, strerror(errno));
        }
        g_free(filename);
        return f;
    }

    static GSList *
    fillBookmarksList(FILE *f, GSList *list){
        gchar buffer[2048];

        while (fgets(buffer, 2047, f) && !feof(f)){
            if (strchr(buffer, '\n')) *strchr(buffer, '\n')=0;
            if (strlen(buffer)==0) continue;
            if (strncmp(buffer, "file://", strlen("file://"))) continue;
            auto p = bookmarkItemNew();
            p->uri = g_strdup(buffer);
            GError *error = NULL;
            p->path = g_filename_from_uri (p->uri, &(p->hostname), &error);
            if (error) {
                p->path = NULL;
                ERROR_("bookmarks.hh %s: %s\n", p->uri, error->message);
                g_error_free(error);
                //continue;
            }
            list = g_slist_prepend(list, p);
        }
        return list;
    }
        
    static GSList *
    readBookmarkFile(GSList * list){ 
        TRACE("now reading bookmark file\n"); 
        list = clearBookmarksList(list);
        auto f = openBookmarkFile();
        if (!f) return NULL;
        list = fillBookmarksList(f, list);
        fclose(f);
        return list;
    }

    static void
    saveBookmarkFile(GSList *list){
        gchar *filename = getBookmarksFilename();
        if (!filename) return;
        if (list==NULL || g_slist_length(list)==0){
            if (g_file_test(filename, G_FILE_TEST_EXISTS)){
                if (unlink(filename) < 0){
                    ERROR_("unlink(%s): %s\n", filename, strerror(errno));
                }
            }
            g_free(filename);
            return;
        }
        FILE *f=fopen(filename, "w");
        g_free(filename);
        GSList *tmp=list;
        if (f) {
            for (auto l=list; l && l->data; l=l->next){
                auto p = (bookmarkItem_t *)l->data;
                fprintf(f,"%s\n", p->uri);
            }
            fclose(f);
        }
        return;
    }

}; //class Bookmarks
} //namespace xf

#endif
