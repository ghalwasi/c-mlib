#define LOG_TAG "c-mlib-util"
#include <dslink/mlib_util.h>

DSLink *link;

void 
on_req_close_ignore(struct DSLink *link, ref_t *req_ref, json_t *resp) 
{
    (void) link;
    (void) resp;
    (void) req_ref;
    json_t *rid = json_object_get(resp, "rid");
    log_info("%s:%d: Request %i closed.\n",__FUNCTION__,__LINE__,
             (int) json_integer_value(rid));
}

static void
configure_request(ref_t *ref) 
{
    RequestHolder *req = ref->data;
    req->close_cb = on_req_close_ignore;
}

static
void mlib_requester_ignore_response(DSLink *link, ref_t *req, json_t *resp) {
    (void) link;
    (void) resp;
    (void) req;
}

void 
mlib_topic_to_path(const char *topic, char **path)
{
    char *token = NULL;
    char *temp_topic;

    temp_topic = strdup(topic);

    strcpy(*path, "/data");
    token = strtok (temp_topic, ".");
    while (token != NULL)
    {
        strcat(*path, "/");
        strcat(*path,token);
        token = strtok (NULL, ".");
    }
}

bool
dslink_delete_topic (DSLink *link, char *topic)
{
    char *token = NULL;
    char *temp_path = NULL;
    json_t *params = NULL;
    uint32_t path_size = 0;
    ref_t *ref = NULL;

    /*
     * Check for NULL pointers.
     */
    if (!link || !topic) {
        log_err("%s:%d: DSLink or Topic can't be NULL\n", __FUNCTION__, __LINE__);
        return (FALSE);    
    }

    /*
     * Allocate dynamic memory for temporary variables.
     */
    path_size = strlen(topic) + MLIB_BUFFER_SIZE;
    temp_path = dslink_malloc(path_size);
    if (!temp_path) {
        log_err("%s:%d: dslink_malloc failed for temp_path\n", __FUNCTION__, __LINE__);
        return (FALSE);    
    }

    strcpy(temp_path, "/data/");
    token = strtok (topic, ".");
    while (token != NULL)
    {
        strcat(temp_path,token);
        strcat(temp_path, "/");
        token = strtok (NULL, ".");
    }
    strcat(temp_path, "deleteNode");

    log_info("%s:%d: Path to delete is %s\n", __FUNCTION__, __LINE__, temp_path);


    params = json_object();
    json_object_set(params, "Recursive", json_boolean(1));
    ref = dslink_requester_invoke(link, temp_path, params, mlib_requester_ignore_response);
    if (ref) {
        configure_request(ref);
    } else {
        log_err("%s:%d: dslink_requester_invoke failed\n", __FUNCTION__, __LINE__);
    }

    /*
     * Free up temporary dynamic allocated memory
     */
    dslink_free(temp_path);
    return (TRUE);
}

bool
dslink_create_topic (DSLink *link, char *topic)
{
    char *token = NULL;
    char *path = NULL;
    char *temp_path = NULL;
    json_t *params = NULL;
    uint32_t path_size = 0;
    ref_t *ref = NULL;

    /*
     * Check for NULL pointers.
     */
    if (!link || !topic) {
        log_err("%s:%d: DSLink or Topic can't be NULL\n", __FUNCTION__, __LINE__);
        return (FALSE);    
    }

    /*
     * Allocate dynamic memory for temporary variables.
     */
    path_size = strlen(topic) + MLIB_BUFFER_SIZE;
    temp_path = dslink_malloc(path_size);
    if (!temp_path) {
        log_err("%s:%d: dslink_malloc failed for temp_path\n", __FUNCTION__, __LINE__);
        return (FALSE);    
    }
    path = dslink_malloc(path_size);
    if (!path) {
        log_err("%s:%d: dslink_malloc failed for path\n", __FUNCTION__, __LINE__);
        dslink_free(temp_path);
        return (FALSE);    
    }

    strcpy(temp_path, "/data/");
    token = strtok (topic, ".");
    while (token != NULL)
    {
        memset(path, 0, path_size);
        sprintf(path, "%s%s", temp_path, "addValue");
        params = json_object();
        json_object_set(params, "Name", json_string(token));
        json_object_set_new_nocheck(params, "Type",
                                        json_string_nocheck("dynamic"));

        log_info("%s:%d: Path is %s, Token is %s\n", __FUNCTION__, __LINE__, path, token);
        ref = dslink_requester_invoke(link, path, params, mlib_requester_ignore_response);
        if (ref) {
            configure_request(ref);
        } else {
            log_err("%s:%d: dslink_requester_invoke failed\n", __FUNCTION__, __LINE__);
        }
        strcat(temp_path,token);
        strcat(temp_path, "/");
        token = strtok (NULL, ".");
    }

    /*
     * Free up temporary dynamic allocated memory
     */
    dslink_free(temp_path);
    dslink_free(path);
    return (TRUE);
}


void on_value_publish_update(struct DSLink *link, uint32_t sid, json_t *val, json_t *ts) {
    (void) link;
    (void) ts;
    (void) sid;
    printf("%s:Got value %f\n", __FUNCTION__, json_real_value(val));
    //dslink_requester_unsubscribe(link, sid);
}

static
void mlib_requester_response(DSLink *link, ref_t *req, json_t *resp) {
    (void) link;
    (void) resp;
    (void) req;
    char *data = json_dumps(resp, JSON_INDENT(2));
    printf("%s: Got invoke %s\n",__FUNCTION__,  data);
    dslink_free(data);
}


bool
mlib_pubsub_unsubscribe_topic(DSLink *link, RequestHolder *holder) {
    (void)link;
    (void)holder;
    // Extract the rid from holder and then call the following function with rid.
    //dslink_requester_close(link, 1);
    return (TRUE);
}


