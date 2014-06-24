#include "common.h"

#ifdef ENABLE_MEMCACHED

#include <libmemcached-1.0/memcached.h>
#include <libmemcachedutil-1.0/pool.h>

#define DEBUG_FLAG SEAFILE_DEBUG_OTHER
#include "log.h"

struct ObjCache {
    memcached_pool_st *mc_pool;
    int mc_expiry;
    char *existence_prefix;
};
typedef struct ObjCache ObjCache;

ObjCache *
objcache_new (const char *mc_options, int mc_expiry, const char *existence_prefix)
{
    ObjCache *cache = g_new0 (ObjCache, 1);

    cache->mc_pool = memcached_pool (mc_options, strlen(mc_options));
    if (!cache->mc_pool) {
        seaf_warning ("Failed to create memcached pool.\n");
        g_free (cache);
        return NULL;
    }
    cache->mc_expiry = mc_expiry;
    cache->existence_prefix = g_strdup (existence_prefix);

    return cache;
}

void *
objcache_get_object (ObjCache *cache, const char *obj_id, size_t *len)
{
    char *object;
    memcached_st *mc;
    memcached_return_t rc;
    uint32_t flags = 0;
    struct timespec ts = { 0, 0 };

    mc = memcached_pool_fetch (cache->mc_pool, &ts, &rc);
    if (!mc) {
        seaf_warning ("Failed to get memcached connection.\n");
        return NULL;
    }

    object = memcached_get (mc, obj_id, strlen(obj_id), len, &flags, &rc);

    memcached_pool_release (cache->mc_pool, mc);

    return object;
}

int
objcache_set_object (ObjCache *cache,
                    const char *obj_id,
                    const void *object,
                    int len)
{
    memcached_st *mc;
    memcached_return_t rc;
    int ret = 0;
    struct timespec ts = { 0, 0 };

    mc = memcached_pool_fetch (cache->mc_pool, &ts, &rc);
    if (!mc) {
        seaf_warning ("Failed to get memcached connection.\n");
        return -1;
    }

    rc = memcached_set (mc, obj_id, strlen(obj_id), object, len,
                        (time_t)cache->mc_expiry, 0);
    if (rc != MEMCACHED_SUCCESS) {
        seaf_warning ("Failed to set %s to memcached: %s.\n",
                      obj_id, memcached_strerror(mc, rc));
        ret = -1;
    }

    memcached_pool_release (cache->mc_pool, mc);

    return ret;
}

gboolean
objcache_test_object (ObjCache *cache, const char *obj_id)
{
    memcached_st *mc;
    memcached_return_t rc;
    gboolean ret = TRUE;
    struct timespec ts = { 0, 0 };

    mc = memcached_pool_fetch (cache->mc_pool, &ts, &rc);
    if (!mc) {
        seaf_warning ("Failed to get memcached connection.\n");
        return FALSE;
    }

    rc = memcached_exist (mc, obj_id, strlen(obj_id));
    if (rc != MEMCACHED_SUCCESS)
        ret = FALSE;

    memcached_pool_release (cache->mc_pool, mc);

    return ret;
}

int
objcache_delete_object (ObjCache *cache, const char *obj_id)
{
    memcached_st *mc;
    memcached_return_t rc;
    int ret = 0;
    struct timespec ts = { 0, 0 };

    mc = memcached_pool_fetch (cache->mc_pool, &ts, &rc);
    if (!mc) {
        seaf_warning ("Failed to get memcached connection.\n");
        return -1;
    }

    rc = memcached_delete (mc, obj_id, strlen(obj_id), 0);
    if (rc != MEMCACHED_SUCCESS)
        ret = -1;

    memcached_pool_release (cache->mc_pool, mc);

    return ret;
}

int
objcache_set_object_existence (ObjCache *cache, const char *obj_id, int val)
{
    char *key;
    int ret;

    key = g_strdup_printf ("%s%s", cache->existence_prefix, obj_id);

    ret = objcache_set_object (cache, key, &val, sizeof(int));

    g_free (key);
    return ret;
}

int
objcache_get_object_existence (ObjCache *cache, const char *obj_id, int *val_out)
{
    char *key;
    size_t len;
    int *val;
    int ret = 0;

    key = g_strdup_printf ("%s%s", cache->existence_prefix, obj_id);

    val = objcache_get_object (cache, key, &len);
    if (!val)
        ret = -1;
    else 
        *val_out = *((int *)val);

    g_free (key);
    free (val);
    return ret;
}

#endif  /* ENABLE_MEMCACHED */
