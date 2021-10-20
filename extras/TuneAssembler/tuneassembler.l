/**
 * Lexical analyser for TuneAssembler.
 * Written by Jotham Gates
 * Created 16/10/2021
 * Modified 16/10/2021
 * See https://github.com/jgOhYeah/TunePlayer for more information.
 *
 * Based off content taught in the unit FIT2014 and
 * https://epaperpress.com/lexandyacc/download/LexAndYacc.pdf
 */

/* Definitions */
%{
#include <stdlib.h>
void yyerror(char *);
#include "y.tab.h"
%}

%%
    /* Rules */
0 {
    yylval.iValue = atoi(yytext);
    return INTEGER;
}

[1-9][0-9]* {
    yylval.iValue = atoi(yytext);
    return INTEGER;
}

[:#] {
    return *yytext;
}

".title"    return TITLE;
".desc"     return DESC;
[ \t]+      return WSP;
.           yyerror("Unknown character");

%%
    /* Subroutines */

void yyerror(char *s) {
    fprintf(stderr, "%s at line %d. (%s).\n", s, yylineno, yytext);
}

int yywrap(void) {
    return 1;
}