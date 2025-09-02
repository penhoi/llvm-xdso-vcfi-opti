#include "operation.cpp"
#include "operation.h"
#include <iostream>

// Count execution times of all functions in this module
// static long gCounter = 0;

class Subtract : public Operation
{
public:
    double execute(double a, double b) override
    {
        // std::cout << "Subtract: " << a - b << std::endl;
        gCounter += 2;
        return a - b;
    }
};

// 找到了 9 个唯一的类名
extern "C" Operation *create()
{
    // |path|=5 0x12ae861be4d233ca
    return new SubtractOperation();
}
