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

OPCLS_DECLARE_BEGIN(AddFourOperation, AddThreeOperation)
OP_METHOD(execute, a + 4)
OP_METHOD(mixexec, a + 4)
OPCLS_DECLARE_END(AddFourOperation, AddSubMixOp)

OPCLS_DECLARE_BEGIN(AddFiveOperation, AddFourOperation)
OP_METHOD(execute, a + 5)
OP_METHOD(mixexec, a + 5)
OPCLS_DECLARE_END(AddFiveOperation, AddSubMixOp)

OPCLS_DECLARE_BEGIN(AddSixOperation, AddFiveOperation)
OP_METHOD(execute, a + 6)
OP_METHOD(mixexec, a + 6)
OPCLS_DECLARE_END(AddSixOperation, AddSubMixOp)

/**------------------------------------------------------------------
 * Subtract operators
 */
OPCLS_DECLARE_BEGIN(SubtractTenOperation, AddSubMixOp)
OP_METHOD(execute, a - 10)
OP_METHOD(mixexec, a - 10)
OPCLS_DECLARE_END(SubtractTenOperation, AddSubMixOp)

OPCLS_DECLARE_BEGIN(SubtractNineOperation, AddSubMixOp)
OP_METHOD(execute, a - 9)
OP_METHOD(mixexec, a - 9)
OPCLS_DECLARE_END(SubtractNineOperation, AddSubMixOp)

OPCLS_DECLARE_BEGIN(SubtractEightOperation, SubtractNineOperation)
OP_METHOD(execute, a - 8)
OP_METHOD(mixexec, a - 8)
OPCLS_DECLARE_END(SubtractEightOperation, AddSubMixOp)

OPCLS_DECLARE_BEGIN(SubtractSevenOperation, SubtractEightOperation)
OP_METHOD(execute, a - 7)
OP_METHOD(mixexec, a - 7)
OPCLS_DECLARE_END(SubtractSevenOperation, AddSubMixOp)

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

OPCLS_DECLARE_BEGIN(DivideThreeOperation, DivideTwoNineOperation)
OP_METHOD(execute, a / 3)
OPCLS_DECLARE_END(DivideThreeOperation, DivideTwoNineOperation)

/**------------------------------------------------------------------
 * Mix Add, sub, mul, div operators
 */
class ASMDMix : public AddSubMixOp, public MulDivMixOp
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
OPOBJ_CREATE_API(ASMDMix, ASMDMix);

OPCLS_DECLARE_BEGIN(DivideFiveOperation, ASMDMix)
OP_METHOD(execute, a / 5)
OPCLS_DECLARE_END(DivideFiveOperation, ASMDMix)

OPCLS_DECLARE_BEGIN(DivideSevenOperation, DivideFiveOperation)
OP_METHOD(execute, a / 7)
OPCLS_DECLARE_END(DivideSevenOperation, DivideFiveOperation)

extern "C" Operation *Create_Adder()
{
    // |path|=7 0x280249fa8fa9e832
    return new SubtractEightOperation();
}

extern "C" Operation *Create_Subor()
{
    // |path|=6 0x1d76320fa1e2bc79
    return new SubtractNineOperation();
}

extern "C" long Return_Counter()
{
    return gCounter;
}
