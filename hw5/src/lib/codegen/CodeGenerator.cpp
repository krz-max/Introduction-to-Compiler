#include "codegen/CodeGenerator.hpp"
#include "visitor/AstNodeInclude.hpp"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>

#include "visitor/AstNodeInclude.hpp"

static int current_sp = 0;
static int current_fp = 0;
static int current_offset = -12;

enum class RefType
{
    kLHS, // LValue Reference
    kRHS  // RValue Reference
};

static RefType var_ref_type;

static const char *kArgReg[] = {"a0", "a1", "a2", "a3", "a4", "a5",
                                "a6", "a7", "t3", "t4", "t5", "t6"};
CodeGenerator::CodeGenerator(const std::string &source_file_name,
                             const std::string &save_path,
                             SymbolManager *const p_symbol_manager)
    : m_symbol_manager_ptr(p_symbol_manager),
      m_source_file_path(source_file_name)
{
    // FIXME: assume that the source file is always xxxx.p
    const auto &real_path =
        save_path.empty() ? std::string{"."} : save_path;
    auto slash_pos = source_file_name.rfind("/");
    auto dot_pos = source_file_name.rfind(".");

    if (slash_pos == std::string::npos)
        slash_pos = 0;
    else
        slash_pos++;

    auto output_file_path{
        real_path + "/" +
        source_file_name.substr(slash_pos, dot_pos - slash_pos) + ".S"};
    m_output_file.reset(fopen(output_file_path.c_str(), "w"));
    assert(m_output_file.get() && "Failed to open output file");
}

static void dumpInstructions(FILE *p_out_file, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(p_out_file, format, args);
    va_end(args);
}

static void pushStack(FILE *output_file)
{
    constexpr const char *const push_stack_assembly =
        "    addi sp, sp, -128\n"
        "    sw ra, 124(sp)\n"
        "    sw s0, 120(sp)\n"
        "    addi s0, sp, 128\n";
    dumpInstructions(output_file, push_stack_assembly);
    current_sp += -128;
    current_fp = current_sp + 128;
    current_offset = -12;
}

static void popStack(FILE *output_file)
{
    constexpr const char *const pop_stack_assembly =
        "    lw ra, 124(sp)\n"
        "    lw s0, 120(sp)\n"
        "    addi sp, sp, 128\n"
        "    jr ra\n";
    dumpInstructions(output_file, pop_stack_assembly);
    current_sp += 128;
    current_fp = current_sp + 128;
    current_offset = -12;
}

static std::string getSectionLabel()
{
    static int label_count = 0;
    return std::string("L") + std::to_string(label_count++);
}

void CodeGenerator::visit(ProgramNode &p_program)
{
    // Generate RISC-V instructions for program header
    // clang-format off
    constexpr const char *const riscv_assembly_file_prologue =
        "    .file \"%s\"\n"
        "    .option nopic\n";
    // clang-format on
    dumpInstructions(m_output_file.get(), riscv_assembly_file_prologue,
                     m_source_file_path.c_str());

    // Reconstruct the hash table for looking up the symbol entry
    // Hint: Use symbol_manager->lookup(symbol_name) to get the symbol entry.
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(
        p_program.getSymbolTable());

    m_symbol_manager_ptr->pushGlobalScope();
    auto visit_ast_node = [&](auto &ast_node) { ast_node->accept(*this); };
    for_each(p_program.getDeclNodes().begin(), p_program.getDeclNodes().end(), visit_ast_node);
    for_each(p_program.getFuncNodes().begin(), p_program.getFuncNodes().end(), visit_ast_node);
    m_symbol_manager_ptr->popGlobalScope();
    dumpInstructions(m_output_file.get(),
                     ".section    .text\n"
                     "    .align 2\n"
                     "    .globl main\n"
                     "    .type main, @function\n"
                     "main:\n");
    pushStack(m_output_file.get());

    m_symbol_manager_ptr->pushScope();
    const_cast<CompoundStatementNode &>(p_program.getBody()).accept(*this);
    m_symbol_manager_ptr->popScope();

    popStack(m_output_file.get());
    dumpInstructions(m_output_file.get(), "    .size main, .-main\n");
    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(p_program.getSymbolTable());
}

void CodeGenerator::visit(DeclNode &p_decl) { p_decl.visitChildNodes(*this); }

