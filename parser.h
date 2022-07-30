//
// Original parser C++ code from https://github.com/nickgammon/parser/
//

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/timeb.h>
#include <time.h>
#include <memory.h>
#include <setjmp.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>

// normally defined in <math.h>
#ifndef M_PI
#define M_PI (3.1415926535897932385)
#endif

#ifndef M_E
#define M_E (2.7182818284590452354)
#endif

char *GetParserErr(void);
double LookupSymbol(char *lhs);
int SaveSymbol(char *lhs, double rhs);

typedef struct _fun1_entry {
    char *name;  // function name string
    double (*fun)(double p1);
} const FUN1_ENTRY;

typedef struct _fun2_entry {
    char *name;  // function name string
    double (*fun)(double p1, double p2);
} const FUN2_ENTRY;

typedef struct _fun3_entry {
    char *name;  // function name string
    double (*fun)(double p1, double p2, double p3);
} const FUN3_ENTRY;

#define FUN_TABLE_ENTRY(fun) { #fun, fun },

enum TokenType {
    NONE,
    NAME,
    NUMBER,
    END,
    PLUS='+',
    MINUS='-',
    MULTIPLY='*',
    POWER='^',
    DIVIDE='/',
    ASSIGN='=',
    LHPAREN='(',
    RHPAREN=')',
    COMMA=',',
    NOT='!',
    
    // comparisons
    LT='<',
    GT='>',
    LE,     // <=
    GE,     // >=
    EQ,     // ==
    NE,     // !=
    AND,    // &&
    OR,      // ||
    
    // special assignments
    
    ASSIGN_ADD,  //  +=
    ASSIGN_SUB,  //  +-
    ASSIGN_MUL,  //  +*
    ASSIGN_DIV   //  +/
    
    };

const char *pWord_;
const char *pWordStart_;
enum TokenType type_; // last token parsed

#define MAX_WORD 1024  /* maximum size of a token */
char word_[MAX_WORD];
double value_;
double Evaluate(char *string);  // get result

#define bool int
#define true (1)
#define false (0)

enum TokenType GetToken(const bool ignoreSign);  
double CommaList(const bool get);
double Expression(const bool get);
double Comparison(const bool get);
double AddSubtract(const bool get);
double Term(const bool get);      // multiply and divide
double Primary(const bool get);   // primary (base) tokens

#endif // PARSER_H
