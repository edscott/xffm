#ifndef XF_LOCALICONS__HH
# define XF_LOCALICONS__HH

namespace xf
{
template <class Type> class FstabView;
template <class Type> class ClipBoard;
template <class Type>
class LocalIcons {
    using gtk_c = Gtk<Type>;
    using util_c = Util<Type>;
    
public:


    static GdkPixbuf *
    getIcon(const gchar *path, gint size = -24){
        auto directory = g_path_get_dirname(path);
        struct dirent d;
        auto basename = g_path_get_basename(path);
        strncpy(d.d_name, basename, 256);
        d.d_type = DT_UNKNOWN;
        auto xd_p = LocalModel<Type>::get_xd_p(directory, &d, TRUE);
            auto name = getIconname(xd_p);
        auto iconName = name?g_strdup(name):g_strdup("default");
        g_free(directory);
        g_free(basename);
        // Free xd_p
        LocalModel<Type>::free_xd_p(xd_p);

        auto pixbuf = Pixbuf<Type>::getPixbuf(iconName, size);
        g_free(iconName);
        return pixbuf;
    }

    static gchar *
    getIconname(xd_t *xd_p, gboolean doPreviews=FALSE){
        TRACE("getIconname(xd_)..\n");
        return 
            getIconname(xd_p->path, 
                xd_p->d_name,
                xd_p->mimetype, 
                xd_p->d_type, 
                xd_p->st,
                doPreviews);
    }
private:
    static gchar *
    getIconname(const gchar *path, const gchar *basename, 
            const gchar *mimetype, const unsigned char d_type,
            struct stat *st_p, gboolean doPreviews=FALSE){
        TRACE("getIconname(full)..\n");
        // Up directory:
        if (strcmp(basename, "..")==0) return  g_strdup(GO_UP);

        auto name = getBasicIconname(path, mimetype, doPreviews);
        if (!name){
            ERROR("fm/view/icons.hh/::getBasicIconname should not return NULL\n");
            return g_strdup("image-missing");
        }

