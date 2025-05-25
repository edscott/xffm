#ifndef FIND_HH
# define FIND_HH
namespace xf
{

template <class Type>
class Find {

    char *fullPath_ = NULL;
    gboolean gnuGrep_ = true;

public:

    Find(const gchar *path){
        if (!whichGrep()){
            ERROR("grep command not found\n");
            exit(1);
        }
        fullPath(path);
        gchar *fullPath = NULL;
        createDialog();
    }

    ~Find(void){
        g_free(fullPath_);
    }

    void fullPath(const char *path){
        if (!path || !g_file_test(path, G_FILE_TEST_EXISTS)) {
          auto current = g_get_current_dir();
          fullPath_ = realpath(current, NULL);
          g_free(current);
          return;
        }
        fullPath = realpath(path, NULL);
        return;
    }



private:

    gboolean whichGrep(void){
        gchar *grep = g_find_program_in_path ("grep");
        if (!grep) return false;
        FILE *pipe;
        const gchar *cmd = "grep --version";
        gnuGrep_ = false;
        pipe = popen (cmd, "r");
        if(pipe) {
            gchar line[256];
            memset (line, 0, 256);
            if(fgets (line, 255, pipe) == NULL){
                // Definitely not GNU grep. BSD version.
TRACE("pipe for \"grep --version\"\n"); 
                TRACE ("fgets: %s\n", strerror (errno));
    } else {
                if(strstr (line, "GNU")) gnuGrep_ = TRUE;
            }
            pclose (pipe);
        }
        g_free (grep);
        return true;
    }

    void createDialog(){

    }

};
} // namespace xf
#endif

