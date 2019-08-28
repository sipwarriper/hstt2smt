#include <iostream>
#include "XMLParser.h"
#include "basiccontroller.h"
#include "solvingarguments.h"

#include "hsttEncoding.h"

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
        arguments::iop("ub", "upper-bound",UB, -1, "The upper bound for the optimizer, if it's smaller than 0, then it's automatically calculated.. Default -1")
    },"Solve the combinatorial HSTT problem.");


    SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);


    int option = pargs->getIntOption(UB);


    HSTTEncoding * encoding = new HSTTEncoding(pargs->getArgument(0));


    int LB = encoding->calculate_LB();
    int UB = encoding->calculate_UB();
    if (option >0) UB = option;

    BasicController c(sargs,encoding, true, LB,UB);
    std::cout<<"Starting to solve"<<std::endl;
    c.run();

    return 0;
}
