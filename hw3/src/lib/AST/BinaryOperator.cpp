#include "AST/BinaryOperator.hpp"

BinaryOperatorNode::BinaryOperatorNode(const uint32_t line, const uint32_t col,
                                       ExpressionNode *left, Operator *opr,
                                       ExpressionNode *right):
    ExpressionNode(line, col),
    left(left),
    opr(opr),
    right(right) {}

BinaryOperatorNode::~BinaryOperatorNode() {
    delete left;
    delete opr;
    delete right;
}

const char *BinaryOperatorNode::getOperatorCString() const {
    return opr->getTypeCString();
}

void BinaryOperatorNode::accept(AstNodeVisitor &p_visitor) {
    p_visitor.visit(*this);
}

void BinaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    left->accept(p_visitor);
    right->accept(p_visitor);
}
