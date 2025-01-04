#ifndef MOUNT_HH
#define MOUNT_HH

namespace xf {

template <class Type>
class Mount {
   using subClass_t = mountResponse<Type>;
   using dialog_t = DialogComplex<subClass_t>;
   public:
   Mount(GtkWindow *parent, const char *folder, const char *path){
     TRACE("*** Mount...\n");
      auto dialogObject = new dialog_t(parent, folder, path);
    }
};
}
#endif

