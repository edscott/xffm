#ifndef XFFIND__HH
# define XFFIND__HH
#include "xfdialoggui.hh"
namespace xf
{


template <class Type>
class Find: protected FindDialog<Type> {
public:
    Find(const gchar *path){
	if (!this->whichGrep()){
	    std::cerr<<"DBG> grep command not found\n";
	    exit(1);
	}
	this->createDialog(path);
    }
};
}
#endif
