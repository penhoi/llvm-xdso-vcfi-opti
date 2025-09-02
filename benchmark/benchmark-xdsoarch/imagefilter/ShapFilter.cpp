#include "Filter.h"
#include <iostream>
#include <opencv2/opencv.hpp>

// Count execution times of all functions in this module
static long gCounter = 0;

class SharpenFilter : public Filter
{
public:
    void apply(cv::Mat &image) override
    {
        // Create a kernel for the sharpen filter
        cv::Mat kernel = (cv::Mat_<float>(3, 3) << 0, -1, 0,
                          -1, 5, -1,
                          0, -1, 0);

        // Apply the filter
        cv::filter2D(image, image, image.depth(), kernel);

        gCounter += 2;
    }
};

extern "C" Filter *create()
{
    return new SharpenFilter();
}

extern "C" long Return_Counter()
{
    return gCounter;
}
