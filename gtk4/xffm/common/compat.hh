#ifndef XFCOMPAT_HH
#define XFCOMPAT_HH
//#warning "Compiling compat.hh"
namespace xf
{

template <class Type>
class compat {
public:
  static void boxPack0(  
      GtkBox* box,
      GtkWidget* child,
      gboolean expand,
      gboolean fill,
      guint padding)
  {
    GtkOrientation orientation = gtk_orientable_get_orientation(GTK_ORIENTABLE(box));
#ifdef USE_GTK4
    gtk_box_append(box, child);
#else
    gtk_container_add(GTK_CONTAINER(box), child);
#endif
//   gtk_box_pack_start(box, child, FALSE, FALSE, 0);

//    gtk_box_pack_start(box, child, expand, fill, 0);
//    GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET(box));
      
    //gtk_widget_set_halign (GTK_WIDGET(child),fill?GTK_ALIGN_FILL: GTK_ALIGN_CENTER);
    gtk_widget_set_halign (GTK_WIDGET(child),fill?GTK_ALIGN_FILL: GTK_ALIGN_START);
    // other options: GTK_ALIGN_START, GTK_ALIGN_END, GTK_ALIGN_BASELINE
    if (orientation == GTK_ORIENTATION_HORIZONTAL){
      gtk_widget_set_hexpand(GTK_WIDGET(child), expand);
      gtk_widget_set_margin_start(GTK_WIDGET(child), padding);
      gtk_widget_set_margin_end(GTK_WIDGET(child), padding);
    } else if (orientation == GTK_ORIENTATION_VERTICAL){
      gtk_widget_set_vexpand(GTK_WIDGET(child), expand);
      gtk_widget_set_margin_top(GTK_WIDGET(child), padding);
      gtk_widget_set_margin_bottom(GTK_WIDGET(child), padding);
    } else {
      fprintf(stderr, "boxPackStart(): programming error. Exit(2)\n");
      exit(2);
    }
    
  }
  static void boxPackStart(  
      GtkBox* box,
      GtkWidget* child,
      gboolean expand,
      gboolean fill,
      guint padding)
  {
    boxPack0(box,child,expand,fill,padding);
  }
  static void boxPack1(  
      GtkBox* box,
      GtkWidget* child,
      gboolean expand,
      gboolean fill,
      guint padding)
  {
    boxPack0(box,child,expand,fill,padding);
  }
};
}

#endif
