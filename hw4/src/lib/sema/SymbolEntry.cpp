#include "sema/SymbolEntry.hpp"
#include <iostream>
const char *kSymbolKindString[] = {
    "program",
    "function",
    "parameter",
    "variable",
    "loop_var",
    "constant"
};

SymbolEntry::SymbolEntry(const Location location, const char *const name, SymbolKind kind,
                uint8_t level, PTypeSharedPtr type)
    : Name(location.line, location.col, name),
      Kind(kind),
      Level(level),
      Type(type) {}

void SymbolEntry::dump() const {
    printf("%-33s", Name.id.c_str());
    // printf("%-11u", static_cast<uint8_t>(Kind));
    printf("%-11s", kSymbolKindString[static_cast<uint8_t>(Kind)]);
    printf("%d%-10s", Level, Level ? "(local)" : "(global)");
    printf("%-17s", Type->getPTypeCString());

    switch (Kind) {
        case SymbolKind::kConstant:
            printf("%-11s", Attr.constant->getConstantValueCString());
            break;
        case SymbolKind::kFunction:
            printf("%-11s", Attr.function->getParametersTypeCString());
            break;
        default:
            printf("%-11s", "");
            break;
    }

    puts("");
}