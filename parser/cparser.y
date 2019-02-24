/* C Parser

	Jonathan Pedoeem
	Spring 2019
	The Cooper Union

*/

%{
# include <stdio.h>
# include <stdlib.h>
# include "parser.h"
# include "ops.h"
%}


%union{
	struct astnode *a;
	struct listarg *l;
	int integer;
	char *value;
	char *string_literal;
	float f;
	double d;
}

%token <integer> INT
%token <string_literal> IDENT
%token <string_literal> CHAR
%token <string_literal> STRING
%token <f> FLOAT
%token <d> DOUBLE
%token ELLIPSIS
%token PLUSEQ 
%token MINUSEQ
%token TIMESEQ
%token DIVEQ
%token MODEQ
%token SHLEQ
%token SHREQ
%token ANDEQ
%token XOREQ
%token OREQ
%token ARROW
%token PLUSPLUS
%token MINUSMINUS
%token SHL
%token SHR
%token LTEQ
%token GTEQ
%token EQEQ
%token NOTEQ
%token LOGAND
%token LOGOR
%token LTMOD
%token GTMOD
%token LTCOL
%token COLGT
%token MODCOL
%token MODCOLMOD
%token AUTO
%token CHR
%token DEFAULT
%token ELSE
%token FOR
%token INLINE
%token RETURN
%token STATIC
%token UNION
%token WHILE
%token BOOL
%token COMPLEX
%token RESTRICT
%token ENUM
%token GOTO
%token INTEGER
%token  SHORT
%token STRUCT
%token UNSIGNED
%token BREAK
%token CONST
%token DO
%token EXTERN
%token IF
%token LONG
%token SIGNED
%token SWITCH
%token VOID
%token CASE
%token CONTINUE
%token DBLE
%token FLT
%token IMAGINARY
%token REGISTER
%token SIZEOF
%token VOLATILE
%token TYPEDEF
%token <string_litearl> FILEN


%type <a> exp unary_expression sizeof_expression type_name primary_expression numbers characters constant parenthesized_expression cast_expression postfix_expression binary_expression ternary_expression assignment_expression comma_expression 

%type <l> expression_list
%left LOGOR
%left LOGAND
%left '|'
%left '^'
%left '&'
%left EQEQ NOTEQ 
%left GTEQ LTEQ '<' '>'
%left SHL SHR
%left '+' '-'
%left '*' '/' '%'
%start exp_stm_list

%define parse.error verbose
%token-table

%%

exp_stm_list:	exp_stm 
		| exp_stm_list exp_stm
		;
exp_stm: exp ';' {printast($1,0); printf("> ");};

exp:	 comma_expression {$$ = $1;}
	;

primary_expression:	IDENT {$$  = newIdent(IDENT_OP,$1);}
		   	| constant {$$=$1;}
			| parenthesized_expression {$$=$1;}
			;
constant:	numbers {$$=$1;}
		| characters {$$=$1;}
		;

numbers:	INT {	
       			NUMS n;
       			n.i = $1;
       			$$ = newNum(CONST_INT_OP,0, n);
		     }
       		| DOUBLE {
			 	NUMS n;
				n.d = $1;	
				$$ = newNum(CONST_DBL_OP,1,n);
			}
		| FLOAT	{	
				NUMS n;
				n.f = $1;
				$$ = newNum(CONST_FLT_OP,2,n);
			}
		;

characters: 	CHAR {$$ = newIdent(CONST_CHAR_OP,$1);}
	 	| STRING {$$ = newIdent(CONST_STR_OP,$1);}
		;


		

parenthesized_expression:	'(' exp ')' {$$=$2;}
				;

cast_expression:	unary_expression {$$=$1;}
	       		| '(' type_name ')' cast_expression {$$ = newBinop(CAST_OP,' ',$2, $4);}
			;

