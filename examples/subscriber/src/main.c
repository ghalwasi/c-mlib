#define LOG_TAG "main"
#include <pthread.h>
#include <dslink/mlib_util.h>

int count = 1;
extern DSLink *link;

void on_req_close(struct DSLink *link, ref_t *req_ref, json_t *resp) {
    (void) link;
    (void) resp;
    (void) req_ref;
    json_t *rid = json_object_get(resp, "rid");
    printf("Request %i closed.\n", (int) json_integer_value(rid));
}

void configure_request(ref_t *ref) {
    RequestHolder *req = ref->data;
    req->close_cb = on_req_close;
}

void on_invoke_updates(struct DSLink *link, ref_t *req_ref, json_t *resp) {
    (void) link;
    (void) req_ref;
    char *data = json_dumps(resp, JSON_INDENT(2));
    printf("Got invoke %s\n", data);
    dslink_free(data);
}

void on_invoke_timer_fire(uv_timer_t *timer) {
    DSLink *link = timer->data;

    if (count == 3) {
        printf("We are done.\n");
        uv_timer_stop(timer);
        uv_close((uv_handle_t *) timer, (uv_close_cb) dslink_free);
        dslink_close(link);
        return;
    }

    json_t *params = json_object();
    json_object_set_new(params, "command", json_string("ls"));

    configure_request(dslink_requester_invoke(
        link,
        "/downstream/System/Execute_Command",
        params,
        on_invoke_updates
    ));

    configure_request(dslink_requester_set(
        link,
        "/data/c-sdk/requester/testNumber",
        json_real(rand())
    ));

    count++;
}

void on_list_update(struct DSLink *link, ref_t *req_ref, json_t *resp) {
    (void) link;
    RequestHolder *holder = req_ref->data;

    json_t *updates = json_object_get(resp, "updates");
    size_t index;
    json_t *value;

    const char* path = json_string_value(json_object_get(holder->req, "path"));

    printf("======= List %s =======\n", path);
    json_array_foreach(updates, index, value) {
        json_t *name = json_array_get(value, 0);
        json_t *val = json_array_get(value, 1);

        if (val->type == JSON_ARRAY || val->type == JSON_OBJECT) {
            char *data = json_dumps(val, JSON_INDENT(0));
            printf("%s = %s\n", json_string_value(name), data);
            dslink_free(data);
        } else if (val->type == JSON_STRING) {
            printf("%s = %s\n", json_string_value(name), json_string_value(val));
        } else if (val->type == JSON_INTEGER) {
            printf("%s = %lli\n", json_string_value(name), json_integer_value(val));
        } else if (val->type == JSON_REAL) {
            printf("%s = %f\n", json_string_value(name), json_real_value(val));
        } else if (val->type == JSON_NULL) {
            printf("%s = NULL\n", json_string_value(name));
        } else if (val->type == JSON_TRUE) {
            printf("%s = true\n", json_string_value(name));
        } else if (val->type == JSON_FALSE) {
            printf("%s = false\n", json_string_value(name));
        } else {
            printf("%s = (Unknown Type)\n", json_string_value(name));
        }
    }

    dslink_requester_close(link, (uint32_t) json_integer_value(json_object_get(resp, "rid")));
}

void on_value_update(struct DSLink *link, uint32_t sid, json_t *val, json_t *ts) {
    (void) link;
    (void) ts;
    (void) sid;
    printf("%s:Got value %f\n", __FUNCTION__,json_real_value(val));
    //dslink_requester_unsubscribe(link, sid);
}

void init(DSLink *dslink) {
    (void) dslink;
    link = dslink;

    log_info("Initialized!\n");
}

void connected(DSLink *link) {
    (void) link;
    log_info("Connected!\n");
}

void disconnected(DSLink *link) {
    (void) link;
    log_info("Disconnected!\n");
}

void requester_ready(DSLink *link) {
    //char t[100] = "system.devices.d1.sensors.s1"; 
    //(void)dslink_create_topic(link, t);
    //(void)dslink_delete_topic(link, t);
    //(void)mlib_pubsub_subscribe_topic(link, "system.devices.d1.sensors.s1", on_value_update);
    (void)mlib_pubsub_subscribe_topic(link, "system.devices.d1.sensors.s1", on_value_update);
    //dslink_requester_list(link, "/downstream", on_list_update);
    //dslink_requester_list(link, "/downstream/pppppppublisher", on_list_update);
    dslink_requester_list(link, "/downstream/publisher", on_list_update);
    dslink_requester_list(link, "/downstream/token", on_list_update);
}


void* thread_f1(void *ptr) {
     DSLinkCallbacks cbs = {
        init,
        connected,
        disconnected,
        requester_ready
     };
 
     
    printf("\nStarting DSlink\n");
    dslink_init(2, ptr, "subscriber", 1, 0, &cbs);
    printf("\nDSlink Stopped\n");
    pthread_exit(0);
}

int main(int argc, char **argv) {
    (void) argc;
    pthread_t id1;
    pthread_attr_t attr1;

    pthread_attr_init(&attr1);

    if (pthread_create(&id1, &attr1, thread_f1, argv)) {
        printf("Thread init failed");
    }

    pthread_join(id1, NULL);
}
