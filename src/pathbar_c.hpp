#ifndef PATHBAR_C_HPP
#define PATHBAR_C_HPP
#include "utility_c.hpp"

class pathbar_c: utility_c {
    public:
	pathbar_c(void);
	GtkWidget *get_pathbar(void);
	void update_pathbar(GtkWidget *, const gchar *);
    protected:
    private:
	GtkWidget *pathbar;

};

#endif
