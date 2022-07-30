// parser.c - expression parser in pure C
//
// Original parser C++ code from https://github.com/nickgammon/parser/
//
/******************************************************************************

 Parser - an expression parser

 Author:  Nick Gammon 
          http://www.gammon.com.au/ 

(C) Copyright Nick Gammon 2004. Permission to copy, use, modify, sell and
distribute this software is granted provided this copyright notice appears
in all copies. This software is provided "as is" without express or implied
warranty, and with no claim as to its suitability for any purpose.
 
Modified 24 October 2005 by Nick Gammon.

  1. Changed use of "abs" to "fabs"
  2. Changed inclues from math.h and time.h to fmath and ftime
  3. Rewrote DoMin and DoMax to inline the computation because of some
     problems with some libraries.
  4. Removed "using namespace std;" and put "std::" in front of std
     namespace names where appropriate
  5. Removed MAKE_STRING macro and inlined the functionality where required.
  6. Changed Evaluate function to take its argument by reference.

Modified 13 January 2010 by Nick Gammon.
 
  1. Changed getrandom to work more reliably (see page 2 of discussion thread)
  2. Changed recognition of numbers to allow for .5 (eg. "a + .5" where
     there is no leading 0)
     Also recognises -.5 (so you don't have to write -0.5)
  3. Fixed problem where (2+3)-1 would not parse correctly (- sign
     directly after parentheses)
  4. Fixed problem where changing a parameter and calling p.Evaluate
     again would fail because the
     initial token type was not reset to NONE.

Modified 16 February 2010 by Nick Gammon

  1. Fixed bug where if you called Evaluate () twice, the original
     expression would not be reprocessed.

Modified 27 November 2014 by Nick Gammon

  1. Fixed bug where a literal number followed by EOF would throw an error.
  
Thanks to various posters on my forum for suggestions. The relevant post
is currently at:

  http://www.gammon.com.au/forum/?id=4649

******************************************************************************/

#include <setjmp.h>
#include <stdlib.h>

#include "parser.h"

#define DBG if(0)printf

#ifndef WIN32
#include <sys/time.h>
#endif

#define STRNCPY(dst, src, len) \
        { strncpy(dst, src, len); if (len >= 0) dst[len] = '\0'; }

#define CheckToken(wanted) { \
    if (type_ != wanted) { \
      runtime_error("expected '%c'", (int)wanted); \
    } \
}

static jmp_buf parse_err_jmp_buf;

static char ParserErrBuf[256];

char *GetParserErr(void)
{
    return &ParserErrBuf[0];
}

static void runtime_error(const char *String, ...)
{
    va_list ArgPtr;

    strcpy(ParserErrBuf, "Error! ");
    va_start(ArgPtr, String);
    vsnprintf(ParserErrBuf + strlen(ParserErrBuf),
             sizeof(ParserErrBuf) - strlen(ParserErrBuf) - 1, String, ArgPtr);
    va_end(ArgPtr);
    ParserErrBuf[sizeof(ParserErrBuf) - 1] = '\0';  // safety knows no season!

#ifdef ENABLE_PARSER_ERR_OUTPUT
    fflush(stdout);
    fprintf(stderr, "%s\n", ParserErrBuf);
    fflush(stderr);
#endif

    longjmp(parse_err_jmp_buf, 1);
}

// returns a number from 0 up to, but excluding x
const int getrandom(const int x)
{
    double r;
    if (x <= 0)
        return 0;

    // r will be between 0 and 1 (but below 1 as we are dividing by RAND_MAX+1)
    r = ((double) (rand() % RAND_MAX)) / ((double) RAND_MAX + 1.0);
    return floor(r * x);

}

const int roll(const int howmany, const int die)
{
    int count;
    int total = 0;

    for (count = 0; count < howmany; ++count)
        total += getrandom(die) + 1;

    return total;

}

// returns true if a x% probability exists
// eg. percent (80) will be true 80% of the time
const bool percent(const int prob)
{
    if (prob <= 0)
        return false;
    if (prob >= 100)
        return true;

    return getrandom(100) > (100 - prob);

}

static int initRandom()
{
    srand(time(NULL));
#ifndef WIN32
    srand48(time(NULL));
#endif
    return 0;
}

