#ifndef PAGE_BASE
#define PAGE_BASE
#include "hbuttonbox.hh"
#include "pathbar.hh"
#include "vpane.hh"
#include "threadcontrol.hh"

namespace xf {

template <class Type>
class PageBase :
    public HButtonBox<double>,
//    public VButtonBox<double>,
    public Vpane<double>,
    public ThreadControl<double>,
    public Pathbar<double>,
    public Completion<double>
{
private:
    gchar *workDir_;

protected:

public:

    PageBase(void){
        workDir_ = g_strdup(g_get_home_dir());
    }

    const gchar *workDir(void){
        TRACE("workDir_ is %s\n", workDir_);
        return workDir_;
    }

    gboolean setWorkDir(const gchar *g){
	if (!g_file_test(g, G_FILE_TEST_IS_DIR)){
	    ERROR("dialog/notebook/page/base.hh::%s is not a directory\n", g);
	    return FALSE;
	}
        g_free(workDir_);
        workDir_ = g_strdup(g);
        return TRUE;
    }

};

}

#endif
