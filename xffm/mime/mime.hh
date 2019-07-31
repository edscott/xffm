#ifndef XF_MIME_HH
#define XF_MIME_HH
#include "mimesuffix.hh"
#include "mimemagic.hh"
#include "mimetype.hh"
#include "mimeapplication.hh"

// We can use either libmagic or perl mimetype, depending on configuration


namespace xf {
    
// Brainstorm
// For starters, we need mime_type() and mime_file(), 
// then type_from_sfx and alias_type and apps and command

// We have now simplified, removing custom made mime determination
// with now mature shared-mime-info package.
// This follows the same principle used in replacing custom made
// iconview with now mature gtk iconview.
// Things get simpler and maintainance not so complicated (methinks).
// 
// Remake: simplify with now mature shared-mime-info package


template <class Type>
class Mime {
    using util_c = Util<Type>;
public:


    static gchar *
    encoding (const gchar *file) { 
#ifdef HAVE_LIBMAGIC  
	return  MimeMagic<Type>::encoding(file);
#else
	// If not libmagic, assume the encoding is already utf-8. (whatever...)
	return  g_strdup("UTF-8");
#endif
    }

    static gchar *
    mimeMagic (const gchar *file){
#ifdef MIMETYPE_PROGRAM
	return MimeType<Type>::mimeMagic(file);
#else
# if LIBMAGIC
	return MimeMagic<Type>::mimeMagic(file);
# else
        return NULL;
# endif
#endif
    }

    static gchar *
    mimeType (const gchar *file){
        gchar *retval = MimeSuffix<Type>::mimeType(file);
        if (retval) {
	    TRACE("mimeType: %s --> %s\n", file, retval);
            return retval;
        }
#ifdef MIMETYPE_PROGRAM
	return MimeType<Type>::mimeType(file);
#else
	errno=0;
        struct stat st;
        if (stat(file, &st) < 0) {
	    DBG("mime.hh::mimeType(): stat %s (%s)\n",
		file, strerror(errno));
	    errno=0;
	    return g_strdup("inode/unknown");
	}
        gchar *r = mimeType(file, &st);
        return r;
        
#endif
   } 

// FIXME: use language code -l code, --language=code 
    static gchar *
    mimeFile (const gchar *file){
#ifdef MIMETYPE_PROGRAM
	return MimeType<Type>::mimeFile(file);
#else
        return NULL;
#endif
   } 
   


private:

public:

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
	auto type = MimeSuffix<Type>::mimeType(file);
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
