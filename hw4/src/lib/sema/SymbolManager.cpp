#include "sema/SymbolManager.hpp"

SymbolEntry* SymbolManager::lookup(std::string name){
    for(auto it = tables.rbegin(); it != tables.rend(); ++it){
        auto result = (*it)->lookup(name);
        if(result != nullptr){
            return result;
        }
    }
    return nullptr;
}