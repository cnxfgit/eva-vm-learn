#ifndef EvaVM_h
#define EvaVM_h

#include <array>
#include <memory>
#include <string>
#include <vector>
#include "../Logger.h"
#include "../bytecode/OpCode.h"
#include "../compiler/EvaCompiler.h"
#include "../parser/EvaParser.h"
#include "./EvaValue.h"
#include "Global.h"

using syntax::EvaParser;

#define READ_BYTE() *ip++

#define READ_SHORT() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))

#define TO_ADDRESS(index) (&fn->co->code[index])

#define GET_CONST() (fn->co->constants[READ_BYTE()])

#define STACK_LIMIT 512

#define BINARY_OP(op)                \
    do {                             \
        auto op2 = AS_NUMBER(pop()); \
        auto op1 = AS_NUMBER(pop()); \
        push(NUMBER(op1 op op2));    \
    } while (false);

#define COMPARE_VALUES(op, v1, v2) \
    do {                           \
        bool res;                  \
        switch (op) {              \
            case 0:                \
                res = v1 < v2;     \
                break;             \
            case 1:                \
                res = v1 > v2;     \
                break;             \
            case 2:                \
                res = v1 == v2;    \
                break;             \
            case 3:                \
                res = v1 >= v2;    \
                break;             \
            case 4:                \
                res = v1 <= v2;    \
                break;             \
            case 5:                \
                res = v1 != v2;    \
                break;             \
        }                          \
        push(BOOLEAN(res));        \
    } while (false)

struct Frame {
    uint8_t* ra;
    EvaValue* bp;
    FunctionObject* fn;
};

class EvaVM {
   public:
    EvaVM()
        : global(std::make_shared<Global>()),
          parser(std::make_unique<EvaParser>()),
          compiler(std::make_unique<EvaCompiler>(global)) {
        setGlobalVariables();
    }

    void push(const EvaValue& value) {
        if ((size_t)(sp - stack.begin()) == STACK_LIMIT) {
            DIE << "push(): Stack overflow.\n";
        }

        *sp = value;
        sp++;
    }

    EvaValue pop() {
        if (stack.size() == 0) {
            DIE << "pop(): empty stack.\n";
        }

        --sp;
        return *sp;
    }

    EvaValue peek(size_t offset = 0) {
        if (stack.size() == 0) {
            DIE << "peek(): empty stack.\n";
        }

        return *(sp - 1 - offset);
    }

    void popN(size_t count) {
        if (stack.size() == 0) {
            DIE << "popN(): empty stack.";
        }
        sp -= count;
    }

    EvaValue exec(const std::string& program) {
        auto ast = parser->parse("(begin" + program + ")");

        compiler->compile(ast);
        fn = compiler->getMainFunction();
        ip = &fn->co->code[0];
        sp = &stack[0];
        bp = sp;
        compiler->disassenbleBytecode();
        
        return NUMBER(1);
        //return eval();
    }

