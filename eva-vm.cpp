#include "src/vm/EvaVM.h"
#include <iostream>
#include "src/Logger.h"


int main(int argc, char const *argv[])
{
    EvaVM vm;

    auto result = vm.exec(R"(
        42
    )");
    log(AS_CPPSTRING(result));

    std::cout << "All done!\n";

    return 0;
}
