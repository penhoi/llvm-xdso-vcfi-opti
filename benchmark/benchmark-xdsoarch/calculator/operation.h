#ifndef X9b0e1d4e_fc05_4d0f_8e4f_a521af5e8211
#define X9b0e1d4e_fc05_4d0f_8e4f_a521af5e8211

class Operation
{
public:
    virtual ~Operation() {}
    virtual double execute(double a, double b) = 0;
};

extern "C" Operation *create();

#endif // !X9b0e1d4e_fc05_4d0f_8e4f_a521af5e8211
