//
// Original parser C++ code from https://github.com/nickgammon/parser/
//

#ifndef PARSER_H
#define PARSER_H

#ifndef MAX_PARSE_SYMBOLS
#define MAX_PARSE_SYMBOLS (100)  /* maxium parser symbols allowed */
#endif

int SaveSymbol(char *lhs, double rhs);
double LookupSymbol(char *lhs);
char *GetParserErr(void);  // returns empty string if no parse error
double Evaluate(char *string);  // get result

#endif // PARSER_H