/******************************************************************************

Expression-evaluator
--------------------

Author: Nick Gammon
-------------------


Example usage:

    Parser p ("2 + 2 * (3 * 5) + nick");
    
    p.symbols_ ["nick"] = 42;
    
    double v = p.Evaluate ();

    double v1 = p.Evaluate ("5 + 6");   // supply new expression and evaluate it
    
Syntax:

  You can use normal algebraic syntax. 
  
  Multiply and divide has higher precedence than add and subtract.
  
  You can use parentheses (eg. (2 + 3) * 5 )
  
  Variables can be assigned, and tested. eg. a=24+a*2
  
  Variables can be preloaded:
  
    p.symbols_ ["abc"] = 42;
    p.symbols_ ["def"] = 42;
    
  Afterwards they can be retrieved:
  
    x = p.symbols_ ["abc"];

  There are 2 predefined symbols, "pi" and "e".
  
  You can use the comma operator to load variables and then use them, eg.
  
    a=42, b=a+6
    
  You can use predefined functions, see below for examples of writing your own.
  
    42 + sqrt (64)
    
  
  Comparisons
  -----------
  
  Comparisons work by returning 1.0 if true, 0.0 if false.
  
  Thus, 2 > 3 would return 0.0
        3 > 2 would return 1.0
        
  Similarly, tests for truth (eg. a && b) test whether the values are 0.0 or not.
  
  If test
  -------
  
  There is a ternary function: if (truth-test, true-value, false-value)
  
  eg.  if (1 < 2, 22, 33)  returns 22
  
  
  Precedence
  ----------
  
  ( )  =   - nested brackets, including function calls like sqrt (x), and assignment
  * / ^    - multiply, divide, exponentiation
  + -      - add and subtract
  < <= > >= == !=  - comparisons
  && ||    - AND and OR
  ,        - comma operator
    
    Credits:
    
    Based in part on a simple calculator described in "The C++ Programming
    Language" by Bjarne Stroustrup, however with considerable enhancements
    by me, and also based on my earlier experience in writing Pascal
    compilers, which had a similar structure.

******************************************************************************/

// functions we can call from an expression

double DoInt(double arg)
{
    return (int)arg;           // drop fractional part
}

double DoRandom(double arg)
{
    return getrandom(arg);      // random number in range 0 to arg
}

double DoPercent(double arg)
{
    if (percent(arg))   // true x% of the time
         return 1.0;
    else
        return 0.0;
}

double DoMin(const double arg1, const double arg2)
{
    return (arg1 < arg2 ? arg1 : arg2);
}

double DoMax(const double arg1, const double arg2)
{
    return (arg1 > arg2 ? arg1 : arg2);
}

double DoFmod(const double arg1, const double arg2)
{
    if (arg2 == 0.0)
        runtime_error("Divide by zero in mod");

    return fmod(arg1, arg2);
}

double DoPow(const double arg1, const double arg2)
{
    int n;
    double result;

    n = (int)arg2;
    if (n > 0 && n <= 64 && (double)n == arg2) {
        // do it the "hard way", but with more precision
        result = arg1;
        while (--n) result *= arg1;
        return result;
    } else {
        return pow(arg1, arg2);
    }
}

#ifdef HAVE_ROLL
double DoRoll(const double arg1, const double arg2)
{
    return roll(static_cast < int >(arg1), static_cast < int >(arg2));
}
#endif

double DoIf(const double arg1, const double arg2, const double arg3)
{
    if (arg1 != 0.0)
        return arg2;
    else
        return arg3;
}

FUN1_ENTRY fun1_table[] = {
    { "abs", fabs },
    FUN_TABLE_ENTRY(acos)
    FUN_TABLE_ENTRY(asin)
    FUN_TABLE_ENTRY(atan)
#ifndef WIN32
    FUN_TABLE_ENTRY(atanh) // doesn't seem to exist under Visual C++ 6
#endif
    FUN_TABLE_ENTRY(ceil)
    FUN_TABLE_ENTRY(cos)
    FUN_TABLE_ENTRY(cosh)
    FUN_TABLE_ENTRY(exp)
    FUN_TABLE_ENTRY(exp)
    FUN_TABLE_ENTRY(floor)
    FUN_TABLE_ENTRY(log)
    FUN_TABLE_ENTRY(log10)
    FUN_TABLE_ENTRY(sin)
    FUN_TABLE_ENTRY(sinh)
    FUN_TABLE_ENTRY(sqrt)
    FUN_TABLE_ENTRY(tan)
    FUN_TABLE_ENTRY(tanh)
    FUN_TABLE_ENTRY(DoInt)
    { "int", DoInt },
    { "rand", DoRandom },
    { "percent", DoPercent },
    { "", NULL }
};

