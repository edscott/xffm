#ifndef PATHRESPONSE_HH
#define PATHRESPONSE_HH
// template dependencies

namespace xf {

template <class PathOp>
class pathResponse {
public:

    static void *asyncYesArg(void *data, const char *op){
      auto output = Child::getOutput();
      if (!op) {
        Print::printError(output, g_strdup(" pathResponse<>::asyncYesArg: command is NULL\n"));
        return NULL; // should not happen.
      }
      auto op_f = g_find_program_in_path(op);
      if (!op_f) {
        Print::printError(output, g_strdup_printf("*** Error: %s not found\n", op));
        return NULL; // Program not found.
      }
      g_free(op_f);
      PathOp::asyncYes(data, op, output);
      return NULL;
    }

    static void *asyncNo(void *data){
      auto dialogObject = (DialogEntry<pathResponse<PathOp> > *)data;
      auto dialog = dialogObject->dialog();
      auto entry = GTK_ENTRY(g_object_get_data(G_OBJECT(dialog), "entry"));
      auto path = g_object_get_data(G_OBJECT(entry), "path");
      g_free(path);

      return NULL;
    }
};

}
#endif
