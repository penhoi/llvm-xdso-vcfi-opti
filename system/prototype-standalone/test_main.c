#include "lib/hashmap.c"
#include <stdio.h>

// Helper function to print test results
void print_test_result(const char *test_name, bool passed)
{
    printf("%-50s %s\n", test_name, passed ? "PASSED" : "FAILED");
}

// Test case counter
static int tests_passed = 0;
static int tests_failed = 0;

// 1. Test Hash Function
void test_hash_function()
{
    hm_keyv_t kv1 = {.type = 123, .vptr = 456};
    hm_keyv_t kv2 = {.type = 123, .vptr = 456};
    hm_keyv_t kv3 = {.type = 789, .vptr = 101112};

    size_t hash1 = hash_kvpair(kv1);
    size_t hash2 = hash_kvpair(kv2);
    size_t hash3 = hash_kvpair(kv3);

    bool passed = (hash1 == hash2) && (hash1 != hash3);
    print_test_result("Test hash function consistency", passed);
    if (passed)
        tests_passed++;
    else
        tests_failed++;
}

// 2. Test Key Comparison
void test_key_comparison()
{
    hm_keyv_t kv1 = {.type = 123, .vptr = 456};
    hm_keyv_t kv2 = {.type = 123, .vptr = 456};
    hm_keyv_t kv3 = {.type = 123, .vptr = 789};

    bool passed = kvpair_equals(kv1, kv2) && !kvpair_equals(kv1, kv3);
    print_test_result("Test key comparison", passed);
    if (passed)
        tests_passed++;
    else
        tests_failed++;
}

// 3. Test Basic VCALL Validation
void test_basic_vcall_validation()
{
    size_t type_id = 1001;
    size_t vptr = 0xABCDEF;

    // First call should miss and insert into record_cache
    cfi_vcall_validation(type_id, vptr);

    // Verify it's in record_cache but not verify_cache
    hm_keyv_t signature = {.type = type_id, .vptr = vptr};
    bool in_record = hm_find(&record_cache.hashmap, signature) != NULL;
    bool in_verify = hm_find(&verify_cache.hashmap, signature) != NULL;

    bool passed = in_record && !in_verify;
    print_test_result("Test basic VCALL validation (first call)", passed);
    if (passed)
        tests_passed++;
    else
        tests_failed++;
}

// 4. Test VCALL Migration Threshold
void test_vcall_migration_threshold()
{
    // Clear caches first
    hm_clear(&record_cache.hashmap);
    hm_clear(&verify_cache.hashmap);

    size_t type_id = 2001;
    size_t vptr = 0x123456;

    // Insert MIGRATE_VCALL_THRESH+1 entries to trigger migration
    for (int i = 0; i < MIGRATE_VCALL_THRESH + 1; i++)
    {
        cfi_vcall_validation(type_id, vptr);
    }

    // Verify signature was migrated to verify_cache
    hm_keyv_t signature = {.type = type_id, .vptr = vptr};
    bool in_verify = hm_find(&verify_cache.hashmap, signature) != NULL;
    bool record_cleared = record_cache.hashmap.items == 0;

    bool passed = in_verify && record_cleared;
    print_test_result("Test VCALL migration threshold", passed);
    if (passed)
        tests_passed++;
    else
        tests_failed++;
}

// 5. Test High Frequency Entry Migration
void test_high_frequency_migration()
{
    // Clear caches first
    hm_clear(&record_cache.hashmap);
    hm_clear(&verify_cache.hashmap);

    size_t high_freq_type = 3001;
    size_t high_freq_vptr = 0x111111;
    size_t low_freq_type = 3002;
    size_t low_freq_vptr = 0x222222;

    // Insert high frequency entry
    for (int i = 0; i < MAP_MIGRATE_MIN_FREQ + 1; i++)
    {
        hm_keyv_t kv = {.type = high_freq_type, .vptr = high_freq_vptr};
        hm_insert(&record_cache.hashmap, kv, i + 1);
    }

    // Insert low frequency entry
    hm_keyv_t kv = {.type = low_freq_type, .vptr = low_freq_vptr};
    hm_insert(&record_cache.hashmap, kv, 1);

    // Trigger migration
    transfer_high_freq_entries(&verify_cache.hashmap, &record_cache.hashmap, MAP_MIGRATE_MIN_FREQ);

    // Verify results
    hm_keyv_t high_freq_sig = {.type = high_freq_type, .vptr = high_freq_vptr};
    hm_keyv_t low_freq_sig = {.type = low_freq_type, .vptr = low_freq_vptr};

    bool high_migrated = hm_find(&verify_cache.hashmap, high_freq_sig) != NULL;
    bool low_not_migrated = hm_find(&verify_cache.hashmap, low_freq_sig) == NULL;

    bool passed = high_migrated && low_not_migrated;
    print_test_result("Test high frequency entry migration", passed);
    if (passed)
        tests_passed++;
    else
        tests_failed++;
}

