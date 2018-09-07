#ifndef PAGE_BASE
#define PAGE_BASE

namespace xf {

template <class Type>
class PageBase {
private:
    gchar *workDir_;

protected:

public:

    PageBase(void){
        workDir_ = NULL;
    }

    const gchar *workDir(void){
        DBG("workDir_ is %s\n", workDir_);
        return workDir_;
    }

    gboolean setWorkDir(const gchar *g){
        g_free(workDir_);
        workDir_ = g_strdup(g);
        return g_file_test(workDir_, G_FILE_TEST_IS_DIR);
    }

};

}

#endif
