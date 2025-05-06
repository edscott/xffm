#ifndef BFISH_HH
#define BFISH_HH

namespace xf {

template <class Type>
class Bfish {
   using subClass_t = bfishResponse<Type>;
   using dialog_t = DialogComplex<subClass_t>;
   public:
   Bfish(GtkWindow *parent, const char *path){
      auto gpg = g_find_program_in_path("gpg");
      if (!gpg){
        auto message = g_strconcat(" ", _("Sorry, file not found"), ": gpg (from GnuPG)\n",NULL); 
        Print::printError(Child::getOutput(), message);
        throw(1);
      }
      auto dialogObject = new dialog_t(parent, Child::getWorkdir(), path);
   }

};
}
#endif

