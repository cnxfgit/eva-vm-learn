#ifndef EvaVM_h
#define EvaVM_h

#include <string>
#include <vector>
#include <array>
#include <memory>
#include "../bytecode/OpCode.h"
#include "../Logger.h"
#include "./EvaValue.h"
#include "../parser/EvaParser.h"
#include "../compiler/EvaCompiler.h"

using syntax::EvaParser;

#define READ_BYTE() *ip++

#define READ_SHORT() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))

#define TO_ADDRESS(index) (&co->code[index])

#define GET_CONST() (co->constants[READ_BYTE()])

#define STACK_LIMIT 512

#define BINARY_OP(op)                \
    do                               \
    {                                \
        auto op2 = AS_NUMBER(pop()); \
        auto op1 = AS_NUMBER(pop()); \
        push(NUMBER(op1 op op2));    \
    } while (false);

#define COMPARE_VALUES(op, v1, v2) \
    do                             \
    {                              \
        bool res;                  \
        switch (op)                \
        {                          \
        case 0:                    \
            res = v1 < v2;         \
            break;                 \
        case 1:                    \
            res = v1 > v2;         \
            break;                 \
        case 2:                    \
            res = v1 == v2;        \
            break;                 \
        case 3:                    \
            res = v1 >= v2;        \
            break;                 \
        case 4:                    \
            res = v1 <= v2;        \
            break;                 \
        case 5:                    \
            res = v1 != v2;        \
            break;                 \
        }                          \
        push(BOOLEAN(res));        \
    } while (false)

class EvaVM
{
public:
    EvaVM() : parser(std::make_unique<EvaParser>()), compiler(std::make_unique<EvaCompiler>())
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
        auto ast = parser->parse(program);

        co = compiler->compile(ast);
        ip = &co->code[0];
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
            case OP_COMPARE:
            {
                auto op = READ_BYTE();
                auto op2 = pop();
                auto op1 = pop();

                if (IS_NUMBER(op1) && IS_NUMBER(op2))
                {
                    auto v1 = AS_NUMBER(op1);
                    auto v2 = AS_NUMBER(op2);
                    COMPARE_VALUES(op, v1, v2);
                }
                else if (IS_STRING(op1) && IS_STRING(op2))
                {
                    auto s1 = AS_STRING(op1);
                    auto s2 = AS_STRING(op2);
                    COMPARE_VALUES(op, s1, s2);
                }
                break;
            }
            case OP_JMP_IF_FALSE:
            {
                auto cond = AS_BOOLEAN(pop());
                auto address = READ_SHORT();
                if (!cond)
                {
                    ip = TO_ADDRESS(address);
                }

                break;
            }
            case OP_JMP:
            {
                ip = TO_ADDRESS(READ_SHORT());
                break;
            }
            default:
                DIE << "Unknown opcode: " << std::hex << opcode;
            }
        }
    }

    std::unique_ptr<EvaParser> parser;

    std::unique_ptr<EvaCompiler> compiler;

    uint8_t *ip;

    EvaValue *sp;

    std::array<EvaValue, STACK_LIMIT> stack;

    std::vector<EvaValue> constants;

    CodeObject *co;
};

#endif