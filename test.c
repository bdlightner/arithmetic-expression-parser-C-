#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

int main(void)
{
    double result = 0;;
    char expr[1024];

    while (1) {
        printf("? "); fflush(stdout);
        memset(expr, 0, sizeof(expr));
        gets(expr);
        if (*expr == '\0') break;
        result = Evaluate(expr);
        printf("%s = %.16g\n", expr, result);
        if (*GetParserErr()) printf("%s\n", GetParserErr());
    }
    return 0;
}