// 6. Test Verify Cache Eviction (FIFO)
void test_verify_cache_eviction()
{
#define NUM_EACH_GENERATION 40

    hm_clear(&verify_cache.hashmap);

    int remain = VERIFY_GROUP_NUM * HM_GROUP_SIZE;
    assert(verify_cache.hashmap.meta.oldest_generation == 1 && "Initial oldest generation should be 1");
    verify_cache.hashmap.meta.newest_generation = 0;
    // Store oldest generation
    int oldest_gen = verify_cache.hashmap.meta.oldest_generation;

    // Fill verify cache to trigger eviction
    for (int i = 0; i < VERIFY_GROUP_NUM * HM_GROUP_SIZE; i++)
    {
        if (i % NUM_EACH_GENERATION == 0)
            verify_cache.hashmap.meta.newest_generation++;

        hm_keyv_t kv = {.type = 4000 + i, .vptr = 0x1000 + i};
        hm_insert(&verify_cache.hashmap, kv, i);

        if (verify_cache.hashmap.meta.oldest_generation == oldest_gen + 1)
        { // evicted.
            remain -= NUM_EACH_GENERATION;
            oldest_gen = verify_cache.hashmap.meta.oldest_generation;
        }
    }

    bool be_evicted = verify_cache.hashmap.items == remain;
    // Verify oldest generation was incremented
    bool expected = (verify_cache.hashmap.meta.oldest_generation - 1) * NUM_EACH_GENERATION == VERIFY_GROUP_NUM * HM_GROUP_SIZE - remain;
    bool oldest_gen_correct = verify_cache.hashmap.meta.oldest_generation < verify_cache.hashmap.meta.newest_generation;
    bool passed = be_evicted && expected && oldest_gen_correct;
    print_test_result("Test verify cache FIFO eviction", passed);
    if (passed)
        tests_passed++;
    else
        tests_failed++;
}

// 7. Test Record Cache Eviction (Frequency)
void test_record_cache_eviction()
{
    hm_clear(&record_cache.hashmap);

    int remain = RECORD_GROUP_NUM * HM_GROUP_SIZE;
    int count = 0;
    // Fill record cache with low frequency entries
    for (int i = 0; i < RECORD_GROUP_NUM * HM_GROUP_SIZE; i++)
    {
        hm_keyv_t kv = {.type = 6000 + i, .vptr = 0x3000 + i};
        if (i == 0)
            hm_insert(&record_cache.hashmap, kv, MAP_EVICT_MIN_COUNT * 2); // high frequency
        else
            hm_insert(&record_cache.hashmap, kv, 1); // Low frequency

        if (record_cache.hashmap.items == 2)
        { // evicted or inserted the first 2 elements.
            remain -= count;
            count = 0;
        }
        count++;
    }

    // Verify all old entries are evicted;
    bool passed = record_cache.hashmap.items == remain + 1;
    print_test_result("Test record cache frequency eviction", passed);
    if (passed)
        tests_passed++;
    else
        tests_failed++;
}

// Main Test Runner
int main()
{
    printf("Running VCFI Hashmap Tests...\n\n");

    // Basic functionality tests
    test_hash_function();
    test_key_comparison();

    // VCFI verification tests
    test_basic_vcall_validation();
    test_vcall_migration_threshold();
    test_high_frequency_migration();

    // Cache eviction tests
    test_verify_cache_eviction();
    test_record_cache_eviction();

    printf("\nTest Summary: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
