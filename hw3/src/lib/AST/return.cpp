#include "AST/return.hpp"

// TODO
ReturnNode::ReturnNode(const uint32_t line, const uint32_t col,
                       ExpressionNode* expr):
    AstNode{line, col},
    expression(expr) {}

void ReturnNode::accept(AstNodeVisitor &p_visitor) {
    p_visitor.visit(*this);
}

void ReturnNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    expression->accept(p_visitor);
}
