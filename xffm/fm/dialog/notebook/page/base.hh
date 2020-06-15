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

    void
    setImageSize(gint pixels){ 
	auto hasKey = Settings<Type>::keyFileHasGroupKey("ImageSize", this->workDir());
	if (pixels < 48) {
	    if (hasKey) {
		Settings<Type>::removeKey("ImageSize", this->workDir());
	    }
	} 
	else if (pixels > MAX_PIXBUF_SIZE){
	    auto message = g_strdup_printf(" %s: (%d) [%s]\n",_("Maximum image size for thumbnailing"),  MAX_PIXBUF_SIZE, this->workDir());
	    Print<Type>::showTextSmall(this->output());
	    Print<Type>::print_icon(this->output(), "image-x-generic/SE/list-add/1.5/220", message);
	    return;
	}
	if (pixels >= 48) {
	    auto message = g_strdup_printf(" %s: (%d) [%s]\n",_("Reset image size"), pixels, this->workDir());
	    Print<Type>::showTextSmall(this->output());
	    Print<Type>::print_icon(this->output(), "image-x-generic/SE/list-add/1.5/220", message);
	    Settings<Type>::setSettingInteger("ImageSize", this->workDir(), pixels);
	}
    }
    
    gint
    getImageSize(void) {
	auto pixels = Settings<Type>::getSettingInteger("ImageSize", this->workDir());
	if (pixels < 0) pixels = 48;
	return pixels;
	//return this->imageSize_;
    }


};

}

#endif
