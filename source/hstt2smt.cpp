#include <iostream>
#include "XMLParser.h"
#include "basiccontroller.h"
#include "solvingarguments.h"

#include "satbasicmodel.h"

enum ProgramArg {
    UB
};



int main(int argc, char **argv) {



    Arguments<ProgramArg> * pargs = new Arguments<ProgramArg>(
    //Program arguments
    {
        arguments::arg("filename","Instance file path.")
    },1,{
        //Program Options
        arguments::iop("ub", "upper-bound",UB, 100, "The upper bound for the optimizer. Default 100")
    },"Solve the combinatorial HSTT problem.");


    SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);


    int option = pargs->getIntOption(UB);


    SATBasicModel * encoding = new SATBasicModel(pargs->getArgument(0));

    BasicController c(sargs,encoding, true, 0,option);
    c.run();

    return 0;
}
