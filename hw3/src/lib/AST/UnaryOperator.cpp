#include "AST/UnaryOperator.hpp"

UnaryOperatorNode::UnaryOperatorNode(const uint32_t line, const uint32_t col,
                                     Operator *op, ExpressionNode *expr):
    ExpressionNode{line, col},
    op(op),
    expression(expr) {}

UnaryOperatorNode::~UnaryOperatorNode() {
    delete op;
    delete expression;
}

const char *UnaryOperatorNode::getOperatorCString() const {
    return op->getTypeCString();
}

void UnaryOperatorNode::accept(AstNodeVisitor &p_visitor) {
    p_visitor.visit(*this);
}

void UnaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    expression->accept(p_visitor);
}
