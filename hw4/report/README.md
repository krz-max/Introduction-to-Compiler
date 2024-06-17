# hw4 report

|||
|-:|:-|
|Name|黃威凱|
|ID|109511028|

## How much time did you spend on this project

> ~ 10 hrs

## Project overview

> Please describe the structure of your code and the ideas behind your implementation in an organized way.
> The point is to show us how you deal with the problems. It is not necessary to write a lot of words or paste all of your code here.
### Semantic Analyzer
* Brief:
    * Traverse AST using visitor pattern.
    * Stores `Symbol Table` and request for update when visiting nodes in the program.
    * Dump error message after finishing analyzing the program.
* Inherit from `AstNodeVisitor` with virtual functions `visit`
* Data Member
    * bool dump_table;
        * Indicates whether this analyzer should dump the symbol table or not.
    * bool is_in_loop_body = false;
        * This flag is set when entering `ForNode` and checked in the `Assignment Node`, so that it prompts an error when trying to assign the `loop variable` value within the body of `ForNode`.
        * Handling the error message: `the value of loop variable cannot be modified inside the loop body`
    * std::stack<SymbolEntry *> FunctionStack;
        * Pushes a `SymbolEntry` whenever entering a `Program` or `Function`, so that we can trace what kind of type should this section of code returns.
        * Handling the error messages:
            * `program/procedure should not return a value`
            * `return '\<type1\>' from a function with return type '\<type2>'`
    * SymbolManager symbol_manager;
        * The manager for the stack of symbol tables.
    * SymbolEntry::Attribute current_attribute;
        * `VariableNode` enters class `constant` after entering a `ConstantValueNode`. 
        * Therefore, we stores the `constant` when entering `ConstantValueNode`, so that we can set `Attribute` for the `SymbolEntry` in `SymbolTable` after we are back in the `VariableNode` from `ConstantValueNode`.
    * std::vector<ErrorMessage> errors;
        * Just a list of `ErrorMessage`.
    * SymbolEntry::SymbolKind current_symbol_kind;
        * Control the `Kind` of  each `SymbolEntry` when traversing the `AST`
        * Used in:
            * `VariableNode`
            * `ConstantValueNode`
            * `ForNode`
            * `ReturnNode`
            * `FunctionNode`
            * `ProgramNode`
* Member Functions
    * bool hasErrors() const { return !errors.empty(); }
    * void dumpError(const char* srcPath);
        * Read from srcFile for the original code.
        * Calls `dumpErrors()`
    * void dumpErrors(const ErrorMessage &error, const std::string &sourceLine);
        * With `Location` in `error`, we can use `line` and `col` to print the errormessage properly.
    * All other `visit`.
        * Basically we `visitchildNodes`, and then check the error condition, and finally add to `SymbolTable` if there are no errors.
#### Handling Error Message
* Use a `struct` to store the messages.
    * Data Member
        * Location location;
        * std::string msg;
    * No member function
### Symbol Entry
* Brief:
    * Contains the information of `Symbol Entries`.
    * Output the information when requested.
* enum class SymbolKind:
    ```
    enum class SymbolKind : uint8_t {
        kProgram,
        kFunction,
        kParameter,
        kVariable,
        kLoopVar,
        kConstant
    };
    ```
* Union Attribute:
    ```
    union Attribute {
        const Constant *constant;
        const FunctionNode *function;
    };
    ```
* Data Member
    * IdInfo Name; // Name & Location
        * The column `Name` in `Symbol Table`
    * PTypeSharedPtr Type;
        * The column `Type` in `Symbol Table`
    * SymbolKind Kind;
        * The column `Kind` in `Symbol Table`
    * uint8_t Level;
        * The column `Level` in `Symbol Table`
        * Also used in handling error message: `redeclaration`
    * Attribute Attr;
        * `Function Argument Types` or `Constant Value`
    * bool DeclError = false;
        * For handling error message: ` declared as an array with an index that is not greater than 0`
* Member functions
    * Just some setters and getters.
    ```
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
    ```
### Symbol Manager
* Brief:
    * Manages `Symbol Tables` and current `Scope level`. 
    * Do some `lookup` for declared `Symbol Entries`.
    * Print the information of `Symbol Tables`
* Data Member:
    * uint8_t level;
    * std::vector<SymbolTableSharedPtr> tables;
* Member Function:
    ```
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
    ```
    * The `lookup` function here traverse **all the tables** currently in `tables`.
### Symbol Table
* Data Member
    * uint8_t level;
    * std::vector<SymbolEntry> entries;
* Member Function:
    * `void dumpTable()`
    * `SymbolEntry &addEntry(const Location location, const char *const name,SymbolEntry::SymbolKind kind, PTypeSharedPtr type)`
    * `SymbolEntry *lookup(std::string name)`
        * Look up in the Table to see if the Symbol is already declared

## What is the hardest you think in this project

> Not required, but bonus point may be given.

## Feedback to T.A.s

> Not required, but bonus point may be given.
