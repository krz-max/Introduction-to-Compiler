# hw3 report

|||
|-:|:-|
|Name|黃威凱|
|ID|109511028|

## How much time did you spend on this project

> I have no idea...

## Project overview

### How to decide each member in each class?

Take `BinaryOperator` for example:

From the rule:

![image](https://github.com/compiler-f23/hw3-krz-max/assets/79355721/11212795-41d3-4a1b-8281-211b6c577a48)

We can draw an AST like this:

![image](https://github.com/compiler-f23/hw3-krz-max/assets/79355721/adf639ea-c5e1-4b53-a8d2-a39d454f79fc)

So the members should contain:

![image](https://github.com/compiler-f23/hw3-krz-max/assets/79355721/ea88ebde-cc42-4d17-8914-048677c5f4ca)

#### AssignmentNode

![image](https://github.com/compiler-f23/hw3-krz-max/assets/79355721/aafac777-ae09-44e3-80df-adeacf6477d9)

#### UnaryOperator

![image](https://github.com/compiler-f23/hw3-krz-max/assets/79355721/363549b8-b965-488b-ae61-4f7b80eb19b8)

#### CompoundStatement

![image](https://github.com/compiler-f23/hw3-krz-max/assets/79355721/bdf979d4-36ae-4a61-87ec-294028c6e745)

#### For

![image](https://github.com/compiler-f23/hw3-krz-max/assets/79355721/5fe97159-65e9-48a4-84d1-7ab1b0cafed4)

Because they are similar, I just list a few here. You can see more in `report/notes.pdf`

### How to handle variable values and types?

I created a new class `AstType`, which contains:
1. DataType
2. Dimensions ( for array )

The values of variables are handled in `ConstantValueNode`, it has four members to store diffrent datatype values and corresponding constructors:
```
public:
	ConstantValueNode(const uint32_t line, const uint32_t col, int int_value);
	ConstantValueNode(const uint32_t line, const uint32_t col, float real_value);
	ConstantValueNode(const uint32_t line, const uint32_t col, bool bool_value);
	ConstantValueNode(const uint32_t line, const uint32_t col,
					  const char *const string_value);
private:
  int int_value = 0;
	float real_value = 0.0f;
	bool bool_value = false;
	std::string string_value;
```

It also contains another member to store the types:
```
DataType type;
```

By reading the value in `type`, we can decide which member to access and return the correct value.

![image](https://github.com/compiler-f23/hw3-krz-max/assets/79355721/cb090bcb-1b51-4450-8bca-0e3ea07098d4)

### Scanner.l and Parser.y

#### Parser.y

I figure that we are creating nodes when doing reduction, so each actions should be added at the end of each rule. Take the below derivation for example:
```
Expression -> IntegerAndReal -> INT_LITERAL
```

When `INT_LITERAL` is reduced to `IntegerAndReal`, a `ConstantValueNode` is created and assigned to `IntegerAndReal`
```
IntegerAndReal:
    INT_LITERAL {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column,
                                   $1);
    }
```

When `IntegerAndReal` is reduced to `Expression`,  the node is just assigned to `Expression`
```
Expression:
    IntegerAndReal {
        $$ = $1;
    }
```

#### Scanner.l

We also have to assign the value obtained from token to `yylval`, or `Parser.y` cannot know the value of the variable.
```
    /* Integer (decimal/octal) */
{integer} { 
    LIST_LITERAL("integer", yytext);
    yylval.int_value = atoi(yytext);
    return INT_LITERAL; 
}
0[0-7]+   { 
    LIST_LITERAL("oct_integer", yytext); 
    yylval.int_value = strtol(yytext, nullptr, 8);
    return INT_LITERAL; 
}
```

## What is the hardest you think in this project

I think the hardest part is to understand the structure and know what members should be added to each class ( I know there is a guideline, but it still cost me a lot of time to write the code ).

Besides, there are dependencies between diffrent classes, it is difficult to debug one testcase-by-one testcase.
I still think it is easier to build the nodes bottom-up, though it doesn't seem practical.

## Feedback to T.A.s

Although it is a great opportunity to practice designing a large program, I still hope that the data member will be predefined in each class so we can focus more on the function implementation ( also easier to understand the codebase ).
