#ifndef TAR_HH
#define TAR_HH

namespace xf {

template <class Type>
class Tar {
   using subClass_t = tarResponse<Type>;
   using dialog_t = DialogComplex<subClass_t>;
   public:
   Tar(const char *folder, const char *path){
      auto dialogObject = new dialog_t(parent, folder, path);
   }

};

template <class Type>
class unTar {
   using subClass_t = tarResponse<Type>;
   using dialog_t = DialogComplex<subClass_t>;
   public:
   Tar(const char *folder, const char *path){
      auto dialogObject = new dialog_t(parent, folder, path);
   }

};
}
#endif

