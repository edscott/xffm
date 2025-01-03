#ifndef MOUNT_HH
#define MOUNT_HH

namespace xf {

template <class Type>
class Mount {
   using dialog_t = DialogComplex<mountResponse<Type> >;
   public:
   Mount(GtkWindow *parent, const char *folder, const char *path){
     TRACE("*** Mount...\n");
      auto dialogObject = new dialog_t(parent, folder, path);
    }
};
}
#endif

