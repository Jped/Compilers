/* C Parser

	Jonathan Pedoeem
	Spring 2019
	The Cooper Union

*/

%{
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include "parser.h"
# include "ops.h"
# include "namespace.h"
# include "types.h"

char file_name[300];
int line;
%}


%union{
	struct astnode *a;
	struct listarg *l;
	struct init *i;
	struct superSpec *super;
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
%token LONGLONG
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


%type <a> exp unary_expression primary_expression numbers characters constant parenthesized_expression cast_expression postfix_expression binary_expression ternary_expression assignment_expression comma_expression type_name declaration_spec 
%type <super> declaration_specifier
%type <i> initialized_declarator_list
%type <l> expression_list
%type <integer> type_n type_qualifier storage_class_specifier specs 
%type <string_literal> initialized_declarator

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
%start decl_or_stmt

%define parse.error verbose

%%

decl_or_stmt: 	declaration {printf(">");}
		| statement
		;

declaration:	decl
	     	| declaration decl
		;

decl:   	declaration_specifier initialized_declarator_list ';' {
	   									struct scope * escope = malloc(sizeof(struct scope));
										escope->last = NULL;
										escope->next = NULL;
										escope->previous = NULL;	
										// have to recover the type astnode here and pass it in.
										struct astnode * specs = $1->s;
										struct astnode * type = $1->t; 
										enterNewVariable(escope,$2,specs,GLOBALSCOPE,type);
										printVariable(escope,line, file_name);
										
										}
	  	;

declaration_specifier:	declaration_spec { 
		     			 	struct superSpec * lastSpec = malloc(sizeof(struct superSpec));
						lastSpec->t = NULL;
						lastSpec->s = NULL;
						if ($1->nodetype == TYPE){	
							lastSpec->t = $1;
						}else if($1->nodetype == SPEC){
						 	lastSpec->s = $1;
						} 
						$$ = lastSpec; 
					 } 
		     	| declaration_specifier declaration_spec {
									
									struct superSpec * previous = $1;
									// see which type of value we have here
									// and add it to the corresponding nodetype
									 if($2->nodetype == TYPE){
										struct astnode * t = previous->t;
										if(t){		
											t->u.spec.next = $2;
										}else{
											previous->t = $2;
										}
									}else if($2->nodetype == SPEC){
										struct astnode *s = previous->s;
										if(s){
											s->u.spec.next = $2;
										}else{
											previous->s = $2;
										}
									}
									$$ = previous; 
								 }
			;

declaration_spec: 		specs {
					struct astnode * spec = malloc(sizeof(struct astnode));
					spec->nodetype = SPEC;
					spec->u.spec.val = $1;
					$$ = spec;
					}
		      		| type_name
				;

specs: 	storage_class_specifier
     	| type_qualifier
	;

initialized_declarator_list:	initialized_declarator {
								struct init * lastInit = malloc(sizeof(struct init));
								lastInit->value = strdup($1); 	
								lastInit->next=NULL;
								$$ = lastInit; 

							}
			   	| initialized_declarator_list ',' initialized_declarator {
												struct init * newInit = malloc(sizeof(struct init));
												newInit->value = strdup($3); 
												newInit->next=$1;
												$$ = newInit;				
											 }
				;

initialized_declarator: declarator
			;

declarator:	direct_declarator
	  	;

direct_declarator: 	simple_declarator
		 	|'(' declarator ')'
			| function_declarator
			| array_declarator
		 	;

simple_declarator:	IDENT 
		 	;
	  	
function_declarator:	direct_declarator '(' parameter_type_list ')'
		   	| direct_declarator '(' identifier_list ')'
			| direct_declarator '(' ')'
			;

parameter_type_list: 	parameter_list
		   	| parameter_list ',' ELLIPSIS
			;

parameter_list:		parameter_declaration
			| parameter_list ',' parameter_declaration
			;

parameter_declaration: 	declaration_specifier declarator
		     	| declaration_specifier
			;

identifier_list: 	IDENT
			| parameter_list ',' IDENT
			;

array_declarator:	direct_declarator '['']'
			| direct_declarator '[' INT ']'
			;


storage_class_specifier:	AUTO {$$=AUTO;}
				| EXTERN {$$=EXTERN;}
				| REGISTER {$$=REGISTER;}
				| STATIC {$$=STATIC;}
				| TYPEDEF {$$=TYPEDEF;}
				;

type_qualifier:	CONST {$$=CONST;}
		| VOLATILE {$$=VOLATILE;}
		| RESTRICT {$$=RESTRICT;}
		| SIGNED {$$=SIGNED;}
		| UNSIGNED {$$=UNSIGNED;}
		;


statement:	exp_stm
	 	| exp_stm_list exp_stm
		;

exp_stm_list:	exp_stm 
		| exp_stm_list exp_stm
		;

exp_stm: exp ';' {printast($1,0); printf("File: %s line: %d> ",file_name,line);};

exp:	 comma_expression {$$ = $1;}
	;

primary_expression:	constant {$$=$1;}
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


type_name:	type_n {
	 		struct astnode * simpleType = malloc(sizeof(struct astnode));
			simpleType->nodetype = TYPE;
			simpleType->u.spec.val = $1;
			$$ = simpleType;
			}
	 	;

type_n: 	SHORT {$$=SHORT;}
	 	| INTEGER {$$=INTEGER;}
	 	| LONG {$$=LONG;}
		| LONGLONG {$$=LONGLONG;}
	 	| CHR {$$=CHR;}	
	 	| BOOL	{$$=BOOL;}
	 	| ENUM	{$$=ENUM;}
	 	| FLT	{$$=FLT;}
	 	| DBLE	{$$=DBLE;}
	 	| UNION {$$=UNION;} 	 
	 	| VOID	 {$$=VOID;}
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

