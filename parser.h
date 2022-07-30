// parser.h

#ifndef PARSER_H
#define PARSER_H

#ifndef MAX_PARSE_SYMBOLS
#define MAX_PARSE_SYMBOLS (100)  /* maxium parser symbols allowed */
#endif

#define PARSE_ERROR (sqrt(-1)) /* indicates Evaluate()/LookupSymbol failure */

int SaveSymbol(char *lhs, double rhs); // returns 1:success, 0:malloc() failed
double LookupSymbol(char *lhs); // returns NO_LHS_MATCH if lookup fails
char *GetParserErr(void); // returns non-empty error string on Evaluate() fails
double Evaluate(char *string); // returns result (or NO_LHS_MATCH if error)

#endif // PARSER_H
