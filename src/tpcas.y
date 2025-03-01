%{

/* 	Author : LE COQUIL - TOUSSAINT */
/* 	Syntactic analyzer */
/* 	tpcas.y */

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "tree.h"

int yylex();
int yyerror(char * msg);
Node * node_type;

%}

%union {
  Node * node;
  char byte;
  int num;
  char ident[64];
  char comp[3];
  char character[3];
}

%type <node> Prog DeclVarsGlobale Declarateurs DeclVarsLocale InitVarsLocale DeclFoncts DeclFonct EnTeteFonct Parametres ListTypVar Corps SuiteInstr Instr  Exp TB FB M E T F LValue Arguments ListExp

%token <node> VOID IF ELSE WHILE RETURN OR AND
%token <ident> TYPE IDENT ARRAY
%token <comp> EQ ORDER
%token <byte> ADDSUB DIVSTAR
%token <num> NUM
%token <character> CHARACTER

%precedence ')'
%precedence ELSE
%precedence IDENT IF WHILE RETURN ';' '{'

%%


Prog: DeclVarsGlobale DeclFoncts{
			$$ = makeNode(Prog);
			addChild($$, $1);
			addChild($$, $2);
			root = $$;
			}
    ;

DeclVarsGlobale: DeclVarsGlobale TYPE Declarateurs ';' {
		if($$ != NULL) {
					$$ = $1; }
		else{
			$$ = makeNode(DeclVarsGlobale);
			}
			node_type = makeNode(Type); strcpy(node_type->u.ident, $2);
			addChild($$, node_type); addChild(node_type, $3);
			}
		| { $$ = NULL; }
    ;

Declarateurs: IDENT{
			$$ = makeNode(Ident); strcpy($$->u.ident, $1);
			}

		| IDENT ',' Declarateurs {
			$$ = makeNode(Ident); strcpy($$->u.ident, $1);
			addSibling($$, $3);
			}

		| IDENT '[' NUM ']' ',' Declarateurs {
			$$ = makeNode(Array); strcpy($$->u.ident, $1);
			node_type = makeNode(Num); node_type->u.num = $3;
			addSibling($$, node_type); addChild($$, $6);
			}

		| IDENT '[' NUM ']' {
			$$ = makeNode(Array); strcpy($$->u.ident, $1);
			Node* tmp = makeNode(Num); tmp->u.num = $3;
			addSibling($$, tmp);
			}

    | ',' Declarateurs { $$ = $2; }
    ;

DeclVarsLocale: DeclVarsLocale TYPE InitVarsLocale ';' {
		if($$ != NULL) {
			$$ = $1;}
		else{
			$$ = makeNode(DeclVarsLocale);}
			node_type = makeNode(Type);
			strcpy(node_type->u.ident, $2);
			addChild($$, node_type); addChild(node_type, $3); }

		| {$$ = NULL; }
		;

InitVarsLocale: IDENT {
			$$ = makeNode(Ident); strcpy($$->u.ident, $1);
			}

		| IDENT ',' InitVarsLocale  {
		  $$ = makeNode(Ident); strcpy($$->u.ident, $1);
			addSibling($$, $3);
			}

		| IDENT '[' NUM ']' ',' InitVarsLocale {
			$$ = makeNode(Array); strcpy($$->u.ident, $1);
			node_type = makeNode(Num); node_type->u.num = $3;
			addChild($$, node_type); addSibling($$, $6);
			}

		|	IDENT '[' NUM ']' {
			$$ = makeNode(Array); strcpy($$->u.ident, $1);
			node_type = makeNode(Num); node_type->u.num = $3;
			addChild($$, node_type);
			}

		| ',' InitVarsLocale { $$ = $2; }
		;

DeclFoncts: DeclFoncts DeclFonct {
			$$ = $1; addSibling($1, $2);
			}

    | DeclFonct
			{ $$ = $1; }
    ;

DeclFonct: EnTeteFonct Corps {
			$$ = makeNode(DeclFonct);
			addChild($$, $1); addChild($$, $2); }
    ;

EnTeteFonct: TYPE IDENT '(' Parametres ')' {
			$$ = makeNode(Type); strcpy($$->u.ident, $1);
			node_type = makeNode(Ident); strcpy(node_type->u.ident, $2);
			addChild($$, node_type); addSibling($$, $4);
			}

    | VOID IDENT '(' Parametres ')' {
			$$ = makeNode(Void);
			node_type = makeNode(Ident); strcpy(node_type->u.ident, $2);
			addSibling($$, node_type); addSibling($$, $4); }
    ;

Parametres: VOID {
			$$ = makeNode(ParamList);
			addChild($$, makeNode(Void));
			}

		| ListTypVar {
			$$ = makeNode(ParamList);
			addChild($$, $1);
			}
    ;

