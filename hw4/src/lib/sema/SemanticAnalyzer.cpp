#include "sema/SemanticAnalyzer.hpp"
#include "visitor/AstNodeInclude.hpp"
#include <iostream>
#include <fstream>

void SemanticAnalyzer::dumpError(const ErrorMessage &error, const std::string &line)
{
    fprintf(stderr, "<Error> Found in line %d, column %d: %s\n",
            error.location.line, error.location.col, error.msg.c_str());
    fprintf(stderr, "    %s\n", line.c_str());
    fprintf(stderr, "    %*c\n", error.location.col, '^');
}

void SemanticAnalyzer::dumpErrors(const char *srcPath)
{
    std::ifstream srcFile(srcPath, std::ios::in);

    std::vector<std::string> srcLines;
    std::string line;
    while (std::getline(srcFile, line))
    {
        srcLines.push_back(line);
    }
    srcFile.close();
    for (auto &error : errors)
    {
        dumpError(error, srcLines[error.location.line - 1]);
    }
}

void SemanticAnalyzer::visit(ProgramNode &p_program)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    symbol_manager.enterScope();
    current_symbol_kind = SymbolEntry::SymbolKind::kProgram;

    // Program Node is a Void Type.
    PTypeSharedPtr type_ptr = std::make_shared<PType>(PType::PrimitiveTypeEnum::kVoidType);
    auto symbol = symbol_manager.addEntry(p_program.getLocation(), p_program.getNameCString(),
                                          SymbolEntry::SymbolKind::kProgram, type_ptr);

    FunctionStack.push(&symbol);
    p_program.visitChildNodes(*this);
    FunctionStack.pop();

    // std::cout << "ProgramNode" << std::endl;
    if (dump_table)
        symbol_manager.dumpTopTable();
    symbol_manager.exitScope();
}

void SemanticAnalyzer::visit(DeclNode &p_decl)
{
    p_decl.visitChildNodes(*this);
}

void SemanticAnalyzer::visit(VariableNode &p_variable)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */

    // This will traverse its children and also set its attributes.
    p_variable.visitChildNodes(*this);

    // Redeclaration Error
    SymbolEntry *temp = symbol_manager.lookup(p_variable.getName());
    if (temp != nullptr)
    {
        if (temp->getLevel() == symbol_manager.getLevel() - 1 || temp->getKind() == SymbolEntry::SymbolKind::kLoopVar)
        {
            // fprintf(stderr, "Redeclaration !!!\n");
            errors.emplace_back(ErrorMessage(p_variable.getLocation(), "symbol '" + std::string(p_variable.getName()) + "' is redeclared"));
            return;
        }
    }
    // Array size must be greater than 0
    bool DeclarationError = false;
    if (p_variable.getType()->getDimension() > 0)
    {
        std::vector<uint64_t> dimension = p_variable.getType()->getDimensionArray();
        // Check if all the dimensions are greater than 0
        for (auto &dim : dimension)
        {
            if (dim <= 0)
            {
                // fprintf(stderr, "Array size must be greater than 0 !!!\n");
                errors.emplace_back(ErrorMessage(p_variable.getLocation(), "'" + p_variable.getName() + "' declared as an array with an index that is not greater than 0"));
                DeclarationError = true;
            }
        }
    }
    switch (current_symbol_kind)
    {
    /*
    In the scope of ProgramNode, every variableNode is of Kind kVariable.
     */
    case SymbolEntry::SymbolKind::kProgram:
        symbol_manager.addEntry(p_variable.getLocation(), p_variable.getNameCString(),
                                SymbolEntry::SymbolKind::kVariable, p_variable.getType());
        break;
    /*
    In the scope of FunctionNode, every variableNode is of Kind kParameter.
     */
    case SymbolEntry::SymbolKind::kFunction:
        symbol_manager.addEntry(p_variable.getLocation(), p_variable.getNameCString(),
                                SymbolEntry::SymbolKind::kParameter, p_variable.getType());
        break;
    case SymbolEntry::SymbolKind::kVariable:
        symbol_manager.addEntry(p_variable.getLocation(), p_variable.getNameCString(),
                                SymbolEntry::SymbolKind::kVariable, p_variable.getType())
            .setDeclarationError(DeclarationError);
        break;
    case SymbolEntry::SymbolKind::kConstant:
        symbol_manager.addEntry(p_variable.getLocation(), p_variable.getNameCString(),
                                SymbolEntry::SymbolKind::kConstant, p_variable.getType())
            .setConstantAttribute(current_attribute.constant);
        current_symbol_kind = SymbolEntry::SymbolKind::kVariable;
        break;
    case SymbolEntry::SymbolKind::kLoopVar:
        symbol_manager.addEntry(p_variable.getLocation(), p_variable.getNameCString(),
                                SymbolEntry::SymbolKind::kLoopVar, p_variable.getType());
        break;
    default:
        break;
    }
}

