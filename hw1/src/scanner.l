%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define LIST_SOURCE                 appendToCurrentLine(yytext)
#define LIST_TOKEN(name)            do { if(opt_src) LIST_SOURCE; if(opt_tok) printf("<%s>\n", name); } while(0)
#define LIST_KEYWORDS(name)            do { if(opt_src) LIST_SOURCE; if(opt_tok) printf("<KW%s>\n", name); } while(0)
#define LIST_LITERAL(name, literal) do { if(opt_src) LIST_SOURCE; if(opt_tok) printf("<%s: %s>\n", name, literal); } while(0)
#define MAX_LINE_LENG               512

uint32_t line_num     = 1;
static char current_line[MAX_LINE_LENG];

static uint32_t opt_src = 1;
static uint32_t opt_tok = 1;
static char *current_line_ptr = current_line;

static void appendToCurrentLine(const char *yytext_ptr);

%}

/* Basic 3~6 */
digits [0-9]
letters [a-zA-Z]
abced [,:;(\)[\]]
equal [<>:]?=
relation <|>|<>
logic and|or|not
arith mod|[+\-*/]

/* KeyWords */
flow while|do|if|then|else|for|to
value true|false
type array|of|boolean|integer|real|string
decl var|def
block begin|end
statement print|read|return
KW {flow}|{value}|{type}|{decl}|{block}|{statement}

/* Identifier */
idtfr {letters}+({letters}|{digits})*

/* Integer */
/* Decimal */
dec 0|[1-9][0-9]*
/* Octal */
oct 0[0-7]+

/* Floating */
float "0.0"|{dec}\.[0-9]*[1-9]+
/* real */
real [1-9][0-9]*|0\.[1-9]+[0-9]*|[1-9]*\.[0-9]*
string_const \"([^\"]|\"{2})*\"

%State comment

%%
<comment>[^\n]*"*/" {
    LIST_SOURCE;
    BEGIN INITIAL;
}
<comment>[^*\n]* {
    LIST_SOURCE;
}
"/*" {
    LIST_SOURCE;
    BEGIN comment;
}
"//&S"[+\-].* {
    opt_src = (yytext[4] == '+') ? 1 : 0;
    LIST_SOURCE;
}
"//&T"[+\-].* {
    opt_tok = (yytext[4] == '+') ? 1 : 0;
    LIST_SOURCE;
}

"//".* {
    LIST_SOURCE;
}

{string_const} {
    char temp[MAX_LINE_LENG];
    int j = 0;
    for(int i = 1; i < yyleng-1; i++){
        if(yytext[i] == '"' && yytext[i+1] == '"'){
            i++;
        }
        temp[j++] = yytext[i];
    }
    temp[j] = '\0';
    LIST_LITERAL("string", temp);
}

{KW} { LIST_KEYWORDS(yytext); }

{logic}|{relation}|{equal}|{arith} { LIST_TOKEN(yytext); }
{abced} { LIST_TOKEN(yytext); }

{idtfr} { LIST_LITERAL("id", yytext); }

{dec} { LIST_LITERAL("integer", yytext); }
{oct} { LIST_LITERAL("oct_integer", yytext); }
{float} { LIST_LITERAL("float", yytext); }

{real}(E|e)([+\-]?){dec} { LIST_LITERAL("scientific", yytext); }


" " |
\t  { LIST_SOURCE; }
\n {
    if (opt_src) {
        printf("%d: %s\n", line_num, current_line);
    }
    ++line_num;
    current_line[0] = '\0';
    current_line_ptr = current_line;
}

    /* Catch the character which is not accepted by rules above */
. {
    printf("Error at line %d: bad character \"%s\"\n", line_num, yytext);
    exit(-1);
}

%%

static void appendToCurrentLine(const char *yytext_ptr) {
    while (*yytext_ptr) {
        *current_line_ptr = *yytext_ptr;
        ++current_line_ptr;
        ++yytext_ptr;
    }
    *current_line_ptr = '\0';
}

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [filename]\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        fprintf(stderr, "Open file error\n");
        exit(-1);
    }

    yylex();

    fclose(yyin);
    /* Free resources used by the scanner; otherwise, unused data structures
    will persist throughout the successive compiler phases. */
    yylex_destroy();

    return 0;
}
