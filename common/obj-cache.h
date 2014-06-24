#ifndef OBJ_CACHE_H
#define OBJ_CACHE_H

struct ObjCache;

struct ObjCache *
objcache_new (const char *mc_options, int mc_expiry, const char *existence_prefix);

void *
objcache_get_object (struct ObjCache *cache, const char *obj_id, size_t *len);

int
objcache_set_object (struct ObjCache *cache,
                    const char *obj_id,
                    const void *object,
                    int len);

gboolean
objcache_test_object (struct ObjCache *cache, const char *obj_id);

int
objcache_delete_object (struct ObjCache *cache, const char *obj_id);

int
objcache_set_object_existence (struct ObjCache *cache, const char *obj_id, int val);

int
objcache_get_object_existence (struct ObjCache *cache, const char *obj_id, int *val_out);

#endif
