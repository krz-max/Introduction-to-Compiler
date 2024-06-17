#include "AST/program.hpp"

// TODO
ProgramNode::ProgramNode(const uint32_t line, const uint32_t col,
                         const char *const p_name,
                         std::vector<DeclNode *> *var_decls,
                         std::vector<FunctionNode *> *funcs,
                         CompoundStatementNode *body):
    AstNode{line, col},
    name(p_name),
    declarations(var_decls),
    functions(funcs),
    body(body) {}

// visitor pattern version: 
const char *ProgramNode::getNameCString() const { return name.c_str(); }

void ProgramNode::accept(AstNodeVisitor &p_visitor) { p_visitor.visit(*this); }

void ProgramNode::visitChildNodes(AstNodeVisitor &p_visitor) { // visitor pattern version
    for (auto &decl : *declarations) {
        decl->accept(p_visitor);
    }
    for (auto &func : *functions) {
        func->accept(p_visitor);
    }
    body->accept(p_visitor);
}
