
#include "hashmap.h"
#include <assert.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

size_t rand_integer(void)
{
    static int seed = 25011984;
    srand(time(NULL) + ++seed);
    size_t num = rand();
    return num;
}

int main()
{
    const size_t cnt = 200;
    hm_keyv_t keys[cnt];
    size_t values[cnt];

    for (uint64_t i = 0; i < cnt; i++)
    {
        keys[i].type = rand_integer();
        keys[i].vptr = rand_integer();
        values[i] = rand_integer();
        printf("<%lu,%lu> at idx %lu is %lu\n", keys[i].type, (uint64_t)keys[i].vptr, i, values[i]);
    }

    for (uint64_t j = 0; j < cnt; j++)
        for (int i = 0; i < 10; i++)
        {
            bool res = cfi_vcall_validation(keys[i].type, keys[i].vptr);
            if (res)
                printf("Signature <%lu,%lu> is cached\n", keys[i].type, (uint64_t)keys[i].vptr);
            else
                printf("Did not find <%lu,%lu> in cache\n", keys[i].type, (uint64_t)keys[i].vptr);
        }
}
