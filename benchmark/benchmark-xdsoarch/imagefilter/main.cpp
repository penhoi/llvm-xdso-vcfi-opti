
#include "BlurFilter.cpp"
#include "Filter.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <typeinfo>

typedef Filter *(*Creator_fty)();
typedef long (*Counter_fty)();

int main(int argc, char *argv[])
{
    int nCycles = 0;

    if (argc == 2)
        nCycles = atoi(argv[1]);
    else
        return -1;

    void *handles[4] = {nullptr};
    Filter *filters[4] = {nullptr};
    const char *plugins[] = {"./libBlurFilter.so", "./libShapFilter.so"};
    Counter_fty DSO_Counter[2] = {nullptr};
    Counter_fty EXE_Counter = Return_Counter;

    for (const auto &plugin : plugins)
    {
        static int i = 0, j = 0, k = 0;
        void *handle = dlopen(plugin, RTLD_LAZY);
        if (!handle)
        {
            perror("Cannot open plugin");
            return 1;
        }
        handles[i++] = handle;

        // Dynamically load the Create_Adder
        Creator_fty create = nullptr;
        create = (Creator_fty)dlsym(handle, "create");
        if (!create)
        {
            perror("Cannot load symbol create");
            return 1;
        }
        filters[j++] = create();

        // Dynamically load the Return_Counter
        Counter_fty counter = nullptr;
        counter = (Counter_fty)dlsym(handle, "Return_Counter");
        if (!counter)
        {
            perror("Cannot load symbol Return_Counter");
            return 1;
        }
        DSO_Counter[k++] = counter;
    }

    // Load an image
    cv::Mat image = cv::imread("lena_color.jpg", cv::IMREAD_COLOR);
    if (image.empty())
    {
        perror("Could not open or find the image!");
        return 1;
    }

    int nfilter = 2;
    if (NULL != getenv("MIXVCALL"))
    {
        filters[nfilter++] = create(); // Create an instance of the logger in the main executable
    }
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int i = 0; i < nCycles; ++i)
    {
        // Apply filters
        for (int j = 0; j < nfilter; j++)
        {
            auto filter = filters[j];
            filter->apply(image);
        }
    }
    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    long total_microseconds = seconds * 1000000 + microseconds;
    printf("Elapsed time: %ld microseconds\n", total_microseconds);

    printf("EXE_Counter=%ld; DSO_ADD_Counter=%ld, DSO_SUB_Counter=%ld/2\n",
           EXE_Counter(), DSO_Counter[0](), DSO_Counter[1]());

    // Save the processed image
    cv::imwrite("output.jpg", image);
    for (auto handle : handles)
    {
        if (handle != nullptr)
            dlclose(handle);
    }

    return 0;
}
