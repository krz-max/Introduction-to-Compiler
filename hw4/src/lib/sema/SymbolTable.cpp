#include "sema/SymbolTable.hpp"

void SymbolTable::dumpTable(){
    dumpDemarcation('=');
    printf("%-33s%-11s%-11s%-17s%-11s\n", "Name", "Kind", "Level", "Type",
           "Attribute");
    dumpDemarcation('-');

    auto dump_entry = [](auto &symbol_entry) { symbol_entry.dump(); };
    std::for_each(entries.begin(), entries.end(), dump_entry);

    dumpDemarcation('-');
}

void SymbolTable::dumpDemarcation(const char chr){
    for(size_t i = 0; i < 110; ++i){
        printf("%c", chr);
    }
    puts("");
}

SymbolEntry &SymbolTable::addEntry(const Location location, const char *const name,
                                   SymbolEntry::SymbolKind kind, PTypeSharedPtr type)
{
    entries.emplace_back(SymbolEntry(location, name, kind, level, type));
    return entries.back();
}

// Check if the symbol is already defined in the current scope
// Return true if the symbol is already defined
SymbolEntry* SymbolTable::lookup(std::string name){
    auto lookup_entry = [name](auto &symbol_entry) { return symbol_entry.getName() == name; };
    auto result = std::find_if(entries.begin(), entries.end(), lookup_entry);
    if(result != entries.end()){
        return &(*result);
    }
    return nullptr;
}