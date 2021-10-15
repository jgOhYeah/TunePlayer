/**
 * Parser for TuneAssembler.
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
#include <stdio.h>

int yylex(void);
void yyerror(char *s);
%}

%union {
    int iValue;
};

%token <iValue> INTEGER
%token TITLE DESC WSP

%%
/* Rules */
start           : line              {}
                | line '\n'         {}
                | line '\n' line    {}
                ;

line            :                   {}
                | WSP               { printf("Got some whitespace"); }
                ;


%%
/* Subroutines */

int main(void) {
    yyparse();
    return 0;
}