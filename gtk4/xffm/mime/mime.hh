#ifndef XF_MIME_HH
#define XF_MIME_HH
#include "magic.h"
#include "mimesuffix.hh"
#include "mimemagic.hh"
#include "mimeapplication.hh"

// We can use either libmagic or perl mimetype, depending on configuration
// Prefer perl mimetype...


namespace xf {
    
// For starters, we need mime_type() and mime_file(), 


class Mime {
public:

  static bool is_image (const char *path) {
    if (!path || !g_file_test(path, G_FILE_TEST_IS_REGULAR)) return false;

    auto mimetype = mimeType(path);
    bool want_magic = (!mimetype || strcmp(mimetype, _("unknown"))==0);
    if (want_magic) {
      mimetype = MimeMagic::mimeMagic(path);
    }
    if (!mimetype) mimetype = _("unknown");
      
    TRACE("mimetype:: %s --> %s\n", path, mimetype);
    if (strstr(mimetype, "image")) return true;
    bool retval = mimetype_is_image(mimetype);
    return retval;

  }

private:

  static gint
  mimetype_is_image(const gchar *mimetype){
      static GSList *pix_mimetypes = NULL;
      static gsize initialized = 0;
      if (g_once_init_enter(&initialized)){
          // This gdk call is thread safe. 
          GSList *pix_formats = gdk_pixbuf_get_formats ();// check OK
          GSList *list = pix_formats;
          for(; list && list->data; list = list->next) {
              gchar **pix_mimetypes_p;
              auto fmt = (GdkPixbufFormat *)list->data;
              // This gdk call is thread safe.
              pix_mimetypes_p = gdk_pixbuf_format_get_mime_types (fmt);// check OK
              pix_mimetypes = g_slist_prepend(pix_mimetypes, pix_mimetypes_p);
          }
          g_slist_free(pix_formats);
          g_once_init_leave(&initialized, 1);
      }
      /* check for image support types */
      GSList *list = pix_mimetypes;
      for(; list && list->data; list = list->next) {
        auto pix_mimetypes_p = (gchar **)list->data;
        for(; pix_mimetypes_p && *pix_mimetypes_p; pix_mimetypes_p++) {
            TRACE(stderr, "allowable pix_format=%s --> %s\n", *pix_mimetypes_p, mimetype);
            if(g_ascii_strcasecmp (*pix_mimetypes_p, mimetype) == 0) {
          return 1;
            }
        }
      }
      return 0;
  }



    static gchar *
    mimeMagic (const gchar *file){
        return MimeMagic::mimeMagic(file);
    }

    static gchar *
    mimeType (const gchar *file){
        gchar *retval = MimeSuffix::mimeType(file);
        if (retval) {
            TRACE("mimeType: %s --> %s\n", file, retval);
            return retval;
        }
        return MimeMagic::mimeMagic(file);
   } 

    static gchar *
    mimeFile (const gchar *file){
        return MimeMagic::mimeFile(file);
   } 
   
public: 
    static gchar *
    basicMimeType(unsigned char d_type){
        gchar *retval=NULL;
        if (d_type == DT_DIR ) retval= g_strdup("inode/directory");
        // Character device:
        else if (d_type == DT_CHR ) retval= g_strdup("inode/chardevice");   
        // Named pipe (FIFO):
        else if (d_type == DT_FIFO ) retval= g_strdup("inode/fifo");
        // UNIX domain socket:
        else if (d_type == DT_SOCK ) retval= g_strdup("inode/socket");
        // Block device
        else if (d_type == DT_BLK ) retval= g_strdup("inode/blockdevice");
        // Unknown:
        else if (d_type == DT_UNKNOWN) retval= g_strdup("inode/unknown");
        // Regular file:
        else if (d_type == DT_REG ) retval= g_strdup("inode/regular");
        else if (d_type == DT_LNK ) retval= g_strdup("inode/symlink");
        if (!d_type || d_type == DT_UNKNOWN) {
            TRACE("Mime::basicMimeType: %d: %s\n", d_type, retval);
        }
        if (retval) return retval;
        return  g_strdup("inode/unknown");
    }
    
    static gchar *
    statMimeType (struct stat *st_p) {
            if(S_ISSOCK (st_p->st_mode)) return g_strdup("inode/socket");
            else if(S_ISBLK (st_p->st_mode)) return g_strdup("inode/blockdevice");
            else if(S_ISCHR (st_p->st_mode)) return g_strdup("inode/chardevice");
            else if(S_ISFIFO (st_p->st_mode)) return g_strdup("inode/fifo");
            else if (S_ISLNK(st_p->st_mode)) return g_strdup("inode/symlink");
            else if(S_ISDIR (st_p->st_mode)) return g_strdup("inode/directory");
            else if(S_ISREG (st_p->st_mode)) return g_strdup("inode/regular");
        return  g_strdup("inode/unknown");
    }

    static gchar *
    mimeType (const gchar *file, struct stat *st_p) {
        if (!file){
            ERROR("mimeType (file, st_p) file cannot be nil\n");
            return g_strdup("inode/regular");
        }



        if (!st_p){
            if(S_ISSOCK (st_p->st_mode)) return g_strdup("inode/socket");
            else if(S_ISBLK (st_p->st_mode)) return g_strdup("inode/blockdevice");
            else if(S_ISCHR (st_p->st_mode)) return g_strdup("inode/chardevice");
            else if(S_ISFIFO (st_p->st_mode)) return g_strdup("inode/fifo");
            else if (S_ISLNK(st_p->st_mode)) return g_strdup("inode/symlink");
            else if(S_ISDIR (st_p->st_mode)) return g_strdup("inode/directory");
        }

        if(file[strlen (file) - 1] == '~' || file[strlen (file) - 1] == '%') {
            gchar *r_file = g_strdup(file);
            r_file[strlen (r_file) - 1] = 0;
            gchar *retval = mimeType(r_file, st_p);
            g_free(r_file);
            return retval;
        }
        auto type = MimeSuffix::mimeType(file);
        if (!type) type = g_strdup("inode/regular");
        return type;
        //return g_strdup("inode/regular");
        //return mimeMagic(file);
    }

private:
    static const gchar *
    get_mimetype_iconname(const gchar *mimetype){
        //FIXME: pull in value built from hash:
        return NULL;
        //return MimeHash<txt_hash_t>::lookup(mimetype, hash_data[GENERIC_ICON]); 
    }

};
}
#endif
