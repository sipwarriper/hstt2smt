#include "testencoding.h"
#include "util.h"
#include "errors.h"
#include <limits.h>

TestEncoding::TestEncoding(int mult) : Encoding(){
    _mult = mult;
    _x.resize(10);
    _Ivar.resize(10);
}

TestEncoding::~TestEncoding(){ }

SMTFormula * TestEncoding::encode(int lb, int ub){
    SMTFormula * f = new SMTFormula();

    boolvar v; //variable booleana
    literal l = !v; //literal, podria ser v tb

    for(int i = 0; i<10; i++)
        f->newIntVar("Ivar", i);


    //nomes 3 variables enteres certes
    vector<literal> x(10);
    for(int i=0; i<10; i++)
        x[i] = f->newBoolVar("x",i);
    f->addAMK(x,3);


    //suma variables enteres*_mult esta entre 10 i 30 (ambdós inclosos)
    intsum a;
    for(int i = 0; i<10; i++)
        a += _mult*f->ivar("Ivar", i);

    clause c1 = a <= 30;
    clause c2 = a>=10;

    f->addClause(c1);
    f->addClause(c2);



    //clausules per triar o bé variable booleana certa o bé variable entera == 0
    vector<clause> cs(10);
    for (int i = 0; i<10; i++)
        cs[i] = f->bvar("x", i) | f->ivar("Ivar", i)==0;

    f->addClauses(cs);

    return f;

}


void TestEncoding::setModel(const EncodedFormula &ef, int lb, int ub, const vector<bool> &bmodel, const vector<int> &imodel){
    for(int i=0; i<10;i++){
        _Ivar[i] = SMTFormula::getIValue(ef.f->ivar("Ivar",i),imodel);
        _x[i] = SMTFormula::getBValue(ef.f->bvar("x",i),bmodel);
    }
}

int TestEncoding::getObjective() const{
    return 0;
}

bool TestEncoding::printSolution(ostream &os) const{
    os << "Num ---- variables booleanes ---- valor" << std::endl;
    for(int i=0; i<10;i++){
        os << i << " ---- " << std::boolalpha <<  _x[i] << " -----" << _Ivar[i] << std::endl;
    }
}
