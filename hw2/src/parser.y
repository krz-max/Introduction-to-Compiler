%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern int32_t line_num;    /* declared in scanner.l */
extern char current_line[]; /* declared in scanner.l */
extern FILE *yyin;          /* declared by lex */
extern char *yytext;        /* declared by lex */

extern int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

%token ',' ';' ':' '[' ']' '(' ')'
%token VAR ARRAY OF ASSIGN
%token BOOLEAN INTEGER REAL STRING
%token YES NO
%token DEF RETURN
%token START END WHILE DO IF THEN ELSE FOR TO
%token PRINT READ
%token ID INTCONST REALCONST STRLITERAL SCIENTIFIC

%left AND OR NOT
%left LT LE NE GE GT EQ
%left '+' '-'
%left '/' '%'
%left '*'

%%

/* Program Unit: Program, Function */
programUnit: program | function;
program: ID ';' dataDeclList functionList compoundStmt END;
function: functionDeclaration | functionDefinition;
functionList: /* empty */ | functionList1;
functionList1: function | functionList1 function;
functionDeclaration: ID '(' formalArgumentList ')' ':' scalarType ';' | ID '(' formalArgumentList ')' ';';
formalArgumentList: /* empty */ | formalArgumentList1;
formalArgumentList1: formalArgument | formalArgumentList1 ',' formalArgument;
functionDefinition: ID '(' formalArgumentList ')' ':' scalarType compoundStmt END | ID '(' formalArgumentList ')' compoundStmt END;

/* Data Types and Declaration */
dataDeclaration: VAR idList ':' type ';' | VAR idList ':' literalConst ';';
idList: ID | idList ',' ID;
formalArgument: idList ':' type;
type: scalarType | arrayType;
arrayType: ARRAY INTCONST OF type;
scalarType: BOOLEAN | INTEGER | REAL | STRING;
literalConst: INTCONST | '-' INTCONST | REALCONST | '-' REALCONST | STRLITERAL | SCIENTIFIC | '-' SCIENTIFIC | YES | NO;

/* Statements: Compound, Simple(Assignment, Print, Read), Conditional, While, For, Return, Function Call */
statements: compoundStmt | simpleStmt | conditionalStmt | whileStmt | forStmt | returnStmt | funcCallStmt;
/* Compound */
compoundStmt: START dataDeclList statementList END;
dataDeclList: /* empty */ | dataDeclList1;
dataDeclList1: dataDeclaration | dataDeclList1 dataDeclaration;
statementList: /* empty */ | statementList1;
statementList1: statements | statementList1 statements;
/* Simple */
simpleStmt: assignmentStmt | printStmt | readStmt;
assignmentStmt: variableReference ASSIGN expression ';';
variableReference: ID bracketExpressionList;
bracketExpressionList: /* empty */ | bracketExpressionList1;
bracketExpressionList1: '[' expression ']' | bracketExpressionList1 '[' expression ']';
printStmt: PRINT expression ';';
readStmt: READ variableReference ';';
/* Conditional */
conditionalStmt: ifStmt | ifElseStmt;
ifStmt: IF expression THEN compoundStmt END IF;
ifElseStmt: IF expression THEN compoundStmt ELSE compoundStmt END IF;
/* While */
whileStmt: WHILE expression DO compoundStmt END DO;
/* For */
forStmt: FOR ID ASSIGN INTCONST TO INTCONST DO compoundStmt END DO;
/* Return */
returnStmt: RETURN expression ';';
/* Function Call */
funcCallStmt: ID '(' commaExpressionList ')' ';';
commaExpressionList: /* empty */ | commaExpressionList1;
commaExpressionList1: expression | commaExpressionList1 ',' expression;

/* Expression: literal constant, variable reference, function call w/o the ';', arithmetic expression */
expression: literalConst | variableReference | ID '(' commaExpressionList ')' | arithmeticExpression;
arithmeticExpression: expression AND expression | expression OR expression | NOT expression
                    | expression Relational expression | expression Additive expression | expression mulDivMod expression
                    | '-' expression %prec '*' | '(' expression ')';

Relational: LT | LE | NE | GE | GT | EQ;
Additive: '+' | '-';
mulDivMod: '/' | '%' | '*';

%%

void yyerror(const char *msg) {
    fprintf(stderr,
            "\n"
            "|-----------------------------------------------------------------"
            "---------\n"
            "| Error found in Line #%d: %s\n"
            "|\n"
            "| Unmatched token: %s\n"
            "|-----------------------------------------------------------------"
            "---------\n",
            line_num, current_line, yytext);
    exit(-1);
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed");
        exit(-1);
    }

    yyparse();

    fclose(yyin);
    yylex_destroy();

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");
    return 0;
}
