#include "src/vm/EvaVM.h"
#include <iostream>
#include "src/Logger.h"


int main(int argc, char const *argv[])
{
    EvaVM vm;

    auto result = vm.exec(R"(
        (var x 10)
        (def foo () x)
        (begin
            (var y 100)
            (var q 300)
            q
            (+ y x)
            (begin
                (var z 200)
                z
                (def bar () (+ y z))
                (bar)
            )
        )
    )");
    // vm.dumpStack();
    std::cout << "\n";
    log(result);

    std::cout << "All done!\n";

    return 0;
}