void CodeGenerator::visit(VariableNode &p_variable)
{
    const char *var_name = p_variable.getNameCString();
    SymbolEntry *symbol_entry =
        const_cast<SymbolEntry *>(m_symbol_manager_ptr->lookup(var_name));

    // Global Scope
    if (symbol_entry->getLevel() == 0)
    {
        // Constants
        if (p_variable.getConstantPtr() != nullptr)
        {
            dumpInstructions(m_output_file.get(),
                             ".section    .rodata\n"
                             "    .align 2\n"
                             "    .globl %s\n"
                             "    .type %s, @object\n"
                             "%s:\n"
                             "    .word %s\n",
                             var_name, var_name, var_name,
                             p_variable.getConstantPtr()->getConstantValueCString());
        }
        // Variables
        else
        {
            dumpInstructions(m_output_file.get(), ".comm %s, %d, %d\n", var_name, 4, 4);
        }
        return;
    }
    // Function Parameters
    if (symbol_entry->getKind() == SymbolEntry::KindEnum::kParameterKind)
    {
        symbol_entry->setPosition(current_fp + current_offset);
        dumpInstructions(m_output_file.get(),
                         "    sw %s, %d(s0)\n",
                         kArgReg[(-current_offset - 12) / 4], current_offset);
        current_offset += -4;
        return;
    }
    // Local Scope
    if (symbol_entry->getLevel() > 0)
    {
        symbol_entry->setPosition(current_fp + current_offset);

        std::string init_value;
        if (p_variable.getConstantPtr() != nullptr)
        {
            init_value = p_variable.getConstantPtr()->getConstantValueCString();
        }
        else
        {
            init_value = "0";
        }

        dumpInstructions(m_output_file.get(),
                         "    addi t0, s0, %d\n"
                         "    addi sp, sp, -4\n"
                         "    sw t0, 0(sp)\n"
                         "    li t0, %s\n"
                         "    addi sp, sp, -4\n"
                         "    sw t0, 0(sp)\n"
                         "    lw t0, 0(sp)\n"
                         "    addi sp, sp, 4\n"
                         "    lw t1, 0(sp)\n"
                         "    addi sp, sp, 4\n"
                         "    sw t0, 0(t1)\n",
                         current_offset, init_value.c_str());
        current_offset += -4;
        return;
    }
    // The rest of the cases
    return;
}

void CodeGenerator::visit(ConstantValueNode &p_constant_value)
{
    const char *constant_value = p_constant_value.getConstantValueCString();

    if (p_constant_value.getTypePtr()->isBool())
        constant_value = p_constant_value.getConstantPtr()->boolean() ? "1" : "0";

    dumpInstructions(m_output_file.get(),
                     "    li t0, %s\n"
                     "    addi sp, sp, -4\n"
                     "    sw t0, 0(sp)\n",
                     constant_value);
    current_sp += -4;
}

void CodeGenerator::visit(FunctionNode &p_function)
{
    // Reconstruct the hash table for looking up the symbol entry
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(
        p_function.getSymbolTable());

    dumpInstructions(m_output_file.get(),
                     ".section    .text\n"
                     "    .align 2\n"
                     "    .globl %s\n"
                     "    .type %s, @function\n"
                     "%s:\n",
                     p_function.getName().c_str(), p_function.getName().c_str(),
                     p_function.getName().c_str());
    pushStack(m_output_file.get());

    p_function.visitChildNodes(*this);

    popStack(m_output_file.get());
    dumpInstructions(m_output_file.get(), "    .size %s, .-%s\n",
                     p_function.getName().c_str(), p_function.getName().c_str());
    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(p_function.getSymbolTable());
}

void CodeGenerator::visit(CompoundStatementNode &p_compound_statement)
{
    // Reconstruct the hash table for looking up the symbol entry
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(
        p_compound_statement.getSymbolTable());

    p_compound_statement.visitChildNodes(*this);

    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(
        p_compound_statement.getSymbolTable());
}

void CodeGenerator::visit(PrintNode &p_print)
{
    var_ref_type = RefType::kRHS;
    p_print.visitChildNodes(*this);

    dumpInstructions(m_output_file.get(),
                     "    lw a0, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    jal ra, printInt\n");
    current_sp += 4;
}

