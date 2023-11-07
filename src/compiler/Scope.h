#ifndef Scope_h
#define Scope_h

#include <map>
#include <memory>
#include <set>
#include <string>
#include "../Logger.h"

enum class ScopeType { GLOBAL, FUNCTION, BLOCK };

enum class AllocType { GLOBAL, LOCAL, CELL };

struct Scope {
    Scope(ScopeType type, std::shared_ptr<Scope> parent)
        : type(type), parent(parent) {}

    void addLocal(const std::string& name) {
        allocInfo[name] =
            type == ScopeType::GLOBAL ? AllocType::GLOBAL : AllocType::LOCAL;
    }

    void addCell(const std::string& name) {
        cells.insert(name);
        allocInfo[name] = AllocType::CELL;
    }

    void addFree(const std::string& name) {
        free.insert(name);
        allocInfo[name] = AllocType::CELL;
    }

    void maybePromote(const std::string& name) {
        auto initAllocType =
            type == ScopeType::GLOBAL ? AllocType::GLOBAL : AllocType::LOCAL;

        if (allocInfo.count(name) != 0) {
            initAllocType = allocInfo[name];
        }

        auto [ownerScope, allocType] = resolve(name, initAllocType);
        allocInfo[name] = allocType;

        if (allocType == AllocType::CELL) {
            promote(name, ownerScope);
        }
    }

    void promote(const std::string& name, Scope* ownerScope) {
        ownerScope->addCell(name);

        auto scope = this;

        while (scope != ownerScope) {
            scope->addFree(name);
            scope = scope->parent.get();
        }
    }

    std::pair<Scope*, AllocType> resolve(const std::string& name,
                                         AllocType allocType) {
        if (allocInfo.count(name) != 0) {
            return std::make_pair(this, allocType);
        }

        if (type == ScopeType::FUNCTION) {
            allocType = AllocType::CELL;
        }

        if (parent == nullptr) {
            DIE << "[Scope] Reference error:" << name << " is not defined.";
        }

        if (parent->type == ScopeType::GLOBAL) {
            allocType = AllocType::GLOBAL;
        }

        return parent->resolve(name, allocType);
    }

    int getNameGetter(const std::string& name) {
        switch (allocInfo[name]) {
            case AllocType::GLOBAL:
                return OP_GET_GLOBAL;
            case AllocType::LOCAL:
                return OP_GET_LOCAL;
            case AllocType::CELL:
                return OP_GET_CELL;
        }
    }

    int getNameSetter(const std::string& name) {
        switch (allocInfo[name]) {
            case AllocType::GLOBAL:
                return OP_SET_GLOBAL;
            case AllocType::LOCAL:
                return OP_SET_LOCAL;
            case AllocType::CELL:
                return OP_SET_CELL;
        }
    }

    ScopeType type;

    std::shared_ptr<Scope> parent;

    std::map<std::string, AllocType> allocInfo;

    std::set<std::string> free;

    std::set<std::string> cells;
};

#endif