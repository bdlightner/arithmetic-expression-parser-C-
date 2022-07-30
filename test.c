#include <stdio.h>
#include <string.h>

#include "parser.h"

int main(void)
{
    char *p, expr[1024];
    double result = 0;;

    while (1) {
        printf("? "); fflush(stdout);  // prompt user for expression string
        fgets(expr, sizeof(expr), stdin);  // read text line
        p = expr + strlen(expr) - 1;  // trim input line
        while (p >= expr && *p < ' ') *(p--) = '\0';
        if (*expr == '\0') break;  // exit if no expression
        result = Evaluate(expr);  // evaluate entered expression
        printf("%s = %.16g\n", expr, result);  // show expression and result
        if (*GetParserErr()) printf("%s\n", GetParserErr()); // print error?
    }
    return 0;
}
