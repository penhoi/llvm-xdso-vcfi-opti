#include "hashmap.h"
#include <assert.h>
#include <stdalign.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

static size_t hash_kvpair(hm_keyv_t vcall_sign)
{
    return (vcall_sign.type * 2654435761) ^ (vcall_sign.vptr * 2246822519);
}

static bool kvpair_equals(hm_keyv_t kv1, hm_keyv_t kv2)
{
    return (kv1.type == kv2.type) && (kv1.vptr == kv2.vptr);
}

#define HASHFUNC hash_kvpair
#define COMPFUNC kvpair_equals

//-----------------------Begin: Define global variables-----------------------------------
static alignas(PAGE_SIZE) hm_recordcache_layout_t record_cache = {
    .hashmap = {
        .meta = {.cache_type = HM_TYPE_RECORD, 0, 0, MAP_MIGRATE_MIN_FREQ + 1},
        .items = 0,
        .size = RECORD_GROUP_NUM * HM_GROUP_SIZE,
        .n_groups = RECORD_GROUP_NUM,
        .sentinel = 0,
    },
    .map_groups = {[0 ...(RECORD_GROUP_NUM - 1)] = {._ctrl = {HM_EMPTY8B, HM_EMPTY8B}}},
};

// Force the instance to be page-aligned
static alignas(PAGE_SIZE) hm_verifycache_layout_t verify_cache = {
    .hashmap = {
        .meta = {.cache_type = HM_TYPE_VERIFY, 1, 0, 0},
        .items = 0,
        .size = VERIFY_GROUP_NUM * HM_GROUP_SIZE,
        .n_groups = VERIFY_GROUP_NUM,
        .sentinel = 0,
    },
    .map_groups = {[0 ...(VERIFY_GROUP_NUM - 1)] = {._ctrl = {HM_EMPTY8B, HM_EMPTY8B}}},
};
//-------------------------End: Define global variables-----------------------------------

static inline hm_control_t zero_lowest_n_bytes(hm_control_t _ctrl, hm_metadata_t n) __attribute__((always_inline));
static inline size_t hm_pos(size_t hash) __attribute__((always_inline));
static inline hm_metadata_t hm_meta(size_t hash) __attribute__((always_inline));
static inline hm_metadata_t hm_group_pos(size_t idx) __attribute__((always_inline));
static inline size_t hm_idx(size_t group, hm_metadata_t group_pos) __attribute__((always_inline));
static inline hm_hash_t hm_hash(hm_map_t *map, hm_keyv_t keyv) __attribute__((always_inline));
static inline bool hm_should_reduce(hm_map_t *map) __attribute__((always_inline));
static inline uint16_t hm_match_full(hm_map_t *map, size_t group) __attribute__((always_inline));
static inline size_t hm_group(size_t idx) __attribute__((always_inline));
static inline size_t hm_sentinel_group(hm_map_t *map) __attribute__((always_inline));
static inline size_t hm_last_group(hm_map_t *map) __attribute__((always_inline));

static inline uint16_t _hm_probe(hm_metadata_t meta, hm_control_t _ctrl) __attribute__((always_inline));
static inline uint16_t _hm_probe_from(hm_metadata_t group_pos, hm_metadata_t meta, hm_control_t _ctrl) __attribute__((always_inline));
static inline bool _hm_match_metadata(hm_map_t *map, hm_metadata_t meta, size_t group, size_t *match_idx) __attribute__((always_inline));
static inline bool _hm_match_metadata_from(hm_map_t *map, hm_metadata_t meta, hm_hash_t *hash, size_t group, hm_metadata_t group_pos, size_t *match_idx) __attribute__((always_inline));
static inline hm_keyv_t *_hm_find_hash(hm_map_t *map, hm_hash_t *hash, hm_keyv_t keyv, size_t group, hm_metadata_t group_pos) __attribute__((always_inline));
static inline void _hm_insert_at(hm_map_t *map, size_t group, hm_metadata_t group_pos, hm_hash_t hash, hm_keyv_t keyv) __attribute__((always_inline));

