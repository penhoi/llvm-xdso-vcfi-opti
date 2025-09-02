#include "Operation.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <typeinfo>

typedef Operation *(*Creator_fty)();
typedef long (*Counter_fty)();

// Count execution times of all functions in this module
static long gCounter = 0;
extern "C" long Return_Counter()
{
    return gCounter;
}

int main(int argc, char *argv[])
{
    int nCycles = 0;

    if (argc == 2)
        nCycles = atoi(argv[1]);
    else
        return -1;

    void *handles[4] = {nullptr};
    Operation *operations[4] = {nullptr};
    const char *plugins[] = {"./libOpn.so"};

    Counter_fty DSO_Counter = nullptr;
    Counter_fty EXE_Counter = Return_Counter;

    for (const auto &plugin : plugins)
    {
        static int i = 0, j = 0;
        void *handle = dlopen(plugin, RTLD_LAZY);
        if (!handle)
        {
            perror("Cannot open plugin");
            return 1;
        }
        handles[i++] = handle;

        // Dynamically load the Create_Adder
        Creator_fty create_add = nullptr;
        create_add = (Creator_fty)dlsym(handle, "Create_Adder");
        if (!create_add)
        {
            perror("Cannot load symbol Create_Adder");
            return 1;
        }
        operations[j++] = create_add();

        // Dynamically load the Create_Subor
        Creator_fty create_sub = nullptr;
        create_sub = (Creator_fty)dlsym(handle, "Create_Subor");
        if (!create_sub)
        {
            perror("Cannot load symbol Create_Subor");
            return 1;
        }
        operations[j++] = create_sub();

        // Dynamically load the Create_Subor

        DSO_Counter = (Counter_fty)dlsym(handle, "Return_Counter");
        if (!DSO_Counter)
        {
            perror("Cannot load symbol Return_Counter");
            return 1;
        }
    }

    // Adjust function pointers based on VCFI mode
    const char *env = getenv("VCFI_MODE");
    if (env == nullptr)
    {
        printf("Please set VCFI_MODE to XVCFI or INTER.\n");
        return -1;
    }
    else if (strcmp(env, "INTER") == 0)
    {
        printf("Unexpected:Intra-module VCFI is enabled.\n");
        // operations[0] = Create_Adder();
        // operations[1] = Create_Subor();
        return -1;
    }
    else if (strcmp(env, "XVCFI") == 0)
    {
        printf("Cross-module VCFI is enabled.\n");
    }
    else
    {
        printf("Please set VCFI_MODE to XVCFI or INTER.\n");
        return -1;
    }

    // Measure running time
    double res = 0;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int i = 0; i < nCycles; ++i)
    {
        auto add = operations[0];
        res += add->execute(i, i + 1);

        auto sub = operations[1];
        res += sub->execute(i, i + 1);
    }
    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    long total_microseconds = seconds * 1000000 + microseconds;
    printf("Elapsed time: %ld microseconds\n", total_microseconds);

    printf("Final result: %f, EXE_Counter=%ld; DSO_Counter=%ld\n", res, EXE_Counter(), DSO_Counter());

    for (auto handle : handles)
    {
        if (handle != nullptr)
            dlclose(handle);
    }

    return 0;
}
