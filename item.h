/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *  MemcacheQ - Simple Queue Service over Memcache
 *
 *      http://memcacheq.googlecode.com
 *
 *  The source code of MemcacheQ is most based on MemcachDB:
 *
 *      http://memcachedb.googlecode.com
 *
 *  Copyright 2008 Steve Chu.  All rights reserved.
 *
 *  Use and distribution licensed under the BSD license.  See
 *  the LICENSE file for full text.
 *
 *  Authors:
 *      Haibo Pang <panghaibofei99@gmail.com>
 *
 */
#ifndef _MQ_ITEM_H_
#define _MQ_ITEM_H_
//max item list length
#define MAX_ITEM_FREELIST_LENGTH 4000
#define INIT_ITEM_FREELIST_LENGTH 500

typedef struct _stritem {
    int nbytes;      /* size of data */
    uint8_t nsuffix; /* length of flags-and-length string */
    uint8_t nkey;    /* key length, w/terminating null and padding */
    void * end[];
} item;

#define ITEM_key(item) ((char*)&((item)->end[0]))

/* warning: don't use these macros with a function, as it evals its arg twice */
#define ITEM_suffix(item) ((char *) &((item)->end[0]) + (item)->nkey + 1)
#define ITEM_data(item) ((char *) &((item)->end[0]) + (item)->nkey + 1 + (item)->nsuffix)
#define ITEM_ntotal(item) (sizeof(struct _stritem) + (item)->nkey + 1 + (item)->nsuffix + (item)->nbytes)

//the all about item function define
void item_init(void);
item *do_item_from_freelist(void);
int do_item_add_to_freelist(item *);
item *item_alloc1(char *key, const size_t nkey, const int flags, const int nbytes);
item *item_alloc2(void);
int item_free(item *it);
#endif