// Operation.h

#ifndef D57EE673_BABD_4DF3_A5A5_CB06A321737B
#define D57EE673_BABD_4DF3_A5A5_CB06A321737B

class Operation
{
public:
    virtual ~Operation() {}
    virtual double execute(double a, double b);
};

extern "C"
{
    Operation *Create_Adder();
    Operation *Create_Subor();
    long Return_Counter();
}

#endif /* D57EE673_BABD_4DF3_A5A5_CB06A321737B */
