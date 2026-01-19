/* COMPILE

gcc `pkg-config --cflags gtk4` -o image_sample image_sample.c `pkg-config --libs gtk4`
*/
#include <gtk/gtk.h>

GdkTexture *load(const char *item){
  GError *error_ = NULL;
  if (!item) return NULL;
  if (g_file_test(item, G_FILE_TEST_EXISTS)) {
    GdkTexture *texture = gdk_texture_new_from_filename(item, &error_);
    if (error_){
      fprintf(stderr, "Texture::load(): %s\n", error_->message);
      return NULL;
    }
    return texture;
  }
}


int main (int argc, char *argv[])
{
  GtkWidget *MainWidget;
  GdkTexture *texture;
  GdkPaintable *tex2;
  GtkSnapshot *snapshot;
  gtk_init ();
    
  MainWidget = gtk_window_new ();
  texture = load("/usr/share/icons/Adwaita/scalable/mimetypes/application-certificate.svg");
  snapshot = gtk_snapshot_new();
  gdk_paintable_snapshot(GDK_PAINTABLE(texture), snapshot, 96.,96.);
  graphene_size_t size;
  size.width = size.height = 96.0;
  tex2 = gtk_snapshot_free_to_paintable(snapshot, &size);
  
  GtkWidget *image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
//  GtkWidget *image = gtk_image_new_from_paintable(GDK_PAINTABLE(tex2));

  
  gtk_window_set_child(GTK_WINDOW(MainWidget), image);
  gtk_window_present (GTK_WINDOW(MainWidget));

  while (g_list_model_get_n_items (gtk_window_get_toplevels ()) > 0)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}

