#ifndef XF_TERM__HH
# define XF_TERM_HH
#include "dialog.hh"
namespace xf
{
namespace term
{


template <class Type>
class Term: protected TermDialog<Type> {
public:
    Term(const gchar *path){
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