void CodeGenerator::visit(BinaryOperatorNode &p_bin_op)
{
    var_ref_type = RefType::kRHS;
    p_bin_op.visitChildNodes(*this);

    // Read the operands from t0 and t1.
    dumpInstructions(m_output_file.get(),
                     "    lw t0, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    lw t1, 0(sp)\n"
                     "    addi sp, sp, 4\n");

    switch (p_bin_op.getOp())
    {
    case Operator::kPlusOp:
    {
        dumpInstructions(m_output_file.get(), "    add t0, t1, t0\n");
        break;
    }
    case Operator::kMinusOp:
    {
        dumpInstructions(m_output_file.get(), "    sub t0, t1, t0\n");
        break;
    }
    case Operator::kMultiplyOp:
    {
        dumpInstructions(m_output_file.get(), "    mul t0, t1, t0\n");
        break;
    }
    case Operator::kDivideOp:
    {
        dumpInstructions(m_output_file.get(), "    div t0, t1, t0\n");
        break;
    }
    case Operator::kModOp:
    {
        dumpInstructions(m_output_file.get(), "    rem t0, t1, t0\n");
        break;
    }
    case Operator::kLessOp:
    {
        dumpInstructions(m_output_file.get(), "    slt t0, t1, t0\n");
        break;
    }
    case Operator::kLessOrEqualOp:
    {
        dumpInstructions(m_output_file.get(),
                         "    slt t0, t0, t1\n"
                         "    seqz t0, t0\n");
        break;
    }
    case Operator::kGreaterOp:
    {
        dumpInstructions(m_output_file.get(), "    slt t0, t0, t1\n");
        break;
    }
    case Operator::kGreaterOrEqualOp:
    {
        dumpInstructions(m_output_file.get(),
                         "    slt t0, t1, t0\n"
                         "    seqz t0, t0\n");
        break;
    }
    case Operator::kEqualOp:
    {
        dumpInstructions(m_output_file.get(),
                         "    sub t0, t1, t0\n"
                         "    seqz t0, t0\n");
        break;
    }
    case Operator::kNotEqualOp:
    {
        dumpInstructions(m_output_file.get(),
                         "    sub t0, t1, t0\n"
                         "    snez t0, t0\n");
        break;
    }
    case Operator::kAndOp:
    case Operator::kOrOp:
    case Operator::kNotOp:
        break;
    default:
        break;
    }

    // Store the result in t0.
    dumpInstructions(m_output_file.get(),
                     "    addi sp, sp, -4\n"
                     "    sw t0, 0(sp)\n");
    current_sp += 4;
}

void CodeGenerator::visit(UnaryOperatorNode &p_un_op)
{
    var_ref_type = RefType::kRHS;
    p_un_op.visitChildNodes(*this);

    if (p_un_op.getOp() == Operator::kNegOp)
    {
        dumpInstructions(m_output_file.get(),
                         "    lw t0, 0(sp)\n"
                         "    addi sp, sp, 4\n"
                         "    sub t0, zero, t0\n"
                         "    addi sp, sp, -4\n"
                         "    sw t0, 0(sp)\n");
    }
    else if (p_un_op.getOp() == Operator::kNotOp)
    {
        dumpInstructions(m_output_file.get(),
                         "    lw t0, 0(sp)\n"
                         "    addi sp, sp, 4\n"
                         "    seqz t0, t0\n"
                         "    addi sp, sp, -4\n"
                         "    sw t0, 0(sp)\n");
    }
}

void CodeGenerator::visit(FunctionInvocationNode &p_func_invocation)
{
    p_func_invocation.visitChildNodes(*this);

    for (int i = p_func_invocation.getArguments().size() - 1; i >= 0; i--)
    {
        dumpInstructions(m_output_file.get(),
                         "    lw %s, 0(sp)\n"
                         "    addi sp, sp, 4\n",
                         kArgReg[i]);
        current_sp += 4;
    }

    dumpInstructions(m_output_file.get(),
                     "    jal ra, %s\n"
                     "    mv t0, a0\n"
                     "    addi sp, sp, -4\n"
                     "    sw t0, 0(sp)\n",
                     p_func_invocation.getName().c_str());
    current_sp += -4;
}

void CodeGenerator::visit(VariableReferenceNode &p_variable_ref)
{
    const auto var_name = p_variable_ref.getName();
    const auto entry = m_symbol_manager_ptr->lookup(var_name);

    if (var_ref_type == RefType::kLHS)
    {
        if (entry->getLevel() == 0)
        {
            dumpInstructions(m_output_file.get(),
                             "    la t0, %s\n",
                             entry->getNameCString());
        }
        else
        {
            dumpInstructions(m_output_file.get(),
                             "    addi t0, s0, %d\n",
                             entry->getPosition() + current_fp);
        }
        dumpInstructions(m_output_file.get(),
                         "    addi sp, sp, -4\n"
                         "    sw t0, 0(sp)\n");
        current_sp += -4;
        return;
    }
    if (var_ref_type == RefType::kRHS)
    {
        if (entry->getLevel() == 0)
        {
            dumpInstructions(m_output_file.get(),
                             "    la t0, %s\n"
                             "    lw t1, 0(t0)\n"
                             "    mv t0, t1\n",
                             entry->getNameCString());
        }
        else
        {
            dumpInstructions(m_output_file.get(),
                             "    lw t0, %d(s0)\n",
                             entry->getPosition() - current_fp);
        }
        dumpInstructions(m_output_file.get(),
                         "    addi sp, sp, -4\n"
                         "    sw t0, 0(sp)\n");
        current_sp += -4;
        return;
    }
    return ;
}