void SemanticAnalyzer::visit(ConstantValueNode &p_constant_value)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    current_symbol_kind = SymbolEntry::SymbolKind::kConstant;
    current_attribute.constant = p_constant_value.getConstant();
    p_constant_value.setReturnType(p_constant_value.getTypeSharedPtr()->getPrimitiveType());
}

void SemanticAnalyzer::visit(FunctionNode &p_function)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    SymbolEntry *symbol = nullptr;

    SymbolEntry *temp = symbol_manager.lookup(p_function.getName());
    if (temp != nullptr)
    {
        if (temp->getLevel() == symbol_manager.getLevel() - 1 || temp->getKind() == SymbolEntry::SymbolKind::kLoopVar)
        {
            // fprintf(stderr, "Redeclaration !!!\n");
            errors.emplace_back(ErrorMessage(p_function.getLocation(), "symbol '" + std::string(p_function.getName()) + "' is redeclared"));
        }
    }
    else
    {
        symbol = &(symbol_manager.addEntry(p_function.getLocation(), p_function.getNameCString(),
                                           SymbolEntry::SymbolKind::kFunction, p_function.getReturnType()));
        symbol->setFunctionAttribute(&p_function);
    }

    symbol_manager.enterScope();

    FunctionStack.push(symbol);
    current_symbol_kind = SymbolEntry::SymbolKind::kFunction;
    p_function.visitParameters(*this);

    current_symbol_kind = SymbolEntry::SymbolKind::kVariable;
    p_function.visitBody(*this);
    FunctionStack.pop();

    // std::cout << "FunctionNode" << std::endl;
    if (dump_table)
    {
        symbol_manager.dumpTopTable();
    }
    symbol_manager.exitScope();
}

void SemanticAnalyzer::visit(CompoundStatementNode &p_compound_statement)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    symbol_manager.enterScope();

    p_compound_statement.visitChildNodes(*this);

    if (dump_table)
        symbol_manager.dumpTopTable();
    symbol_manager.exitScope();
}

void SemanticAnalyzer::visit(PrintNode &p_print)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    p_print.visitChildNodes(*this);

    auto *returnType = p_print.getTarget().getReturnType();

    if( returnType && !returnType->isScalar() ){
        errors.emplace_back(ErrorMessage(p_print.getTarget().getLocation(), "expression of print statement must be scalar type"));
        return;
    }
}

