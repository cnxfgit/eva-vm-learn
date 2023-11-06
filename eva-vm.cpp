#include "src/vm/EvaVM.h"
#include <iostream>
#include "src/Logger.h"


int main(int argc, char const *argv[])
{
    EvaVM vm;

    auto result = vm.exec(R"(
        (def square (x) (* x x))
        // (def sum (a b) 
        //     (begin 
        //         (var x 10)
        //         (+ x (+ a b))
        //     )
        // )
        //(square 2)
    )");
    // vm.dumpStack();
    std::cout << "\n";
    log(result);

    std::cout << "All done!\n";

    return 0;
}
