## CONDITIONAL BRANCHING SYNTAX (STRICT LINE TERMINATOR)

Each line must end with `;`, including inner blocks and closing clauses.

### IF Statement

if <condition> then;
    <statement_1>;
    <statement_2>;
endif;

### IF-ELSE Statement

if <condition> then;
    <statement_1>;
else;
    <statement_2>;
endif;

### IF-ELSEIF-ELSE Statement

if <condition> then;
    <statement_1>;
elseif <other_condition> then;
    <statement_2>;
else;
    <statement_3>;
endif;
## LOGIC SYMBOLS
> gt
< lt
>= gte
<= lte
!= di

> **Note:** Every line, including `then`, `else`, `elseif`, and `endif`, must end with `;` to be valid.