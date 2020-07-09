#ifndef XML_HH
# define XML_HH
namespace xf
{
namespace XML {

class XmlNode{

private:
    
public:
    gint level;
    gchar *name;
    gchar *text;
    gchar **attNames;
    gchar **attValues;
    XmlNode *next;
    XmlNode *child;
    XmlNode *parent;

    XmlNode(gint in_level, const gchar *in_name, const gchar **aNames, const gchar **aValues):
        text(NULL), next(NULL), child(NULL),level(in_level)
    {
        name = g_strdup(in_name);
        gint aCount = 0;
        for (auto p=aNames; p && *p; p++) aCount++;
        attNames = (gchar **)calloc(aCount+1, sizeof(gchar *));
        attValues = (gchar **)calloc(aCount+1, sizeof(gchar *));
        auto n = attNames;
        auto v = attValues;
        auto q = aValues;
        for (auto p=aNames; p && *p; p++, q++, n++, v++) {
            *n = (*p)? g_strdup(*p): NULL;
            *v = (*q)? g_strdup(*q): NULL;
        }
    }
    ~XmlNode(void){
        auto p= attNames;
        auto q= attValues;
        for (; p && *p; p++){
            g_free(*p);
            g_free(*q);
        }
        g_free(name);
        g_free(attNames);
        g_free(attValues);
    }

    
}; // class XmlNode

//template <class Type>
class Xml{
    FILE *input_;
public:
    gint items_;

    gint level;
    XmlNode **lastNode;
    XmlNode *topNode;

    Xml(void): level(-1), items_(0){
        lastNode = (XmlNode **)calloc(256, sizeof(XmlNode *));
        
        if (!lastNode) throw 1;
    }


    gboolean init(const gchar *xmlFile){
        input_ = NULL;
        auto resourcePath = g_build_filename(PREFIX, "share", "xml", "xffm+", xmlFile, NULL);
        if (!g_file_test(resourcePath, G_FILE_TEST_EXISTS)){
            DBG("initXml(): %s %s\n", resourcePath, strerror(ENOENT));
            g_free(resourcePath);
            resourcePath = g_build_filename(buildXml, xmlFile, NULL);
            if (!g_file_test(resourcePath, G_FILE_TEST_EXISTS)){
                DBG("parseXMLfile(): %s %s\n", resourcePath, strerror(ENOENT));
                g_free(resourcePath);
                return FALSE;
            }
        }
        DBG("initXml(): %s\n", resourcePath);
        input_ = fopen (resourcePath, "r");
        g_free(resourcePath);
        if (!input_) return FALSE;
        return TRUE;
    }

    void close(void){ 
        if (input_) fclose(input_);
    }

    void
    parse(
            void (*start)(GMarkupParseContext *, const gchar *, 
                  const gchar **, const gchar **, gpointer, GError **), 
            void (*end)(GMarkupParseContext *, const gchar *, 
                  gpointer, GError **), 
            void (*text)(GMarkupParseContext *, 
                  const gchar *, gsize, gpointer, GError **)=NULL, 
            void (*passthrough)(GMarkupParseContext *,
                  const gchar *, gsize, gpointer, GError **)=NULL)
    {
        rewind(input_);
        GMarkupParser mainParser = { start, end, text, passthrough, NULL};
        auto mainContext = g_markup_parse_context_new (&mainParser, (GMarkupParseFlags)0, NULL, NULL);
        
        gchar line[2048];
        memset(line, 0, 2048);
        while(!feof (input_) && fgets (line, 2048, input_)) {
            //fprintf(stderr, "%s", line);
            GError *error = NULL;
            if (!g_markup_parse_context_parse (mainContext, line, strlen(line), &error) )
            {
                DBG("parse(): %s\n", error->message);
                g_error_free(error);
            }
        }

        g_markup_parse_context_free (mainContext);

    }


    
}; // class Xml<class Type>
Xml xml;



class XmlStructure {
    GSList *nodeList;

public:
    gint sweepItems;
    XmlNode *topNode(void){
        Xml& xml = XML::xml;
        return xml.topNode;
    }

    XmlStructure(const gchar *xmlFile){
        Xml& xml = XML::xml;
        if (!xml.init(xmlFile)){
            DBG("parseXMLfile() initProperties failed.\n");
            return;
        }
        xml.parse(startXML,endXML);
        xml.close();
        DBG("XmlStructure:: total source items=%d\n", xml.items_);
        sweepItems=0;
    }
    ~XmlStructure(void){

    }
        //XmlNode *node = topNode();

    void sweep(XmlNode *node, 
            void (*function)(XmlNode *, void *data)=NULL,
            void *data=NULL)
    {
        sweepItems++;
        TRACE("sweep: %s (%d)\n", node->name, node->level);
        if (function) (*function)(node, data);
        if (node->child) sweep(node->child, function, data);
        if (node->next) sweep(node->next, function, data);
    }

private:
    
    static void
    startXML (GMarkupParseContext * context,
                    const gchar * elementName,
                    const gchar ** attributeNames, 
                    const gchar ** attributeValues, 
                    gpointer functionData, 
                    GError ** error) 
    {
        Xml& xml = XML::xml;
        xml.items_++;
        
        xml.level++;
        if (xml.level > 255){
            DBG("Error: Xml nesting level limited to 256.\n");
            throw 1;
        }
        TRACE ("startXML -> %s (%d)\n",elementName, xml.level); 
        auto node = new(XML::XmlNode)(xml.level, elementName, attributeNames, attributeValues);

        if (((void *)(xml.lastNode[xml.level]))==NULL) {
            // First child detected. Second child will be sibling of the first.
            if (xml.level - 1 >= 0){
                xml.lastNode[xml.level-1]->child = node;
                node->parent = xml.lastNode[xml.level-1];
            } else {
                xml.topNode = node;
                node->parent = NULL;
            }
        } else {
            // Node is a sibling.
            xml.lastNode[xml.level]->next = node;
            node->parent = xml.lastNode[xml.level]->parent;

        }
        xml.lastNode[xml.level] = node;
        return;
    }


    static void endXML(GMarkupParseContext *context,
                              const gchar         *elementName,
                              gpointer             functionData,
                              GError             **error){
        Xml& xml = XML::xml;
        TRACE ("endXML -> %s (%d)\n",elementName, xml.level); 
        xml.level--;

     }
     
 
    /* text is not nul-terminated */
    static void mainText (GMarkupParseContext *context,
                              const gchar         *text,
                              gsize                text_len,
                              gpointer             functionData,
                              GError             **error){
        DBG ("mainText -> %s\n",text); 

    }
      /* text is not nul-terminated. */
    static void mainPassthrough (GMarkupParseContext *context,
                              const gchar         *passthrough_text,
                              gsize                text_len,
                              gpointer             functionData,
                              GError             **error)
    {
        DBG ("mainPassthrough -> %s\n",passthrough_text); 
        
    }
    static void mainError (GMarkupParseContext *context,
                              GError              *error,
                              gpointer             functionData)
    {
        DBG ("mainError -> %s\n",error->message); 
    }
    


};
} // namespace XML

}// namespace xf


#endif

