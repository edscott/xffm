#ifndef FILEDIALOG_HH
#define FILEDIALOG_HH

/* FIXME: when subdialog returns, it should send information to dialog to
 *        update the entry and to remove window from cleanup window list.
 *        */


namespace xf {

  // FIXME: probably need to create this class template object
  //        within the EfsResponse class...
  template <class Type>
  class FileDialog {
    public:
    static void newFileDialog(void **newDialog, const char *startFolder){
      DBG("*** newFileDialog1 startFolder = %s\n", startFolder);
      auto dialogObject = new DialogComplex<FileResponse<Type> >(startFolder);
      TRACE("newFileDialog12\n");

      //
      dialogObject->setParent(GTK_WINDOW(MainWidget));
      auto dialog = dialogObject->dialog();
      newDialog[0] = (void *)dialog;
      
      gtk_window_set_decorated(dialog, true);
      //dialogObject->setSubClassDialog();

      gtk_widget_realize(GTK_WIDGET(dialog));
      Basic::setAsDialog(GTK_WIDGET(dialog), "dialog", "Dialog");
      gtk_window_present(dialog);
      
      TRACE("FileDialog:: newDialog[0] = %p\n", newDialog[0]);

      dialogObject->run();
      

    }



  };


}
#endif

