#ifndef HM_H
#define HM_H

#include <immintrin.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef HM_DEFAULT_N_GROUPS
#define HM_DEFAULT_N_GROUPS (1)
#endif

#ifndef HM_LOAD_FACTOR
#define HM_LOAD_FACTOR (0.75)
#endif

#ifndef HM_RESIZE_FACTOR
#define HM_RESIZE_FACTOR (2)
#endif

#define HM_GROUP_SIZE (16)
#define HM_CONTROL_SIZE (16)

typedef struct
{
    size_t type;
    size_t vptr;
    int data; // extra information
} hm_keyv_t;
typedef size_t hm_data_t;

typedef __m128i hm_control_t;
typedef int8_t hm_metadata_t;

typedef enum // Usage types for this hashmap
{
    HM_TYPE_VERIFY = 0, // Only key-value pairs
    HM_TYPE_RECORD = 1, // with additional data
} hm_usage_t;

typedef enum
{
    HM_EMPTY1B = 0b10000000,
    HM_EMPTY8B = 0x8080808080808080UL,
    HM_DELETED = 0b11111111
} hm_ctrl_e;

typedef struct
{
    hm_metadata_t meta;
    size_t pos;
} hm_hash_t;

typedef struct // Each group of swiss-table contains 16 slots.
{
    hm_control_t _ctrl;
    hm_keyv_t keyv[HM_CONTROL_SIZE];
    hm_hash_t hash[HM_CONTROL_SIZE];
} hm_group_t;

#define MAP_EVICT_MIN_COUNT 10 // Minimum number of entries to evict during reduction
#define MAP_MIGRATE_MIN_FREQ 4 // Minimum value to migrate from record_map

typedef struct
{
    hm_usage_t cache_type;
    int oldest_generation; // The generation of the oldest entries;
    int newest_generation; // Used for eviction entries in verify_cache;
    int eviction_min_freq; // Used for eviction entries in record_cache;
} __attribute__((aligned(8))) hm_cache_t;

// A swiss-table is a hashmap with a fixed number of groups,
typedef struct
{
    hm_cache_t meta;

    int items, size;
    int n_groups, sentinel;
    hm_group_t groups[0] __attribute__((aligned(32)));
} hm_map_t;

//------------------------Begin: Model-level data structures------------------------------
#define PAGE_SIZE 4096 // Assuming a page size of 4096 bytes
// Helper macro to round up to system page size
#define ROUND_TO_PAGESIZE(size) ((size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
// Helper macro to round down to the nearest power of two
#define ROUND_DOWN_TO_POW2(x) ((size_t)1 << (63 - __builtin_clzl(x)))

// Common layout for a recording and verification hashmap caches
typedef struct
{
    hm_map_t hashmap;                                      // The hashmap instance
    hm_group_t map_groups[1] __attribute__((aligned(32))); // Adjacent group array
} hm_cache_layout_t;

// A hashmap for recording VCALL signatures
#define RECORD_GROUP_NUM 10 // 10 groups for recording, ~1 pages
typedef struct              // Inherits from hm_cache_layout_t
{
    hm_map_t hashmap;                                                     // The hashmap instance
    hm_group_t map_groups[RECORD_GROUP_NUM] __attribute__((aligned(32))); // Adjacent group array
} __attribute__((aligned(PAGE_SIZE))) hm_recordcache_layout_t;

// A hashmap for verifying VCALL signatures
#define VERIFY_GROUP_NUM 81 // 81 groups for verification, ~8 pages
typedef struct              // Inherits from hm_cache_layout_t
{
    hm_map_t hashmap;                                                     // The hashmap instance
    hm_group_t map_groups[VERIFY_GROUP_NUM] __attribute__((aligned(32))); // Adjacent group array
} __attribute__((aligned(PAGE_SIZE))) hm_verifycache_layout_t;

// Only this function is exported for external use
bool cfi_vcall_validation(size_t type_id, size_t vptr);
#endif
