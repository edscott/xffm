#ifndef XF_LOCALICONS__HH
# define XF_LOCALICONS__HH

namespace xf
{
template <class Type> class FstabView;
template <class Type> class ClipBoard;
template <class Type>
class LocalIcons {
    using gtk_c = Gtk<Type>;
    using pixbuf_c = Pixbuf<Type>;
    using util_c = Util<Type>;
    
public:
    static gchar *
    get_iconname(const gchar *path){
        struct stat st;
        gchar *name;
        if (stat(path, &st) < 0) 
            name = get_basic_iconname(path, NULL);
        else 
            name = get_basic_iconname(path, &st);
	if (!name) name = g_strdup("image-missing");

        gchar *emblem = getEmblem(path, &st);
        gchar *iconname = g_strconcat(name, emblem, NULL);
        g_free(name);
        g_free(emblem);
        return iconname;
    }

    static gchar *
    get_iconname(const gchar *path, const gchar *mimetype){
	if (!mimetype) return NULL; //return get_iconname(path);
	if (strstr(mimetype, "image")){
	    if (isTreeView) return g_strdup("image-x-generic");
	    if (Gtk<Type>::isImage(mimetype)) return g_strdup(path);
	    return g_strdup("image-x-generic");

	}
	return NULL;
    }

    static gchar *
    getIconname(xd_t *xd_p){
	return 
	    getIconname(xd_p->path, 
		    xd_p->d_name,
		    xd_p->mimetype, 
		    xd_p->d_type, 
		    xd_p->st);
    }

    static gchar *
    getIconname(const gchar *path, const gchar *basename, 
	    const gchar *mimetype, const unsigned char d_type,
	    struct stat *st_p){
        // Up directory:
        if (strcmp(basename, "..")==0) return  g_strdup("go-up");

	auto name = getBasicIconname(path, mimetype);
	if (!name){
	    ERROR("getBasicIconname should not return NULL\n");
	    return g_strdup("image-missing");
	}

	if (g_path_is_absolute(name)) return name; // image previews (no emblem)
	TRACE("basic iconname: %s --> %s\n", basename, name);
        gchar *emblem = getEmblem(path, basename,  d_type, st_p);
        TRACE("emblem=%s\n", emblem);
        gchar *iconname = g_strconcat(name, emblem, NULL);
        g_free(name);
        g_free(emblem);
        return iconname;
    }
    
    static gboolean backupType(const gchar *file){
        if (!file) return FALSE;
        // GNU backup type:
         if(file[strlen (file) - 1] == '~' || 
                 file[strlen (file) - 1] == '%'|| 
                 file[strlen (file) - 1] == '#') return TRUE;
        // MIME backup type:
        const gchar *e = strrchr(file, '.');
        if (e){
            if (strcmp(e,".old")==0) return TRUE;
            else if (strcmp(e,".bak")==0) return TRUE;
            else if (strcmp(e,".sik")==0) return TRUE;
        }
        return FALSE;
    }
     
private:

    static gboolean cHdr(const gchar *file){
        if (!file) return FALSE;
        const gchar *ext = strrchr(file, '.');
        if (!ext) return FALSE;        
        if(strcmp(ext, ".h")==0 || strcmp(ext, ".hpp")==0 
                || strcmp(ext, ".hh")==0) return TRUE;
        return FALSE;
    }
   
    static gboolean cSrc(const gchar *file){
        if (!file) return FALSE;
        const gchar *ext = strrchr(file, '.');
        if (!ext) return FALSE;        
        if(strcmp(ext, ".c")==0 || strcmp(ext, ".cpp")==0 
                || strcmp(ext, ".cc")==0) return TRUE;
        return FALSE;
    }