FUN2_ENTRY fun2_table[] = {
    { "min", DoMin },
    { "max", DoMax },
    { "mod", DoFmod },
    { "pow", DoPow },
#ifdef HAVE_ROLL
    { "roll", DoRoll },
#endif
    { "", NULL }
};

FUN3_ENTRY fun3_table[] = {
    { "if", DoIf },
    { "", NULL }
};

static FUN1_ENTRY *LookupFun1(char *name)
{
    int ix;

    for (ix = 0; fun1_table[ix].fun; ++ix) {
        if (!strcmp(name, fun1_table[ix].name)) return &fun1_table[ix];
    }
    return NULL;
}

static FUN2_ENTRY *LookupFun2(char *name)
{
    int ix;

    for (ix = 0; fun2_table[ix].fun; ++ix) {
        if (!strcmp(name, fun2_table[ix].name)) return &fun2_table[ix];
    }
    return NULL;
}

static FUN3_ENTRY *LookupFun3(char *name)
{
    int ix;

    for (ix = 0; fun3_table[ix].fun; ++ix) {
        if (!strcmp(name, fun3_table[ix].name)) return &fun3_table[ix];
    }
    return NULL;
}

#define MAX_VARS 100
#define NO_LHS_MATCH (sqrt(-1))
static char *vars_lhs[MAX_VARS];
static double vars_rhs[MAX_VARS];
static int num_vars = 0;

int SaveSymbol(char *lhs, double rhs)
{
    int i;

    DBG("SaveSymbol('%s', %g)...\n", lhs, rhs);
    for (i = 0; i < num_vars; ++i) {  // aleady in table?
        if (!strcmp(vars_lhs[i], lhs)) {
            vars_rhs[i] = rhs;
            return 1;  // no error exit
        }
    }
    // symbol not found...add new entry in table
    vars_lhs[num_vars] = malloc(strlen(lhs) + 1);
    if (!vars_lhs[num_vars]) return 0;  // error exit (no free memory!)
    strcpy(vars_lhs[num_vars], lhs);
    vars_rhs[num_vars] = rhs;
    ++num_vars;
    return 1;  // no error exit
}

double LookupSymbol(char *lhs)
{
    int i;
    double rhs;

    DBG("LookupSymbol('%s')", lhs);
    if (!strcmp(lhs, "time")) {  // "time" built-in (secs since 1970 epoch)
        return (double)time(NULL);
    }
#if !defined(WIN32) || defined(HAVE_GETTIMEOFDAY)
    if (!strcmp(lhs, "timems")) {  // "timems" built-in (msecs since 1970 epoch)
        typedef struct timeval {
            long tv_sec;
            long tv_usec;
        } timeval;
        struct timeval tv;

        gettimeofday(&tv, NULL);
        return ((double)tv.tv_sec * 1000 + (double)tv.tv_usec / 1000); 
    }
#endif
    for (i = 0; i < num_vars; ++i) {
        if (!strcmp(vars_lhs[i], lhs)) {
            rhs = vars_rhs[i];  // match
            DBG("=%g\n", rhs);
            return rhs;  // return symbol value
        }
    }
    rhs = NO_LHS_MATCH;
    DBG("=%g\n", rhs);
    return rhs;  // no match
}

enum TokenType GetToken(const bool ignoreSign)
{
    unsigned char cFirstCharacter;
    unsigned char cNextCharacter;
    char *p;

    DBG("GetToken('%s')...'%s'\n", pWord_, word_);
    //word_.erase (0, std::string::npos);
    *word_ = '\0';
    ////memset(word_, 0, sizeof(word_));

    // skip spaces
    while (*pWord_ && isspace(*pWord_)) ++pWord_;

    pWordStart_ = pWord_;       // remember where word_ starts *now*

    // look out for unterminated statements and things
    if (*pWord_ == 0 &&         // we have EOF
        type_ == END)           // after already detecting it
        runtime_error("Unexpected end of expression");