void SemanticAnalyzer::visit(BinaryOperatorNode &p_bin_op)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    p_bin_op.visitChildNodes(*this);

    auto left_type = p_bin_op.getLeftOperandType();
    auto right_type = p_bin_op.getRightOperandType();
    if (!left_type || !right_type)
        return;

    Operator op = p_bin_op.getOp();
    // Check if Type of left operand and right operand are the same or can be upcast.
    switch (op)
    {
    case Operator::kPlusOp:
    {
        if(left_type->isString() && right_type->isString())
        {
            p_bin_op.setReturnType(PType::PrimitiveTypeEnum::kStringType);
            return;
        }
    }
    case Operator::kMinusOp:
    case Operator::kMultiplyOp:
    case Operator::kDivideOp:
    {
        if(!(left_type->isInteger() || left_type->isReal()) || !(right_type->isInteger() || right_type->isReal()))
            break;
        p_bin_op.setReturnType((left_type->isReal() || right_type->isReal()) ? PType::PrimitiveTypeEnum::kRealType : PType::PrimitiveTypeEnum::kIntegerType);
        return;
    }
    // Mod Operator
    case Operator::kModOp:
    {
        if(!left_type->isInteger() || !right_type->isInteger())
            break;
        p_bin_op.setReturnType(PType::PrimitiveTypeEnum::kIntegerType);
        return;
    }
    // Boolean Operator
    case Operator::kAndOp:
    case Operator::kOrOp:
    {
        if (!(left_type->isBool() && right_type->isBool()))
            break;
        p_bin_op.setReturnType(PType::PrimitiveTypeEnum::kBoolType);
        return;
    }
    // Relational Operator
    case Operator::kLessOp:
    case Operator::kLessOrEqualOp:
    case Operator::kGreaterOp:
    case Operator::kGreaterOrEqualOp:
    case Operator::kEqualOp:
    case Operator::kNotEqualOp:
    {
        if (!(left_type->isInteger() || left_type->isReal()) || !(right_type->isInteger() || right_type->isReal()))
            break;
        p_bin_op.setReturnType(PType::PrimitiveTypeEnum::kBoolType);
        return;
    }
    default:
        break;
    }
    errors.emplace_back(ErrorMessage(p_bin_op.getLocation(), "invalid operands to binary operator '" + std::string(p_bin_op.getOpCString()) + "' ('" + p_bin_op.getLeftOperandType()->getPTypeCString() + "' and '" + p_bin_op.getRightOperandType()->getPTypeCString() + "')"));
}

void SemanticAnalyzer::visit(UnaryOperatorNode &p_un_op)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    p_un_op.visitChildNodes(*this);

    auto operand_type = p_un_op.getOperandType();
    Operator op = p_un_op.getOp();
    switch (op)
    {
    case Operator::kNegOp: {
        break;
    }
    case Operator::kNotOp: {
        if(!operand_type->isBool()) {
            errors.emplace_back(ErrorMessage(p_un_op.getLocation(), "invalid operand to unary operator '" + std::string(p_un_op.getOpCString()) + "' ('" + operand_type->getPTypeCString() + "')"));
            return ;
        }
        break;
    }
    default:
        break;
    }
}

void SemanticAnalyzer::visit(FunctionInvocationNode &p_func_invocation)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    p_func_invocation.visitChildNodes(*this);

    SymbolEntry *temp = symbol_manager.lookup(p_func_invocation.getName());

    if(temp == nullptr)
    {
        // fprintf(stderr, "Using undeclared function !!!\n");
        errors.emplace_back(ErrorMessage(p_func_invocation.getLocation(), "use of undeclared symbol '" + std::string(p_func_invocation.getName()) + "'"));
        return;
    }

    if(temp->getKind() != SymbolEntry::SymbolKind::kFunction)
    {
        // fprintf(stderr, "Using non-function symbol !!!\n");
        errors.emplace_back(ErrorMessage(p_func_invocation.getLocation(), "call of non-function symbol '" + std::string(p_func_invocation.getName()) + "'"));
        return;
    }
    const FunctionNode *InvokedFunction = temp->getFunctionAttribute();

    // const std::vector<std::unique_ptr<ExpressionNode>> &arguments = p_func_invocation.getArgs();
    // const std::vector<std::unique_ptr<DeclNode>> &parameters = InvokedFunction->getParameters();
    const auto &arguments = p_func_invocation.getArgs();
    const auto &parameters = InvokedFunction->getParameters();

    if(arguments.size() != parameters.size())
    {
        // fprintf(stderr, "Number of arguments mismatch !!!\n");
        errors.emplace_back(ErrorMessage(p_func_invocation.getLocation(), "too few/much arguments provided for function '" + std::string(p_func_invocation.getName()) + "'"));
        return;
    }
    for(size_t i = 0; i < arguments.size(); i++)
    {
        auto argument_type = arguments[i]->getReturnType();
        auto parameter_type = parameters[i]->getVariables()[0]->getType();
        if(argument_type->isInteger() && parameter_type->isReal()) continue;
        if(! (*argument_type == *parameter_type.get()) ){
            // fprintf(stderr, "Type of arguments mismatch !!!\n");
            errors.emplace_back(ErrorMessage(arguments[i]->getLocation(), "incompatible type passing '" 
                                                                            + std::string(argument_type->getPTypeCString()) 
                                                                            + "' to parameter of type '" 
                                                                            + std::string(parameter_type->getPTypeCString()) + "'"));
            return;
        }
    }
    p_func_invocation.setReturnType(temp->getType()->getPrimitiveType());
}

