#include "Filter.h"
#include <iostream>
#include <opencv2/opencv.hpp>

// Count execution times of all functions in this module
static long gCounter = 0;

class BlurFilter : public Filter
{
public:
    void apply(cv::Mat &image) override
    {
        // Apply a blur filter to the image
        cv::blur(image, image, cv::Size(5, 5));

        gCounter++;
    }
};

extern "C" Filter *create()
{
    return new BlurFilter();
}

extern "C" long Return_Counter()
{
    return gCounter;
}
