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
    getIconname(xd_t *xd_p){
	TRACE("getIconname(xd_)..\n");
	return 
	    getIconname(xd_p->path, 
		    xd_p->d_name,
		    xd_p->mimetype, 
		    xd_p->d_type, 
		    xd_p->st);
    }
private:
    static gchar *
    getIconname(const gchar *path, const gchar *basename, 
	    const gchar *mimetype, const unsigned char d_type,
	    struct stat *st_p){
	TRACE("getIconname(full)..\n");
        // Up directory:
        if (strcmp(basename, "..")==0) return  g_strdup("go-up");

	auto name = getBasicIconname(path, mimetype);
	if (!name){
	    ERROR("fm/view/icons.hh/::getBasicIconname should not return NULL\n");
	    return g_strdup("image-missing");
	}

	if (g_path_is_absolute(name)) return name; // image previews (no emblem)
	TRACE("basic iconname: %s --> %s\n", basename, name);
        gchar *emblem = getEmblem(path, basename,  d_type, st_p);
        TRACE("emblem: %s --> %s\n",  basename, emblem);
        gchar *iconname = g_strconcat(name, emblem, NULL);
        g_free(name);
        g_free(emblem);
        return iconname;
    }
public:
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
    getBasicIconname(const gchar *path, const gchar *mimetype){	
	TRACE("getBasicIconname(path, mimetype) mimetype=%s\n", mimetype);
	if (strcmp(path, g_get_home_dir())==0) return g_strdup("user-home");
	if (!mimetype) {
	    ERROR("fm/view/icons.hh/::getBasicIconname mimetype cannot be null\n");
	    return g_strdup("image-missing");
	}
	if (strcmp(mimetype, "inode/directory")==0) return  g_strdup("folder");

	// Block device
	if (strcmp(mimetype, "inode/blockdevice")==0) return g_strdup("drive-harddisk");
        
        // Character device:
	if (strcmp(mimetype, "inode/chardevice")==0) return  g_strdup("input-keyboard-symbolic");

        // Named pipe (FIFO):
	if (strcmp(mimetype, "inode/fifo")==0) return  g_strdup("network-wired-symbolic");

        // UNIX domain socket:
	if (strcmp(mimetype, "inode/socket")==0) return  g_strdup("network-wired-symbolic");

        auto iconname = specificIconName(path, mimetype);
        TRACE("specificIconName %s: %s \n", path, mimetype);
        if (iconname && !g_path_is_absolute(iconname)){
            if (!Icons<Type>::iconThemeHasIcon(iconname)) {
                DBG("LocalIcons::specificIconName %s not available\n", iconname);
                g_free(iconname);
                iconname=g_strdup("text-x-generic");
            }
        }
        if (iconname) return iconname;
        
        // Regular file:
	if (strcmp(mimetype, "inode/regular")==0) return g_strdup("text-x-preview");
        TRACE("getBasicIconname(): %s mime=%s \n", path, mimetype);
 	return g_strdup("dialog-question");
     }

    static gchar *
    specificIconName(const gchar *path, const gchar *mimetype){
	if (strstr(mimetype, "virtualbox")){
	    auto item = strrchr(mimetype, '-');
	    if (item) {
		auto iconname = g_strconcat("virtualbox", item, NULL);
		if (Icons<Type>::iconThemeHasIcon(iconname)) return iconname;
		g_free(iconname);
	    }
	}
	if (strstr(mimetype, "cd-image"))return g_strdup("media-optical");
        if (strstr(mimetype, "image")){
            if (isTreeView) return g_strdup("image-x-generic");
            if (Gtk<Type>::isImage(mimetype)) return g_strdup(path);
            return g_strdup("image-x-generic");
        }
        if (strstr(mimetype, "compressed")) return g_strdup("package-x-generic");
        if (strstr(mimetype, "x-xz")) return g_strdup("package-x-generic");
        if (strstr(mimetype, "audio")) return g_strdup("audio-x-generic");
        if (strstr(mimetype, "font")) return g_strdup("font-x-generic");
        if (strstr(mimetype, "video")) return g_strdup("video-x-generic");
        if (strstr(mimetype, "script")) return g_strdup("text-x-script");
        if (strstr(mimetype, "html")) return g_strdup("text-html");
        if (strstr(mimetype, "package")) return g_strdup("package-x-generic");
	// office stuff
	if (strstr(mimetype, "document")||strstr(mimetype, "application")){
            // N.A.:
            // if (strstr(mimetype, "")) return g_strdup("x-office-address-book");
            if (strstr(mimetype, "drawing") || strstr(mimetype, "graphics ")|| strstr(mimetype, "image")) {
                if (strstr(mimetype, "template")) return g_strdup("x-office-drawing-template");
                return g_strdup("x-office-drawing");
            }
            if (strstr(mimetype, "presentation")) {
                if (strstr(mimetype, "template")) return g_strdup("x-office-presentation-template");
                return g_strdup("x-office-presentation");
            }
            if (strstr(mimetype, "spreadsheet")) {
                if (strstr(mimetype, "template")) return g_strdup("x-office-spreadsheet-template");
                return g_strdup("x-office-spreadsheet");
            }
            if (strstr(mimetype, "template")) return g_strdup("x-office-document-template");
        }
        if (strstr(mimetype, "calendar")) return g_strdup("x-office-calendar");
	if (strstr(mimetype, "template")) return g_strdup("text-x-generic-template");
	if (strstr(mimetype, "text")) return g_strdup("text-x-generic");
	if (g_file_test(path, G_FILE_TEST_IS_EXECUTABLE)) return g_strdup("application-x-executable");
	if (strstr(mimetype, "application")){
	    if (strstr(mimetype, "pdf"))return g_strdup("x-office-document");
	    if (strstr(mimetype, "excell"))return g_strdup("x-office-spreadsheet");
	    if (strstr(mimetype, "word"))return g_strdup("x-office-document");
	    if (strstr(mimetype, "writer"))return g_strdup("x-office-document");
	    if (strstr(mimetype, "calc"))return g_strdup("x-office-spreadsheet");
	    if (strstr(mimetype, "lotus"))return g_strdup("x-office-spreadsheet");
	    if (strstr(mimetype, "draw"))return g_strdup("x-office-drawing");
	    if (strstr(mimetype, "dia"))return g_strdup("x-office-drawing");
	    if (strstr(mimetype, "presentation"))return g_strdup("x-office-presentation");
	    if (strstr(mimetype, "math"))return g_strdup("x-office-document-template");
	    if (strstr(mimetype, "lyx"))return g_strdup("x-office-document-template");

	}
        if (strstr(mimetype, "document")) return g_strdup("x-office-document");
        auto fileInfo = Util<Type>::fileInfo(path);
        if (fileInfo){
	    TRACE(" fileinfo: %s -> %s\n", path, fileInfo);
            gchar *iconname = NULL;
            if (strstr(fileInfo, "text") || strstr(fileInfo, "ASCII"))
                iconname =g_strdup("text-x-generic");
            if (strstr(fileInfo, "compressed") )
                iconname =g_strdup("package-x-generic");
            g_free(fileInfo);
            return iconname;
        }
	if (strstr(mimetype, "application")){
	    return g_strdup("text-x-generic-template");
	}
        return  NULL;
            
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
        ERROR("statEmblem is deprecated. Only emblem on d_type.\n");
#if 0
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
#endif
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
	    TRACE("link %s --> %s\n", path, realpath(path,NULL));
            if (g_file_test(path, G_FILE_TEST_EXISTS))
                emblem = g_strdup("/SW/emblem-symbolic-link/2.0/220");
            else
                emblem = g_strdup("/SW/emblem-unreadable/2.0/220");
	    if (FstabView<Type>::isMounted(path)){
		emblem = addEmblem(emblem, "/NW/greenball/3.0/220");
		return emblem;
	    }
        }

        if (RootView<Type>::isBookmarked(path)){
            emblem = g_strdup("/SE/bookmark-new/2.0/220");
        }

	gchar *clipEmblem = ClipBoard<Type>::clipBoardEmblem(path);
        emblem = addEmblem(emblem, clipEmblem);
        g_free(clipEmblem);

	if (d_type == DT_DIR || d_type == DT_BLK){

	    if (FstabView<Type>::isMounted(path)){
		emblem = addEmblem(emblem, "/NW/greenball/3.0/180");
	    } else if (FstabView<Type>::isInFstab(path)){
		emblem = addEmblem(emblem, "/NW/grayball/3.0/180");
	    }
	}

	//stat for all emblems? limit to d_types
	//if (!emblem && st_p) {
	//    emblem = statEmblem(path, st_p);
	//}
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
public:
    static unsigned char
    getDType (const gchar *path, struct stat *st_p) {
	TRACE("getDType path=%s mode= 0%o\n",path, st_p->st_mode); 
	if(S_ISSOCK (st_p->st_mode)) return DT_SOCK;
	else if(S_ISBLK (st_p->st_mode)) return DT_BLK;
	else if(S_ISCHR (st_p->st_mode)) return DT_CHR;
	else if(S_ISFIFO (st_p->st_mode)) return DT_FIFO;
	else if (S_ISLNK(st_p->st_mode)) return DT_LNK;
	else if(S_ISDIR (st_p->st_mode)) return DT_DIR;
	else if(S_ISREG (st_p->st_mode)) return DT_REG;
	return  DT_UNKNOWN;
    }
private:
    static gchar *
    getEmblem(const gchar *path, struct stat *st){
	auto basename = g_path_get_basename(path);
        gchar *emblem = getEmblem(path, basename,  getDType(path, st), st);
	g_free(basename);
	return emblem;
    }
};

}

#endif