ListTypVar: TYPE IDENT{
			$$ = makeNode(Type); strcpy($$->u.ident, $1);
			node_type = makeNode(Ident); strcpy(node_type->u.ident, $2);
			addChild($$, node_type);
			}

		| TYPE IDENT ',' ListTypVar {
			$$ = makeNode(Type); strcpy($$->u.ident, $1);
			node_type = makeNode(Ident); strcpy(node_type->u.ident, $2);
			addChild($$, node_type); addSibling($$, $4);
			}

		| TYPE IDENT '[' ']' ',' ListTypVar {
			$$ = makeNode(Type); strcpy($$->u.ident, $1);
			node_type = makeNode(Array); strcpy(node_type->u.ident, $2);
			addChild($$, node_type); addSibling($$, $6);
			}

		| TYPE IDENT '['']' {
			$$ = makeNode(Type); strcpy($$->u.ident, $1);
			Node* tmp = makeNode(Array); strcpy(tmp->u.ident, $2);
			addChild($$, tmp);
			}

    | ',' ListTypVar { $$ = $2; }
    ;

Corps: '{' DeclVarsLocale SuiteInstr '}' {
		if($2 != NULL || $3 != NULL) {
			$$ = makeNode(Body);
			addChild($$, $2); addChild($$, $3);
				}
		else{ $$ = NULL; }
			}
    ;

SuiteInstr: SuiteInstr Instr {
		if($1 != NULL) {
			$$ = $1;
			addSibling($1, $2);
			}
		else { $$ = $2; }
					}
    | { $$ = NULL; }
    ;

Instr: LValue '=' Exp ';' {
			$$ = makeNode(Egual);
			addChild($$, $1); addChild($$, $3);
			}

    | IF '(' Exp ')' Instr {
			$$ = makeNode(If);
			addChild($$, $3); addChild($$, $5);
			}

    | IF '(' Exp ')' Instr ELSE Instr {
			$$ = makeNode(If); $6 = makeNode(Else);
			addChild($$, $3); addChild($$, $5);
			addChild($$, $6); addChild($6, $7);
                }

    | WHILE '(' Exp ')' Instr {
			$$ = makeNode(While);
			addChild($$, $3); addChild($$, $5);
			}

    | IDENT '(' Arguments ')' ';'{
			$$ = makeNode(Ident);
			strcpy($$->u.ident, $1);
			addChild($$, $3);
			}

    | RETURN Exp ';' {
			$$ = makeNode(Return);
			addChild($$, $2);
			}

    | RETURN ';' {
			$$ = makeNode(Return);
			}

    | '{' SuiteInstr '}'
			{ $$ = $2; }

    | ';' { $$ = NULL; }
    ;

Exp : Exp OR TB {
			$$ = makeNode(Or);
			addChild($$, $1); addChild($$, $3);
			}

    | TB
			{ $$ = $1; }
    ;

TB  : TB AND FB {
			$$ = makeNode(And);
			addChild($$, $1); addChild($$, $3);
			}

    | FB
			{ $$ = $1; }
    ;

FB  : FB EQ M {
			$$ = makeNode(Eq); strcpy($$->u.comp, $2);
			addChild($$, $1); addChild($$, $3);
			}

    | M
			{ $$ = $1; }
    ;

M   : M ORDER E {
			$$ = makeNode(Order); strcpy($$->u.comp, $2);
			addChild($$, $1); addChild($$, $3);
			}

    | E
			{ $$ = $1; }
    ;

E   : E ADDSUB T {
			$$ = makeNode(Addsub); $$->u.byte = $2;
			addChild($$, $1); addChild($$, $3);
			}

    | T
			{ $$ = $1; }
    ;

T   : T DIVSTAR F	{
			$$ = makeNode(Divstar); $$->u.byte = $2;
			addChild($$, $1); addChild($$, $3);
			}

    | F
			{ $$ = $1; }
    ;

F   : ADDSUB F {
			$$ = makeNode(Addsub); $$->u.byte = $1;
			addChild($$, $2);
			}

    | '!' F {
			$$ = makeNode(ExclamationPoint);
			addChild($$, $2);
			}

    | '(' Exp ')'
			{ $$ = $2; }

    | NUM
			{ $$ = makeNode(Num); $$->u.num = $1; }

    | CHARACTER
			{ $$ = makeNode(Character); strcpy($$->u.character, $1); }

    | LValue
			{ $$ = $1; }

    | IDENT '(' Arguments ')' {
			$$ = makeNode(Ident); strcpy($$->u.ident, $1);
			addChild($$, $3);
			}
    ;

LValue: IDENT
			{ $$ = makeNode(Ident); strcpy($$->u.ident, $1); }

		|	IDENT '[' Exp ']' {
			$$ = makeNode(Array); strcpy($$->u.ident, $1);
			addChild($$, $3);
			}
    ;

Arguments: ListExp {
			$$ = makeNode(Arguments);
			addChild($$, $1);
				}

   		| { $$ = makeNode(Arguments);
				addChild($$, makeNode(Void));
				}
    ;

ListExp: ListExp ',' Exp {
			$$ = $1; addSibling($1, $3);
				}

		| Exp { $$ = $1; }
    ;

%%
