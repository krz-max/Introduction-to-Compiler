#ifndef SEMA_SYMBOL_TABLE_H
#define SEMA_SYMBOL_TABLE_H

#include <vector>
#include <algorithm>
#include "sema/SymbolEntry.hpp"

class SymbolTable
{
public:
    SymbolTable(uint8_t level) : level(level) {};
    ~SymbolTable() = default;
    void dumpTable();
    SymbolEntry &addEntry(const Location location, const char *const name, 
                          SymbolEntry::SymbolKind kind, PTypeSharedPtr type);

    SymbolEntry *lookup(std::string name);
private:
    /* data */
    uint8_t level;
    std::vector<SymbolEntry> entries;

    void dumpDemarcation(const char chr);
};

#endif