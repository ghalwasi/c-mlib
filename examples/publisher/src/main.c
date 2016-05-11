#define LOG_TAG "main"
#include <pthread.h>
#include <dslink/mlib_util.h>
#include <dslink/log.h>
#include <dslink/requester.h>

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


void on_value_update(struct DSLink *link, uint32_t sid, json_t *val, json_t *ts) {
    (void) link;
    (void) ts;
    (void) sid;
    printf("%s:Got value %f\n", __FUNCTION__,json_real_value(val));
    //dslink_requester_unsubscribe(link, sid);
}

static void on_value_publish_update(struct DSLink *link, ref_t *req, json_t *resp) {
    (void) link;
    (void) req;
    char *data = json_dumps(resp, JSON_INDENT(2));
    printf("%s: Got invoke %s\n", __FUNCTION__, data);
    dslink_free(data);
}

void init(DSLink *dslink) {
    (void) dslink;
    link = dslink;
    json_t * linkData = json_object();
    json_object_set_nocheck(linkData, "mlib-service", json_string("publisher"));
    dslink->linkData = linkData;
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

TupleContext *tcxt = NULL;
PubSubCxt *cxt = NULL;
void on_invoke_timer_fire(uv_timer_t *timer) {
    PubSubCxt *cxt = timer->data;

    double x = rand() / 1000000.0;
    json_t *v1 = json_integer((int)x);
    json_t *v2 = json_integer((int)x+1);
    json_t *v = json_array();
    json_t *val = json_array();
    json_t *values = json_object();
    json_array_append_new(v, v1);
    json_array_append_new(v, v2);
    json_object_set(values, "values", v);
    json_array_append_new(val, values);
    tcxt->timestamp = mlib_util_current_time_in_msec();
    (void)mlib_pubsub_publish_topic(cxt, tcxt, val);
}

void requester_ready(DSLink *link) {

    char t[100] = "system.devices.d1.sensors.s1"; 
    //(void)dslink_create_topic(link, t);

    cxt = dslink_malloc (sizeof(PubSubCxt));
    if (!cxt) {
        printf("\n Error!!! malloc failed");
        return;
    }
    cxt->topic = dslink_strdup(t);
    cxt->cb = on_value_publish_update;
    cxt->is_pub = 1;

    tcxt = dslink_malloc (sizeof(TupleContext));
    if (!tcxt) {
        printf("\n Error!!! malloc failed");
        return;
    }
    tcxt->sourceId = dslink_strdup("GPSSensor");
    tcxt->dataSchemaId = dslink_strdup("GPSSchema");
    tcxt->timestamp = mlib_util_current_time_in_msec();
    tcxt->dataSourceType = SENSOR;
    tcxt->deviceId = dslink_strdup("DeviceA");
    
    uv_timer_t *timer = malloc(sizeof(uv_timer_t));
    timer->data = cxt;
    uv_timer_init(&link->loop, timer);
    uv_timer_start(timer, on_invoke_timer_fire, 0, 5000);
}


void* thread_f1(void *ptr) {
     DSLinkCallbacks cbs = {
        init,
        connected,
        disconnected,
        requester_ready
     };
 
     
    printf("\nStarting DSlink\n");
    dslink_init(2, ptr, "publisher", 1, 1, &cbs);
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
