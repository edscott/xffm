#ifndef XML_HH
# define XML_HH
namespace xf
{
namespace XML {
    GList *nodeList=NULL;
    GList *parentList=NULL;
    gboolean textMode=FALSE;
    gboolean passthroughMode=FALSE;

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
    struct stat st_;
    void *structure_;
public:
    gint items;

    gint level;
    XmlNode **lastNode;
    XmlNode *topNode;
    
    void *structure(void){return structure_;}

    Xml(void): level(-1), items(0){
        // 255 node levels, more than enough.
        lastNode = (XmlNode **)calloc(256, sizeof(XmlNode *));
        
        if (!lastNode) throw 1;
    }

    ~Xml(void){
        g_free(lastNode);
    }


    gboolean init(const gchar *xmlFile, void *structure){
        input_ = NULL;
        structure_ = structure;
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
        struct stat st;
        stat(resourcePath, &st_);

        
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
        
        auto buffer = (gchar *)calloc(1, st_.st_size);
        if (!buffer){
            DBG("calloc: %s\n", strerror(errno));
            throw 1;
        }
            
        GError *error = NULL;
        

        fread(buffer, st_.st_size, 1, input_);
        if (!g_markup_parse_context_parse (mainContext, buffer, st_.st_size, &error) )
        {
            DBG("parse(): %s\n", error->message);
            g_error_free(error);
        }
        g_free(buffer);

/*
        gchar line[2048];
        memset(line, 0, 2048);
        while(!feof (input_) && fgets (line, 2048, input_)) {
            //fprintf(stderr, "%s", line);
            if (!strlen(line)) continue;
            if (!g_markup_parse_context_parse (mainContext, line, strlen(line), &error) )
            {
                DBG("parse(): %s\n", error->message);
                g_error_free(error);
            }
        }
*/
        g_markup_parse_context_free (mainContext);

    }


    
}; // class Xml<class Type>
Xml xml;

class XmlStructure {
    gint sweepCount_;

public:
    void initSweepCount(void){sweepCount_=0; }
    gint sweepCount(void){return sweepCount_;}
//    gint items(void){return XML::xml.items;}

    XmlNode *topNode(void){
        Xml& xml = XML::xml;
        return xml.topNode;
    }

    XmlStructure(const gchar *xmlFile){
        Xml& xml = XML::xml;
        if (!xml.init(xmlFile, (void *)this)){
            DBG("parseXMLfile() initProperties failed.\n");
            return;
        }
        xml.parse(startXML, endXML, textXML);
        xml.close();
        DBG("XmlStructure:: total source items=%d\n", xml.items);
        sweepCount_=0;
    }
    ~XmlStructure(void){
        // FIXME:
        // Get a list of all nodes
        // Free every item of the list
        // free list

    }
        //XmlNode *node = topNode();

    const gchar *
    getText(XmlNode *node){
        if (!node) return NULL;
        return node->text;
    }
    
    XmlNode *
    getSubNode(XmlNode *parent, const gchar *name, const gchar *value){
        if (!parent || !parent->child) return NULL;
        auto node = parent->child;
        do {
            auto v = getAttribute(node, name);
            DBG("%s: %s->%s (%s)\n", node->name, name, v, value);
            if (v && strcmp(value, v)==0) return node;
            node = node->next;
        } while (node);
        return node;
    }

    GList *getList(const gchar *tag, const gchar *subtag){
        const gchar *data[] = {tag, subtag, NULL};
        auto countActions = countSweepItems(this,(void *)data);
        GList *list = NULL;
        for (auto l=XML::nodeList; l && l->data; l=l->next){
            list = g_list_prepend(list, l->data);
        }
        list = g_list_reverse(list);
        return list;
    }


    const gchar *
    getAttribute(XmlNode *node, const gchar *attribute){
        auto p = node->attNames;
        auto q = node->attValues;
        for (p = node->attNames;p && *p; p++, q++){
            if (strcmp(*p, attribute)==0 && *q){
                return *q;
            }
        }
        return NULL;
    }


    void sweep(XmlNode *node, 
            gboolean (*function)(XmlNode *, void *data)=NULL,
            void *data=NULL)
    {
        
        TRACE("sweep: %s (%d)\n", node->name, node->level);
        if (function) {
            auto result = (*function)(node, data);
            if (result) sweepCount_++;
        }
        if (node->child) sweep(node->child, function, data);
        if (node->next) sweep(node->next, function, data);
    }

private:
 
