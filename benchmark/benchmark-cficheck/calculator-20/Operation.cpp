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

class PowerOperation : public Operation
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return std::pow(a, b);
    }
};
OPOBJ_CREATE_API(PowerOperation, Operation);

class ModOperation : public Operation
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return std::fmod(a, b);
    }
};
OPOBJ_CREATE_API(ModOperation, Operation);

class SqrtOperation : public Operation
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return std::sqrt(a);
    }
};
OPOBJ_CREATE_API(SqrtOperation, Operation);

class DummyOperation : public Operation
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return a + b;
    }
};
OPOBJ_CREATE_API(DummyOperation, Operation);

/**------------------------------------------------------------------
 * Add operators
 */
OPCLS_DECLARE_BEGIN(AddOneOperation, AddSubMixOp)
OP_METHOD(execute, a + 1)
OP_METHOD(mixexec, a + 1)
OPCLS_DECLARE_END(AddOneOperation, AddSubMixOp)

OPCLS_DECLARE_BEGIN(AddTwoOperation, AddSubMixOp)
OP_METHOD(execute, a + 2)
OP_METHOD(mixexec, a + 2)
OPCLS_DECLARE_END(AddTwoOperation, AddSubMixOp)

OPCLS_DECLARE_BEGIN(AddThreeOperation, AddSubMixOp)
OP_METHOD(execute, a + 3)
OP_METHOD(mixexec, a + 3)
OPCLS_DECLARE_END(AddThreeOperation, AddSubMixOp)

/**------------------------------------------------------------------
 * Subtract operators
 */
OPCLS_DECLARE_BEGIN(SubtractTenOperation, AddSubMixOp)
OP_METHOD(execute, a - 10)
OP_METHOD(mixexec, a - 10)
OPCLS_DECLARE_END(SubtractTenOperation, AddSubMixOp)

/**------------------------------------------------------------------
 * Multiplication operators
 */
OPCLS_DECLARE_BEGIN(SquareOperation, MultiplyOperation)
OP_METHOD(execute, a *a)
OPCLS_DECLARE_END(SquareOperation, MultiplyOperation)

OPCLS_DECLARE_BEGIN(CubeOperation, MultiplyOperation)
OP_METHOD(execute, a *a *a)
OPCLS_DECLARE_END(CubeOperation, MultiplyOperation)

OPCLS_DECLARE_BEGIN(MultiplyByTwoOperation, MulDivMixOp)
OP_METHOD(execute, a * 2)
OP_METHOD(mixecec, a * 2)
OPCLS_DECLARE_END(MultiplyByTwoOperation, MulDivMixOp)

OPCLS_DECLARE_BEGIN(MultiplyByThreeOperation, MultiplyByTwoOperation)
OP_METHOD(execute, a * 3)
OPCLS_DECLARE_END(MultiplyByThreeOperation, MultiplyByTwoOperation)

/**------------------------------------------------------------------
 * Divide operators
 */
OPCLS_DECLARE_BEGIN(DivideTwoNineOperation, DivideOperation)
OP_METHOD(execute, a / 2)
OPCLS_DECLARE_END(DivideTwoNineOperation, DivideOperation)

extern "C" Operation *Create_Adder()
{
    // |path|=6 0x218ea0e7a2446ad7
    return new AddOperation();
}

extern "C" Operation *Create_Subor()
{
    // |path|=5 0xb50a92ab058b5800
    return new AddSubMixOp();
}

extern "C" long Return_Counter()
{
    return gCounter;
}
