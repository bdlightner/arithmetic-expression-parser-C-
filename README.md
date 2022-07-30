# arithmetic-expression-parser-C-
Full-featured arithmetic expression parser with variables implemented in the C language

Parsing is done by processing a "string" expression (i.e., text) and producing a result. The calculations are done using floating-point numbers (doubles) to give maximum accuracy and allow for fractional results.

	Example: 2 + 2
	Result: 4
	Example: log10(1234)
	Result: 3.09132

The parser builds up results "on the fly" reporting syntax problems by returning an exception.

	Example: 1 + 3 + )
	exception: Unexpected token: ‘)’
	
Run-time errors (principally "divide by zero") are also returned as an exception:

	Example: 2 / 0
	exception: Divide by zero

The value NaN is returned by the expression parser whenever an exception is returned (i.e., sqrt(-1)).

### Supported operations

Basic arithmetic: + - / * ^

	Example: 1000 + 123
	Result: 1123
	Example: 44 * 55
	Result: 2420
	Example: 10^3
	Result: 1000
	
Expressions are evaluated from left-to-right, with divide, multiply and exponentiation taking precedence over add and subtract.

	Example: 2 + 3 * 6
	Result: 20 (the multiply is done first)
	Example: 1+10^2
	Result: 101
	
Parentheses can be used to change evaluation order.

	Example: (2 + 3) * 6
	Result: 30 (the add is done first)
	
Whitespace is ignored. However 2-character symbols (such as ==) cannot have imbedded spaces.

### Using in a Program

To use the parser in C code do this:

```C
#include "parser.h"

double result = Evaluate ("2 + 3 * 6");
```

### Symbols

The parser supports an unlimited number of named symbols (eg. "str", "dex") which can be pre-assigned values or assigned during use of the parser.

	Example: a=42, b=6, a*b
	Result: 252
	
There are two built-in symbols:

	pi = 3.1415926535897932385
	e = 2.7182818284590452354
	
Symbols can be any length up to 256 characters, and must consist of A-Z, a-z, or 0-9, or the underscore character. They must start with A-Z or a-z. Symbols are case-sensitive.

### Assignment

Pre-loaded symbols, or ones created on-the-fly, can be assigned to, including the standard C operators of +=, -=, *= and /=.

	Example: a=42, a/=7
	Result: 6
	Example: dex = 10, dex += 22
	Result: 32

Example of symbol pre-assignment and later retrieval (in C):
```C
#include "parser.h"
int ok;
double str;
double result;
SaveSymbol("str", 55); // assign value to "str"
SaveSymbol("dex", 67); // assign value to "dex"
result = Evaluate ("str + dex * 2"); // use in expression
ok = LookupSymbol("pi", &str);  // retrieve value of "str"
```
This effectively lets you not only return a result (the evaluated expression) but change other symbols as side-effects.

### Comparisons

You can compare values for less, greater, greater-or-equal etc. using the normal C operators.

	Example: 2 + 3 > 6 + 8
	Result: 0 (false)
	Example; 2 + 3 < 6 + 8
	Result: 1 (true)
	
The comparison operators are: <, <=, >, >=, ==, !=
These are a lower precedence than arithmetic, in other words addition and subtraction, divide and multiply will be done before comparisons.

### Logical operators

You can use AND, OR, and NOT (using these C symbols: &&, ||, ! )

	Example: a > 4 && b > 8 // (a > 4 AND b > 8)
	Example: a < 10 || b == 5 // (a < 10 OR b == 5)
	Example: !(a < 4) // NOT (a < 4)
	
These are a lower precedence than comparisons, so the examples above will work "naturally".

### Other functions

Various standard scientific functions are supported, by using:

	function (argument) or function (argument1, argument2)
	
#### Single-argument functions 

Single-argument functions include: abs, acos, asin, atan, atanh, ceil, cos, cosh, exp, exp, floor, log, log10, sin, sinh, sqrt, tan, and tanh.

These behave as documented in the C runtime library.

	Example: sqrt (64)
	Result: 8
	
Note that functions like sin, cos and tan use radians, not degrees. To convert to radians, take degrees and multiply by pi / 180 (the value of pi is built-in).

	Example: sin (45 * pi / 180)
	Result: 0.7071
	
Three other functions which are not directly in the standard library are:

#### int

	int (arg) <-- drops the fractional part

	Example: int (1.2)
	Result: 1
	Example: int (-1.2)
	Result: -1
	
#### rand

	rand (arg) <-- returns a number in the range 0 to arg

The rand function returns an integer (whole number) result, it will never return arg itself.
eg. rand (3): might return: 0, 1 or 2 (and nothing else)

#### percent

	percent (arg) <-- returns true (1.0) arg % of the time, and false (0.0) the rest of the time

eg. percent (40) will be true 40% of the time

### Two-argument functions

	min (arg1, arg2) <-- returns whichever is the lower
	max (arg1, arg2) <-- returns whichever is the higher
	mod (arg1, arg2) <-- returns the remainder of arg1 / arg2 - throws an exception if arg2 is zero
	pow (arg1, arg2) <-- returns arg1 to the power arg2 (same result as arg1^arg2)
	roll (arg1, arg2) <-- rolls an arg2-sided dice arg1 times. (unsupported with Windows)

	Example: min (10, 20)
	Result: 10
	
### If Test

You can do "if" tests by using the "if" function.

if (test-value, true-value, false-value)

	Example: if (a > 5, 22, 33)
	Result: if a > 5, returns 22
		if a <= 5, returns 33
		
### Comma operator

Some of the examples above use the "comma operator" without really explaining it.
In C, you can separate an expression into parts by using the comma operator. eg.

	a=10, b=20, a + b

This effectively breaks the expression into sub-expressions, as the comma operator has the lowest priority. This means that everything between the commas is done first.
However, as the expression is evaluated left-to-right, what the above example does is:

1.	Assign 10 to a
2.	Assign 20 to b
3.	Add a to b
4.	The result of the expression is 30

## Credits
This work derived from “Expression Parser written in C++” by Nick Gammon (14 September 2004) located at https://github.com/nickgammon/parser. First converted to pure ANSI C by Bruce D. Lightner (lightner@lightner.net), La Jolla, California in July 2022.

## Licensing
Original C++ code (c) Copyright Nick Gammon 2004. Permission to copy, use, modify, sell and distribute this software is granted provided this copyright notice appears in all copies. This software is provided "as is" without express or implied warranty, and with no claim as to its suitability for any purpose.

Modified C code (c) Copyright Bruce D Lightner 2022. Permission to copy, use, modify, sell and distribute this software is granted provided both copyright notices appear in all copies. This software is provided "as is" without express or implied warranty, and with no claim as to its suitability for any purpose.
