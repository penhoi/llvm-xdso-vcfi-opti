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

//*------------------------------------------------------------------

double Operation::execute(double a, double b)
{
    gCounter++;
    return 0.0;
};

extern "C" Operation *Create_Adder()
{
    // |path|=5 0xd090d720d5871da
    return new Operation();
}

extern "C" Operation *Create_Subor()
{
    // |path|=4 0xb50a92ab058b5800
    return new Operation();
}

extern "C" long Return_Counter()
{
    return gCounter;
}