bool
mlib_pubsub_subscribe_topic(DSLink *link, const char *topic, value_sub_cb cb)
{
    (void)cb;
    char *temp_path = NULL;
    const char *t = NULL;
    char *p = NULL;
    uint32_t path_size = 0;
    json_t *params = NULL;
    ref_t *ref = NULL;

    /*
     * Check for NULL pointers.
     */
    if (!link || !topic) {
        log_err("%s:%d: DSLink or Topic can't be NULL\n", __FUNCTION__, __LINE__);
        return (FALSE);
    }

    t = topic;

    /*
     * Allocate dynamic memory for temporary variables.
     */
    path_size = strlen(topic) + MLIB_BUFFER_SIZE;
    temp_path = dslink_malloc(path_size);
    if (!temp_path) {
        log_err("%s:%d: dslink_malloc failed for temp_path\n", __FUNCTION__, __LINE__);
        return (FALSE);
    }

    log_info("%s:%d: Topic is %s\n", __FUNCTION__, __LINE__, topic);
    memset(temp_path, 0, path_size);
    p = temp_path;
    strcat(temp_path, "list /data/");
    p = p + 11;
    while (*t != '\0') {
         if (*t == '.') {
             *p = '/';
         } else if ((*t == '*') && (*(t+1) == '*')) {
             *p = *t;
             ++t;
         } else if (*t == '*') {
             *p = '?';
         } else {
             *p = *t;
         }
         ++p;
         ++t;
    }
    strcat(temp_path, "|subscribe");
    log_info("%s:%d: Path is %s\n", __FUNCTION__, __LINE__, temp_path);

    params = json_object();
    json_object_set(params, "query", json_string(temp_path));

    ref = dslink_requester_invoke(link, "/sys/query", params, mlib_requester_response);
    if (ref) {
        RequestHolder *holder;
        holder = ref->data;
        log_info("%s:%d: rid is %d\n", __FUNCTION__, __LINE__, holder->rid);
        configure_request(ref);
    } else {
        log_err("%s:%d: dslink_requester_invoke failed\n", __FUNCTION__, __LINE__);
    }

    /*
     * Free up temporary dynamic allocated memory
     */
    dslink_free(temp_path);
    return (TRUE);
}


/*------------------------------------------------------------------
 |  Function: mlib_pubsub_publish_topic 
 |
 |  Purpose: This function is used to publish data to a topic. 
 |
 |  Parameters:
 |      cxt (IN) This is a PubSubCxt.
 |      tcxt (IN) This is a TupleContext.
 |      value (IN) This is a value to be published. 
 |
 |  Returns: bool. TRUE if publish is sucessfull, FALSE otherwise.   
 *-------------------------------------------------------------------*/
bool
mlib_pubsub_publish_topic(PubSubCxt *cxt, TupleContext *tcxt, json_t *value)
{
    ref_t *ref = NULL;
    json_t *params = NULL;
    json_t *context = NULL;
    json_t *val = NULL;
    char *path = NULL;
    char *data_dump = NULL;

    /*
     * Check for NULL pointers.
     */
    if (!cxt || !value) {
        log_err("%s:%d: PubSub Context or Value can't be NULL\n", __FUNCTION__, __LINE__);
        return (FALSE);
    }

    /*
     * Prepare a JSON object for Tuple Context if it is present.
     */
    if (tcxt) {
        context = json_object();
        json_object_set(context, TIMESTAMP, json_integer((uint64_t)tcxt->timestamp));
        json_object_set(context, DATA_SOURCE_TYPE, json_string(DATA_SOURCE_TYPE_STRING[tcxt->dataSourceType]));
        json_object_set(context, DATA_SCHEMA_ID, json_string(tcxt->dataSchemaId));
        json_object_set(context, SOURCE_ID, json_string(tcxt->sourceId));
        json_object_set(context, DEVICE_ID, json_string(tcxt->deviceId));
    }

    /*
     * This is the case of first time publish to a topic. 
     */
    if (cxt->rid == 0) {
        path = (char *)dslink_malloc(MLIB_BUFFER_SIZE);
        mlib_topic_to_path(cxt->topic, &path);
        cxt->path = path;
        params = json_object();
        val = json_object();
        json_object_set(params, "Path", json_string(cxt->path));
        json_object_set(params, "Value", val);
        if (context) {
            json_object_set(val, "c", context);
        }
        json_object_set(val, "d", value);
        ref = dslink_requester_invoke(link, "/data/publish", params, cxt->cb);
        data_dump = json_dumps(params, JSON_INDENT(2));
        log_info("%s:%d: First time publish to path %s with params %s\n",
                 __FUNCTION__, __LINE__, cxt->path, data_dump);
        if (ref) {
            RequestHolder *holder;
            holder = ref->data;
            cxt->rid = holder->rid;
            configure_request(ref);
        } else {
            log_err("%s:%d: dslink_requester_invoke failed\n", __FUNCTION__, __LINE__);
            dslink_free(data_dump);
            return (FALSE);
        }
    } else {
        /*
         * This is a continuous publish case.
         */
        params = json_object();
        val = json_object();
        json_object_set(params, "Path", json_string(cxt->path));
        json_object_set(params, "Value", val);
        if (context) {
            json_object_set(val, "c", context);
        }
        json_object_set(val, "d", value);
        dslink_requester_invoke_update_params(link, cxt->rid, params);
        data_dump = json_dumps(params, JSON_INDENT(2));
        log_info("%s:%d: Continuous invoke for publish to path %s with params as %s\n",
                 __FUNCTION__, __LINE__, cxt->path, data_dump);
    }

    dslink_free(data_dump);
    return (TRUE);
}