void SemanticAnalyzer::visit(VariableReferenceNode &p_variable_ref)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    p_variable_ref.visitChildNodes(*this);

    // Check if using undeclared variable
    // printf("VRef Name: %s\n", p_variable_ref.getName().c_str());
    SymbolEntry *temp = symbol_manager.lookup(p_variable_ref.getName());
    if (temp == nullptr)
    {
        // fprintf(stderr, "Using undeclared variable !!!\n");
        errors.emplace_back(ErrorMessage(p_variable_ref.getLocation(), "use of undeclared symbol '" + std::string(p_variable_ref.getName()) + "'"));
        return;
    }
    // Use of Non-variable Symbol
    if (temp->getKind() != SymbolEntry::SymbolKind::kVariable && temp->getKind() 
    !=  SymbolEntry::SymbolKind::kParameter && temp->getKind() 
    !=  SymbolEntry::SymbolKind::kLoopVar && temp->getKind() 
    !=  SymbolEntry::SymbolKind::kConstant)
    {
        errors.emplace_back(ErrorMessage(p_variable_ref.getLocation(), "use of non-variable symbol '" + std::string(p_variable_ref.getName()) + "'"));
        return;
    }
    // Referencing a improper declared variable, skip error message.
    if (temp->isDeclarationError())
    {
        return;
    }
    // index of array reference must be an integer
    for (auto &index : p_variable_ref.getIndices())
    {
        if (index->getReturnType() == nullptr)
            return;
        if (!index->getReturnType()->isInteger())
        {
            errors.emplace_back(ErrorMessage(index->getLocation(), "index of array reference must be an integer"));
            return;
        }
    }
    // there is an over array subscript on 'arr'
    if (p_variable_ref.getDimension() > temp->getType()->getDimension())
    {
        errors.emplace_back(ErrorMessage(p_variable_ref.getLocation(), "there is an over array subscript on '" + std::string(p_variable_ref.getName()) + "'"));
        return;
    }
    auto variableRefType = temp->getType()->getArrayReferenceType(p_variable_ref.getDimension());
    PType *p_type = new PType(temp->getType()->getPrimitiveType());
    p_type->setDimensions(variableRefType->getDimensionArray());
    p_variable_ref.setReturnType(p_type);
}

void SemanticAnalyzer::visit(AssignmentNode &p_assignment)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    p_assignment.visitChildNodes(*this);

    auto *lvalue = p_assignment.getLvalue();
    SymbolEntry *temp = symbol_manager.lookup(lvalue->getName());
    if(temp == nullptr) return ;

    if (lvalue->getReturnType()->isArray()) {
        errors.emplace_back(ErrorMessage(lvalue->getLocation(), "array assignment is not allowed"));
        return;
    }
    if (temp->getKind() == SymbolEntry::SymbolKind::kConstant)
    {
        errors.emplace_back(ErrorMessage(lvalue->getLocation(), "cannot assign to variable '" 
                                                                    + std::string(lvalue->getName())
                                                                    + "' which is a constant"));
        return;
    }
    if(is_in_loop_body && temp->getKind() == SymbolEntry::SymbolKind::kLoopVar)
    {
        errors.emplace_back(ErrorMessage(lvalue->getLocation(), "the value of loop variable cannot be modified inside the loop body"));
        return;
    }
    auto *rvalue = p_assignment.getRvalue();
    if(rvalue == nullptr) return ;
    if(rvalue->getReturnType() == nullptr) return ;
    if(rvalue->getReturnType()->isArray()){
        errors.emplace_back(ErrorMessage(rvalue->getLocation(), "array assignment is not allowed"));
        return ;
    }
    if(rvalue->getReturnType()->isInteger() && lvalue->getReturnType()->isReal()) return ;
    if(!(*rvalue->getReturnType() == *lvalue->getReturnType()))
    {
        errors.emplace_back(ErrorMessage(p_assignment.getLocation(), "assigning to '"
                                                                    + std::string(lvalue->getReturnType()->getPTypeCString())
                                                                    + "' from incompatible type '"
                                                                    + std::string(rvalue->getReturnType()->getPTypeCString()) + "'"));
        return ;
    }
    return ;
}

