/*
 * Copyright 2005-2018 Edscott Wilson Garcia 
 * license: GPL v.3
 */

#define STRUCTURE_CC
#define URIFILE "file://"
#define PERL_PARSER "parse11.pl";
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <math.h>
# undef TRACE
# define TRACE(...)   { (void)0; }
//# define TRACE(...)  fprintf(stderr, "TRACE> "); fprintf(stderr, __VA_ARGS__);
# undef DBG
//# define DBG(...)   { (void)0; }
# define DBG(...)  {fprintf(stderr, "DBG> "); fprintf(stderr, __VA_ARGS__);}
# undef ERROR
# define ERROR(...)  {fprintf(stderr, "*** ERROR> "); fprintf(stderr, __VA_ARGS__);}
# undef WARN
# define WARN(...)  {fprintf(stderr, "warning> "); fprintf(stderr, __VA_ARGS__);}

#include <sys/wait.h>
#include <iostream>


gchar line[2048];
//static GtkIconTheme *icon_theme=NULL;
GdkPixbuf *focusPixbuf;

gchar *sourceFile;
gchar *templates;
gchar *extraIncludes=NULL;

GtkTreeIter *filesParent;
GtkTreePath *tpathParent;
GtkTreeIter fileChild;
GtkTreeIter *tmpParent=NULL;





} // End namespace xf
#include "treemodel.hh"
#include "structure.hh"
#include "signals.hh"
#include "parser.hh"


int
main (int argc, char *argv[]) {
    xf::Structure<double>  *structure;
    try {
        structure = new(xf::Structure<double>)(argv);
    } catch (int e){
        exit(1);
    }

/*    xf::TreeModel<double>  *treemodel;
    try {
        treemodel = new(xf::TreeModel<double>)(argv);
    } catch (int e){
        exit(1);
    }*/
 

//    parseXML(argv[1]?argv[1]:"structure.xml");
    parseXML(structure->xmlFile());
	
    
    gtk_init(&argc, &argv);
    xf::StructureWindow<xf::Structure<double>>  *structureWindow;
    try {
        structureWindow = new(xf::StructureWindow<xf::Structure<double>>)(structure);
    } catch (int e){
        exit(1);
    }


    gtk_widget_show_all(GTK_WIDGET(structureWindow->mainWindow));
    gtk_main();
    return 0;
}
