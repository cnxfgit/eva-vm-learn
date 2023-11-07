#include "src/vm/EvaVM.h"
#include <iostream>
#include "src/Logger.h"


int main(int argc, char const *argv[])
{
    EvaVM vm;

    auto result = vm.exec(R"(
        (var x 1)
        (def foo () x)
        (foo)
    )");
    // vm.dumpStack();
    std::cout << "\n";
    log(result);

    std::cout << "All done!\n";

    return 0;
}