        if (g_path_is_absolute(name)) return name; // image previews (no emblem)
        TRACE("getIconname(%s,%s) --> %s\n", basename, mimetype, name);
        gchar *emblem = getEmblem(path, basename,  d_type, st_p);
        TRACE("emblem: %s --> %s\n",  basename, emblem);
        gchar *iconname = g_strconcat(name, emblem, NULL);
        TRACE("iconname: --> %s\n",  iconname);
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

public:
    static gchar *
    getBasicIconname(const gchar *path, const gchar *mimetype, gboolean doPreviews=FALSE){        
        TRACE("getBasicIconname(path, mimetype) mimetype=%s\n", mimetype);
        if (strcmp(path, g_get_home_dir())==0) return g_strdup(USER_HOME);
        if (!mimetype) {
            ERROR("fm/view/icons.hh/::getBasicIconname mimetype cannot be null\n");
            return g_strdup("image-missing");
        }
        if (strcmp(mimetype, "inode/directory")==0) return  g_strdup(FOLDER);

        // Block device
        if (strcmp(mimetype, "inode/blockdevice")==0) return g_strdup(DRIVE_HARDDISK);
        
        // Character device:
        if (strcmp(mimetype, "inode/chardevice")==0) return  g_strdup(INPUT_KEYBOARD_SYMBOLIC);

        // Named pipe (FIFO):
        if (strcmp(mimetype, "inode/fifo")==0) return  g_strdup(NETWORK_WIRED_SYMBOLIC);

        // UNIX domain socket:
        if (strcmp(mimetype, "inode/socket")==0) return  g_strdup(NETWORK_WIRED_SYMBOLIC);
        //if (strcmp(mimetype, "inode/regular")==0) return g_strdup(DEFAULT_ICON);
        


        auto iconname = specificIconName(path, mimetype, doPreviews);
        struct stat st;
        if (iconname && !g_path_is_absolute(iconname) && stat(path, &st) == 0) {
            gchar *g = NULL; 
            if (st.st_mode & S_IXOTH) g = g_strconcat(iconname, "/SE/run/1.2/220", NULL);
            else { 
                if (st.st_mode & S_IXGRP){
                    g = g_strconcat(iconname, "/SE/run/1.5/220", NULL);
                } else {
                    if (st.st_mode & S_IXUSR) g = g_strconcat(iconname, "/SE/run/2.0/220", NULL);
                }
            }
            if (g) {
                g_free(iconname);
                iconname = g;
            }
        }

        TRACE("specificIconName %s: %s \n", path, mimetype);
        /*if (iconname && !g_path_is_absolute(iconname)){
            if (!Pixbuf<Type>::iconThemeHasIcon(iconname)) {
                DBG("LocalIcons::specificIconName %s not available\n", iconname);
                g_free(iconname);
                iconname=g_strdup(DEFAULT_ICON);
            }
        }*/
        if (iconname) return iconname;
        
        TRACE("getBasicIconname(): %s mime=%s \n", path, mimetype);
         return g_strdup(DEFAULT_ICON);
     }
private:
    static gchar *
    specificIconName(const gchar *path, const gchar *mimetype, gboolean doPreviews=FALSE){
        if (Gtk<Type>::isImage(mimetype, doPreviews)) {
            return g_strdup(path);
        }
        if (doPreviews && Pixbuf<Type>::isZipThumbnailed(path)){
            TRACE("LocalIcons::File \"%s\" is in zip format\n", path);
            return g_strdup(path);
        }
        TRACE("specificIconName(%s, %s)\n", path, mimetype);

        static const gchar *type1[] = {"image", "text", "audio", "font", "video", NULL};
        for (auto p=type1; p && *p; p++){
            if (strncmp(mimetype, *p, (size_t)strlen("*p"))==0){
                return g_strconcat(*p, "-x-generic", NULL);
            }

        }


        if (strcmp(mimetype, "inode/regular")==0) {
            return g_strdup(DEFAULT_ICON);
        }

        if (strncmp(mimetype, "model/",(size_t)strlen("model/"))==0){
                return g_strdup("applications-science");
        }

        if (strncmp(mimetype, "application/",(size_t)strlen("application/"))==0){
            auto type = mimetype + strlen("application/");
            if (strstr(type, "cd-image"))return g_strdup("media-optical");
            if (strstr(type, "audio")) return g_strdup("audio-x-generic");
            if (strstr(type, "font")) return g_strdup("font-x-generic");
            if (strstr(type, "video")) return g_strdup("video-x-generic");
            if (strstr(type, "script")) return g_strdup("text-x-script");
            if (strstr(type, "html")) return g_strdup("text-html");
            if (strstr(type, "calendar")) return g_strdup("x-office-calendar");

            if (strstr(type, "compressed") || 
                    strstr(type, "x-cbr") ||
                    strstr(type, "zip") ||
                    strstr(type, "x-xz") ||
                    strstr(type, "package")
            ) return g_strdup("package-x-generic");
            
            if (strstr(type, "pdf") ||
                strstr(type, "word") ||
                strstr(type, "writer") ||
                strstr(type, "lyx") ||
                strstr(type, "math")
            ) {
                if (strstr(type, "template")) 
                    return g_strdup("x-office-document-template");
                return g_strdup("x-office-document");
            }
            
            if (strstr(type, "excell") ||
                strstr(type, "calc") ||
                strstr(type, "lotus") ||
                strstr(type, "spreadsheet")
            ) {
                if (strstr(type, "template")) 
                    return g_strdup("x-office-spreadsheet-template");
                return g_strdup("x-office-spreadsheet");
            }

            if (strstr(type, "draw") ||
                strstr(type, "drawing") || 
                strstr(type, "graphics ") || 
                strstr(type, "image") ||
                strstr(type, "dia")
            ) {
                if (strstr(type, "template")) 
                    return g_strdup("x-office-drawing-template");
                return g_strdup("x-office-drawing");
            }
            if (strstr(type, "presentation")){
                if (strstr(type, "template")) 
                    return g_strdup("x-office-presentation-template");
                return g_strdup("x-office-presentation");
            }

            if (strstr(type, "x-trash")) return g_strdup(DEFAULT_ICON);
            return g_strdup("application-x-executable");


        }
        return  NULL;            
    }
private:
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
            return g_strdup("#999999");
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

#if 0
        ///// deprecated...
        //
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
        TRACE("icons.hh::getEmblem()...\n"); 
        gchar *emblem = NULL;

        gboolean is_lnk = (d_type == DT_LNK);
        // Symlinks:
        if (is_lnk) {
            TRACE("link %s --> %s\n", path, realpath(path,NULL));
            if (g_file_test(path, G_FILE_TEST_EXISTS))
                emblem = g_strdup("/SW/" SYMLINK "/2.0/220");
            else
                emblem = g_strdup("/SW/" EMBLEM_UNREADABLE "/2.0/220");
#ifdef ENABLE_FSTAB_MODULE
            if (FstabView<Type>::isMounted(path)){
                emblem = addEmblem(emblem, "/NW/" GREENBALL "/3.0/220");
                return emblem;
            }
#endif
        }

        if (RootView<Type>::isBookmarked(path)){
            emblem = g_strdup("/SE/" BOOKMARK_NEW "/2.0/220");
        }

        gchar *clipEmblem = ClipBoard<Type>::clipBoardEmblem(path);
        emblem = addEmblem(emblem, clipEmblem);
        g_free(clipEmblem);
        TRACE("emblem = %s\n", emblem);

#ifdef ENABLE_FSTAB_MODULE
        if (d_type == DT_DIR || d_type == DT_BLK){

            if (FstabView<Type>::isMounted(path)){
                emblem = addEmblem(emblem, "/NW/" GREENBALL "/3.0/180");
            } else if (FstabView<Type>::isInFstab(path)){
                emblem = addEmblem(emblem, "/NW/" GRAYBALL "/3.0/180");
            }
        }
#endif

        if (!emblem && st_p) {
            // for directories...
            emblem = statEmblem(path, st_p);
        }
        if (!emblem) emblem = g_strdup("");

        TRACE("getEmblem: %s --> %s\n", path, emblem);
        gchar *extend;
        extend = extension(basename);
        /*if (d_type != DT_REG) extend = g_strdup("");
        else extend = extension(basename);*/
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

