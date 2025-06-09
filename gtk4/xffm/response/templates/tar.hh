#ifndef TAR_HH
#define TAR_HH

namespace xf {

template <class Type>
class Tar {
   using subClass_t = tarResponse<Type>;
   using dialog_t = DialogComplex<subClass_t>;
   public:
   Tar(GtkWindow *parent, const char *path){
      // Figure out tar/zip
      auto dialogObject = new dialog_t(parent, Child::getWorkdir(), path);
   }

};
}
#endif

