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
#include <sys/time.h>

#define MLIB_BUFFER_SIZE 255
#define TRUE 1
#define FALSE 0

static uint64_t 
mlib_util_current_time_in_msec(void) __attribute__((unused));

/*------------------------------------------------------------------
 |  Function: mlib_util_current_time_in_msec
 |
 |  Purpose: This function is used to find out the current system time 
 |           in milli seconds.
 |
 |  Parameters:
 |      void
 |
 |  Returns: uint64_t  
 *-------------------------------------------------------------------*/
static uint64_t 
mlib_util_current_time_in_msec(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    uint64_t msec = t.tv_sec * 1000 + t.tv_usec / 1000;
    return (msec);
}

// tuple context

#define FOREACH_DST(DST) \
        DST(SENSOR)      \
        DST(PROBE)       \
        DST(SERVICE)     \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum DATA_SOURCE_TYPE_ENUM {
    FOREACH_DST(GENERATE_ENUM)
};
static __attribute__((unused)) const char *DATA_SOURCE_TYPE_STRING[] = {
    FOREACH_DST(GENERATE_STRING)
};


static __attribute__((unused)) const char *TIMESTAMP = "timestamp";
static __attribute__((unused)) const char *DATA_SOURCE_TYPE = "sourceType";
static __attribute__((unused)) const char *DATA_SCHEMA_ID = "dataSchemaId";
static __attribute__((unused)) const char *SOURCE_ID = "sourceId";
static __attribute__((unused)) const char *DEVICE_ID = "deviceId";

typedef struct TupleContext_ {
    char *sourceId;
    char *dataSchemaId;
    uint64_t timestamp;
    enum DATA_SOURCE_TYPE_ENUM dataSourceType;
    char *deviceId;
} TupleContext;

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
mlib_pubsub_publish_topic(PubSubCxt *cxt, TupleContext *tcxt, json_t *value);

#endif //MLIB_UTIL_H
