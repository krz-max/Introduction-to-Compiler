#include "AST/VariableReference.hpp"

VariableReferenceNode::VariableReferenceNode(const uint32_t line,
                                             const uint32_t col,
                                             const char *const name):
    ExpressionNode{line, col},
    name(name),
    dimensions(new std::vector<ExpressionNode *>()) {}

VariableReferenceNode::VariableReferenceNode(const uint32_t line,
                                             const uint32_t col,
                                             const char *const name,
                                             std::vector<ExpressionNode *> *dimensions):
    ExpressionNode{line, col},
    name(name),
    dimensions(dimensions) {}

VariableReferenceNode::~VariableReferenceNode() {
    for (auto &dim : *dimensions) {
        delete dim;
    }
    delete dimensions;
}

const char* VariableReferenceNode::getNameCString() const {
    return name.c_str();
}

void VariableReferenceNode::accept(AstNodeVisitor &p_visitor) {
    p_visitor.visit(*this);
}

void VariableReferenceNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    for (auto &dim : *dimensions) {
        dim->accept(p_visitor);
    }
}
