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
# include "typeTypes.h"
# include "paramTypes.h"
# include "scopeTypes.h"

char file_name[300];
int line;
struct scope * currentScope;
int withinLoop;
int tmpCounter;
%}


%union{
	struct astnode *a;
	struct listarg *l;
	struct superSpec *super;
	struct smallSpec *small;
	struct symbol * params;
	struct scope * scope;
	struct initializedTypes * initType;
	int integer;
	char *value;
	float f;
	double d;
	char * string_literal;
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
%token <string_literal> FILEN 


%type <a> exp exp_stm unary_expression primary_expression numbers characters constant parenthesized_expression cast_expression postfix_expression binary_expression ternary_expression assignment_expression comma_expression type_name declaration_spec pointer type_qualifier_list union_type_definition structure_type_definition statement decl_or_stmt compound_stmt if_else_statement if_statement conditional_statement null_statement labeled_statement label goto_statement switch_statement for_statement do_statement while_statement case_label named_label default_label initial_clause exp_opt break_statement continue_statement return_statement iterative_statement function_body
%type <super> declaration_specifier initialized_declarator_list component_declaration component_declarator_list 
%type <small> simple_declarator initialized_declarator direct_declarator array_declarator declarator function_declarator pointer_declarator
%type <l> expression_list
%type <integer> type_n type_qualifier storage_class_specifier specs 
%type <params>  parameter_list parameter_declaration parameter_type_list struct_definition union_definition 
%type <scope> field_list
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

decl_or_stmt: 	declaration 			{
	    					 // create dud astnode 
						 struct astnode * emptyNode = NULL;
						}
		| declaration decl_or_stmt 	{
						  $$ = $2;
						}
		| statement {	
				$$ = $1;
			    }
		| statement decl_or_stmt {
						$1->next = $2;
						$$ = $1; 
					 } 
		;

statement:	  compound_stmt 
	 	| exp_stm
		| conditional_statement
		| labeled_statement
		| switch_statement
		| goto_statement
		| iterative_statement
		| break_statement
		| continue_statement
		| return_statement
		| null_statement;


compound_stmt: '{'{	int scopetype = currentScope->scopeType !=SWITCHSCOPE ? BLOCKSCOPE : SWITCHSCOPE; 
	     		currentScope = newSymbolTable(currentScope, scopetype);} decl_or_stmt[decl] '}' {
				// pop out of current scope. 
				// also verify that you are not in the outermost scope
				currentScope = destroySymbolTable(currentScope);
				$$ = createCompoundAst(COMPOUND,$decl); 
		}
		;

declaration: decl;

decl:   	declaration_specifier initialized_declarator_list ';' 	{
										enterNewVariable(currentScope,OTHERSPACE,$2);
										printVariable(currentScope,line, file_name);
										
									}
		| declaration_specifier function_declarator function_body {
										// assign function scope to function astnode...
										struct astnode * fnNode = $2->types->t;
										 fnNode->u.spec.functionScope = currentScope;		
										// why am I destroying the current scope? 
										// We are not destroying it here, we are just pointing it
										// to the previous scope because we have no exited it
										currentScope->done = 1;
										currentScope = destroySymbolTable(currentScope);
										struct superSpec * super = $1;
										struct init * i = malloc(sizeof(struct init));
										i->value = strdup($2->value);
										i->next = NULL;
										super->i = i;
										super->initialType = $2->types;
										enterNewVariable(currentScope,OTHERSPACE,super); 
										printVariable(currentScope,line, file_name);
										struct astnode * fnAst = createCompoundAst(FUNCTION,$3);
										printast(fnAst,0);
										emitQuads(fnAst);
									}	
		| structure_type_definition ';' 
		| union_type_definition ';'
		;

function_body: '{'{currentScope = newSymbolTable(currentScope, FUNCTIONSCOPE);} decl_or_stmt[decl] '}' { $$ = $decl;}
													
	     	;

declaration_specifier:	declaration_spec { 
		     			 	struct superSpec * lastSpec = malloc(sizeof(struct superSpec));
						lastSpec->generalType = NULL;
						lastSpec->s = NULL;
						if ($1->nodetype == TYPE){	
							lastSpec->generalType = $1;
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
										struct astnode * t = previous->generalType;
										if(t){		
											t->u.spec.next = $2;
										}else{
											previous->generalType = $2;
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
			   					struct superSpec * super = $<super>0;
								struct init * i = malloc(sizeof(struct init));
								i->value = $1->value;
								i->next = NULL;
								super->i = i;
								super->initialType = $1->types;
								$$=super;		
							}
			   	| initialized_declarator_list ',' initialized_declarator {
												// add the new initialized_declarator to 
												// the super spec
												struct superSpec * super = $1;
												struct init * new = malloc(sizeof(struct init));
												// get the new init
												new->value = $3->value;
												new->next = super->i;
												super->i = new;
												// get the new inital type
												struct initializedTypes * newTypes = $3->types;
												newTypes->next = super->initialType;
												super->initialType = newTypes; 

											}
				;

initialized_declarator: declarator
			;

declarator:	 direct_declarator
	  	|pointer_declarator
	  	;


direct_declarator: 	simple_declarator
		 	|'(' declarator ')' {
						
						struct astnode * t = $2->types->t;
						$$= $2;
					     }
			| array_declarator	
			| function_declarator 
			;

pointer_declarator:	pointer direct_declarator {
							// direct_declarator returns a smallSpec
	  						// pointers will return intialized type.
							// this thing has to return at smallSpec
						  	struct smallSpec * direct_decl = $2;
							struct initializedTypes * old = direct_decl->types;	
							struct astnode * newPointerTypes = $1;
							// go to end of newPointerTypes and tack in old...
							// got a major type  problem going on here.
							// newPointerTypes is a pointer to astnode, while old is an initialzied type
							struct initializedTypes * new = malloc(sizeof(struct initializedTypes));
							struct astnode * a = old->t;
							while(a){
								if(a->u.spec.next == NULL){
									a->u.spec.next = newPointerTypes;
									break;
								}
								a = a->u.spec.next;
							}
							if (old->t == NULL){
								new->t = newPointerTypes;
							}else{
								new->t = old->t;
							}
							direct_decl->types = new; 
							$$ = direct_decl;
						}
			;

pointer:	'*' {
			struct astnode * pntr = addInitType(TYPE,PNTRTYPE,NULL);	
			$$ =pntr;
			
       		    }
       		| '*' type_qualifier_list {
							// here and the next one we need to build the pntr node to include the type qualifiers
							// this should all be done with the addInitType function
   		        				struct astnode * pntr =  addInitType(TYPE,PNTRTYPE,$2);	
							$$ =pntr;

		 				}
		| '*' type_qualifier_list pointer {
   		    					struct astnode * pntr = addInitType(TYPE,PNTRTYPE,$2);
							pntr->u.spec.next = $3;
							$$ =pntr;

						  }
		| '*' pointer{
				
   		    		struct astnode * pntr = addInitType(TYPE,PNTRTYPE,NULL);	
				pntr->u.spec.next = $2;
				int count = 0;
				struct astnode * a = pntr;
				$$ = pntr;

				}
		;

type_qualifier_list: 	type_qualifier {
						// what I got to do here is start building the astnode
						struct astnode * initType = malloc(sizeof(struct astnode));
						initType->u.spec.val = $1;
						$$ =initType;

					}
		   	| type_qualifier_list type_qualifier {
								struct astnode * previousInit = $1;
								struct astnode * newInit = malloc(sizeof(struct astnode));
								newInit->u.spec.next = previousInit;
								newInit->u.spec.val = $2;
								$$ =newInit;
							     }
			;


simple_declarator:	IDENT {
				 struct smallSpec * starter = malloc(sizeof(struct smallSpec));
				 struct initializedTypes * types = malloc(sizeof(struct initializedTypes)); 
				 types->t = NULL;
				 types->next = NULL;
				 starter->value = strdup($1);
				 starter->types = types;
				 $$ = starter;			
				}
		 	;
	  	
array_declarator:	direct_declarator '['']' {
							struct smallSpec * direct_decl = $1;
							struct initializedTypes * previous = direct_decl->types;
							// create new type here that has to do with arrays of unknown size
							struct astnode * arrayType = addInitType(TYPE,ARRAYTYPE,NULL);
							struct initializedTypes * new = malloc(sizeof(struct initializedTypes));
							arrayType->u.spec.size = -1;
							struct astnode * a  = previous->t;
							while(a){
								if(a->u.spec.next == NULL){
									a->u.spec.next = arrayType;
									break;
								}
								a = a->u.spec.next;
							}
							if (previous->t==NULL){
								new->t = arrayType;
							}else{
								new->t = previous->t;
							}
							direct_decl->types = new; 
							$$ = direct_decl;
							}
			| direct_declarator '[' INT ']' {
							struct smallSpec * direct_decl = $1;
							struct initializedTypes * previous = direct_decl->types;
							// create new type here that has to do with arrays of unknown size
							struct astnode * arrayType = addInitType(TYPE,ARRAYTYPE,NULL);
							struct initializedTypes * new = malloc(sizeof(struct initializedTypes));
							arrayType->u.spec.size = $3;
							struct astnode * a  = previous->t;
							while(a){
								if(a->u.spec.next == NULL){
									a->u.spec.next = arrayType;
									break;
								}
								a = a->u.spec.next;
							}
							if (a==NULL){
								new->t = arrayType;
							}else{
								new->t = previous->t;
							}
							direct_decl->types = new;
							$$ = direct_decl;

							}
			;


function_declarator:	direct_declarator '(' parameter_type_list ')' {
									struct smallSpec * direct_decl = $1;
									struct initializedTypes * previous = direct_decl->types;
									struct astnode * fnType = addInitType(TYPE,FNTYPE,NULL);
									struct initializedTypes * new = malloc(sizeof(struct initializedTypes));
									struct symbol * topSymbol = $3;	
									struct symbol * prevs = NULL;
									struct symbol * next;

									// reverse the order here! 
									while(topSymbol->previous){
										next = topSymbol->previous;
										topSymbol->previous = prevs;
										prevs = topSymbol;
										topSymbol = next;
									}
									fnType->u.spec.params = topSymbol;
									struct astnode * a = previous->t;
									while(a){
										if(a->u.spec.next == NULL){
											a->u.spec.next = fnType;
											break;
										}
										a = a->u.spec.next;
									}
									if(previous->t == NULL){
										new->t = fnType;
									}else{
										new->t = previous->t;
									}	
									direct_decl->types = new; 
									$$ = direct_decl;		
								      }
		   	| direct_declarator '(' ')' {
							struct smallSpec * direct_decl = $1;
							struct initializedTypes * previous = direct_decl->types;
							struct astnode * fnType = addInitType(TYPE,FNTYPE,NULL);
							struct initializedTypes * new = malloc(sizeof(struct initializedTypes));
							fnType->u.spec.params = NULL;
							struct astnode * a = previous->t;
							while(a){
								if(a->u.spec.next == NULL){
									a->u.spec.next = fnType;
									break;
								}
								a = a->u.spec.next;
							}
							if(previous->t == NULL){
								new->t = fnType;
							}else{
								new->t = previous->t;
							}								
							direct_decl->types = new;
							$$ = direct_decl;	
						    }
			;

parameter_type_list: 	parameter_list
			;

parameter_list:		parameter_declaration
			| parameter_list ',' parameter_declaration {
								   	struct symbol * top = $1;
									struct symbol * new = $3;
									new->previous = top;
									$$ = new;
								   }
			;

parameter_declaration: 	declaration_specifier declarator {
		     						// declaration_specifier returns a super.
								// declarator returns a smallspec
								struct superSpec * super = $1;
								struct smallSpec * small = $2;
								struct init * i = malloc(sizeof(struct init));
								i->value = small->value;
								i->next = NULL;
								super->i = i;
								super->initialType = small->types;
								enterNewVariable(currentScope, OTHERSPACE, super);
								$$ = currentScope->last;
 							  }
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


structure_type_definition:	struct_definition '{'field_list '}' 	{
			 								
		     								$$ = createStruct(currentScope, $1,$3,0);
									}
				;


struct_definition: 	STRUCT IDENT 	{
						struct symbol * structSymbol = malloc(sizeof(struct symbol));
						structSymbol->nameSpace = STRUCTSPACE;	
						structSymbol->name = strdup($2);
						$$=structSymbol;	
					}
		 	;


union_type_definition: union_definition '{' field_list '}' 	{
		     							$$ = createStruct(currentScope, $1,$3,1);
								}
		      ;

union_definition: UNION IDENT 	{
					struct symbol * unionSymbol = malloc(sizeof(struct symbol));
					unionSymbol->nameSpace = STRUCTSPACE;	
					unionSymbol->name = strdup($2);
					$$=unionSymbol;	
	
				
				};


field_list:	component_declaration {
	  				// here we create the entering scope	
					struct scope * newScope = malloc(sizeof(struct scope));
					newScope->scopeType = STRUCTSCOPE;
					newScope->previous = currentScope; 
					// then we take the new symbol and add it to the last of this new scope
					enterNewVariable(newScope,OTHERSPACE,$1);
					$$ = newScope; 
				      }
	  	| field_list component_declaration	{
								// here we add the symbol to the correct scope and link
								// it to the previous symbol
								struct scope * structScope = $1;
								enterNewVariable(structScope,OTHERSPACE,$2);
								$$ = structScope;
							}
		;	

component_declaration: type_name component_declarator_list ';' 	{
									$2->generalType = $1;
									$$ = $2;
								}
		     						;		
component_declarator_list:	declarator 	{
							struct superSpec * super = malloc(sizeof(struct superSpec));
							struct init * i = malloc(sizeof(struct init));
							i->value = $1->value;
							i->next = NULL;
							super->i = i;
							super->initialType = $1->types;
							$$=super;
						}
			 	| component_declarator_list ',' declarator 	{
												struct superSpec * super = $1;
												struct init * new = malloc(sizeof(struct init));
												new->value = $3->value;
												new->next = super->i;
												super->i = new;
												struct initializedTypes * newTypes = $3->types;
												newTypes->next = super->initialType;
												super->initialType = newTypes; 
										}
				;



exp_stm: exp ';' {$$ = $1;};

exp:	 comma_expression {$$ = $1;}
	;

primary_expression:	 IDENT {
			  		// look for this ident in the symbol table
					struct symbol * ident = findSymbol(currentScope, $1, OTHERSPACE);			
					struct astnode * symb = malloc(sizeof(struct astnode));
					if(ident){
						struct symbol * cident = malloc(sizeof(struct symbol));
						memcpy(cident,ident,sizeof(struct symbol));
						symb->u.symbol = cident;
						symb->nodetype = SYMBOL;
							
					}else{
						yyerror("%s is not in the symbol table.",$1);
					}
					$$=symb;	
				}
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
	 	
		
		| struct_definition 		{
					
						char * ident = $1->name;
						struct symbol * queriedStruct = findSymbol(currentScope, ident, STRUCTSPACE);	
						if (queriedStruct){
							// make sure it is a struct and not a union
							if(queriedStruct->type->u.spec.val == STRUCTTYPE){
								$$ = queriedStruct->type;	
							} else {
								yyerror("You are trying to reference a union as a struct");
							}
						}else{
							// so here we need to create incomplete type, and then eventually
							// verify if it is good or not.
							struct astnode * incompleteType = malloc(sizeof(struct astnode));
							incompleteType->nodetype = TYPE;
							incompleteType->u.spec.val = INCOMPLETETYPE;
							incompleteType->u.spec.incompleteName = strdup(ident);
							$$ = incompleteType;	
						}
					}
		| union_definition 	{
						char * ident = $1->name;
						struct symbol * queriedUnion = findSymbol(currentScope, ident, STRUCTSPACE);	
						if (queriedUnion){
							// make sure it is a struct and not a union
							if(queriedUnion->type->u.spec.val == UNIONTYPE){
								$$ = queriedUnion->type;	
							} else {
								yyerror("You are trying to reference a struct as a union");
							}
						}else{
							// so here we need to create incomplete type, and then eventually
							// verify if it is good or not.
							struct astnode * incompleteType = malloc(sizeof(struct astnode));
							incompleteType->nodetype = TYPE;
							incompleteType->u.spec.val = INCOMPLETEUNION;
							incompleteType->u.spec.incompleteName = strdup(ident);
							$$ = incompleteType;	
						}		
					}
		| structure_type_definition 	{
							$$ = $1;
						}
		| union_type_definition {
						$$ = $1;
					}
		;

type_n: 	SHORT {$$=SHORT;}
	 	| INTEGER {$$=INTEGER;}
	 	| LONG {$$=LONG;}
		| LONGLONG {$$=LONGLONG;}
	 	| CHR {$$=CHR;}	
	 	| BOOL	{$$=BOOL;}
	 	| FLT	{$$=FLT;}
	 	| DBLE	{$$=DBLE;}
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


conditional_statement: 		if_statement
		     		| if_else_statement;
		     

if_statement: 		IF '(' exp ')' statement 		{		
									$$ = addCondStatement(IFNODE,$3,$5, NULL);
								};

if_else_statement:	if_statement ELSE statement 		{
		 							$1->u.ifNode.elseStatement = $3;			
									$$ = $1;
								};

labeled_statement: label ':' statement {$$ = addLabelStatement($1,$3);};

label:	named_label
     	| case_label
	| default_label;

switch_statement: SWITCH '(' exp[express] ')' {
			// jump into fake scope just so that we know we are in a switch
					currentScope = newSymbolTable(currentScope, SWITCHSCOPE);
			}  statement[stmt] {
					currentScope = destroySymbolTable(currentScope);
					$$ = addCondStatement(SWITCHNODE,$express,$stmt,NULL);
			};
			
case_label:CASE INT     {$$ = addCaseStmt(CASE,$2);} 
	   | CASE CHAR  {$$ = addCaseStmt(CASE,(int) $2);};


default_label: DEFAULT {$$ = addCaseStmt(DEFAULT,NULL);};



goto_statement: GOTO named_label ';'{
	      				struct astnode * n = malloc(sizeof(struct astnode));
					$$ = n;
					$$ = addSimpleStmt(GOTO,$2);
	      
				};

named_label: IDENT {$$ = newIdent(NAMEDLABEL,$1);};

iterative_statement:	while_statement
		   	| do_statement
			| for_statement
			;

while_statement: WHILE '(' exp[expr] ')'{withinLoop+=1;} statement[stmt] {  
										withinLoop-=1;
										notInGlobalScope();
										$$ = addCondStatement(WHILENODE,$expr,$stmt, NULL);
									};

do_statement: DO {withinLoop+=1;} statement[stmt] {withinLoop-=1;} WHILE '(' exp[expr2] ')'{  
	    					notInGlobalScope();
						$$ = addCondStatement(DONODE,$expr2,$stmt,NULL);
					    };

for_statement: FOR '(' initial_clause[init] ';' exp_opt[expo] ';' exp_opt[expo2]')'{withinLoop+=1;} statement[stmt] { withinLoop-=1;notInGlobalScope(); $$ = addForNode($init,$expo,$expo2,$stmt);}; 

initial_clause:  { struct astnode * empty = NULL;
	      	   $$ = empty;
		
		 }
	      	| exp;

exp_opt: { struct astnode * empty = NULL;
	      	   $$ = empty;
		
         }
       	| exp;



break_statement: BREAK ';' { 
	       				$$ = addBreakStmt();};

continue_statement: CONTINUE ';'{ $$ = addContStmt();};

return_statement:	RETURN exp ';' {	
						notInGlobalScope();
						$$ = addSimpleStmt(RETURN,$2);
					}

			| RETURN ';' {
						notInGlobalScope();
						$$ = addSimpleStmt(RETURN,NULL);
				     };

null_statement: ';'  {struct astnode * empty = malloc(sizeof(struct astnode)); $$ = empty;} 


%%