    cFirstCharacter = *pWord_;  // first character in new word_
    DBG("cFirstCharacter='%c'\n", cFirstCharacter);

    if (cFirstCharacter == 0)   // stop at end of file
    {
        strcpy(word_, "<end of expression>");
        DBG("return END\n");
        return type_ = END;
    }

    cNextCharacter = *(pWord_ + 1);     // 2nd character in new word_
    DBG("cNextCharacter='%c'\n", cNextCharacter);

    // look for number
    // can be: + or - followed by a decimal point
    // or: + or - followed by a digit
    // or: starting with a digit
    // or: decimal point followed by a digit
    if ((!ignoreSign &&
         (cFirstCharacter == '+' || cFirstCharacter == '-') &&
         (isdigit(cNextCharacter) || cNextCharacter == '.'))
        || isdigit(cFirstCharacter)
        // allow decimal numbers without a leading 0. e.g. ".5"
        // Dennis Jones 01-30-2009
        || (cFirstCharacter == '.' && isdigit(cNextCharacter))) {
        // skip sign for now
        if ((cFirstCharacter == '+' || cFirstCharacter == '-'))
            pWord_++;
        while (isdigit(*pWord_) || *pWord_ == '.')
            pWord_++;

        // allow for 1.53158e+15
        if (*pWord_ == 'e' || *pWord_ == 'E') {
            pWord_++;           // skip 'e'
            if ((*pWord_ == '+' || *pWord_ == '-'))
                pWord_++;       // skip sign after e
            while (isdigit(*pWord_))    // now digits after e
                pWord_++;
        }

        //word_ = std::string(pWordStart_, pWord_ - pWordStart_);
        STRNCPY(word_, pWordStart_, pWord_ - pWordStart_);
        DBG("pWordStart_='%s'\n", pWordStart_);
        DBG("pWord_='%s'\n", pWord_);

        //std::istringstream is(word_);
        // parse std::string into double value
        //is >> value_;
        p = NULL;
        value_ = strtod(word_, &p);
        DBG("strtod('%s')\n", word_);
        //////pWord_ += p - word_;  // skip it

        //if (is.fail() && !is.eof())
        if (p && *p != '\0')
             runtime_error("Bad numeric literal: %s", word_);
        DBG("return NUMBER\n");
        return type_ = NUMBER;
    }

    // special test for 2-character sequences: <= >= == !=
    // also +=, -=, /=, *=
    DBG("check %c for 2-char...\n", cNextCharacter);
    if (cNextCharacter == '=') {
        switch (cFirstCharacter) {
            // comparisons
        case '=':
            type_ = EQ;
            break;
        case '<':
            type_ = LE;
            break;
        case '>':
            type_ = GE;
            break;
        case '!':
            type_ = NE;
            break;
            // assignments
        case '+':
            type_ = ASSIGN_ADD;
            break;
        case '-':
            type_ = ASSIGN_SUB;
            break;
        case '*':
            type_ = ASSIGN_MUL;
            break;
        case '/':
            type_ = ASSIGN_DIV;
            break;
            // none of the above
        default:
            type_ = NONE;
            break;
        }

        if (type_ != NONE) {
            //word_ = std::string(pWordStart_, 2);
            STRNCPY(word_, pWordStart_, 2);
            pWord_ += 2;        // skip both characters
            DBG("return 2-char\n");
            return type_;
        }
    }

    DBG("check %c for 1-char...\n", cFirstCharacter);
    switch (cFirstCharacter) {
    case '&':
        if (cNextCharacter == '&')      // &&
        {
            //word_ = std::string(pWordStart_, 2);
            STRNCPY(word_, pWordStart_, 2);
            pWord_ += 2;        // skip both characters
            DBG("return AND\n");
            return type_ = AND;
        }
        break;
    case '|':
        if (cNextCharacter == '|')      // ||
        {
            //word_ = std::string(pWordStart_, 2);
            STRNCPY(word_, pWordStart_, 2);
            pWord_ += 2;        // skip both characters
            DBG("return OR\n");
            return type_ = OR;
        }
        break;
        // single-character symboles
    case '=':
    case '<':
    case '>':
    case '+':
    case '-':
    case '/':
    case '*':
    case '^':
    case '(':
    case ')':
    case ',':
    case '!':
        //word_ = std::string(pWordStart_, 1);
        STRNCPY(word_, pWordStart_, 1);
        ++pWord_;               // skip it
        //type_ = TokenType(cFirstCharacter);
        type_ = cFirstCharacter;
        DBG("return %c\n", cFirstCharacter);
        return type_;
    }

