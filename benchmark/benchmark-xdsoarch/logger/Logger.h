
#ifndef bfbf61a1_d1d4_4fdd_9742_87faf068149a
#define bfbf61a1_d1d4_4fdd_9742_87faf068149a

#include <string>

class Logger
{
public:
    virtual ~Logger() {}
    virtual void log(const std::string &message) = 0;
};

extern "C" Logger *create();

#endif // !bfbf61a1_d1d4_4fdd_9742_87faf068149a
