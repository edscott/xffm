#ifndef XFFIND__HH
# define XFFIND__HH
#include "dialog.hh"
namespace xf
{
namespace find
{


template <class Type>
class Find: protected FindDialog<Type> {
public:
    Find(const gchar *path){
	if (!this->whichGrep()){
	    std::cerr<<"DBG> grep command not found\n";
	    exit(1);
	}
        gchar *fullPath = NULL;
        if (path){
            if (g_path_is_absolute(path)) fullPath = g_strdup(path);
            else fullPath = g_build_filename(g_get_home_dir(), path, NULL);
        }
	this->createDialog(fullPath);
        g_free(fullPath);
    }
};
} // namespace find
} // namespace xf
#endif
