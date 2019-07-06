#include <iostream>
#include "XMLParser.h"
#include "basiccontroller.h"
#include "solvingarguments.h"

#include "satbasicmodel.h"

enum ProgramArg {
    MULT
};



int main(int argc, char **argv) {



    Arguments<ProgramArg> * pargs = new Arguments<ProgramArg>(
    //Program arguments
    {
        arguments::arg("filename","Instance file path.")
    },1,{
        //Program Options
        arguments::iop("mult", "multiplier",MULT, 1, "The multiplier set on the testing. Default: 1")
    },"Solve the combinatorial HSTT problem.");


    SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);


    int option = pargs->getIntOption(MULT);


    SATBasicModel * encoding = new SATBasicModel(pargs->getArgument(0));

    BasicController c(sargs,encoding, false, 0,0);
    c.run();




    //    auto printer = XHSTTPrinterModel(pargs->getArgument(0));




    return 0;
}