postfix_expression:	primary_expression {$$=$1;}
			| postfix_expression '[' exp ']' {
								struct astnode *b;
								b = newBinop(SIMPLE_BNOP,'+',$1,$3);
								$$ = newUnop(SIMPLE_UNOP,'*',b);
							}
			| postfix_expression '.' IDENT {	
								struct astnode *b;
								b = newIdent(IDENT_OP, $3);
								$$ = newBinop(SELECT_OP,' ',$1,b);
							}
			| postfix_expression ARROW IDENT {
								struct astnode *b;
								b = newIdent(IDENT_OP,$3);
								struct astnode *c;
								c = newUnop(SIMPLE_UNOP,'*',$1);
								$$ = newBinop(SELECT_OP,' ',c,b);
							  }
			| postfix_expression '(' expression_list ')' {$$ = newFN(FNCALL_OP,$1,$3);}
			| postfix_expression PLUSPLUS  {$$ = newUnop(SIMPLE_UNOP,PLUSPLUS, $1);}
			| postfix_expression MINUSMINUS {$$ = newUnop(SIMPLE_UNOP,MINUSMINUS,$1);}
			;

expression_list: 
	       		| assignment_expression {
							$$ = newList(13,$1);
						}
			| expression_list ',' assignment_expression {
									$$ = newArg(13,$1,$3);
								    }
			;



binary_expression:	cast_expression {$$ = $1;}
			| binary_expression '*' binary_expression {$$ = newBinop(SIMPLE_BNOP,'*',$1,$3);} 
		 	| binary_expression '/' binary_expression {$$ = newBinop(SIMPLE_BNOP,'/',$1,$3);} 
		 	| binary_expression '%' binary_expression {$$ = newBinop(SIMPLE_BNOP,'%',$1,$3);} 
		   	| binary_expression '+' binary_expression {$$ = newBinop(SIMPLE_BNOP,'+',$1,$3);}
		   	| binary_expression '-' binary_expression {$$ = newBinop(SIMPLE_BNOP,'-',$1,$3);}
			| binary_expression SHL binary_expression {$$ = newBinop(SIMPLE_BNOP,SHL,$1,$3);}
			| binary_expression SHR binary_expression {$$ = newBinop(SIMPLE_BNOP,SHR,$1,$3);}
		     	| binary_expression '<' binary_expression {$$ = newBinop(SIMPLE_BNOP,'<',$1,$3);}
		     	| binary_expression '>' binary_expression {$$ = newBinop(SIMPLE_BNOP,'>',$1,$3);}
		     	| binary_expression GTEQ binary_expression {$$ = newBinop(SIMPLE_BNOP,GTEQ,$1,$3);}
		     	| binary_expression LTEQ binary_expression {$$ = newBinop(SIMPLE_BNOP,LTEQ,$1,$3);}
			| binary_expression EQEQ binary_expression  {$$ = newBinop(SIMPLE_BNOP,EQEQ,$1,$3);}
			| binary_expression NOTEQ binary_expression  {$$ = newBinop(SIMPLE_BNOP,NOTEQ,$1,$3);}
			| binary_expression '&' binary_expression  {$$ = newBinop(SIMPLE_BNOP,'&',$1,$3);}
			| binary_expression '^' binary_expression  {$$ = newBinop(SIMPLE_BNOP,'^',$1,$3);}
			| binary_expression '|' binary_expression  {$$ = newBinop(SIMPLE_BNOP,'|',$1,$3);}
			| binary_expression LOGAND binary_expression  {$$ = newBinop(SIMPLE_BNOP,LOGAND,$1,$3);}
			| binary_expression LOGOR binary_expression  {$$ = newBinop(SIMPLE_BNOP,LOGOR,$1,$3);}
			;

unary_expression:	postfix_expression {$$ = $1;}
			|sizeof_expression {$$ = $1;}
			| '-' cast_expression {$$=newUnop(SIMPLE_UNOP,'-',$2);} 
			| '+' cast_expression {$$=newUnop(SIMPLE_UNOP,'+',$2);} 
			| '!' cast_expression {$$=newUnop(SIMPLE_UNOP,'!',$2);}
			| '~' cast_expression {$$=newUnop(SIMPLE_UNOP,'~',$2);}
			| '&' cast_expression {$$=newUnop(SIMPLE_UNOP,'&',$2);}
			| '*' cast_expression {$$=newUnop(SIMPLE_UNOP,'*',$2);}
			| PLUSPLUS unary_expression {	
							struct astnode *b;
							struct astnode *c;
							NUMS n;	
							n.i = 1;
							c = newNum(CONST_INT_OP,0,n);
							b = newBinop(SIMPLE_BNOP,'+',$2,c);
							$$ = newBinop(SIMPLE_BNOP,'=',$2,b);   
						}
			| MINUSMINUS unary_expression {	
							struct astnode *b;
							struct astnode *c;
							NUMS n;	
							n.i = 1;
							c = newNum(CONST_INT_OP,0,n);
							b = newBinop(SIMPLE_BNOP,'-',$2,c);
							$$ = newBinop(SIMPLE_BNOP,'=',$2,b);   
						}

			;