// Interfaces for external manipulation
// static hm_map_t *hm_create(size_t n_groups, hm_usage_t type);
// static void hm_destroy(hm_map_t *map);
// static void hm_remove(hm_map_t *map, hm_keyv_t keyv);

static hm_keyv_t *hm_find(hm_map_t *map, hm_keyv_t keyv);
static void hm_insert(hm_map_t *map, hm_keyv_t keyv, hm_data_t data);
static void hm_clear(hm_map_t *map);
static bool hm_iterate(hm_map_t *map, size_t *idx, hm_keyv_t **key_ref);
static bool transfer_high_freq_entries(hm_map_t *dest_map, hm_map_t *src_map, int freq);

alignas(32) static const hm_metadata_t mask[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

static hm_control_t zero_lowest_n_bytes(hm_control_t _ctrl, hm_metadata_t n)
{
    hm_control_t _m = _mm_loadu_si128((hm_control_t *)&mask[16 - n]);
    return _mm_and_si128(_ctrl, _m);
}

static size_t hm_pos(size_t hash)
{
    // Use the upper 56 bits for position
    return hash >> 8;
}

static hm_metadata_t hm_meta(size_t hash)
{
    return hash & 0x7f;
}

static size_t hm_idx(size_t group, hm_metadata_t group_pos)
{
    return (group * HM_GROUP_SIZE) + group_pos;
}

static size_t hm_group(size_t idx)
{
    return idx / HM_GROUP_SIZE;
}

static hm_metadata_t hm_group_pos(size_t idx)
{
    return idx % HM_GROUP_SIZE;
}

static size_t hm_sentinel_group(hm_map_t *map)
{
    return hm_group(map->sentinel) + 1;
}

static size_t hm_last_group(hm_map_t *map)
{
    return hm_group(map->size - 1) + 1;
}

static hm_hash_t hm_hash(hm_map_t *map, hm_keyv_t key)
{
    hm_hash_t hash;
    size_t h = HASHFUNC(key);
    hash.pos = hm_pos(h);
    hash.meta = hm_meta(h);
    return hash;
}

static bool hm_should_reduce(hm_map_t *map)
{
    return map->items >= (HM_LOAD_FACTOR * map->size);
}

static uint16_t hm_match_full(hm_map_t *map, size_t group)
{
    return ~(_mm_movemask_epi8(map->groups[group]._ctrl));
}

static uint16_t _hm_probe(hm_metadata_t meta, hm_control_t _ctrl)
{
    hm_control_t _match = _mm_set1_epi8(meta);
    return _mm_movemask_epi8(_mm_cmpeq_epi8(_match, _ctrl));
}

static uint16_t _hm_probe_from(hm_metadata_t group_pos, hm_metadata_t meta,
                               hm_control_t _ctrl)
{
    hm_control_t _match = _mm_set1_epi8(meta);
    return _mm_movemask_epi8(
        zero_lowest_n_bytes(
            _mm_cmpeq_epi8(_match, _ctrl), group_pos));
}

static bool _hm_match_metadata(hm_map_t *map, hm_metadata_t meta,
                               size_t group, size_t *match_idx)
{
    hm_metadata_t match_group_pos = _tzcnt_u32(_hm_probe(
        meta, map->groups[group]._ctrl));

    *match_idx = hm_idx(group, match_group_pos);
    return (match_group_pos < 32) ? true : false;
}

static bool _hm_match_metadata_from(hm_map_t *map, hm_metadata_t meta,
                                    hm_hash_t *hash, size_t group,
                                    hm_metadata_t group_pos, size_t *match_idx)
{
    hm_metadata_t match_group_pos = _tzcnt_u32(_hm_probe_from(
        group_pos, meta, map->groups[group]._ctrl));

    *match_idx = hm_idx(group, match_group_pos);
    return (match_group_pos < 32) ? true : false;
}

// Return the hm_keyv_t* for the key-value pair in the hashmap.
// If the key is not found, return NULL.
// keyv = _hm_find_hash(map, keyv, ...)
static hm_keyv_t *_hm_find_hash(hm_map_t *map, hm_hash_t *hash, hm_keyv_t kv, size_t group, hm_metadata_t group_pos)
{
    uint16_t matches = _hm_probe_from(group_pos, hash->meta, map->groups[group]._ctrl);

    while (matches)
    {
        hm_metadata_t match_group_pos = _tzcnt_u32(matches);

        if (COMPFUNC(map->groups[group].keyv[match_group_pos], kv))
        {
            return &map->groups[group].keyv[match_group_pos];
        }

        matches = _blsr_u32(matches);
    }
    // If we reach here, we didn't find the key in the current group.
    size_t match_idx;
    if (_hm_match_metadata_from(map, HM_EMPTY1B, hash, group, group_pos, &match_idx))
        return NULL;

    size_t end_group = hm_sentinel_group(map);
    while (true)
    {
        group = (group + 1) % end_group;

        matches = _hm_probe(hash->meta, map->groups[group]._ctrl);

        while (matches)
        {
            hm_metadata_t match_group_pos = _tzcnt_u32(matches);

            if (COMPFUNC(map->groups[group].keyv[match_group_pos], kv))
            {
                return &map->groups[group].keyv[match_group_pos];
            }

            matches = _blsr_u32(matches);
        }
        if (_hm_match_metadata(map, HM_EMPTY1B, group, &match_idx))
            return NULL;
    }
}

/**----------------------------------------------------------------------
 *  Interfaces for external manipulation
 ----------------------------------------------------------------------*/
// static hm_map_t *hm_create(size_t n_groups, hm_usage_t type)
// {
//     assert(false && "hm_create() is not supported in this implementation.");
//     return NULL;
// }

// static void hm_destroy(hm_map_t *map)
// {
//     assert(false && "hm_destroy() is not supported in this implementation.");
// }

// void hm_remove(hm_map_t *map, hm_keyv_t keyv)
// {
//     assert(false && "hm_remove() is not supported in this implementation.");
// }

// Return the element pointer of the key-value pair in the hashmap.
// If the key is not found, return NULL.
static hm_keyv_t *hm_find(hm_map_t *map, hm_keyv_t keyv)
{
    hm_hash_t hash = hm_hash(map, keyv);
    size_t idx = hash.pos % map->size;
    size_t group = hm_group(idx);
    hm_metadata_t group_pos = hm_group_pos(idx);
    return _hm_find_hash(map, &hash, keyv, group, group_pos);
}

void hm_clear(hm_map_t *map)
{
    hm_control_t _empty = _mm_set1_epi8(HM_EMPTY1B);
    size_t end_group = hm_sentinel_group(map);

    for (size_t group = 0; group < end_group; group++)
        map->groups[group]._ctrl = _empty;
    map->items = 0;
}

// Use the FIFO policy to evict entries in the verify_cache.
bool _hm_reduce_verify(hm_map_t *map)
{
    if (map->n_groups > 0)
    {
        hm_metadata_t group_pos;
        size_t group, end_group;
        uint16_t match_full;
        int num_evicted = 0; // Number of entries evicted during the reduction

        while (num_evicted <= MAP_EVICT_MIN_COUNT) // Ensure at least N entries are evicted
        {
            end_group = hm_sentinel_group(map);
            for (group = 0; group < end_group; group++)
            {
                match_full = hm_match_full(map, group);

                while (match_full)
                {
                    group_pos = _tzcnt_u32(match_full);

                    if (map->groups[group].keyv[group_pos].data <= map->meta.oldest_generation)
                    {
                        ((hm_metadata_t *)&(map->groups[group]._ctrl))[group_pos] = HM_DELETED;
                        num_evicted++;
                        map->items--;
                    }

                    match_full = _blsr_u32(match_full);
                }
            }
            map->meta.oldest_generation++;
            assert(map->meta.oldest_generation <= map->meta.newest_generation &&
                   "oldest_generation should not exceed newest_generation");
        }
        if (map->items == 0)
            map->sentinel = 0;
    }
    return true;
}

// Use the frequency policy to evict entries in the record_cache.
bool _hm_reduce_record(hm_map_t *map)
{
    if (map->n_groups > 0)
    {
        hm_metadata_t group_pos;
        size_t group, end_group;
        uint16_t match_full;
        int num_evicted = 0; // Number of entries evicted during the reduction

        int min_freq = map->meta.eviction_min_freq;
        while (num_evicted <= MAP_EVICT_MIN_COUNT) // Ensure at least N entries are evicted
        {
            end_group = hm_sentinel_group(map);
            for (group = 0; group < end_group; group++)
            {
                match_full = hm_match_full(map, group);

                while (match_full)
                {
                    group_pos = _tzcnt_u32(match_full);

                    if (map->groups[group].keyv[group_pos].data <= min_freq)
                    {
                        ((hm_metadata_t *)&(map->groups[group]._ctrl))[group_pos] = HM_DELETED;
                        num_evicted++;
                        map->items--;
                    }

                    match_full = _blsr_u32(match_full);
                }
            }
            min_freq *= 2;
        }
        if (map->items == 0)
            map->sentinel = 0;
    }

    return true;
}

static void _hm_insert_at(hm_map_t *map, size_t group, hm_metadata_t group_pos, hm_hash_t hash, hm_keyv_t keyv)
{
    size_t match_idx, match_idx_emp, match_idx_del;

    if (_hm_match_metadata_from(map, HM_EMPTY1B, &hash, group, group_pos, &match_idx_emp) |
        _hm_match_metadata_from(map, HM_DELETED, &hash, group, group_pos, &match_idx_del))
    {
        match_idx = (match_idx_emp < match_idx_del) ? match_idx_emp : match_idx_del;
        group_pos = hm_group_pos(match_idx);

        ((hm_metadata_t *)&(map->groups[group]._ctrl))[group_pos] = hash.meta;
        map->groups[group].keyv[group_pos] = keyv;
        map->groups[group].hash[group_pos] = hash;

        map->items++;

        if (match_idx > map->sentinel)
            map->sentinel = match_idx;
        return;
    }

    size_t end_group = hm_last_group(map);

    while (true)
    {
        group = (group + 1) % end_group;

        if (_hm_match_metadata(map, HM_EMPTY1B, group, &match_idx_emp) |
            _hm_match_metadata(map, HM_DELETED, group, &match_idx_del))
        {
            match_idx = (match_idx_emp < match_idx_del) ? match_idx_emp : match_idx_del;
            group_pos = hm_group_pos(match_idx);

            ((hm_metadata_t *)&(map->groups[group]._ctrl))[group_pos] = hash.meta;
            map->groups[group].hash[group_pos] = hash;
            map->groups[group].keyv[group_pos] = keyv;

            map->items++;

            if (match_idx > map->sentinel)
                map->sentinel = match_idx;
            return;
        }
    }
}

// map_ref[hash(keyv)] = keyv;
// Insert a key-value pair into the hashmap.
void hm_insert(hm_map_t *map_ref, hm_keyv_t keyv, hm_data_t value)
{
    if (hm_should_reduce(map_ref))
    {
        assert(map_ref->size > 0 && "hm_map_t::size must be greater than 0");
        if (map_ref->meta.cache_type == HM_TYPE_VERIFY)
            _hm_reduce_verify(map_ref);
        else // assert (map_ref->meta.cache_type == HM_TYPE_RECORD);
            _hm_reduce_record(map_ref);
    }

    hm_hash_t hash = hm_hash(map_ref, keyv);
    size_t idx = hash.pos % map_ref->size;
    size_t group = hm_group(idx);
    hm_metadata_t group_pos = hm_group_pos(idx);

    if (map_ref->meta.cache_type == HM_TYPE_VERIFY)
        keyv.data = map_ref->meta.newest_generation;
    else
        keyv.data = value;

    _hm_insert_at(map_ref, group, group_pos, hash, keyv);
}

// Iterate through the hashmap, idx is the current index in the hashmap.
// *key_ref = map->groups[idx/GROUP_SIZE].keyv[idx%GROUP_SIZE];
// *value_ref = map->data_arr[idx];
bool hm_iterate(hm_map_t *map, size_t *idx, hm_keyv_t **key_ref)
{
    if (*idx > map->sentinel)
        return false;

    size_t group = hm_group(*idx);
    hm_metadata_t group_pos = hm_group_pos(*idx);

    *key_ref = NULL;
    if (((hm_metadata_t *)&(map->groups[group]._ctrl))[group_pos] >= 0)
        *key_ref = &map->groups[group].keyv[group_pos];

    (*idx)++;
    return true;
}

//-----------------Begin: Functions for VCFI verification---------------------------------
static void track_vcall_signature(hm_map_t *map_ref, hm_keyv_t keyv)
{
    assert(map_ref->meta.cache_type == HM_TYPE_RECORD && "hm_map_t::type must be HM_TYPE_MOREDATA");
    hm_keyv_t *kv = hm_find(map_ref, keyv);
    if (kv)
        kv->data++;
    else
        hm_insert(map_ref, keyv, 1);
}

#define MIGRATE_VCALL_THRESH 100 // Trigger migration when this many entries are recorded

// Transfer high frequency entries from src_map to dest_map
static bool transfer_high_freq_entries(hm_map_t *verify_cache, hm_map_t *record_cache, int freq)
{
    size_t idx = 0;
    hm_keyv_t *key_ref;

    verify_cache->meta.newest_generation++;
    while (hm_iterate(record_cache, &idx, &key_ref))
    {
        if (key_ref == NULL)
            continue; // Skip empty slots
        if (key_ref->data <= freq)
            continue; // Skip empty values
        hm_insert(verify_cache, *key_ref, key_ref->data);
    }
    return true;
}

// Add all VCALL signatures from recording map to validating map
static void migrate_vcall_signature(hm_map_t *verify_map, hm_map_t *record_map)
{
    assert(verify_map->meta.cache_type == HM_TYPE_VERIFY && "verify_map must be of type HM_TYPE_ONLYKEYV");
    assert(record_map->meta.cache_type == HM_TYPE_RECORD && "record_map must be of type HM_TYPE_MOREDATA");

    transfer_high_freq_entries(verify_map, record_map, MAP_MIGRATE_MIN_FREQ);
    // Clear the record_map after migration
    hm_clear(record_map);
}

/**
 * Checks if the vcall signature (type_id, vptr) exists in the verification cache.
 * If not found, inserts it into the record cache and may trigger migration of high-frequency entries.
 *
 * @param type_id The type identifier for the vcall class.
 * @param vptr The virtual pointer value for the vtable pointer.
 */
bool cfi_vcall_validation(size_t type_id, size_t vptr)
{
    // Check if the vcall_signature exists in the verify_cache
    hm_keyv_t vcall_signature = {.type = type_id, .vptr = vptr};
    hm_keyv_t *kv = hm_find(&verify_cache.hashmap, vcall_signature);
    if (!kv)
    {
        static int missed_call = 0;
        // VCALL signature not found, insert it with a count of 1
        track_vcall_signature(&record_cache.hashmap, vcall_signature);
        missed_call++;
        if (missed_call > MIGRATE_VCALL_THRESH)
        {
            // Migrating VCALL signatures from record_cache to verify_cache...
            migrate_vcall_signature(&verify_cache.hashmap, &record_cache.hashmap);
            missed_call = 0; // Reset the missed call counter
        }
    }
    return kv != NULL; // Return true if the signature was cached
}
//------------------End: Functions for VCFI verification----------------------------------