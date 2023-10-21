#ifndef EvaVM_h
#define EvaVM_h

#include <string>
#include <vector>
#include <array>
#include "../bytecode/OpCode.h"
#include "../Logger.h"
#include "./EvaValue.h"

#define READ_BYTE() *ip++

#define GET_CONST() constants[READ_BYTE()]

#define STACK_LIMIT 512

class EvaVM
{
public:
    EvaVM()
    {
    }

    void push(const EvaValue& value){
        if ((size_t)(sp-stack.begin()) == STACK_LIMIT)
        {
            DIE << "push(): Stack overflow.\n";
        }
        
        *sp = value;
        sp++;
    }

    EvaValue pop(){
        if (stack.size() == 0)
        {
            DIE << "pop(): empty stack.\n";
        }
        
        --sp;
        return *sp;
    }

    EvaValue exec(const std::string &program)
    {
        constants.push_back(NUMBER(42));
        code = {OP_CONST, 0, OP_HALT};
        ip = &code[0];
        return eval();
    }

    EvaValue eval()
    {
        for (;;)
        {
            auto opcode = READ_BYTE();

            switch (opcode)
            {
            case OP_HALT:
                return pop();
            case OP_CONST:
                push(GET_CONST());
                break;
            default:
                DIE << "Unknown opcode: " << std::hex << opcode;
            }
        }
    }

    uint8_t *ip;

    EvaValue *sp;

    std::array<EvaValue, STACK_LIMIT> stack;

    std::vector<EvaValue> constants;

    std::vector<uint8_t> code;
};

#endif