    if (!isalpha(cFirstCharacter)) {
        if (cFirstCharacter < ' ') {
            runtime_error("Unexpected character 0x%02x", cFirstCharacter);
        } else
            runtime_error("Unexpected character '%c'", cFirstCharacter);
    }
    // we have a word (starting with A-Z) - pull it out
    while (isalnum(*pWord_) || *pWord_ == '_')
        ++pWord_;

    //word_ = std::string(pWordStart_, pWord_ - pWordStart_);
    STRNCPY(word_, pWordStart_, pWord_ - pWordStart_);
    DBG("return NAME/%d (%s)\n", (int)NAME, word_);
    return type_ = NAME;
}

double Primary(const bool get)  // primary (base) tokens
{
    if (get)
        GetToken(false);                // one-token lookahead  

    DBG("---------Primary(%d)\n", type_);

    switch (type_) {
    case NUMBER:
        {
            double v = value_;
            GetToken(true);     // get next one (one-token lookahead)
            return v;
        }

    case NAME:
        {
            //std::string word = word_;
            char word[1024];
            double v;
            FUN1_ENTRY *si;
            FUN2_ENTRY *di;
            FUN3_ENTRY *ti;

            STRNCPY(word, word_, sizeof(word) - 2);
            GetToken(true);
            if (type_ == LHPAREN) {
                // might be single-argument function (eg. abs (x) )
                //std::map < std::string,
                 //   OneArgFunction >::const_iterator si;
                //si = OneArgumentFunctions.find(word);
                //if (si != OneArgumentFunctions.end()) 
                if ((si = LookupFun1(word)) != NULL) {
                    double v = Expression(true);        // get argument
                    CheckToken(RHPAREN);
                    GetToken(true);     // get next one (one-token lookahead)
                    return si->fun(v);  // evaluate function
                }
                // might be double-argument function (eg. roll (6, 2) )
                //std::map < std::string,
                 //   TwoArgFunction >::const_iterator di;
                //di = TwoArgumentFunctions.find(word);
                //if (di != TwoArgumentFunctions.end()) 
                if ((di = LookupFun2(word)) != NULL) {
                    double v1 = Expression(true);
                    CheckToken(COMMA);
                    double v2 = Expression(true);
                    CheckToken(RHPAREN);
                    GetToken(true);     // get next one (one-token lookahead)
                    return di->fun(v1, v2);     // evaluate function
                }
                // might be three-argument function (eg. if (a > b, 6, 2) )
                //std::map < std::string,
                 //   ThreeArgFunction >::const_iterator ti;
                //ti = ThreeArgumentFunctions.find(word);
                //if (ti != ThreeArgumentFunctions.end()) 
                if ((ti = LookupFun3(word)) != NULL) {
                    double v1 = Expression(true);
                    CheckToken(COMMA);
                    double v2 = Expression(true);
                    CheckToken(COMMA);
                    double v3 = Expression(true);
                    CheckToken(RHPAREN);
                    GetToken(true);     // get next one (one-token lookahead)
                    return ti->fun(v1, v2, v3); // evaluate function
                }
                runtime_error("Function '%s' not implemented", word);
            }
            // not a function? must be a symbol in the symbol table
            //double &v = symbols_[word]; // get REFERENCE to symbol table entry
            if ((v = LookupSymbol(word)) == NO_LHS_MATCH) {
                SaveSymbol(word, v);  // not found, add to table
            }
            // change table entry with expression? (eg. a = 22, or a = 22)
            switch (type_) {
                // maybe check for NaN or Inf here (see: isinf, isnan functions)
            case ASSIGN:
                v = Expression(true);
                SaveSymbol(word, v);// save new value
                break;
            case ASSIGN_ADD:
                v += Expression(true);
                SaveSymbol(word, v);// save new value
                break;
            case ASSIGN_SUB:
                v -= Expression(true);
                SaveSymbol(word, v);// save new value
                break;
            case ASSIGN_MUL:
                v *= Expression(true);
                SaveSymbol(word, v);// save new value
                break;
            case ASSIGN_DIV:
                {
                    double d = Expression(true);
                    if (d == 0.0)
                        runtime_error("Divide by zero");
                    v /= d;
                    SaveSymbol(word, v);// save new value
                    break;      // change table entry with expression
                }
            default:
                break;          // do nothing for others
            }
            return v;           // and return new value
        }

    case MINUS:         // unary minus
        return -Primary(true);

    case NOT:                   // unary not
        return (Primary(true) == 0.0) ? 1.0 : 0.0;;

    case LHPAREN:
        {
            double v = CommaList(true); // inside parens, you could have commas
            CheckToken(RHPAREN);
            GetToken(true);     // eat the )
            return v;
        }

    default:
        if (type_ == END) {
            runtime_error("Unexpected end of expression");
        } else {
            runtime_error("Unexpected token: '%s'", word_);
        }

    }
    return 0;

}

