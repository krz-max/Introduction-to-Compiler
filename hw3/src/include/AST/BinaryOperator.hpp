#ifndef __AST_BINARY_OPERATOR_NODE_H
#define __AST_BINARY_OPERATOR_NODE_H

#include "AST/expression.hpp"

#include <memory>

class BinaryOperatorNode : public ExpressionNode
{
public:
	BinaryOperatorNode(const uint32_t line, const uint32_t col,
					   ExpressionNode *left,
					   Operator *opr,
					   ExpressionNode *right);
	~BinaryOperatorNode();

	const char *getOperatorCString() const;
	void accept(AstNodeVisitor &p_visitor) override;
	void visitChildNodes(AstNodeVisitor &p_visitor) override;
private:
	// TODO: operator, expressions
	ExpressionNode *left;
	Operator *opr;
	ExpressionNode *right;
};

#endif