    static void 
    initLists(XML::XmlStructure *xmlStructure){
        if (XML::parentList) {
            g_list_free(XML::parentList);
            XML::parentList = NULL;
        }
        if (XML::nodeList) {
            g_list_free(XML::nodeList);
            XML::nodeList = NULL;
        }
        xmlStructure->initSweepCount();
    }
   
    static gint 
    countSweepItems(XML::XmlStructure *xmlStructure, void *data){
        initLists(xmlStructure);
        auto topNode = xmlStructure->topNode();
        xmlStructure->sweep(topNode, countItems, data);

        auto tags = (const gchar **)data;
        DBG("countSweepItems(): %s/%s count = %d/%d parents = %d nodelist=%d\n",
                tags[0], tags[1],
                xmlStructure->sweepCount(),
                XML::xml.items,
                g_list_length(XML::parentList),
                g_list_length(XML::nodeList));
        return xmlStructure->sweepCount();
    }
    static gint 
    countChildlessItems(XML::XmlStructure *xmlStructure, void *data){
        initLists(xmlStructure);
        auto topNode = xmlStructure->topNode();
        xmlStructure->sweep(topNode, countChildless, data);

        auto tags = (const gchar **)data;
        DBG("countChildlessItems(): %s/%s count = %d/%d parents = %d nodelist=%d\n",
                tags[0], tags[1],
                xmlStructure->sweepCount(),
                XML::xml.items,
                g_list_length(XML::parentList),
                g_list_length(XML::nodeList));
        return xmlStructure->sweepCount();
    }

    // check for childless nodes
    static gboolean
    checkLevel1(XML::XmlNode *node, const gchar **tags){
        if (!checkLevel2(node, tags)) return FALSE;
        // Childless:
        if (node->child) return FALSE;

        TRACE("parent: %s (%p) childless %s %p\n",
                node->parent->name,  node->parent,
                node->name,  node);
        return TRUE;
    }

    static gboolean
    checkLevel2(XML::XmlNode *node, const gchar **tags){
        if (!node || !tags) return FALSE;
        if (!node->parent || !node->parent->name) return FALSE;
        if (strcmp(node->parent->name, tags[0])) return FALSE;
        if (strcmp(node->name, tags[1])) return FALSE;
        TRACE("parent: %s (%p)\n",node->parent->name,  node->parent);
        if (!XML::parentList || !g_list_find (XML::parentList, node->parent)){
            XML::parentList = g_list_append(XML::parentList, node->parent);
        }
        return TRUE;
    }

    static gboolean countItems(XML::XmlNode *node, void *data){
        auto tags = (const gchar **)data;
        if (!checkLevel2(node, tags)) return FALSE;
        //printNode(node);
        XML::nodeList = g_list_append(XML::nodeList, node);

        return TRUE;        
    }

    static gboolean countChildless(XML::XmlNode *node, void *data){
        auto tags = (const gchar **)data;
        if (!checkLevel1(node, tags)) return FALSE;
        //printNode(node);
        XML::nodeList = g_list_append(XML::nodeList, node);
        TRACE("appended %p to nodelist\n", node);

        return TRUE;        
    }


    static void
    startXML (GMarkupParseContext * context,
                    const gchar * elementName,
                    const gchar ** attributeNames, 
                    const gchar ** attributeValues, 
                    gpointer functionData, 
                    GError ** error) 
    {
        Xml& xml = XML::xml;
        xml.items++;
        
        if (xml.level > 254){
            DBG("Error: Xml nesting level limited to 256.\n");
            throw 1;
        }
        TRACE ("startXML -> %s (%d)\n",elementName, xml.level); 
        auto node = new(XML::XmlNode)(xml.level, elementName, attributeNames, attributeValues);

        if (xml.level < 0) {
            xml.topNode = node;
        }
        xml.level++;

        // Parent is last node at previous level.
        if (xml.topNode == node) {
            node->parent = NULL;
        } else {
            node->parent = xml.lastNode[xml.level-1];
        }

        if (node->parent && !node->parent->child) {
            // First child.
            node->parent->child = node;
        } else if (node != xml.topNode ){
            // Sibling.
            xml.lastNode[xml.level]->next = node;
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
    static void textXML (GMarkupParseContext *context,
                              const gchar         *text,
                              gsize                text_len,
                              gpointer             functionData,
                              GError             **error){
        Xml& xml = XML::xml;
        // Text not null terminated, so move to terminated buffer.
        gchar buffer[text_len+1];
        memset(buffer, 0, text_len+1);
        memcpy(buffer, text, text_len);
        auto node = xml.lastNode[xml.level];
        node->text = g_strdup(buffer);
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

