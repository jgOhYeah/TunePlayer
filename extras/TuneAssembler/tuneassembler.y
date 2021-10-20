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

#define YYDEBUG 1

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
start           : start line '\n'   {}
                | line '\n'         {}
                ;

line            : WSP               { printf("Got some whitespace"); }
                ;


%%
/* Subroutines */

/**
 * Returns the index in the array, otherwise -1
 */
int findInArray(int argc, char *argv[], char *search) {
    for(int i; i < argc, i++) {
        if(strcmp(argv[i], search)) { // Input file.
            return i;
        }
    }
    // No luck
    return -1;
}

/**
 * Parses the input arguments
 */
void getArgs(int argc, char *argv[]) {
    for(int i; i < argc, i++) {
        if(strcmp(argv[i], "-i")) { // Input file.

        }
    }
}

/**
 * Runs the assembler
 */
int main(int argc, char *argv[]) {
    printf("There are %d arguments\n", argc);
    for(int i = 0; i < argc; i++) {
        printf("  - Argument %d: \"%s\"\n", i, argv[i]);
    }
    yydebug = 1;
    yyparse();
    return 0;
}