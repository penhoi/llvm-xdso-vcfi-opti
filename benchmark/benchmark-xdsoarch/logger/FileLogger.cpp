#include "Logger.h"
#include <fstream>
#include <iostream>

// Count execution times of all functions in this module
static long gCounter = 0;

class FileLogger : public Logger
{
    std::ofstream *file;

public:
    FileLogger()
    {
        file = new std::ofstream("log.txt", std::ios::app);
    }
    ~FileLogger()
    {
        file->close();
    }

public:
    void log(const std::string &message) override
    {
        // std::ofstream file("log.txt", std::ios::app);
        gCounter++;
        *file << message << std::endl;
    }
};

extern "C" Logger *create()
{
    return new FileLogger();
}

extern "C" long Return_Counter()
{
    return gCounter;
}
