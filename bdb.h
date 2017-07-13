#ifndef _MQ_BDB_H
#define _MQ_BDB_H
#include "item.h"
#include <db.h>

/* bdb */
#define DBHOME "/data1/memcacheq"

#define BDB_CLEANUP_DBT() \
    memset(&dbkey, 0, sizeof(dbkey)); \
    memset(&dbdata, 0, sizeof(dbdata))
    
#define CHECK_DB_RET(ret) \
  if (0!=ret) goto dberr

struct bdb_settings {
    char *env_home;
    u_int32_t cache_size;
    u_int32_t txn_lg_bsize;
    u_int32_t log_auto_remove;
    u_int32_t page_size;
    int txn_nosync;
    int deadlock_detect_val;
    int checkpoint_val;
    int mempool_trickle_val;
    int mempool_trickle_percent;
    int qstats_dump_val;
    u_int32_t re_len;
    u_int32_t q_extentsize;
};

typedef struct _queue {
  DB* dbp;
  int64_t set_hits;
  int64_t get_hits;
  int64_t old_set_hits;
  int64_t old_get_hits;
  pthread_mutex_t lock;
} queue_t;


void  qlist_ht_init(void);
void  qlist_ht_close(void);
void  bdb_settings_init(void);
void  bdb_env_init(void);
void  bdb_env_close(void);
void  bdb_qlist_db_open(void);
void  bdb_qlist_db_close(void);
int   bdb_create_queue(char *queue_name);
int   bdb_delete_queue(char *queue_name);
int   bdb_set(char *key, item *it);
item* bdb_get(char *key);
#endif