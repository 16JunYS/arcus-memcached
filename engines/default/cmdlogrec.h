/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * arcus-memcached - Arcus memory cache server
 * Copyright 2019 JaM2in Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MEMCACHED_CMDLOGREC_H
#define MEMCACHED_CMDLOGREC_H

enum log_type {
    LOG_IT_LINK = 0,
    LOG_IT_UNLINK,
#ifdef ENABLE_PERSISTENCE_03_CMDLOG_KV
#else
    LOG_IT_ARITHMETIC,
#endif
    LOG_IT_SETATTR,
#ifdef ENABLE_PERSISTENCE_03_CMDLOG_KV
    LOG_IT_FLUSH,
#endif
    LOG_LIST_ELEM_INSERT,
    LOG_LIST_ELEM_DELETE,
    LOG_SET_ELEM_INSERT,
    LOG_SET_ELEM_DELETE,
    LOG_MAP_ELEM_INSERT,
    LOG_MAP_ELEM_DELETE,
    LOG_BT_ELEM_INSERT,
    LOG_BT_ELEM_DELETE,
    LOG_BT_ELEM_ARITHMETIC,
    LOG_SNAPSHOT_HEAD,
    LOG_SNAPSHOT_TAIL
};

enum upd_type {
    /* key value command */
#ifdef ENABLE_PERSISTENCE_03_CMDLOG_KV
    UPD_SET = 0,
#else
    UPD_ADD = 0,
    UPD_SET,
    UPD_REPLACE,
    UPD_APPEND,
    UPD_PREPEND,
    UPD_CAS,
    UPD_INCR,
    UPD_DECR,
#endif
    UPD_DELETE,
#ifdef ENABLE_PERSISTENCE_03_CMDLOG_KV
    UPD_SETATTR_EXPTIME,
    UPD_SETATTR_EXPTIME_INFO,
    UPD_SETATTR_EXPTIME_INFO_BKEY,
    UPD_FLUSH,
#else
    UPD_SETATTR,
#endif
    /* list command */
    UPD_LIST_CREATE,
    UPD_LIST_ELEM_INSERT,
    UPD_LIST_ELEM_DELETE,
    /* set command */
    UPD_SET_CREATE,
    UPD_SET_ELEM_INSERT,
    UPD_SET_ELEM_DELETE,
    /* map command */
    UPD_MAP_CREATE,
    UPD_MAP_ELEM_INSERT,
    UPD_MAP_ELEM_DELETE,
    UPD_MAP_ELEM_UPDATE,
    /* btree command */
    UPD_BT_CREATE,
    UPD_BT_ELEM_INSERT,
    UPD_BT_ELEM_UPSERT,
    UPD_BT_ELEM_DELETE,
    UPD_BT_ELEM_UPDATE,
    UPD_BT_ELEM_ARITHMETIC,
    /* not command */
    UPD_NONE
};

/* key hash item common */
struct lrec_item_common {
    uint8_t     ittype;         /* item type */
    uint8_t     reserved_8[1];
    uint16_t    keylen;         /* key length */
    uint32_t    vallen;         /* value length */
    uint32_t    flags;          /* item flags */
    uint32_t    exptime;        /* expire time */
};

struct lrec_coll_meta {
    uint8_t     ovflact;        /* overflow action */
    uint8_t     mflags;         /* sticky, readable, trimmed flags */
    uint8_t     maxbkrlen;      /* maxbkeyrange length */
    uint8_t     reserved_8[1];
    int32_t     mcnt;           /* maximum element count */
};

/* Log Record Structure */

typedef struct _loghdr {
    uint8_t     logtype;        /* Log type */
    uint8_t     updtype;        /* update command type */
    uint8_t     reserved_8[2];
    uint32_t    body_length;    /* LogRec body length */
} LogHdr;

typedef struct _logrec {
    LogHdr      header;
    char        *body;          /* specific log record data */
} LogRec;

/* Specific Log Record Structure */

/* Item Link Log Record */
typedef struct _IT_link_data {
    struct lrec_item_common cm;
    union {
        uint64_t cas;
        struct lrec_coll_meta meta;
    } ptr;
    char data[1];
} ITLinkData;

typedef struct _IT_link_log {
    LogHdr      header;
    ITLinkData  body;
    char        *keyptr;
    unsigned char *maxbkrptr;    /* maxbkeyrange value */
} ITLinkLog;

#ifdef ENABLE_PERSISTENCE_03_CMDLOG_KV
/* Item Unlink Log Record */
typedef struct _IT_unlink_data {
    uint16_t keylen;         /* key length */
    char     data[1];
} ITUnlinkData;

typedef struct _IT_unlink_log {
    LogHdr       header;
    ITUnlinkData body;
    char         *keyptr;
} ITUnlinkLog;

/* Item Flush Log Record */
typedef struct _IT_flush_data {
    uint8_t nprefix;        /* prefix length */
    char    data[1];
} ITFlushData;

typedef struct _IT_flush_log {
    LogHdr      header;
    ITFlushData body;
    char        *prefixptr;
} ITFlushLog;

/* Item SetAttr Log Record */
typedef struct _IT_setattr_data {
    uint16_t keylen;         /* key length */
    uint16_t reserved_16[1];
    uint32_t exptime;        /* expire time */
    uint8_t  ovflact;        /* overflow action */
    uint8_t  mflags;         /* sticky, readable, trimmed flags */
    uint8_t  maxbkrlen;      /* maxbkeyrange length */
    uint8_t  reserved_8[1];
    int32_t  mcnt;           /* maximum element count */
    char     data[1];
} ITSetAttrData;

typedef struct _IT_setattr_log {
    LogHdr        header;
    ITSetAttrData body;
    char          *keyptr;
    unsigned char *maxbkrptr;    /* maxbkeyrange value */
} ITSetAttrLog;
#endif

/* Snapshot File Tail Record */
typedef struct _snapshot_tail_log {
    LogHdr header;
} SnapshotTailLog;

/* Construct Log Record Function For Snapshot */
int lrec_construct_snapshot_head(LogRec *logrec);
int lrec_construct_snapshot_tail(LogRec *logrec);
#ifdef ENABLE_PERSISTENCE_03_CMDLOG_KV
int lrec_construct_link_item(LogRec *logrec, hash_item *it);
int lrec_construct_unlink_item(LogRec *logrec, hash_item *it);
int lrec_construct_flush_item(LogRec *logrec, const char *prefix, const int nprefix);
int lrec_construct_setattr(LogRec *logrec, hash_item *it, uint8_t updtype);
#else
int lrec_construct_snapshot_item(LogRec *logrec, hash_item *it);
#endif

void lrec_write_to_buffer(LogRec *logrec, char *bufptr);
#endif