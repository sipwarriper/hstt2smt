#ifndef TESTENCODING_H
#define TESTENCODING_H


#include "encoding.h"

class TestEncoding : public Encoding
{
public:
    TestEncoding(int mult);
    ~TestEncoding();

    SMTFormula * encode(int lb = INT_MIN, int ub = INT_MAX);

    void setModel(const EncodedFormula &ef, int lb, int ub, const vector<bool> &bmodel, const vector<int> &imodel);
    int getObjective() const;
    bool printSolution(ostream &os) const;
private:
    int _mult;
    vector<int> _Ivar;
    vector<bool> _x;
};

#endif // TESTENCODING_H
