#include "src/vm/EvaVM.h"
#include <iostream>
#include "src/Logger.h"


int main(int argc, char const *argv[])
{
    EvaVM vm;

    auto result = vm.exec(R"(
        (!= 1 5)
    )");
    log(result);

    std::cout << "All done!\n";

    return 0;
}
