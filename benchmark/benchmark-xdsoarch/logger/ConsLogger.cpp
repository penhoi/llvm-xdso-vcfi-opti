#include "Logger.h"
#include <iostream>

// Count execution times of all functions in this module
static long gCounter = 0;

class ConsoleLogger : public Logger
{
public:
    void log(const std::string &message) override
    {
        gCounter += 2;
        std::cout << message << std::endl;
    }
};

extern "C" Logger *create()
{
    return new ConsoleLogger();
}

extern "C" long Return_Counter()
{
    return gCounter;
}
