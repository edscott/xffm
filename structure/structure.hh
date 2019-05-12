#ifndef STRUCTURE_HH
#define STRUCTURE_HH
namespace xf {
template <class Type>
class Structure : public TreeModel<Type>{
    gchar *xmlFile_;
    gchar *parserPath_;
public:
    ~Structure(void){
        g_free(xmlFile_);
        g_free(parserPath_);
    }
    Structure(gchar **argv){
        auto parser=PERL_PARSER;
        parserPath_ = g_find_program_in_path(parser);
        if (!parserPath_){
            std::cerr<<"Cannot find "<<parser<<" in path\n";
            throw 1;
        }
        // Get source file, template directory and include directory options
        auto sourceFile = getSourceFile(argv);
        int status;
        auto pid = fork();
        if (pid) wait(&status);
        else {
            gchar *a[10];
            int i=0;
            a[i++] = (gchar *)"/usr/bin/perl";
            a[i++] = (gchar *)parserPath_;
            a[i++] = (gchar *)sourceFile;
            for (auto p=argv+1; p && *p && i<9; p++){
                if (strstr(*p, "--"))a[i++] = (gchar *)*p;
            }
            a[i] = NULL;
            for (auto j=0; j<i; j++)fprintf(stderr,"%s ", a[j]); fprintf(stderr,"\n");
            execvp("/usr/bin/perl", a);
        }
        fprintf(stderr, "OK\n");

        xmlFile_ = g_strdup("structure.xml");
    }

    const gchar *xmlFile(void){return xmlFile_;}
private:
   const gchar *getSourceFile(gchar **argv){
        const gchar *retval=NULL;
        for (auto p=argv+1; p && *p; p++){
            if (strstr(*p, "--"))continue;
            if (!g_file_test(*p, G_FILE_TEST_IS_REGULAR)){
                std::cerr<< *p << "is not a regular file\n";
                throw 2;
            }
            retval = *p;
        }
        if (!retval) throw 3;
        return retval;
    }

};
}


#endif
