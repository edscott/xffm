#ifndef MAP_HH
#define MAP_HH
namespace xf
{
  class Map {
    GList *List_ = NULL;
    GList *dialogList_ = NULL;
    GHashTable *dialogHash_ = NULL;

  public:

    Map(void){
      dialogHash_ = g_hash_table_new(g_hash_direct, g_hash_direct_equal);
    }

    ~Map(void){
      g_hash_table_destroy(dialogHash_);
      for (auto l=dialogList_; l && l->data; l=l->next){
      }
    }

    void push(GtkWindow *dialog, GtkWindow *parent){
      GList place = g_list_find(listList_, parent);
      if (!place){
        GList *parentList = NULL;
        GList parentList = g_list_append(NULL, 
      dialogList_ = g_list_prepend(dialogList_, dialog);
    }

    void pop(GtkWindow *dialog){
      dialogList_ = g_list_remove(dialogList_, dialog);
    }
    
  };

}
#endif