void SemanticAnalyzer::visit(ReadNode &p_read)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    p_read.visitChildNodes(*this);

    auto *returnType = p_read.getTarget().getReturnType();
    if(returnType == nullptr) return;
    if(returnType->isArray())
    {
        errors.emplace_back(ErrorMessage(p_read.getTarget().getLocation(), "variable reference of read statement must be scalar type"));
        return;
    }
    SymbolEntry *temp = symbol_manager.lookup(p_read.getTarget().getName());
    if(temp->getKind() == SymbolEntry::SymbolKind::kConstant)
    {
        errors.emplace_back(ErrorMessage(p_read.getTarget().getLocation(), "variable reference of read statement cannot be a constant or loop variable"));
        return;
    }
    if(temp->getKind() == SymbolEntry::SymbolKind::kLoopVar)
    {
        errors.emplace_back(ErrorMessage(p_read.getTarget().getLocation(), "variable reference of read statement cannot be a constant or loop variable"));
        return;
    }
}

void SemanticAnalyzer::visit(IfNode &p_if)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    p_if.visitChildNodes(*this);

    auto *condition = p_if.getCondition();
    if(condition->getReturnType() == nullptr) return ;
    if(condition->getReturnType()->isBool()) return ;
    errors.emplace_back(ErrorMessage(condition->getLocation(), "the expression of condition must be boolean type"));
    return ;
}

void SemanticAnalyzer::visit(WhileNode &p_while)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    p_while.visitChildNodes(*this);

}

void SemanticAnalyzer::visit(ForNode &p_for)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    symbol_manager.enterScope();
    is_in_loop_body = false;
    current_symbol_kind = SymbolEntry::SymbolKind::kLoopVar;
    p_for.visitLoopVarDecl(*this);
    is_in_loop_body = true;
    current_symbol_kind = SymbolEntry::SymbolKind::kVariable;
    p_for.visitBody(*this);
    is_in_loop_body = false;

    if (dump_table)
        symbol_manager.dumpTopTable();
    symbol_manager.exitScope();

    auto *initialStatementRValue = p_for.getInitStatement()->getRvalue();
    auto *endCondition = p_for.getEndCondition();
    uint64_t initialValue = dynamic_cast<const ConstantValueNode *>(initialStatementRValue)->getConstant()->getConstantValue().integer;
    uint64_t endValue = dynamic_cast<const ConstantValueNode *>(endCondition)->getConstant()->getConstantValue().integer;
    if(initialValue < endValue) return ;
    errors.emplace_back(ErrorMessage(p_for.getLocation(), "the lower bound and upper bound of iteration count must be in the incremental order"));

}

void SemanticAnalyzer::visit(ReturnNode &p_return)
{
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Travere child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
    SymbolEntry::SymbolKind originalKind = current_symbol_kind;
    // Return Node Entering ConstantValueNode affects the correctness of declaring variables.
    p_return.visitChildNodes(*this);
    current_symbol_kind = originalKind;

    auto functionReturnType = FunctionStack.top()->getFunctionAttribute()->getReturnType();
    if(functionReturnType->isVoid() || FunctionStack.top()->getKind() == SymbolEntry::SymbolKind::kProgram) {
        if(p_return.getReturnValue() != nullptr)
            errors.emplace_back(ErrorMessage(p_return.getLocation(), "program/procedure should not return a value"));
        return ;
    }
    auto *returnType = p_return.getReturnValue()->getReturnType();
    if(returnType == nullptr) return ;
    if(*returnType == *functionReturnType) return ;
    errors.emplace_back(ErrorMessage(p_return.getReturnValue()->getLocation(), "return '"
                                                            + std::string(returnType->getPTypeCString())
                                                            + "' from a function with return type '"
                                                            + std::string(functionReturnType->getPTypeCString()) + "'"));
}
