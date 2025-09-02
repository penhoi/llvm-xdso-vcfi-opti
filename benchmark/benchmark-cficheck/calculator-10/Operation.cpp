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

class AddSubMixOp : public AddOperation, public SubtractOperation
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return a + b;
    }
    virtual double mixexec(double a, double b)
    {
        gCounter++;
        return a + b - 1;
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

class MulDivMixOp : public MultiplyOperation, public DivideOperation
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return a / b;
    }
    virtual double mixecec(double a, double b)
    {
        gCounter++;
        return a / b;
    }
};
OPOBJ_CREATE_API(MulDivMixOp, MulDivMixOp);

/**------------------------------------------------------------------
 * Add operators
 */
OPCLS_DECLARE_BEGIN(AddOneOperation, AddSubMixOp)
OP_METHOD(execute, a + 1)
OP_METHOD(mixexec, a + 1)
OPCLS_DECLARE_END(AddOneOperation, AddSubMixOp)

/**------------------------------------------------------------------
 * Multiplication operators
 */

OPCLS_DECLARE_BEGIN(MultiplyByTwoOperation, MulDivMixOp)
OP_METHOD(execute, a * 2)
OP_METHOD(mixecec, a * 2)
OPCLS_DECLARE_END(MultiplyByTwoOperation, MulDivMixOp)

OPCLS_DECLARE_BEGIN(MultiplyByThreeOperation, MultiplyByTwoOperation)
OP_METHOD(execute, a * 3)
OPCLS_DECLARE_END(MultiplyByThreeOperation, MultiplyByTwoOperation)

extern "C" Operation *Create_Adder()
{
    // |path|=5 0xd090d720d5871da
    return new MultiplyByTwoOperation();
}

extern "C" Operation *Create_Subor()
{
    // |path|=5 0x1d21d905589c1725
    return new DivideOperation();
}

extern "C" long Return_Counter()
{
    return gCounter;
}
