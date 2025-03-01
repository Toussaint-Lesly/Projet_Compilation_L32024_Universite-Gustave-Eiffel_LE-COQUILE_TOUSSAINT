%{

/*	Author : LE COQUIL - TOUSSAINT */
/* 	Lexical analyzer */
/* 	tpcas.lex */

#include <string.h>
#include "tree.h"
#include "tpcas.tab.h"

int lineno = 1;
int nb_char = 0;

%}

%x COMMENT_MULTIPLE_LINES COMMENT_ONE_LINE

%option nounput
%option noinput
%option noyywrap
%option never-interactive

%%

"char" 								 {nb_char += 4; strcpy(yylval.ident, yytext); return TYPE;}
"int"									 {nb_char += 3; strcpy(yylval.ident, yytext); return TYPE;}

"else" 								 {nb_char += 4; return ELSE;}
"if"                   {nb_char += 2; return IF;}
"return"               {nb_char += 6; return RETURN;}
"void"                 {nb_char += 4; return VOID;}
"while"                {nb_char += 4; return WHILE;}

[0-9]*                 {nb_char += yyleng; yylval.num = atoi(yytext); return NUM;}
[a-zA-Z_][a-zA-Z_0-9]* {nb_char += yyleng; strcpy(yylval.ident, yytext); return IDENT;}

"=="|"!=" 						 {nb_char += 2; strcpy(yylval.comp, yytext); return EQ;}
"<"|">" 							 {nb_char++; strcpy(yylval.comp, yytext); return ORDER;}
"<="|">=" 						 {nb_char += 2; strcpy(yylval.comp, yytext); return ORDER;}
[+-] 									 {nb_char++; yylval.byte = yytext[0]; return ADDSUB;}
[*/%] 								 {nb_char++; yylval.byte = yytext[0]; return DIVSTAR;}
"||" 									 {nb_char += 2; return OR;}
"&&" 									 {nb_char += 2; return AND;}

[(){},;!=:] 					 {nb_char++; return yytext[0];}
\n										 {lineno++; nb_char = 0;}
\r* ;

"\'\\'\'" 						 {nb_char += 4; yylval.character[0] = yytext[2]; yylval.character[1] = '\0';
												return CHARACTER;}
"\'\\n\'"|"\'\\t\'"|"\'\\0\'" 	{nb_char += 4; yylval.character[0] = yytext[1]; yylval.character[1] = yytext[2];
								yylval.character[2] = yytext[3]; yylval.character[2] = '\0'; return CHARACTER;}
'[^']{1}'							 {nb_char += 3;  yylval.character[0] = yytext[1];
												yylval.character[1] = '\0'; return CHARACTER;}
" "|"\t" {nb_char++;}

"//" 									 {nb_char += 2; BEGIN COMMENT_ONE_LINE;}
"/*" 									 {nb_char += 2; BEGIN COMMENT_MULTIPLE_LINES;}

<COMMENT_ONE_LINE,COMMENT_MULTIPLE_LINES>. {nb_char++;}

<COMMENT_ONE_LINE>\n   {lineno++; nb_char = 0; BEGIN INITIAL;}

<COMMENT_MULTIPLE_LINES>\n {lineno++; nb_char = 0;}
<COMMENT_MULTIPLE_LINES>"*/" {nb_char += 2; BEGIN INITIAL;}


. return yytext[0];

%%

int yyerror(char * msg) {
  fprintf(stderr, "line %d : error detected from position lexeme %d to %d : %s\n", lineno, nb_char - yyleng + 1, nb_char, msg);
  return 0;
}

