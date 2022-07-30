#include <stdio.h>
#include <string.h>

#include "parser.h"

int main(void)
{
    double result = 0;;
    char *p, expr[1024];

    while (1) {
        printf("? "); fflush(stdout);
        fgets(expr, sizeof(expr), stdin);
        p = expr + strlen(expr) - 1;
        while (*p && *p < ' ') *(p--) = '\0';
        if (*expr == '\0') break;
        result = Evaluate(expr);
        printf("%s = %.16g\n", expr, result);
        if (*GetParserErr()) printf("%s\n", GetParserErr());
    }
    return 0;
}
