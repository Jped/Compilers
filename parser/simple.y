/* C Parser

	Jonathan Pedoeem
	Spring 2019
	The Cooper Union

*/

%{
# include <stdio.h>
# include <stdlib.h>
# include "parser.h"

%}


%union{
	struct astnode *a;
	int integer;
	char *value;
	char *string_literal;
	float f;
	double d;
}

%token <integer> INT
%token <value> OP
%token <string_literal> IDENT
%token <string_litearl>KEYWD
%token <string_literal> CHAR
%token <string_literal> STRING
%token <f> FLOAT
%token <d> DOUBLE
%token <s> FILEN
%token ARROW
%token PLUSPLUS
%token EOL


%type <a> exp

%start s


%define parse.error verbose
%%
s: 	
 	|s exp EOL {	printast($2,0);
			printf("> ");
		   }
     	| s EOL {printf("> ");}
	;

exp:	IDENT {$$ = newIdent(1, $1);} 
	| IDENT '+' IDENT {
			  	struct astnode * a = newIdent(1,$1);
				struct astnode * b = newIdent(1,$3);
				$$ = newBinop(0,'+',a,b);
			  }
	| IDENT '=' IDENT {
			 	struct astnode * a= newIdent(1,$1);
				struct astnode * b = newIdent(1,$3);
				$$ = newBinop(0,'=',a,b); 
			 }
	| INT {printf("saw a number!\n");}
	;

%%	