sizeof_expression:	SIZEOF '(' type_name ')' {$$ = newUnop(SIMPLE_UNOP,SIZEOF,$3);}
		 	| SIZEOF unary_expression {$$ = newUnop(SIMPLE_UNOP,SIZEOF,$2);}
			;


type_name: SHORT {$$ = newIdent(TYPE_OP,"SHORT");}
	 | INTEGER {$$ = newIdent(TYPE_OP,"INT");}
	 | LONG	{$$ = newIdent(TYPE_OP,"LONG");}
	 | CHR	{$$ = newIdent(TYPE_OP, "CHAR");}
	 | BOOL	{$$ = newIdent(TYPE_OP, "BOOL");}
	 | ENUM	{$$ = newIdent(TYPE_OP, "ENUM");}
	 | FLT	{$$ = newIdent(TYPE_OP, "FLOAT");}
	 | DBLE	{$$ = newIdent(TYPE_OP, "DOUBLE");}
	 | UNION {$$ = newIdent(TYPE_OP, "UNION");}
	 | VOID	 {$$ = newIdent(TYPE_OP,"VOID");}
	 ;

ternary_expression: 	binary_expression {$$=$1;}
		  	|binary_expression '?' exp ':' ternary_expression {$$ = newTerop(TERNARY_OP,1,$1,$3,$5);}
			;


assignment_expression:	ternary_expression {$$=$1;}
		     	| unary_expression '='  assignment_expression {$$ = newBinop(SIMPLE_BNOP, '=', $1,$3);}
		     	| unary_expression PLUSEQ  assignment_expression {
										struct astnode *b;
										b = newBinop(SIMPLE_BNOP,'+', $1,$3);
										$$ = newBinop(SIMPLE_BNOP, '=', $1,b);
									}
		     	| unary_expression MINUSEQ  assignment_expression {
										struct astnode *b;
										b = newBinop(SIMPLE_BNOP, '-', $1,$3);
										$$ = newBinop(SIMPLE_BNOP, '=', $1,b);
									}
		     	| unary_expression TIMESEQ  assignment_expression {
										struct astnode *b;
										b = newBinop(SIMPLE_BNOP, '*', $1,$3);
										$$ = newBinop(SIMPLE_BNOP, '=', $1,b);
									}
		     	| unary_expression DIVEQ  assignment_expression {
										struct astnode *b;
										b = newBinop(SIMPLE_BNOP, '/', $1,$3);
										$$ = newBinop(SIMPLE_BNOP, '=', $1,b);
									}
		    	| unary_expression MODEQ  assignment_expression {
										struct astnode *b;
										b = newBinop(SIMPLE_BNOP, '%', $1,$3);
										$$ = newBinop(SIMPLE_BNOP, '=', $1,b);
									}
			
			| unary_expression SHLEQ  assignment_expression {
										struct astnode *b;
										b = newBinop(SIMPLE_BNOP, SHL, $1,$3);
										$$ = newBinop(SIMPLE_BNOP, '=', $1,b);
									}

			| unary_expression SHREQ  assignment_expression {
										struct astnode *b;
										b = newBinop(SIMPLE_BNOP, SHR, $1,$3);
										$$ = newBinop(SIMPLE_BNOP, '=', $1,b);
									}
			| unary_expression ANDEQ  assignment_expression {
										struct astnode *b;
										b = newBinop(SIMPLE_BNOP, '&', $1,$3);
										$$ = newBinop(SIMPLE_BNOP, '=', $1,b);
									}

			| unary_expression XOREQ  assignment_expression {
										struct astnode *b;
										b = newBinop(SIMPLE_BNOP, '^', $1,$3);
										$$ = newBinop(SIMPLE_BNOP, '=', $1,b);
									}
			| unary_expression OREQ  assignment_expression {
										struct astnode *b;
										b = newBinop(SIMPLE_BNOP, '|', $1,$3);
										$$ = newBinop(SIMPLE_BNOP, '=', $1,b);
									}
									;
comma_expression:	assignment_expression {$$ = $1;}
			| comma_expression ',' assignment_expression { $$ =newBinop(SIMPLE_BNOP,',',$1, $3);}
			;

%%
