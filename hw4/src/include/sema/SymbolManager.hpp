#ifndef SEMA_SYMBOL_MANAGER_H
#define SEMA_SYMBOL_MANAGER_H

#include <vector>
#include "sema/SymbolTable.hpp"

class SymbolManager {
public:
    using SymbolTableSharedPtr = std::shared_ptr<SymbolTable>;
    SymbolManager() : level(0) {};
    ~SymbolManager() = default;

    // Set the level automatically
    SymbolEntry &addEntry(const Location location, const char *const name,
                          SymbolEntry::SymbolKind kind, PTypeSharedPtr type) {
        return tables.back()->addEntry(location, name, kind, type);
    }
    void enterScope() {
        SymbolTableSharedPtr new_scope = std::make_shared<SymbolTable>(level++);
        tables.push_back(new_scope);
    }
    void exitScope() {
        tables.pop_back();
        level--;
    }
    
    SymbolEntry* lookup(std::string name);

    uint8_t getLevel() const { return level; }
    void dumpTopTable() { tables.back()->dumpTable(); }
private:
    uint8_t level;
    std::vector<SymbolTableSharedPtr> tables;
};
#endif