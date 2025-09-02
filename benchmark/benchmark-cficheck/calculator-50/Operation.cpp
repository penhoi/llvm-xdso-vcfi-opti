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

class PowerOperation : public virtual Operation // FIX: Added virtual
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return std::pow(a, b);
    }
};
OPOBJ_CREATE_API(PowerOperation, Operation);

class ModOperation : public virtual Operation // FIX: Added virtual
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return std::fmod(a, b);
    }
};
OPOBJ_CREATE_API(ModOperation, Operation);

class SqrtOperation : public virtual Operation // FIX: Added virtual for consistency
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return std::sqrt(a);
    }
};
OPOBJ_CREATE_API(SqrtOperation, Operation);

class DummyOperation : public virtual Operation // FIX: Added virtual for consistency
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

OPCLS_DECLARE_BEGIN(AddSevenOperation, AddSubMixOp)
OP_METHOD(execute, a + 7)
OP_METHOD(mixexec, a + 7)
OPCLS_DECLARE_END(AddSevenOperation, AddSubMixOp)

OPCLS_DECLARE_BEGIN(AddEightOperation, AddSevenOperation)
OP_METHOD(execute, a + 8)
OP_METHOD(mixexec, a + 8)
OPCLS_DECLARE_END(AddEightOperation, AddSevenOperation)

class Add87MixOperation : public AddEightOperation, public MulDivMixOp
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return a + b;
    }
};
OPOBJ_CREATE_API(Add87MixOperation, Add87MixOperation);

OPCLS_DECLARE_BEGIN(AddNineOperation, Add87MixOperation)
OP_METHOD(execute, a + 9)
OP_METHOD(mixexec, a + 9)
OPCLS_DECLARE_END(AddNineOperation, Add87MixOperation)

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

OPCLS_DECLARE_BEGIN(SubtractSixOperation, AddSubMixOp)
OP_METHOD(execute, a - 6)
OP_METHOD(mixexec, a - 6)
OPCLS_DECLARE_END(SubtractSixOperation, AddSubMixOp)

OPCLS_DECLARE_BEGIN(SubtractFiveOperation, SubtractSixOperation)
OP_METHOD(execute, a - 5)
OP_METHOD(mixexec, a - 5)
OPCLS_DECLARE_END(SubtractFiveOperation, SubtractSixOperation)

OPCLS_DECLARE_BEGIN(SubtractFourOperation, AddSubMixOp)
OP_METHOD(execute, a - 4)
OP_METHOD(mixexec, a - 4)
OPCLS_DECLARE_END(SubtractFourOperation, AddSubMixOp)

OPCLS_DECLARE_BEGIN(SubtractThreeOperation, SubtractFourOperation)
OP_METHOD(execute, a - 3)
OP_METHOD(mixexec, a - 3)
OPCLS_DECLARE_END(SubtractThreeOperation, SubtractFourOperation)

class Sub73MixOperation : public SubtractSevenOperation, public SubtractThreeOperation, public Add87MixOperation
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return a + b;
    }
};
OPOBJ_CREATE_API(Sub73MixOperation, Sub73MixOperation);

OPCLS_DECLARE_BEGIN(SubtractTwoOperation, Sub73MixOperation)
OP_METHOD(execute, a - 2)
OP_METHOD(mixexec, a - 2)
OPCLS_DECLARE_END(SubtractTwoOperation, Sub73MixOperation)

OPCLS_DECLARE_BEGIN(SubtractOneOperation, SubtractTwoOperation)
OP_METHOD(execute, a - 1)
OP_METHOD(mixexec, a - 1)
OPCLS_DECLARE_END(SubtractOneOperation, SubtractTwoOperation)

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

OPCLS_DECLARE_BEGIN(MultiplyByFourOperation, MultiplyByTwoOperation)
OP_METHOD(execute, a * 4)
OPCLS_DECLARE_END(MultiplyByFourOperation, MultiplyByTwoOperation)

OPCLS_DECLARE_BEGIN(MultiplyByFiveOperation, MultiplyByFourOperation)
OP_METHOD(execute, a * 5)
OPCLS_DECLARE_END(MultiplyByFiveOperation, MultiplyByFourOperation)

class MPMAMixOperation : public ModOperation, PowerOperation, Add87MixOperation
{
public:
    virtual double execute(double a, double b)
    {
        gCounter++;
        return a + b;
    }
};
OPOBJ_CREATE_API(MPMAMixOperation, MPMAMixOperation);

OPCLS_DECLARE_BEGIN(MultiplyBySixOperation, MPMAMixOperation)
OP_METHOD(execute, a * 6)
OPCLS_DECLARE_END(MultiplyBySixOperation, MPMAMixOperation)

OPCLS_DECLARE_BEGIN(MultiplyBySevenOperation, MPMAMixOperation)
OP_METHOD(execute, a * 7)
OPCLS_DECLARE_END(MultiplyBySevenOperation, MPMAMixOperation)

OPCLS_DECLARE_BEGIN(MultiplyByEightOperation, MultiplyBySevenOperation)
OP_METHOD(execute, a * 8)
OPCLS_DECLARE_END(MultiplyByEightOperation, MultiplyBySevenOperation)

OPCLS_DECLARE_BEGIN(MultiplyByNineOperation, MultiplyByEightOperation)
OP_METHOD(execute, a * 9)
OPCLS_DECLARE_END(MultiplyByNineOperation, MultiplyByEightOperation)

/**------------------------------------------------------------------
 * Divide operators
 */
OPCLS_DECLARE_BEGIN(DivideTwoNineOperation, DivideOperation)
OP_METHOD(execute, a / 2)
OPCLS_DECLARE_END(DivideTwoNineOperation, DivideOperation)

/**------------------------------------------------------------------
 * Mix Add, sub, mul, div operators
 */
class ASMDMix : public virtual AddSubMixOp, public MulDivMixOp
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

/**------------------------------------------------------------------
 * Mix Add, sub, mul, div, power, mod operators
 */
class ASMDMPix : public virtual ASMDMix, public PowerOperation
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
OPOBJ_CREATE_API(ASMDMPix, ASMDMPix);

OPCLS_DECLARE_BEGIN(DivideElevenOperation, DivideOperation)
OP_METHOD(execute, a / 11)
OPCLS_DECLARE_END(DivideElevenOperation, DivideOperation)

class ASMDMPMix : public ASMDMPix, public ModOperation
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
OPOBJ_CREATE_API(ASMDMPMix, ASMDMPMix);

OPCLS_DECLARE_BEGIN(DivideThirteenOperation, ASMDMPMix)
OP_METHOD(execute, a / 13)
OPCLS_DECLARE_END(DivideThirteenOperation, ASMDMPMix)

extern "C" Operation *Create_Adder()
{
    // |path|=7 0xa6b9ee21395baabe
    return new SubtractSixOperation();
}

extern "C" Operation *Create_Subor()
{
    // |path|=6 0xe7c66847cbd76a22
    return new AddThreeOperation();
}

extern "C" long Return_Counter()
{
    return gCounter;
}