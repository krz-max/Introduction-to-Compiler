#ifndef SEMA_SYMBOL_ENTRY_H
#define SEMA_SYMBOL_ENTRY_H

#include "AST/PType.hpp"
#include "AST/utils.hpp"
#include "AST/constant.hpp"
#include "AST/function.hpp"

class SymbolEntry
{
public:
    enum class SymbolKind : uint8_t {
        kProgram,
        kFunction,
        kParameter,
        kVariable,
        kLoopVar,
        kConstant
    };
    union Attribute {
        const Constant *constant;
        const FunctionNode *function;
    };
    SymbolEntry(const Location location, const char *const name, SymbolKind kind,
                uint8_t level, PTypeSharedPtr type);
    ~SymbolEntry() = default;

    void dump() const;

    void setConstantAttribute(const Constant *attr) { Attr.constant = attr; }
    void setFunctionAttribute(const FunctionNode *attr) { Attr.function = attr; }
    void setDeclarationError(bool DeclError) { this->DeclError = DeclError; }

    const std::string getName() const { return Name.id; }
    const Location &getLocation() const { return Name.location; }
    const uint8_t getLevel() const { return Level; }
    const SymbolKind getKind() const { return Kind; }
    const PTypeSharedPtr &getType() const { return Type; }
    bool isDeclarationError() const { return DeclError; }
    const FunctionNode *getFunctionAttribute() const { return Attr.function; }
    const Constant *getConstantAttribute() const { return Attr.constant; }

private:
    /* data */
    IdInfo Name; // Name & Location
    PTypeSharedPtr Type;
    SymbolKind Kind;
    uint8_t Level;
    Attribute Attr;

    bool DeclError = false;
};

#endif