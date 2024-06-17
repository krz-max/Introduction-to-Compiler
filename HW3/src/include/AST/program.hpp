#ifndef AST_PROGRAM_NODE_H
#define AST_PROGRAM_NODE_H

#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/function.hpp"
#include "AST/CompoundStatement.hpp"

#include <string>
#include <vector>

class ProgramNode final : public AstNode
{
private:
	std::string name;
	std::vector<DeclNode *> *declarations;
	std::vector<FunctionNode *> *functions;
	CompoundStatementNode *body;
	// TODO: return type, declarations, functions, compound statement

public:
	ProgramNode(const uint32_t line, const uint32_t col,
				const char *const p_name,
				std::vector<DeclNode *> *declarations,
				std::vector<FunctionNode *> *functions,
				CompoundStatementNode *body);
	~ProgramNode() = default;

	// visitor pattern version: 
	const char *getNameCString() const;

	void accept(AstNodeVisitor &p_visitor) override;
	void visitChildNodes(AstNodeVisitor &p_visitor) override;
};

#endif