    EvaValue eval() {
        for (;;) {
            // dumpStack();
            auto opcode = READ_BYTE();

            switch (opcode) {
                case OP_HALT:
                    return pop();
                case OP_CONST:
                    push(GET_CONST());
                    break;
                case OP_ADD: {
                    auto op2 = pop();
                    auto op1 = pop();

                    if (IS_NUMBER(op2) && IS_NUMBER(op1)) {
                        auto n1 = AS_NUMBER(op1);
                        auto n2 = AS_NUMBER(op2);
                        push(NUMBER(n1 + n2));
                    } else if (IS_STRING(op2) && IS_STRING(op1)) {
                        auto s1 = AS_CPPSTRING(op1);
                        auto s2 = AS_CPPSTRING(op2);
                        push(ALLOC_STRING(s1 + s2));
                    }

                    break;
                }
                case OP_SUB: {
                    BINARY_OP(-);
                    break;
                }
                case OP_MUL: {
                    BINARY_OP(*);
                    break;
                }
                case OP_DIV: {
                    BINARY_OP(/);
                    break;
                }
                case OP_COMPARE: {
                    auto op = READ_BYTE();
                    auto op2 = pop();
                    auto op1 = pop();

                    if (IS_NUMBER(op1) && IS_NUMBER(op2)) {
                        auto v1 = AS_NUMBER(op1);
                        auto v2 = AS_NUMBER(op2);
                        COMPARE_VALUES(op, v1, v2);
                    } else if (IS_STRING(op1) && IS_STRING(op2)) {
                        auto s1 = AS_STRING(op1);
                        auto s2 = AS_STRING(op2);
                        COMPARE_VALUES(op, s1, s2);
                    }
                    break;
                }
                case OP_JMP_IF_FALSE: {
                    auto cond = AS_BOOLEAN(pop());
                    auto address = READ_SHORT();
                    if (!cond) {
                        ip = TO_ADDRESS(address);
                    }

                    break;
                }
                case OP_JMP: {
                    ip = TO_ADDRESS(READ_SHORT());
                    break;
                }
                case OP_GET_GLOBAL: {
                    auto globalIndex = READ_BYTE();
                    push(global->get(globalIndex).value);
                    break;
                }
                case OP_SET_GLOBAL: {
                    auto globalIndex = READ_BYTE();
                    auto value = peek(0);
                    global->set(globalIndex, value);
                    break;
                }
                case OP_POP:
                    pop();
                    break;
                case OP_GET_LOCAL: {
                    auto localIndex = READ_BYTE();
                    if (localIndex < 0 || localIndex >= stack.size()) {
                        DIE << "OP_GET_LOCAL: invalid variable index: "
                            << (int)localIndex;
                    }
                    push(bp[localIndex]);
                    break;
                }
                case OP_SET_LOCAL: {
                    auto localIndex = READ_BYTE();
                    auto value = pop();
                    if (localIndex < 0 || localIndex >= stack.size()) {
                        DIE << "OP_GET_LOCAL: invalid variable index: "
                            << (int)localIndex;
                    }
                    bp[localIndex] = value;
                    break;
                }
                case OP_SCOPE_EXIT: {
                    auto count = READ_BYTE();
                    *(sp - 1 - count) = peek(0);
                    popN(count);
                    break;
                }
                case OP_CALL: {
                    auto argsCount = READ_BYTE();
                    auto fnValue = peek(argsCount);

                    if (IS_NATIVE(fnValue)) {
                        AS_NATIVE(fnValue)->function();
                        auto result = pop();

                        popN(argsCount + 1);
                        push(result);
                        break;
                    }

                    auto callee = AS_FUNCTION(fnValue);
                    callStack.push({ip, bp, fn});
                    fn = callee;
                    bp = sp - argsCount - 1;
                    ip = &callee->co->code[0];
                    break;
                }
                case OP_RETURN: {
                    auto callerFram = callStack.top();

                    ip = callerFram.ra;
                    bp = callerFram.bp;
                    fn = callerFram.fn;

                    callStack.pop();
                    break;
                }
                default:
                    DIE << "Unknown opcode: " << std::hex << opcode;
            }
        }
    }

    void setGlobalVariables() {
        global->addNativeFunction(
            "native-square",
            [&]() {
                auto x = AS_NUMBER(peek(0));
                push(NUMBER(x * x));
            },
            1);
        global->addNativeFunction(
            "sum",
            [&]() {
                auto v2 = AS_NUMBER(peek(0));
                auto v1 = AS_NUMBER(peek(1));
                push(NUMBER(v1 + v2));
            },
            2);
        global->addConst("VERSION", 1);
    }

    std::shared_ptr<Global> global;

    std::unique_ptr<EvaParser> parser;

    std::unique_ptr<EvaCompiler> compiler;

    uint8_t* ip;

    EvaValue* sp;

    EvaValue* bp;

    std::array<EvaValue, STACK_LIMIT> stack;

    std::stack<Frame> callStack;

    std::vector<EvaValue> constants;

    FunctionObject* fn;

    void dumpStack() {
        std::cout << "\n------------- Stack -------------\n";
        if (sp == stack.begin()) {
            std::cout << "(empty)";
        }
        auto csp = sp - 1;
        while (csp >= stack.begin()) {
            std::cout << *csp-- << "\n";
        }
        std::cout << "\n";
    }
};

#endif