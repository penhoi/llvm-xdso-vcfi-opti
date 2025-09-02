#include "Operation.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

// Count execution times of all functions in this module
static long gCounter = 0;

/**------------------------------------------------------------------
 * Useful macros
 */
#define OPOBJ_CREATE_API(KID, PAP) \
    extern "C" PAP *Create_##KID() \
    {                              \
        return new KID();          \
    }

#define OP_METHOD(FUNC, EXP)                \
    virtual double FUNC(double a, double b) \
    {                                       \
        gCounter++;                         \
        return EXP;                         \
    }

#define OPCLS_DECLARE_BEGIN(KID, PAP) \
    class KID : public PAP            \
    {                                 \
    public:

#define OPCLS_DECLARE_END(KID, PAP) \
    }                               \
    ;                               \
    OPOBJ_CREATE_API(KID, PAP);

/**------------------------------------------------------------------
 * Base mathematical operations
 */

double Operation::execute(double a, double b)
{
    gCounter++;
    return 0.0;
};

class AddOperation : public virtual Operation
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return a + b;
    }
};

class SubtractOperation : public virtual Operation
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return a - b;
    }
};

class MultiplyOperation : public virtual Operation
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return a * b;
    }
};
OPOBJ_CREATE_API(MultiplyOperation, Operation);

class DivideOperation : public virtual Operation
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return a / b;
    }
};
OPOBJ_CREATE_API(DivideOperation, Operation);

extern "C" Operation *Create_Adder()
{
    // |path|=2 0x35155448ac6eeaf9
    return new MultiplyOperation();
}

extern "C" Operation *Create_Subor()
{
    // |path|=3 0x218ea0e7a2446ad7
    return new AddOperation();
}

extern "C" long Return_Counter()
{
    return gCounter;
}