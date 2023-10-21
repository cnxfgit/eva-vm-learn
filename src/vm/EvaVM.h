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

#define BINARY_OP(op)                \
    do                               \
    {                                \
        auto op2 = AS_NUMBER(pop()); \
        auto op1 = AS_NUMBER(pop()); \
        push(NUMBER(op1 op op2));    \
    } while (false);

#define STACK_LIMIT 512

class EvaVM
{
public:
    EvaVM()
    {
    }

    void push(const EvaValue &value)
    {
        if ((size_t)(sp - stack.begin()) == STACK_LIMIT)
        {
            DIE << "push(): Stack overflow.\n";
        }

        *sp = value;
        sp++;
    }

    EvaValue pop()
    {
        if (stack.size() == 0)
        {
            DIE << "pop(): empty stack.\n";
        }

        --sp;
        return *sp;
    }

    EvaValue exec(const std::string &program)
    {
        constants.push_back(ALLOC_STRING("hello"));
        constants.push_back(ALLOC_STRING(" world"));
        code = {OP_CONST, 0, OP_CONST, 1, OP_ADD, OP_HALT};
        ip = &code[0];
        sp = &stack[0];
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
            case OP_ADD:
            {
                auto op2 = pop();
                auto op1 = pop();

                if (IS_NUMBER(op2) && IS_NUMBER(op1))
                {
                    auto n1 = AS_NUMBER(op1);
                    auto n2 = AS_NUMBER(op2);
                    push(NUMBER(n1 + n2));
                }
                else if (IS_STRING(op2) && IS_STRING(op1))
                {
                    auto s1 = AS_CPPSTRING(op1);
                    auto s2 = AS_CPPSTRING(op2);
                    push(ALLOC_STRING(s1 + s2));
                }

                break;
            }
            case OP_SUB:
            {
                BINARY_OP(-);
                break;
            }
            case OP_MUL:
            {
                BINARY_OP(*);
                break;
            }
            case OP_DIV:
            {
                BINARY_OP(/);
                break;
            }

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