    static gchar *
    get_basic_iconname(const gchar *path, struct stat *st){

        // Directories:
        gchar *base = g_path_get_basename(path);
        if (strcmp(base, "..")==0){
            g_free(base);
            return  g_strdup("go-up");
        }
        g_free(base);
        if ((st && S_ISDIR(st->st_mode))) {
            if (strcmp(path, g_get_home_dir())==0) {
                return get_home_iconname(path);
            }
            return  g_strdup("folder");
        }

        // Character device:
        if ((st && S_ISCHR(st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/input-keyboard-symbolic/2.0/220");
        }
        // Named pipe (FIFO):
        if ((st && S_ISFIFO(st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/emblem-synchronizing-symbolic/2.0/220");
        }
        // UNIX domain socket:
        if ((st && S_ISSOCK(st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/emblem-shared-symbolic/2.0/220");
        }
        // Block device
        if ((st && S_ISBLK(st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/drive-harddisk-symbolic/2.0/220");
        }
        // Regular file:
/*
 * FIXME
        if ((st && S_ISREG(st->st_mode))) {
            const gchar *basic = get_mime_iconname(xd_p->mimetype);
            return g_strdup(basic);
        }
        */
        return  g_strdup("text-x-generic");
    }

    static gchar *
    getBasicIconname(const gchar *path, const gchar *mimetype){	
	if (strcmp(path, g_get_home_dir())==0) return g_strdup("user-home");
	if (!mimetype) {
	    ERROR("getBasicIconname mimetype cannot be null\n");
	    return g_strdup("image-missing");
	}
	if (strcmp(mimetype, "inode/directory")==0) return  g_strdup("folder");

	// Block device
	if (strcmp(mimetype, "inode/blockdevice")==0) return g_strdup("drive-harddisk-symbolic");
        
        // Character device:
	if (strcmp(mimetype, "inode/chardevice")==0) return  g_strdup("input-keyboard-symbolic");

        // Named pipe (FIFO):
	if (strcmp(mimetype, "inode/fifo")==0) return  g_strdup("network-wired-symbolic");

        // UNIX domain socket:
	if (strcmp(mimetype, "inode/socket")==0) return  g_strdup("network-wired-symbolic");
        
        // Regular file:
	if (strcmp(mimetype, "inode/regular")==0) return  g_strdup("text-x-generic");
	if (strstr(mimetype, "image")){
	    if (isTreeView) return g_strdup("image-x-generic");
	    if (Gtk<Type>::isImage(mimetype)) return g_strdup(path);
	    return g_strdup("image-x-generic");
	}

	if (strstr(mimetype, "audio")) return g_strdup("audio-x-generic");
	
	if (strstr(mimetype, "font")) return g_strdup("font-x-generic");
	
	if (strstr(mimetype, "video")) return g_strdup("video-x-generic");
	
	if (strstr(mimetype, "script")) return g_strdup("text-x-script");
	
	if (strstr(mimetype, "template")) return g_strdup("text-x-generic-template");
	
	if (strstr(mimetype, "text")) return g_strdup("text-x-generic");

	if (strstr(mimetype, "html")) return g_strdup("html-x-generic");
	
	if (strstr(mimetype, "package")) return g_strdup("package-x-generic");
	
	//if (strstr(mimetype, "")) return g_strdup("-x-generic");
	
	//XXX office stuff
	
	if (g_file_test(path, G_FILE_TEST_IS_EXECUTABLE)) return g_strdup("application-x-executable");
	return g_strdup("text-x-preview");;
     }


    static gchar *
    get_basic_iconname(xd_t *xd_p){	
        // Up directory:
        if (strcmp(xd_p->d_name, "..")==0) return  g_strdup("go-up");

#ifdef HAVE_STRUCT_DIRENT_D_TYPE
        // Symlinks:
    /*    if (xd_p->d_type == DT_LNK) {
            return  g_strdup("text-x-generic-template/SW/emblem-symbolic-link/2.0/220");
        }
    */
	if (xd_p->mimetype){
	    if (strcmp(xd_p->mimetype, "inode/blockdevice")==0) 
		return g_strdup("drive-harddisk-symbolic");
	}

        if ((xd_p->d_type == DT_DIR )||(xd_p->st && S_ISDIR(xd_p->st->st_mode))) {
            if (strcmp(xd_p->path, g_get_home_dir())==0) {
                return get_home_iconname(xd_p->path);
            }
            return  g_strdup("folder");
        }
        // Character device:
        if (xd_p->d_type == DT_CHR ) {
            return  g_strdup("text-x-generic-template/SW/input-keyboard-symbolic/2.0/220");
        }
        // Named pipe (FIFO):
        if (xd_p->d_type == DT_FIFO ) {
            return  g_strdup("text-x-generic-template/SW/emblem-synchronizing-symbolic/2.0/220");
        }
        // UNIX domain socket:
        if (xd_p->d_type == DT_SOCK ) {
            return  g_strdup("text-x-generic-template/SW/emblem-shared-symbolic/2.0/220");
        }
        // Block device
        if (xd_p->d_type == DT_BLK ) {
            return  g_strdup("drive-harddisk-symbolic");
        }
        // Regular file:

        if (xd_p->d_type == DT_REG ) {
            const gchar *basic = get_mime_iconname(xd_p->mimetype);
            return g_strdup(basic);
        }

        // Unknown:
        if (xd_p->d_type == DT_UNKNOWN) {
            return  g_strdup("dialog-question");
        }
#else
        if ((xd_p->st && S_ISDIR(xd_p->st->st_mode))) {
            if (strcmp(xd_p->path, g_get_home_dir())==0) {
                return get_home_iconname(xd_p->d_name);
            }
            return  g_strdup("folder");
        }

        // Symlinks:
    /*    if (xd_p->st && xd_p->d_type == xd_p->d_type == DT_LNK) {
            return  g_strdup("text-x-generic-template/SW/emblem-symbolic-link/2.0/220");
        }
    */
        // Character device:
        if ((xd_p->st && S_ISCHR(xd_p->st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/input-keyboard-symbolic/2.0/220");
        }
        // Named pipe (FIFO):
        if ((xd_p->st && S_ISFIFO(xd_p->st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/emblem-synchronizing-symbolic/2.0/220");
        }
        // UNIX domain socket:
        if ((xd_p->st && S_ISSOCK(xd_p->st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/emblem-shared-symbolic/2.0/220");
        }
        // Block device
        if ((xd_p->st && S_ISBLK(xd_p->st->st_mode))) {
            return  g_strdup("text-x-generic-template/SW/drive-harddisk-symbolic/2.0/220");
        }
        // Regular file:

        if ((xd_p->st && S_ISREG(xd_p->st->st_mode))) {
            const gchar *basic = get_mime_iconname(xd_p->mimetype);
            return g_strdup(basic);
        }
#endif
        return  g_strdup("text-x-generic");
    }


    static const gchar *
    get_mime_iconname(const gchar *mimetype){
        const gchar *basic = NULL;
#ifdef USE_MIME
        if (mimetype) {
            // here we should get generic-icon from mime-module.xml!
            basic = Mime<Type>::get_mimetype_iconname(mimetype);
            TRACE("xfdir_local_c::get_mime_iconname(%s) -> %s\n", mimetype, basic);
            if (basic) {
                // check if the pixbuf is actually available
                GdkPixbuf *pixbuf = pixbuf_c::get_pixbuf(basic,  GTK_ICON_SIZE_DIALOG);
                if (pixbuf) return basic;
		else return "text-x-generic";
            } else {
                if (strstr(mimetype, "text/html")){
                    return "text-html";
                }
		return "text-x-generic";
            }
        }
#endif
        return "text-x-generic";
    }

    static gchar *
    get_home_iconname(const gchar *path){
        if (!path) return g_strdup("user-home");
        gchar *base = g_path_get_basename(path);
        const gchar *dir[]={N_("Documents"), N_("Downloads"),N_("Music"),N_("Pictures"),
                    N_("Templates"),N_("Videos"),N_("Desktop"),N_("Bookmarks"),
                    N_("Trash"),NULL};
        const gchar *icon[]={"folder-documents", "folder-download","folder-music","folder-pictures",
                      "folder-templates","folder-videos","user-desktop","user-bookmarks",
                      "user-trash",NULL};
        const gchar **p, **i;
        for (p=dir, i=icon; p && *p ; p++, i++){
            if (strcasecmp(*p, base) == 0) {
                g_free(base);
                return g_strdup(*i);
            }
        }
        g_free(base);
        return g_strdup("folder");
    }


    static gchar *
    extension(const gchar *base){
	auto extension = g_strdup("");
	if (strrchr(base, '.') && strrchr(base, '.') != base
		&& strlen(strrchr(base, '.')+1) <= EXTENSION_LABEL_LENGTH) {
	    extension = g_strconcat("*", strrchr(base, '.')+1, NULL) ;
	}
        return extension;
    }

    static gchar *
    getColor(const gchar *d_name){

        // hidden files:
         if (d_name[0] == '.') {
	    return g_strdup("#888888"); 
	}
        if (strcmp(d_name, "core")==0) {
	    return g_strdup("#880000"); 
        }

        // simple file extension coloring fallback
        const gchar *ext = strrchr(d_name, '.');
        if (!ext) return g_strdup("");
        if (cHdr(d_name)){
            return g_strdup("#eed680");
        }
        if (cSrc(d_name)){
            return g_strdup("#887fd3");
        }
        if (backupType(d_name)){
            return g_strdup("#cc7777");
        }
        return g_strdup("");
    }


    static gchar *
    statEmblem( const gchar *path, struct stat *st){
        if (!st){
            TRACE("statEmblem: no stat for st==NULL\n");
            return NULL;
        }
        if ((st->st_mode & S_IFMT) == S_IFDIR) {
            TRACE("dir emblem...\n");

            // all access:
            if (O_ALL(st->st_mode)){
                TRACE("all access: %s\n", path); 
                return NULL;
                //return g_strconcat(emblem, "/C/face-surprise/3.0/180", NULL);
            }
            if ((MY_GROUP(st->st_gid) && G_ALL(st->st_mode)) 
                    || (MY_FILE(st->st_uid) && U_ALL(st->st_mode))){
                TRACE("all access group: %s\n", path); 
                return NULL;
            }
            // read only:
            if (O_RX(st->st_mode) 
                    || (MY_GROUP(st->st_gid) && G_RX(st->st_mode)) 
                    || (MY_FILE(st->st_uid) && U_RX(st->st_mode))){
                TRACE("read only: %s\n", path); 
                return g_strdup("/NW/dialog-warning/3.0/180");
            }
            else {
                // no access:
                TRACE("no access: %s\n", path); 
                return g_strdup("/NW/dialog-error/3.0/180");
            }
        }
        // The rest is only for regular files (links too?)
        if ((st->st_mode & S_IFMT) != S_IFREG) return NULL;


	// all access:
	if (O_ALL(st->st_mode) || O_RW(st->st_mode)){
		return g_strdup("/C/face-surprise-symbolic/2.5/180/NW/application-x-executable-symbolic/3.0/180");
	// read/write/exec
	} else if((MY_GROUP(st->st_gid) && G_ALL(st->st_mode)) 
		|| (MY_FILE(st->st_uid) && U_ALL(st->st_mode))){
		return g_strdup("/NW/application-x-executable-symbolic/3.0/180");
	// read/exec
	} else if (O_RX(st->st_mode)
		||(MY_GROUP(st->st_gid) && G_RX(st->st_mode)) 
		|| (MY_FILE(st->st_uid) && U_RX(st->st_mode))){
		return g_strdup("/NW/application-x-executable-symbolic/3.0/180");

	// read/write
	} else if ((MY_GROUP(st->st_gid) && G_RW(st->st_mode))
		|| (MY_FILE(st->st_uid) && U_RW(st->st_mode))) {
		return NULL;

	// read only:
	} else if (O_R(st->st_mode) 
		|| (MY_GROUP(st->st_gid) && G_R(st->st_mode)) 
		|| (MY_FILE(st->st_uid) && U_R(st->st_mode))){
		return g_strdup("/NW/dialog-warning/3.0/130");
	} else if (S_ISREG(st->st_mode)) {
	    // no access: (must be have stat info to get this emblem)
	    return g_strdup("/NW/dialog-error/3.0/180");
	}
        return NULL;
    }

    static gchar *
    addEmblem(gchar *emblem, const gchar *newEmblem){
        if (!newEmblem) return emblem;
        if (!emblem) emblem = g_strdup(newEmblem);
        else {
            gchar *g = g_strconcat(emblem, newEmblem, NULL);
            g_free(emblem);
            emblem = g;
        }
        return emblem;
    }
    static gchar *
    getEmblem(const gchar *path, const gchar *basename, 
	    const unsigned char d_type, struct stat *st_p){
        // No emblem for go up
        if (strcmp(basename, "..")==0) return g_strdup("");
    
        gchar *emblem = NULL;

        gboolean is_lnk = (d_type == DT_LNK);
        // Symlinks:
        if (is_lnk) {
            if (g_file_test(path, G_FILE_TEST_EXISTS))
                emblem = g_strdup("/SW/emblem-symbolic-link/2.0/220");
            else
                emblem = g_strdup("/SW/emblem-unreadable/2.0/220");
	    if (FstabView<Type>::isMounted(path)){
		emblem = addEmblem(emblem, "/NW/greenball/3.0/180");
		return emblem;
	    }
        }

        if (RootView<Type>::isBookmarked(path)){
            emblem = g_strdup("/SE/bookmark-new/2.0/220");
        }

	gchar *clipEmblem = ClipBoard<Type>::clipBoardEmblem(path);
        emblem = addEmblem(emblem, clipEmblem);
        g_free(clipEmblem);

        if (FstabView<Type>::isMounted(path)){
            emblem = addEmblem(emblem, "/NW/greenball/3.0/180");
        } else if (FstabView<Type>::isInFstab(path)){
            emblem = addEmblem(emblem, "/NW/grayball/3.0/180");
        }

	//stat for all emblems? limit to d_types
	if (!emblem && st_p) {
	    emblem = statEmblem(path, st_p);
	}
	if (!emblem) emblem = g_strdup("");

        TRACE("getEmblem: %s --> %s\n", path, emblem);
        gchar *extend;
        if (d_type != DT_REG) extend = g_strdup("");
        else extend = extension(basename);
        TRACE("extend: %s --> %s\n", path, extend);
        auto color = getColor(basename);
        auto fullEmblem = g_strconcat(extend, color, emblem, NULL);

        //auto fullEmblem = addColors(xd_p, extend, emblem);
        g_free(color);
        g_free(emblem);
        g_free(extend);
        TRACE("fullEmblem: %s --> %s\n", path, fullEmblem);
	return fullEmblem;
    }

    static gchar *
    getEmblem(const gchar *path, struct stat *st){
	TRACE("getEmblem path st\n");
        // No emblem for go up
        gchar *base = g_path_get_basename(path);
        if (strcmp(base, "..")==0){
            g_free(base);
            return g_strdup("");
        }
        gchar *emblem = NULL;
        if ((st->st_mode & S_IFMT) == S_IFLNK){
            if (g_file_test(path, G_FILE_TEST_EXISTS))
                return g_strdup("/SW/emblem-symbolic-link/2.0/220");
            else
                return g_strdup("/SW/emblem-unreadable/2.0/220");
        }
        if (RootView<Type>::isBookmarked(path)){
            emblem = g_strdup("/SE/bookmark-new/2.0/220");
        }

	gchar *clipEmblem = ClipBoard<Type>::clipBoardEmblem(path);
        emblem = addEmblem(emblem, clipEmblem);
        g_free(clipEmblem);

        if (FstabView<Type>::isMounted(path)){
            emblem = addEmblem(emblem, "/NW/greenball/3.0/180");
        } else if (FstabView<Type>::isInFstab(path)){
            emblem = addEmblem(emblem, "/NW/grayball/3.0/180");
        }

        if (!emblem) emblem = statEmblem(path, st);
	if (!emblem) emblem = g_strdup("");
        TRACE("getEmblem: %s --> %s\n", path, emblem);

        gchar *extend;
        if ((st->st_mode & S_IFMT) == S_IFDIR) extend = g_strdup("");
        else extend = extension(base);
        TRACE("extend: %s --> %s\n", path, extend);
        auto color = getColor(base);
        TRACE("color: %s --> %s\n", path, color);
        auto fullEmblem = g_strconcat(extend, color, emblem, NULL);

        //auto fullEmblem = addColors(xd_p, extend, emblem);
        g_free(color);
        g_free(emblem);
        g_free(extend);
        TRACE("fullEmblem: %s --> %s\n", path, fullEmblem);
        g_free(base);
	return fullEmblem;
    }


};

}

#endif

