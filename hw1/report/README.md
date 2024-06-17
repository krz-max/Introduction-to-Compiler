# hw1 report

|Field|Value|
|-:|:-|
|Name|黃威凱|
|ID|109511028|

## How much time did you spend on this project

roughly 5 hours(14:00~19:00)

## Project overview

I merged all keywords into one class and called it in the transition rule section

I implemented `arithmetic`, `delimeter`, `relational`, and `logic` respectively and merge them using `|`

For integers and floats, I separate `zero` and `nonzero` as different regex because it is easier.

I defined a new state for the `comment` section, it enters `comment` state when it captures `/*` and leaves upon recognizing the closing symbol `*/`

These are my regex definition:
```
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
```


## What is the hardest you think in this project

To deal with multiple '"' in string constant

I didn't know I can specify the existence using the syntax `\"{2}`

Besides, to remove `"` in string, this is how I implemented:
```
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
```
So basically, if the input is `"aaaa""bbbb"`, the resulting string will become `aaaa"bbbb`

## Feedback to T.A.s
Good Assignment for a beginner !
