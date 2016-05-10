#ifndef MLIB_UTIL_H
#define MLIB_UTIL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dslink/dslink.h"
#include <dslink/requester.h>
#include <dslink/log.h>
#include <dslink/utils.h>

#define MLIB_BUFFER_SIZE 100

#define TRUE 1
#define FALSE 0

typedef struct PubSubContext_ {
    uint32_t rid;
    char *topic;
    char *path;
    request_handler_cb cb;
    bool is_pub;
    bool is_sub;
    struct PubSubCxt *next;
} PubSubCxt;


bool
dslink_create_topic (DSLink *link, char *topic);
bool
dslink_delete_topic (DSLink *link, char *topic);

bool
mlib_pubsub_subscribe_topic(DSLink *link, const char *topic, value_sub_cb cb);
bool
mlib_pubsub_publish_topic(PubSubCxt *cxt, json_t *value);

#endif //MLIB_UTIL_H