void CodeGenerator::visit(AssignmentNode &p_assignment)
{
    var_ref_type = RefType::kLHS;
    const_cast<VariableReferenceNode &>(p_assignment.getLvalue()).accept(*this);

    var_ref_type = RefType::kRHS;
    const_cast<ExpressionNode &>(p_assignment.getExpr()).accept(*this);

    dumpInstructions(m_output_file.get(),
                     "    lw t0, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    lw t1, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    sw t0, 0(t1)\n");
    current_sp += 8;
}

void CodeGenerator::visit(ReadNode &p_read)
{

    var_ref_type = RefType::kLHS;
    const_cast<VariableReferenceNode &>(p_read.getTarget()).accept(*this);

    dumpInstructions(m_output_file.get(),
                     "    jal ra, readInt\n"
                     "    lw t0, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    sw a0, 0(t0)\n");
    current_sp += 4;
}

void CodeGenerator::visit(IfNode &p_if)
{
    const_cast<ExpressionNode &>(p_if.getCondition()).accept(*this);

    const auto true_label = getSectionLabel();
    const auto false_label = getSectionLabel();
    const auto end_label = getSectionLabel();

    dumpInstructions(m_output_file.get(),
                     "    lw t0, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    beqz t0, %s\n"
                     "%s:\n",
                     false_label.c_str(), true_label.c_str());
    current_sp += 4;

    const_cast<CompoundStatementNode &>(p_if.getBody()).accept(*this);

    dumpInstructions(m_output_file.get(),
                     "    j %s\n"
                     "%s:\n",
                     end_label.c_str(), false_label.c_str());

    const_cast<CompoundStatementNode &>(p_if.getElseBody()).accept(*this);

    dumpInstructions(m_output_file.get(), "%s:\n", end_label.c_str());
}

void CodeGenerator::visit(WhileNode &p_while)
{
    const auto start_label = getSectionLabel();
    const auto body_label = getSectionLabel();
    const auto end_label = getSectionLabel();

    dumpInstructions(m_output_file.get(),
                     "%s:\n",
                     start_label.c_str());

    const_cast<ExpressionNode &>(p_while.getCondition()).accept(*this);

    dumpInstructions(m_output_file.get(),
                     "    lw t0, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    beqz t0, %s\n"
                     "%s:\n",
                     end_label.c_str(), body_label.c_str());
    current_sp += 4;

    const_cast<CompoundStatementNode &>(p_while.getBody()).accept(*this);

    dumpInstructions(m_output_file.get(),
                     "    j %s\n"
                     "%s:\n",
                     start_label.c_str(), end_label.c_str());
}

void CodeGenerator::visit(ForNode &p_for)
{
    // Reconstruct the hash table for looking up the symbol entry
    m_symbol_manager_ptr->reconstructHashTableFromSymbolTable(p_for.getSymbolTable());

    const_cast<DeclNode &>(p_for.getDecl()).accept(*this);
    const_cast<AssignmentNode &>(p_for.getInitStmt()).accept(*this);

    const auto condition_label = getSectionLabel();
    const auto body_label = getSectionLabel();
    const auto end_label = getSectionLabel();

    dumpInstructions(m_output_file.get(), "%s:\n", condition_label.c_str());

    var_ref_type = RefType::kRHS;
    const_cast<VariableReferenceNode &>(p_for.getInitStmt().getLvalue()).accept(*this);
    const_cast<ConstantValueNode &>(p_for.getUpperBound()).accept(*this);

    dumpInstructions(m_output_file.get(),
                     "    lw t0, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    lw t1, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    bge t1, t0, %s\n"
                     "%s:\n",
                     end_label.c_str(), body_label.c_str());
    current_sp += 8;

    const_cast<CompoundStatementNode &>(p_for.getBody()).accept(*this);

    var_ref_type = RefType::kLHS;
    const_cast<VariableReferenceNode &>(p_for.getInitStmt().getLvalue()).accept(*this);
    var_ref_type = RefType::kRHS;
    const_cast<VariableReferenceNode &>(p_for.getInitStmt().getLvalue()).accept(*this);
    dumpInstructions(m_output_file.get(),
                     "    li t0, 1\n"
                     "    lw t1, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    add t0, t1, t0\n"
                     "    lw t1, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    sw t0, 0(t1)\n"
                     "    j %s\n"
                     "%s:\n",
                     condition_label.c_str(), end_label.c_str());
    current_sp += 8;

    // Remove the entries in the hash table
    m_symbol_manager_ptr->removeSymbolsFromHashTable(p_for.getSymbolTable());
}

void CodeGenerator::visit(ReturnNode &p_return)
{
    var_ref_type = RefType::kRHS;
    p_return.visitChildNodes(*this);

    dumpInstructions(m_output_file.get(),
                     "    lw t0, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    mv a0, t0 \n");
    current_sp += 4;
}