double Term(const bool get)       // multiply and divide
{
    double left = Primary(get);
    DBG("---------Term(%d)=%g\n", type_, left);
    while (true) {
        switch (type_) {
        case POWER:
            left = pow(left, Primary(true));
            break;
        case MULTIPLY:
            left *= Primary(true);
            break;
        case DIVIDE:
            {
                double d = Primary(true);
                if (d == 0.0)
                    runtime_error("Divide by zero");
                left /= d;
                break;
            }
        default:
            return left;
        }
    }
}

double AddSubtract(const bool get)        // add and subtract
{
    double left = Term(get);
    DBG("---------AddSubtract(%d)=%g\n", type_, left);
    while (true) {
        switch (type_) {
        case PLUS:
            left += Term(true);
            break;
        case MINUS:
            left -= Term(true);
            break;
        default:
            return left;
        }
    }
}

double Comparison(const bool get) // LT, GT, LE, EQ etc.
{
    double left = AddSubtract(get);
    DBG("---------Comparison(%d)=%g\n", type_, left);
    while (true) {
        switch (type_) {
        case LT:
            left = left < AddSubtract(true) ? 1.0 : 0.0;
            break;
        case GT:
            left = left > AddSubtract(true) ? 1.0 : 0.0;
            break;
        case LE:
            left = left <= AddSubtract(true) ? 1.0 : 0.0;
            break;
        case GE:
            left = left >= AddSubtract(true) ? 1.0 : 0.0;
            break;
        case EQ:
            left = left == AddSubtract(true) ? 1.0 : 0.0;
            break;
        case NE:
            left = left != AddSubtract(true) ? 1.0 : 0.0;
            break;
        default:
            return left;
        }
    }
}

double Expression(const bool get) // AND and OR
{
    double left = Comparison(get);
    DBG("---------Expression(%d)=%g\n", type_, left);
    while (true) {
        switch (type_) {
        case AND:
            {
                double d = Comparison(true); // don't want short-circuit eval
                left = (left != 0.0) && (d != 0.0);
            }
            break;
        case OR:
            {
                double d = Comparison(true); // don't want short-circuit eval
                left = (left != 0.0) || (d != 0.0);
            }
            break;
        default:
            return left;
        }
    }
}

// initialise random number generator
static int someNumber = 0;

double CommaList(const bool get)  // expr1, expr2
{
    double left;

    if (someNumber == 0)
        initRandom();

    left = Expression(get);
    DBG("---------CommaList(%d)=%g\n", type_, left);
    while (true) {
        switch (type_) {
        case COMMA:
            left = Expression(true);
            break;              // discard previous value
        default:
            return left;
        }
    }
}

double Evaluate(char *expr)  // get result
{
    int ok;
    double v;

    ParserErrBuf[0] = '\0';  // default to NULL error string

    if( !setjmp(parse_err_jmp_buf) ) {
        SaveSymbol("pi", M_PI); // 3.1415926535897932385
        SaveSymbol("e",  M_E);  // 2.7182818284590452354
        DBG("ok=%d, v=%g\n", ok, LookupSymbol("pi"));

        pWord_ = expr;
        type_ = NONE;
        v = CommaList(true);
        if (type_ != END)
            runtime_error("Unexpected text at end of expression: '%s'",
                                                                 pWordStart_);
        return v;
    } else {
        return sqrt(-1.0); // error, return NaN silently
    }
}

