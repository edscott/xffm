#ifndef BFISH_HH
#define BFISH_HH

namespace xf {

template <class Type>
class Bfish {
   using subClass_t = bfishResponse<Type>;
   using dialog_t = DialogComplex<subClass_t>;
   public:
   Bfish(GtkWindow *parent, const char *path){
      auto dialogObject = new dialog_t(parent, Child::getWorkdir(), path);
   }

};
}
#endif

