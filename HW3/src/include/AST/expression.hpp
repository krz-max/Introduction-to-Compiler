#ifndef __AST_EXPRESSION_NODE_H
#define __AST_EXPRESSION_NODE_H

#include <string>

#include "AST/ast.hpp"
#include "visitor/AstNodeVisitor.hpp"

namespace OperatorType
{
	const int UNARY_MINUS_OP = 0;
	const int MULTIPLY_OP = 1;
	const int DIVIDE_OP = 2;
	const int MOD_OP = 3;
	const int PLUS_OP = 4;
	const int MINUS_OP = 5;
	const int LESS_OP = 6;
	const int LESS_OR_EQUAL_OP = 7;
	const int GREATER_OP = 8;
	const int GREATER_OR_EQUAL_OP = 9;
	const int EQUAL_OP = 10;
	const int NOT_EQUAL_OP = 11;
	const int NOT_OP = 12;
	const int AND_OP = 13;
	const int OR_OP = 14;
	const int NONE = 15;
};

class Operator
{
public:
	Operator(int type);

	const char *getTypeCString();
private:
	int type;
	std::string type_str = "";

	void setTypeString();
};

class ExpressionNode : public AstNode
{
public:
	ExpressionNode(const uint32_t line, const uint32_t col);
	~ExpressionNode() = default;

protected:
	// for carrying type of result of an expression
	// TODO: for next assignment

};

#endif
