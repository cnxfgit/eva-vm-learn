#ifndef __EvaVM_h
#define __EvaVM_h

#include <string>
#include <vector>
#include "../bytecode/OpCode.h"
#include "../Logger.h"

#define READ_BYTE() *ip++

class EvaVM
{
public:
    EvaVM()
    {
    }

    void exec(const std::string &program)
    {
        code = {OP_HALT};
        ip = &code[0];
        return eval();
    }

    void eval()
    {
        for (;;)
        {
            auto opcode = READ_BYTE();

            switch (opcode)
            {
            case OP_HALT:
                return;
            default:
                DIE << "Unknown opcode: " << std::hex << opcode;
            }
        }
    }

    uint8_t *ip;

    std::vector<uint8_t> code;
};

#endif