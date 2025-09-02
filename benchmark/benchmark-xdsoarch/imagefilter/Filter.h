#ifndef eb7878f4_b5f8_4a7a_b7fb_398246aab3fa
#define eb7878f4_b5f8_4a7a_b7fb_398246aab3fa

#include <opencv2/opencv.hpp>

class Filter
{
public:
    virtual ~Filter() {}
    virtual void apply(cv::Mat &image) = 0;
};

extern "C" Filter *create();

#endif // !eb7878f4_b5f8_4a7a_b7fb_398246aab3fa
