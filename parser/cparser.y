/* C Parser

	Jonathan Pedoeem
	Spring 2019
	The Cooper Union

*/

%{
# include <stdio.h>
# include <stdlib.h>
# include parser.h

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
%token ARROW
%token PLUSPLUS
%token EOL


%type <a> exp, primary_expression, postfix_expression, subscript_expression

%start exp

%%
exp:	primary_expresion
	| postfix_expression
	;
primary_expression: 	IDENT 
			|paranthesized_expression
			;

paranthesized_expression: 	'(' exp ')'
		    		;


postfix_expression:	primary_expression
			| subscript_expression
			| component_selection_expression
			| function_call
			| postincrement_expression
			| post_decrement_expression
			| compound_literal
			;

subscript_expression: postfix_expression [exp]
		      ;

component_selection_expression:	direct_component_selection
				| indirect_component_selection
				;

direct_component_selection:	postfix_expression '.' IDENT
				;
indirect_compoent_selection: 	postfix_expression ARROW IDENT
				;
function_call:	postfix_expression 
	     	| postfix_expresssion '(' expression_list ')'
		;

expression_list:	assignment_expression
	       		| expression_list,assignment_expression
			;

postincrement_expression:	postfix_expression PLUSPLUS
				;

compound_literal: 	'(' type_name ')' 
			| '(' type_name ')' '{' initializer_list '}'
			;


%%	



