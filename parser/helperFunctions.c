# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>
# include "parser.h"
# include "cparser.tab.h"
# include "types.h"
# include "typeTypes.h"	
# include "scopeTypes.h"
# include "namespace.h"
# include "ops.h" 

	
struct astnode * 
newTerop(int nodetype, int op, struct astnode *l, struct astnode *c, struct astnode *r)
{
	struct astnode *a = malloc(sizeof(struct astnode));
	a->nodetype= nodetype;
	a->u.terop.op = op;
	a->u.terop.l = l;
	a->u.terop.c = c;
	a->u.terop.r = r;
	return a;

}
struct astnode *
newBinop(int nodetype, int op, struct astnode *l, struct astnode *r)
{
	struct astnode *a = malloc(sizeof(struct astnode));
	a->nodetype= nodetype;
	a->u.binop.op = op;
	a->u.binop.l = l;
	a->u.binop.r = r;
	return a;
}

struct astnode * 
newUnop(int nodetype, int op, struct astnode *c)
{

	struct astnode *a = malloc(sizeof(struct astnode));
	a->nodetype = nodetype;
	a->u.unop.op = op;
	a->u.unop.c = c;
	return a;	

}


struct astnode * 
newIdent(int nodetype, char *name)
{	
	struct astnode *a = malloc(sizeof(struct astnode));
	a->nodetype = nodetype;
	a->u.ident.name = strdup(name);
	return a;
}
struct astnode *
newNum(int nodetype,short type,NUMS val)
{
	struct astnode *a = malloc(sizeof(struct astnode));
	a->nodetype = nodetype;
	if (type==0){
		a->u.num.val.i = val.i;
	} else if (type==1){
	        a->u.num.val.d = val.d;	
	}else{
		a->u.num.val.f = val.f;
	}
	return a;
}

struct astnode *
newFN(int nodetype, struct astnode *l, struct listarg *r)
{
	struct astnode *a = malloc(sizeof(struct astnode));
	a->nodetype = nodetype;
	a->u.fn.l = l;
	a->u.fn.r = r;
	return a;

}

struct listarg *
newArg(int nodetype, struct listarg * current, struct astnode * nextarg)
{
	struct listarg * next = malloc(sizeof(struct listarg));
	next->size = (current->size) + 1;
	next->previous = current;
	next->nodetype = nodetype; 
	current->next = next;
	next->next = NULL;
	next->start = current-> start;
	next->ast = nextarg;
	return next;
}

struct listarg *
newList(int nodetype, struct astnode * a)
{
	struct listarg * first = malloc(sizeof(struct listarg));
	first->size = 1;
	first->ast = a;
	first->previous = NULL;
	first->next = NULL;
	first->nodetype = nodetype;
	first->start = first;
	return first;
}
	
void 
printast(struct astnode * a, int level)
{
	if(!a) { 
		goto FINISH;
	}
	for (int i =0;i<level;i++){
		printf("\t");
	}
	switch (a->nodetype){
		case 0:
		       if (a->u.binop.op<256){
			       if(a->u.binop.op == '='){
					printf("ASSIGNMENT\n");	
			       }else{
			    		printf("BINARY OP %c\n",a->u.binop.op);
		       		}
			} else {
				printf("BINARY OP %s\n", token2name(a->u.binop.op));
		       }
		       level++;
		       printast(a->u.binop.l,level);
	       	       printast(a->u.binop.r,level);
		       break;
		case 1:  
		       printf("IDENT %s\n", a->u.ident.name);	       
		       break;
		case 2:
		       if (a->u.unop.op<256){
			      if (a->u.unop.op == 42){
			 	printf("DEREF \n"); 
			      }else{	
		 	      	printf("UNARY OP %c\n",a->u.unop.op);
	       	      	      } 
			 }else{
		 	     printf("UNARY OP %s\n",token2name(a->u.unop.op));
			}
		       level++;
		       printast(a->u.unop.c,level);
		       break;
		case 3: 
		       printf("TYPE %s\n", a->u.ident.name);
		       break;
		case 4: 
		       printf("CONSTANT: (type=int) %d\n",  a->u.ident.name[0]);
		       break;
		case 5: 
		       printf("STRING %s\n", a->u.ident.name);
		       break;
		case 6:
		       printf("CONSTANT: (type=int)%d\n",a->u.num.val.i); 
		       break;
		case 7:
			printf("CONSTANT: (type=double)%f\n",a->u.num.val.d); 
		       	break;
		case 8:
			printf("CONSTANT: (type=float)%.e\n",a->u.num.val.f); 
		       	break;

		case 9:
			printf("SELECT \n");
			level++;
			printast(a->u.binop.l,level);
			printast(a->u.binop.r,level);
			break;
		case 10: 
			printf("TERNARY OP, IF:\n");
			level++;
			printast(a->u.terop.l,level);
			for (int i =0;i<level-1;i++){
				printf("\t");
			}	
			printf("THEN:\n");
			printast(a->u.terop.c,level);
			for (int i =0;i<level-1;i++){
				printf("\t");
			}
			printf("ELSE:\n");
			printast(a->u.terop.r,level);
			break;		
		case 12: 
			printf("FNCALL, %d arguments\n",a->u.fn.r->size);
			level++;
			printast(a->u.fn.l,level);
			printarg(a->u.fn.r->start,level--);
			break;
		case 13: 
			printf("CAST OPERATION\n");
			level++;
			printast(a->u.binop.l, level);
			printast(a->u.binop.r,level);
		case FORNODE: 
		      	printf("FOR\n");
			for (int i =0;i<level;i++){
				printf("\t");
			}
			printf("INIT:\n");
			level++;
			printast(a->u.forNode.initial_clause,level);
			for (int i =0;i<level;i++){
				printf("\t");
			}
			printf("COND:\n");
			printast(a->u.forNode.expression1,level);
			for (int i =0;i<level;i++){
				printf("\t");
			}
			printf("BODY:\n");
			printast(a->u.forNode.statement,level);
			for (int i =0;i<level;i++){
				printf("\t");
			}
			printf("INCR:\n");
			printast(a->u.forNode.expression2,level); 
	       	     	break;	      
			
		case SYMBOL:
			printf("SYMBOL: %s\n", a->u.symbol->name); 
			break;	
		case COMPOUND:
			printf("LIST:\n {\n");
			level++;
			a = a->u.compound.statements;
			while(a){
				printast(a,level);
				a = a->next;
			}
			
			printf("}\n");
			break;
		case FUNCTION:
			printf("FUNCTION:\n");
			for (int i =0;i<level;i++){
				printf("\t");
			}
			printf("LIST:\n {\n");
			level++;
			a = a->u.compound.statements;
			while(a){
				printast(a,level);
				a = a->next;
			}
			printf("}\n");
			break;
		case LABELNODE:
			printf("LABELNODE \n ");
			level++;
			for (int i =0;i<level;i++){
				printf("\t");
			}
			printast(a->u.label.label,level);
			a = a->u.label.statement;
			while(a) { 
				printast(a,level);
			       	a = a->next; 	
			} 
			break;
		case SIMPLESTMT:
		        // check here what type of simple statement is present and deal
			// with it accordingly	
			switch(a->u.simple.type){
				case CASE:
					printf("CASE");
					level++;
					printast(a->u.simple.exp,level);
					break;
				case DEFAULT: 
					printf("DEFAULT");
					level++;
					printast(a->u.simple.exp,level);
					break;
				case CONTINUE: 
					printf("CONTINUE STATEMENT\n");
					break;
				case BREAK:
					printf("BREAK STATEMENT\n");
					break;
				case RETURN:
					printf("RETURN STATEMENT\n");
					if (a->u.simple.exp) { 
						printf("RETURN EXP");
						level++;
						printast(a->u.simple.exp,level);
					} 
					break;
			}
			printf("____________________________\n");
			break;
		case  SWITCHNODE: 
			printf("SWITCH: CONDITION\n");
			level++;
			printast(a->u.ifNode.exp,level);
			printast(a->u.ifNode.statement,level);	
			break;
		case IFNODE: 
			printf("IF: \n");
			level++;
			for (int i =0;i<level;i++){
				printf("\t");
			}
			printf("CONDITION:\n");
			level++;
			printast(a->u.ifNode.exp,level);
			level--;
			for (int i =0;i<level;i++){
				printf("\t");
			}
			printf("STATEMENT:\n");
			level++;
			printast(a->u.ifNode.statement,level);
			if(a->u.ifNode.elseStatement){
				level--;
				for (int i =0;i<level;i++){
					printf("\t");
				}
				printf("ELSE STATEMENT");
				level++;
				printast(a->u.ifNode.elseStatement, level);
			}
			break;
		case WHILENODE:
			printf("WHILE: \n");
			level++;
			for (int i =0;i<level;i++){
				printf("\t");
			}
			printf("CONDITION:\n");
			level++;
			printast(a->u.ifNode.exp,level);
			level--;
			for (int i =0;i<level;i++){
				printf("\t");
			}
			printf("STATEMENT:\n");
			level++;
			printast(a->u.ifNode.statement,level);
			break;
		case DONODE:
			printf("DO WHILE: \n");
			level++;
			for (int i =0;i<level;i++){
				printf("\t");
			}
			printf("STATEMENT:\n");
			level++;
			printast(a->u.ifNode.statement,level);
			level--;
			for (int i =0;i<level;i++){
				printf("\t");
			}
			printf("CONDITION:\n");
			level++;
			printast(a->u.ifNode.exp,level);
			break;
	}
	FINISH:;	
}
void printarg(struct listarg * l ,int level)
{
	for (int i =0;i<level-1;i++){
		printf("\t");
	}
	switch(l->nodetype){
		case 13: 
			printf("arg #%d=\n",l->size);
			level++;
			printast(l->ast,level);
			level--;
			if (l->next) {
				printarg(l->next,level);
			}
			break;
	}

}

void 
yyerror(const char * s,...)
{
	va_list ap;
	va_start(ap, s);
	fprintf(stderr, "%d: error: ", yylineno);
	vfprintf(stderr,s,ap);
	fprintf(stderr, "\n");
}

int
main()
{
	// need to create global scope here...
	currentScope = malloc(sizeof(struct scope));	
	currentScope->last = NULL;
	currentScope->next = NULL;
	currentScope->scopeType = GLOBALSCOPE;
	currentScope->previous = NULL;
	withinLoop = 0;
	tmpCounter = 0;
	return yyparse();
}

char *
token2name(int token)
{
	switch(token){
		case ELLIPSIS:
		       return "...";
		case PLUSPLUS:
		       return "++";
		case MINUSMINUS:
		       return "--";
		case SHL: 
		       return "<<";
		case SHR:
		       return ">>";
		case LTEQ:
		       return "<=";
		case GTEQ:
		       return ">=";
		case NOTEQ:
		       return "!=";
		case LOGAND:
		       return "&&";
		case LOGOR:
		       return "||";
		case SIZEOF:
		       return "SIZEOF";
		case EQEQ:
		       return "==";
	}
}

char *
type2name(int token)
{
	switch(token){
		case 0:
			return "NONE";	
		case ARRAYTYPE:
			return "ARRAY";
		case FNTYPE:
			return "FUNCTION";
		case PNTRTYPE:
			return "POINTER";
		case STRUCTTYPE:
			return "STRUCT";
		case INCOMPLETETYPE:
			return "INCOMPLETE STRUCT";
		case UNIONTYPE:
			return "UNION";
		case INCOMPLETEUNION:
			return "INCOMPLETE UNION";
		case SHORT:
		       return "SHORT";
		case  INTEGER:
		      return "INTEGER";
		case  LONG:
		      return "LONG";
		case LONGLONG:
		     return "LONG LONG";
		case CHR:
		    return "CHAR";
		case BOOL:
		    return "BOOLEAN";
		case FLT:
	 	     return "FLOAT";
		case DBLE:
		     return "DOUBLE";
		case VOID:
		     return "VOID";
		case AUTO:
		     return "AUTO";
		case EXTERN:
		     return "EXTERN";
		case STATIC:
		     return "STATIC";
		case REGISTER:
		     return "REGISTER";
		case CONST:
		     return "CONST";
		case VOLATILE:
		     return "VOLATILE";
		case RESTRICT:
		     return "RESTRICT"; 
	}
}

char *
scope2name(int token)
{

	switch(token){
		case GLOBALSCOPE:
			return "GLOBAL SCOPE";
		case FUNCTIONSCOPE:
			return "FUNCTION SCOPE";
		case BLOCKSCOPE:
			return "BLOCK SCOPE";
		case STRUCTSCOPE:
			return "STRUCT SCOPE";

	}
}

struct scope *
newSymbolTable(struct scope * currentScope, int scopeType)
{
	// Make sure you are allowed to create this cope here. 
	if(currentScope->scopeType == GLOBALSCOPE && scopeType !=FUNCTIONSCOPE) { 
		yyerror("YOU ARE ONLY ALLOWED TO CREATE A FUNCTIONSCOPE IN GLOBALSCOPE");
		return NULL;
	} 

	//create a new scope
	struct scope * newScope = malloc(sizeof(struct scope));
	//link them
	newScope->previous = currentScope;
	newScope->scopeType = scopeType;
	newScope->done = 0;
	currentScope->next = newScope;
	//return new current scope
	return newScope;
}

struct scope *  
destroySymbolTable(struct scope *s)
{
	// need to find the next scope that isnt function,
	// unless we are still within a function...
	struct scope * returningScope  = s->previous;
	while(returningScope->scopeType == FUNCTIONSCOPE && returningScope->done==1){
		returningScope = returningScope->previous;
	}

	struct symbol * sym = s->last;
	while (sym){
		
		struct symbol * temp = sym->previous;
		free(sym);
		sym = temp;
	}
	// only free if it is not a function scope
	if (s->scopeType != FUNCTIONSCOPE) {
		free(s);
	}
	return returningScope;
}

struct symbol * 
findSymbol(struct scope *lookingScope, char * name, int nameSpace)
{
	// look in current scope
	struct symbol * currSymbol = lookingScope->last;
	while (currSymbol){
		if (currSymbol->nameSpace==nameSpace && strcmp(name, currSymbol->name)==0){
			return currSymbol;
		}
		currSymbol = currSymbol->previous;
	}
	// if not found  go to next scope
	struct scope * nextScope = lookingScope->previous;
	if (nextScope){
		return findSymbol(nextScope,name,nameSpace);
	}
	return NULL;
	
	// if there are no scopes just return null

}

void
enterNewVariable(struct scope *enteringScope, int nameSpace, struct superSpec * super)
{
	// need to iterate over each of the initialized variables
	// and create a symbol for each of them...

	/*
	 	We do the following error checking in this function:
			1. we check to see if this variable name is used in this name space
			2. check if the type qualifier combination is allowed.
	 */

	// intialize val, to hold the int value of the new type/spec
	int val;
	//specs include all the decl specs
	struct astnode * specs = super->s;
	// i includes all the variables being initialized (ie compound intialization)
	struct init * i = super->i;
	// currentType gives you the add ons when you initialize a specefic variable, so 
	// for example if you have int *p, q[10] it would have the information that p is
	// a pointer and q is an array of 10

	struct initializedTypes * currentType = super->initialType;
	// right here we will do a quick check for incomplete types
	if (super->generalType && (super->generalType->u.spec.val == INCOMPLETETYPE || super->generalType->u.spec.val == INCOMPLETEUNION ) && (!currentType->t || currentType->t->u.spec.val != PNTRTYPE)){
		yyerror("This type (%s) is incomplete and can not be used as a forward reference unless it is a pointer.",super->generalType->u.spec.incompleteName);
		goto FINISHED;
	}

	// here we are going to check if this is a struct defined as a component,
	// if this is the case we pop its scope out one.
	// but we need to keep it in old scope too.
	struct scope * superScope = NULL;
	if (super->generalType->u.spec.val == STRUCTTYPE && enteringScope->scopeType == STRUCTSCOPE){
		superScope = enteringScope->previous;
	}
	while (i) {
		// look up variable name in scope right here
		struct symbol * sameName = findSymbol(enteringScope, i->value, nameSpace);
		if (sameName && sameName->definedScope == enteringScope){
			//RAISE ERROR
			yyerror("YOU ALREADY USED THIS VARIABLE NAME IN THIS SCOPE: %s", i->value);
			goto FINISHED;
		}
		struct symbol * newSymbol = malloc(sizeof(struct symbol));
		struct astnode * typeNode = malloc(sizeof(struct astnode));
		struct symbol * superSymbol = malloc(sizeof(struct symbol));
		newSymbol->name = strdup(i->value);
		newSymbol->type = typeNode;
		newSymbol->previous = enteringScope->last;
		if (superScope){
			superSymbol->previous = superScope->last;
			superSymbol->type =typeNode;
			superSymbol->nameSpace = nameSpace;
			superSymbol->definedScope = enteringScope;
			superSymbol->type = currentType->t;
			superSymbol->name = strdup(i->value);
		}
		newSymbol->nameSpace = nameSpace;
		newSymbol->definedScope = enteringScope;
		typeNode->u.spec.sign = 1;
		typeNode->nodetype = TYPE;
		if (super->generalType){
			typeNode->u.spec.val = super->generalType->u.spec.val;
		} 
		
		while(specs) {
			val = specs->u.spec.val;
			if (val == AUTO || val == EXTERN || val == REGISTER || val == STATIC){
				if (enteringScope->scopeType == GLOBALSCOPE && val == AUTO){
					yyerror("YOU CAN NOT USE AUTO IN THE GLOBAL SCOPE");
				}
				typeNode->u.spec.storageClass = val;
			}else if (val == CONST || val == VOLATILE || val == RESTRICT){
				typeNode->u.spec.type_qualifier = val;
			}else if (val == UNSIGNED) {
				typeNode->u.spec.sign = 0;
			}
			if ((val == UNSIGNED || val == SIGNED || val == CONST) && super->generalType == NULL) {
				// here we have to assign the type here to int.
			       	typeNode->u.spec.val = INTEGER;	
			}
			specs = specs->u.spec.next;
			
		}
		if (typeNode->u.spec.storageClass == 0){
			if (enteringScope->scopeType == GLOBALSCOPE){
				typeNode->u.spec.storageClass = EXTERN;
			}else{
				typeNode->u.spec.storageClass = AUTO;
			}
		}
	        if(typeNode->u.spec.val == 0){
			yyerror("You did not pass in a type for %s", i->value);
		       	goto FINISHED;	
		}	

		// now got to resolve the type here.
		// in other words we have the general type, but
		// we now need to put in if it is a pointer or array
		
		if (currentType->t){
			struct astnode * newType = currentType->t;
			while(newType){
				if(newType->u.spec.next == NULL){
					newType->u.spec.next = typeNode;	
					break;
				}
				newType = newType->u.spec.next;	
			}
			newSymbol->type = currentType->t;
				
		} 
		currentType = currentType->next;
		enteringScope->last = newSymbol;
		if (superScope){
			superScope->last = superSymbol;
		}
		specs = super->s; 
		i = i->next;
	}
   FINISHED:;
}

void 
printVariable(struct scope *enteringScope, int line, char * filenm)
{
	// this function will print the most recent thing o nthe 
	struct symbol * last = enteringScope->last;
	if (last) {
		struct astnode * a = last->type;
		printf("YO! FILENAME:%s line: %d in Scope:%s \n\t NameSpace:%d, StorageClass:%s,  type_qualifier:%s, sign:%d,\n\t name:%s size:%d\n \t type:%s",filenm, line, scope2name(last->definedScope->scopeType), last->nameSpace, type2name(a->u.spec.storageClass), type2name(a->u.spec.type_qualifier), a->u.spec.sign, last->name, a->u.spec.size, type2name(a->u.spec.val));
		while(a->u.spec.next){
			a = a->u.spec.next;
			printf(" -> %s", type2name(a->u.spec.val));
		}
		printf("\n");
	} 
}

void 
printSymbol(struct symbol * last)
{
	if (last && last->name) {
		struct astnode * a = last->type;
		printf("Member in Scope:%s \n\t NameSpace:%d, StorageClass:%s,  type_qualifier:%s, sign:%d,\n\t name:%s size:%d\n \t type:%s", scope2name(last->definedScope->scopeType),last->nameSpace, type2name(a->u.spec.storageClass), type2name(a->u.spec.type_qualifier), a->u.spec.sign, last->name, a->u.spec.size, type2name(a->u.spec.val));
		while(a->u.spec.next){
			a = a->u.spec.next;
			printf(" -> %s", type2name(a->u.spec.val));
		}
		printf("\n");
	} 
}
void 
printStructMembers(struct scope * structScope)
{
	printf("With the following members:\n");
	printf("_______________________________________________________\n");
	struct symbol * structSymbol = structScope->last;
	while(structSymbol){
		printSymbol(structSymbol);
		structSymbol = structSymbol->previous;
	}
	printf("_______________________________________________________\n");
}

struct  astnode *
addInitType(int nodetype, int newVal, struct astnode * decl_specs)
{
	// creat new astnode
	struct astnode * newAst = malloc(sizeof(struct astnode));

	// initiate its nodeType
	newAst->nodetype = nodetype;	
	
	// initiate its type value
	newAst->u.spec.val = newVal;
	
	// add the decleration specs nad make sure it is legal	
	int val;		
	while(decl_specs) {
		val  = decl_specs->u.spec.val;
		// this is technically wrong because it can have all three of these
		// but we dont really need to worry for them in our compiler so I will 
		// keep it like this for now and will fix if i have early time
		if (val == CONST || val == VOLATILE || val == RESTRICT){
			newAst->u.spec.type_qualifier = val;
		}
		decl_specs = decl_specs->u.spec.next;
		
	} 	
	return newAst;
}

struct astnode *
createStruct(struct scope * currentScope, struct symbol * structSymbol, struct scope * field_list, int isUnion)
{
	struct symbol * ident = findSymbol(currentScope, structSymbol->name, STRUCTSPACE);
	if (ident== NULL){			
		// need to enter into the symbol table over here.
		// field list must return a stack of symbols? (or mayb
		// a paramter list) 
		struct astnode * structType = malloc(sizeof(struct astnode));
		structType->nodetype = TYPE;
		if (isUnion){
			structType->u.spec.val = UNIONTYPE ;
		}else{
			structType->u.spec.val = STRUCTTYPE ;
		}
		structType->u.spec.params = field_list->last;	
		structSymbol->type = structType;
		structSymbol->previous = currentScope->last;
		structSymbol->definedScope = currentScope;
		currentScope->last = structSymbol;
		printVariable(currentScope,line, file_name);
		printStructMembers(field_list);
		return structType;
	} else {
		yyerror("This (%s) has already been defined, you are not allowed to redefine it.", structSymbol->name);
	}

}

struct astnode *
addCondStatement(int conditionType,struct astnode * exp, struct astnode * statement, struct astnode * elseStatement) 
{
	
	
	struct astnode * ifNode = malloc(sizeof(struct astnode));
	ifNode->nodetype = conditionType;
	ifNode->u.ifNode.exp = exp;
	ifNode->u.ifNode.statement = statement;
	ifNode->u.ifNode.elseStatement = elseStatement;
	return ifNode; 
	
}

struct astnode * 
createCompoundAst(int type, struct astnode * decl_or_stmt)
{
	struct astnode * compound  = malloc(sizeof(struct astnode));
	compound->nodetype = type;
	compound->u.compound.statements = decl_or_stmt;
	return compound;
}

void
notInGlobalScope() 
{ 
	if(currentScope -> scopeType == GLOBALSCOPE) { 
		yyerror("STATEMENT IS NOT ALLOWED TO BE IN GLOBAL SCOPE");
	} 	

} 	

struct astnode * 
addForNode(struct astnode * initial_clause, struct astnode * expression1, struct astnode * expression2, struct astnode * statement)
{
		struct astnode * forNode = malloc(sizeof(struct astnode));
		forNode->nodetype = FORNODE;
		forNode->u.forNode.initial_clause = initial_clause;
		forNode->u.forNode.expression1 = expression1;
		forNode->u.forNode.expression2 = expression2;
		forNode->u.forNode.statement = statement;
		return forNode;
}

struct astnode * 
addSimpleStmt(int type, struct astnode * exp)
{

	struct astnode * simple = malloc(sizeof(struct astnode));
	simple->nodetype = SIMPLESTMT;
	simple->u.simple.type = type;
	simple->u.simple.exp = exp;
	return simple;
	
}

struct astnode * 
addBreakStmt()
{
	struct scope * enteringScope = currentScope; 	
	while (enteringScope && enteringScope->scopeType != SWITCHSCOPE)  {
			enteringScope = enteringScope->previous;
	} 
	// need to verify there is some loop or switch statement.
	if (withinLoop == 0 && !enteringScope) {
		yyerror("BREAK STATEMENT MUST BE WITHIN A LOOP OR SWITCH STATEMENT");
		return NULL;
	}	
	struct astnode * simple = malloc(sizeof(struct astnode));
	simple->nodetype = SIMPLESTMT;
	simple->u.simple.type = BREAK;
	simple->u.simple.exp = NULL;
	return simple;
	
}

struct astnode * 
addContStmt()
{
	if (withinLoop == 0 ) {
		yyerror("CONTINUE STATEMENT MUST BE WITHIN A LOOP");
		return NULL;
	}	
	struct astnode * simple = malloc(sizeof(struct astnode));
	simple->nodetype = SIMPLESTMT;
	simple->u.simple.type = CONTINUE;
	simple->u.simple.exp = NULL;
	return simple;


}


struct astnode * 
addCaseStmt(int type, int val)
{
	if(currentScope->scopeType != SWITCHSCOPE){
		yyerror("CASE OR DEFAULT CAN ONLY BE USED IN SWITCH STATEMENTS %d", currentScope->scopeType);
		return NULL;
	}
	// need to make sure it is not in the symbol table
	// and if it is we need to raise an error
	char integerHolder[40]= "default";
	if ( type == CASE) {
		 sprintf(integerHolder, "%d", val);	
	} 

	char * name = integerHolder; 
	
	
	struct symbol * sameName = findSymbol(currentScope, name, SWITCHSPACE);
	if (sameName != NULL) {
		yyerror("DUPLICATE CASE OR DEFAULT STATEMENT %s", name);
		return NULL;
	}
	// now need to add it to the symbol table	
	struct symbol * labelSymb = malloc(sizeof(struct symbol));
	labelSymb->nameSpace = SWITCHSPACE;
	labelSymb->name = strdup(name);
	labelSymb->previous = currentScope->last;
	labelSymb->definedScope  = currentScope;
	currentScope->last = labelSymb;
	struct astnode * simple = malloc(sizeof(struct astnode));
	simple->nodetype = SIMPLESTMT;
	simple->u.simple.type = type;
	if (type == CASE) {
		NUMS v;
		v.i = val;
		simple->u.simple.exp = newNum(CONST_INT_OP,0,v) ;
	} 
	return simple;


}

struct astnode * 
addLabelStatement( struct astnode * label, struct astnode * statement) 
{
	struct scope * enteringScope = currentScope;
	struct astnode * labelNode = malloc(sizeof(struct astnode));
	char * name = NULL;
	struct symbol * sameName = NULL;
	// only proceed if we are in a simple named label.
	if (label->nodetype == NAMEDLABEL){
		while (enteringScope && enteringScope->scopeType != FUNCTIONSCOPE)  {
			enteringScope = enteringScope->previous;
		} 
		
		if( !enteringScope ||enteringScope -> scopeType != FUNCTIONSCOPE || enteringScope -> done ==1) {
			yyerror("LABEL CAN ONLY BE IN A FUNCTION SCOPE");
			return NULL;
		}
		name = label->u.ident.name; 
		// PUT IT INTO THE SYMBOL TABLE
		struct symbol * labelSymb = malloc(sizeof(struct symbol));
		labelSymb->nameSpace = LABELSSPACE;
		labelSymb->type = labelNode;
		labelSymb->name = name;
		labelSymb->previous = enteringScope->last;
		labelSymb->definedScope  = enteringScope;
		enteringScope->last = labelSymb;
		sameName = findSymbol(currentScope, name, LABELSSPACE);
	}
	if (sameName){
		//RAISE ERROR
		yyerror("DUPLICATE LABEL DEFINITION: %s", name);
		return NULL;
	}else{ 
		labelNode -> nodetype = LABELNODE;
		labelNode -> u.label.label = label;
		labelNode -> u.label.statement = statement; 
	}
	return labelNode;
}


char * 
resolveTarget(struct astnode * target, char * buf) 
{
	// need to verify if thing can be a left value.
	// at this point just chech if it is a tmp
	// if it is not raise and error, later we can have more
	// complete type checking.
	
	if (target ->nodetype == TMP) {
		sprintf(buf, "%%T%d", target->u.tmp.num);
		return buf;
	}else if(target->nodetype == SIMPLE_BNOP){
		return resolveTarget(target->q->target, buf);
	}else if(target->nodetype==SYMBOL){ 
		sprintf(buf, "%s",target->u.symbol->name );
		return buf;
	}else if(target->nodetype == CONST_CHAR_OP){
		sprintf(buf,"%s", target->u.ident.name);
		return buf;
	}else if(target->nodetype == CONST_STR_OP){
		sprintf(buf, "%s", target->u.ident.name);
	        return buf;	
	}else{
		yyerror("TARGET IS NOT A PROPER LVALUE");
		return NULL;
	}	
}

char *
resolveQ(struct astnode * node, char * buf, int op)
{
       	if(op != MOV && op !=LEA && op != LOAD && op!='<' && op!='>'  && op!=BREQ && node->q && node->q->target->nodetype) {
		return resolveTarget(node->q->target, buf);
	}else if(node->nodetype == CONST_INT_OP) {
		sprintf(buf, "%d", node->u.num.val.i);
		return buf;
	}else if (node->nodetype == SYMBOL) { 
		sprintf(buf, "%s", node->u.symbol->name);
		return buf;
	}else if (node->nodetype == BASIC){
		sprintf(buf, "BB%d", node->u.b->num);
		return buf;
	}else{
		return resolveTarget(node, buf);
	}  
}
struct quad *  
printQuad(struct quad * j, int i)
{
	// need to reverse the order and start from the bottom.
	// we do not need to re-traverse the tree, everything we need 
	// is at the head node, just in reverse order. 
	struct quad * q = j;
	struct quad * prev = NULL;
	struct quad * next = NULL;
	char buf[10];
	char buf1[10];
	char buf2[10];
	while( i && q && q->prevQuad && q->prevQuad->opcode!=-1){
		next = q->prevQuad;
		q->prevQuad = prev;
		prev  = q;
		if (next == NULL) {
			break;
		} 	
		q = next;
	}
	if(i && q){
		q->prevQuad = prev;
	}
	j = q;
	// now that the Quads are the right side up we will just print it
	while(q) {
		switch(q->opcode) 
			{

				case '*':
					printf("%s = 	MUL %s, %s\n",resolveTarget(q->target, buf), resolveQ(q->left, buf1,q->opcode), resolveQ(q->right,buf2,q->opcode));	
				break;
				case '/':
					printf("%s = 	DIV %s, %s\n",resolveTarget(q->target, buf), resolveQ(q->left, buf1,q->opcode), resolveQ(q->right,buf2,q->opcode));	
				break;
				case '%':
					printf("%s = 	MOD %s, %s\n",resolveTarget(q->target, buf), resolveQ(q->left, buf1,q->opcode), resolveQ(q->right,buf2,q->opcode));	
				break;
				case '+':
					printf("%s = 	ADD %s, %s\n",resolveTarget(q->target, buf), resolveQ(q->left, buf1,q->opcode), resolveQ(q->right,buf2,q->opcode));	
				break;
				case '-':
					printf("%s = 	SUB %s, %s\n", resolveTarget(q->target,buf), resolveQ(q->left,buf1,q->opcode), resolveQ(q->right,buf2,q->opcode)); 
				break;
				case SHL:
					printf("%s = 	SHL %s, %s\n", resolveTarget(q->target,buf), resolveQ(q->left,buf1,q->opcode), resolveQ(q->right,buf2,q->opcode)); 
				break;
				case SHR:
					printf("%s = 	SHR %s, %s\n", resolveTarget(q->target,buf), resolveQ(q->left,buf1,q->opcode), resolveQ(q->right,buf2,q->opcode)); 
				break;
				case '&':
					printf("%s = 	AND %s, %s\n", resolveTarget(q->target,buf), resolveQ(q->left,buf1,q->opcode), resolveQ(q->right,buf2,q->opcode)); 
				break;
				case '^':
					printf("%s = 	XOR %s, %s\n", resolveTarget(q->target,buf), resolveQ(q->left,buf1,q->opcode), resolveQ(q->right,buf2,q->opcode)); 
				break;
				case '|':
					printf("%s = 	OR %s, %s\n", resolveTarget(q->target,buf), resolveQ(q->left,buf1,q->opcode), resolveQ(q->right,buf2,q->opcode)); 
				break;
				case '=':
					printf("%s =	MOV %s\n", resolveTarget(q->left,buf), resolveQ(q->left, buf1,q->opcode));
				break;
				case MOV:
					printf("%s =	MOV %s\n", resolveTarget(q->target, buf), resolveQ(q->left,buf1, MOV));
				break;
				case LOAD:
					printf("%s =	LOAD %s\n", resolveTarget(q->target, buf), resolveQ(q->left, buf1,q->opcode));
				break;	
				case STORE:
					printf("	STORE %s %s\n", resolveTarget(q->right,buf), resolveQ(q->left, buf1, q->opcode));
				break;
				case LEA:
					printf("%s =	LEA %s\n", resolveTarget(q->target, buf), resolveQ(q->left, buf1,q->opcode));
				break;
				case CMP:
					printf("	CMP %s, %s\n", resolveQ(q->left,buf,q->opcode), resolveQ(q->right, buf1, q->opcode));
				break;
				case '<':
					printf("	BRLT %s, %s\n", resolveQ(q->left,buf,q->opcode), resolveQ(q->right, buf1, q->opcode));
				break;
				case '>':
					printf("	BRGT %s, %s\n", resolveQ(q->left,buf,q->opcode), resolveQ(q->right, buf1, q->opcode));
				break;
				case BR:
					printf("	BR %s\n", resolveQ(q->left,buf,q->opcode));
				break;
				case BREQ:
					printf(" 	BREQ %s %s\n", resolveQ(q->left,buf,q->opcode), resolveQ(q->right, buf1, q->opcode));
				break;
				case BRNEQ:
					printf(" 	BRNEQ %s %s\n", resolveQ(q->left,buf,q->opcode), resolveQ(q->right, buf1, q->opcode));
				break;
				case PLUSPLUS:
					printf(" INCR %s \n", resolveTarget(q->left,buf));
				break;
				case ARG:
					printf(" ARG %s  %s\n", resolveQ(q->left, buf, 	q->opcode), resolveQ(q->right,buf1, q->opcode));
				break;
				case CALL:
					printf("%s =  CALL  %s, %s \n", resolveTarget(q->target,buf), resolveTarget(q->left, buf1), resolveQ(q->right, buf2, q->opcode));
				break;
				case RET:
			       		if (q->left)
						printf("RETURN %s\n", resolveQ(q->left,buf, q->opcode));
					else	
						printf("RETURN\n");
				break;
				case CONQUAD:
					printf("THIS should not be here\n");
				break;
			}
		q =q->prevQuad;
	} 
	return j;
}


struct astnode * 
generateTarget() 
{
	struct astnode * target = NULL; 
	target  = malloc(sizeof(struct astnode));
	target->nodetype = TMP;
	target->u.tmp.num = tmpCounter;
	tmpCounter +=1;
	return target;
}

struct quad * 
linkQuads(struct quad  * l, struct quad * r) 
{
	// puts |_r_| on the bottom
	struct quad * tmpR;
	if (l && l->opcode !=-1){ 
		// get to the end of l tack on r
		tmpR = l;
		while(tmpR && tmpR->opcode >-1 && tmpR->prevQuad && tmpR->prevQuad->opcode >-1) { 
			tmpR = tmpR->prevQuad;
		}
		tmpR->prevQuad = r;
		return l;
	} else { 
		return r;
	}
}

int 
getTmpSize(struct astnode * left)
{
	if(left->nodetype == TMP){
		return left->u.tmp.size;
	}else if (left->nodetype == SYMBOL){
		return size(left->u.symbol->type, 1);
	}else if (left->nodetype == CONST_INT_OP){
		return 4;
	}else {
		printf("no size");
		return 0;
	}
}

struct astnode *
resolveLvalues(struct quad * q, struct astnode * a)
{
	struct quad * j  = malloc(sizeof(struct quad));
	if(a->nodetype == SYMBOL || a->nodetype == CONST_INT_OP || a->nodetype ==CONST_CHAR_OP) {
		q->opcode = -1;
		return a;	
	}else if(a->nodetype == SIMPLE_BNOP) {
		// need to check here for pointer arithmetic,
		// I can do this by looking down either side and seeing what
		// the type is ...
		struct quad * k  = malloc(sizeof(struct quad));
		q = malloc(sizeof(struct quad));
		if (a->u.binop.op == '=')
			printf("WHY IS THIS HERE\n");
		q->opcode = a->u.binop.op;
		q->left = resolveLvalues(j , a->u.binop.l);
		q->right= resolveLvalues(k, a->u.binop.r);
		q->target= generateTarget();
		q->target->u.tmp.size = getTmpSize(q->left);
		struct quad * newQ  = pointerArithmetic(q,a);
		q->prevQuad = linkQuads(j,k);
		if (newQ) {
			newQ->prevQuad = q->prevQuad;
			q->prevQuad  = newQ;
		} 
		return q->target;
	} else if(a->nodetype == SIMPLE_UNOP) {
		q = malloc(sizeof(struct quad));
		if(a->u.unop.op == '*'){
			q->opcode = STORE;
			q->left = NULL;
			q->right = resolveRvalues(j, a->u.unop.c,1);
			q->prevQuad = j;
			a->q= q;
			a->nodetype = STORE;
			return a;
		}else {
			q->opcode = a->u.unop.op;
			q->left   = resolveRvalues(j,a->u.unop.c,1);
			q->target = generateTarget();
			q->target->u.tmp.size = getTmpSize(q->left);
			q->prevQuad = j;
			return q->target;
		} 
	     
	}
	a->nodetype = TMP;
	a->u.tmp.num = q->target->u.tmp.num;
	a->u.tmp.size = q->target->u.tmp.size;
       	a->q = q;
	return a;	

}


struct astnode * 
resolveRvalues(struct quad* q, struct astnode * a,int p) 
{
	struct quad * j  = malloc(sizeof(struct quad));
	if(a->nodetype == SYMBOL || a->nodetype == CONST_INT_OP || a->nodetype == CONST_CHAR_OP || a->nodetype == CONST_STR_OP) {
		if (p == 1 && a->nodetype == SYMBOL && getType(a) == ARRAYTYPE){
			q->opcode = LEA;
			q->left = a;
			q->target = generateTarget();
			q->target->u.tmp.size = 4;
			q->target->u.tmp.p = p;
			q->prevQuad = NULL;
			struct astnode * b = malloc(sizeof(struct astnode));
			b->nodetype = TMP;
			b->u.tmp.num = q->target->u.tmp.num;
			b->u.tmp.p = p;
			b->u.tmp.size = 4;
			b->q = q;	
			return b;
		}else{
			q->prevQuad = NULL;
			q->opcode = -1;
			return a;
		}
	} else if(a->nodetype == SIMPLE_BNOP && a->u.binop.op != LOGAND && a->u.binop.op !=LOGOR && a->u.binop.op != '<' && a->u.binop.op != '>' && a->u.binop.op != EQEQ && a->u.binop.op != LTEQ && a->u.binop.op !=GTEQ && a->u.binop.op !=NOTEQ) {
		// need to check here for pointer arithmetic,
		// I can do this by looking down either side and seeing what
		// the type is ...
		struct quad * k  = malloc(sizeof(struct quad));
		q->opcode = a->u.binop.op;
		q->left = resolveRvalues(j, a->u.binop.l,p);
		q->right = resolveRvalues(k, a->u.binop.r,p);
		q->target = generateTarget();	
		q->target->u.tmp.size = getTmpSize(q->left);
		struct quad * newQ  = pointerArithmetic(q,a);
		q->prevQuad = linkQuads(j,k);
		if (newQ) {
			newQ->prevQuad = q->prevQuad;
			q->prevQuad  = newQ;
		} 
	}else if (a->nodetype == SIMPLE_BNOP && (a->u.binop.op == '<' ||  a->u.binop.op == '>' || a->u.binop.op == EQEQ || a->u.binop.op == LTEQ || a->u.binop.op == GTEQ || a->u.binop.op ==NOTEQ)) {
		// Here we need to do the branch and all that cause we got a compare in here buddies.
		struct basicBlock * b1 = createNewBlock(currentBlock);
		struct basicBlock * b2 = createNewBlock(b1);
		struct astnode  * b1A = malloc(sizeof(struct astnode));
		struct astnode * b2A = malloc(sizeof(struct astnode));
		b1A->u.b = b1;
		b2A->u.b = b2;
		b1A->nodetype =  BASIC;
		b2A->nodetype = BASIC;
		currentBlock = resolveComp(a,j,p);
		if (a->u.binop.op == LOGOR){
			currentBlock->q->left = b2A;
			currentBlock->q->right->u.b = b1;
		}else if(a->u.binop.op == LOGAND){ 
			currentBlock->q->left->u.b = b2;
			currentBlock->q->right = b1A;

		}else{	
			currentBlock->q->left = b1A;
			currentBlock->q->right = b2A;
		}
		currentBlock->next = b1;
		b1->next = b2;
		b2->prev = b1;
		b1->prev = currentBlock;
		// need to populate b1 and b2...
		
		// b1 needs to have a quad for t1 and also
		// needs to have a BR to pass b2..
		struct astnode * t = generateTarget(); 
		struct quad * qb1 = malloc(sizeof(struct quad));
		struct quad * qb1r = malloc(sizeof(struct quad));
		struct quad * n = malloc(sizeof(struct quad));
		qb1->opcode = MOV;
		qb1->target = t;
		NUMS v;
		v.i = 1;
		qb1->prevQuad = NULL;
		qb1->left = newNum(CONST_INT_OP,0,v);	
		qb1->target->u.tmp.size = 4;
		qb1r->prevQuad = qb1;
		qb1r->opcode = BR;
		n->opcode = MOV;
		n->target = t;
		n->prevQuad = NULL;
		NUMS w;
		w.i = 0;
		n->left = newNum(CONST_INT_OP,0,w);	
		b2->q = n;
		// and each of them need to lead in to  currentBlock
		currentBlock = createNewBlock(b2);
		currentBlock->q = NULL;
		struct astnode * currentBlockA=malloc(sizeof(struct astnode));
		currentBlockA->u.b = currentBlock;
		currentBlockA->nodetype = BASIC;
		qb1r->left = currentBlockA;
		b1->q =  qb1r;
		b2->next = currentBlock;
		currentBlock->prev = b2;
		q->opcode = -1;
		q->target = t;
	}else if(a->nodetype == SIMPLE_UNOP) {
		int jon = p;
		int p = a->u.unop.op == '*' ? 1: 0;
		struct astnode * left = NULL;
		if (a->u.unop.op != SIZEOF)
			left = resolveRvalues(j,a->u.unop.c,p);
		// I think i did this here in order to get rid of a double load, but now  I dont get any load...
		if(p  && (!jon || a->u.unop.c->nodetype==SYMBOL || a->u.unop.c->q->target->u.tmp.p!=1)){
			q->opcode = LOAD;
		}else if (a->u.unop.op == '&') {
			q->opcode = ADDR;
		}else if (a->u.unop.op == SIZEOF){
			q->opcode = MOV;
			left = mySizeOf(a->u.unop.c);
		}else {
			q->opcode = a->u.unop.op;
		}	
		// why is this here? 
		if(p && jon &&a->u.unop.c->nodetype!=SYMBOL &&a->u.unop.c->q->target->nodetype == TMP && a->u.unop.c->q->target->u.tmp.p ==1){	
			memcpy(q,j, sizeof(struct quad));
		} else {
	       		q->left = left;
			q->target = generateTarget();
			q->target->u.tmp.size = getTmpSize(left);
			q->prevQuad = j;
		}
	} else if(a->nodetype == TERNARY_OP) {
		// TODO.
	}else if (a->nodetype == FNCALL_OP){
		// loop through all the arguments and load them in...
		struct listarg  * args= a->u.fn.r->start;
		int count = 0;
		struct quad * prev= NULL;
		struct quad * b, * j, *p;
		while (args){
			b = malloc(sizeof(struct quad));
			b->opcode = ARG;
			NUMS v;
			v.i = count;
			b->left = newNum(CONST_INT_OP,0,v);
			j = malloc(sizeof(struct quad));
			b->right = resolveRvalues(j, args->ast, 0);
			b->prevQuad = linkQuads(prev, j);
			count+=1;
			prev = b;
			args= args->next;
		}
		// do the call here.
	        q->opcode = CALL;
		q->target = generateTarget();
		NUMS v;
		v.i = count;
		q->right = newNum(CONST_INT_OP,0,v);
		struct quad *n = malloc(sizeof(struct quad));
		q->left = resolveRvalues(n,a->u.fn.l ,0);
		q->prevQuad = linkQuads(b,n);
		a->nodetype = TMP;
		a->u.tmp.num = q->target->u.tmp.num;
		a->u.tmp.p = q->target->u.tmp.p;
		a->u.tmp.size = q->target->u.tmp.size;
		a->q = q;
		return a;
			
	}
	a = q->target;
	a->q = q;
	return a;       
}



void 
buildQuads(struct astnode * topAst)
{
	struct quad * q = malloc(sizeof(struct quad));
	//check if it is an equal statement
	if (topAst->nodetype == SIMPLE_BNOP && topAst->u.binop.op == '='){
		struct quad * k = malloc(sizeof(struct quad));
		if (topAst->nodetype == SIMPLE_BNOP && topAst->u.binop.r->q && topAst->u.binop.r->q->opcode == STORE) {
			q->left = topAst->u.binop.l;
			topAst->q = linkQuads(q,k);
		} else {
			// make it a mov or a store.
			struct quad * topQ = malloc(sizeof(struct quad));
			struct astnode * a = resolveLvalues(q, topAst->u.binop.l);
			topQ->left = resolveRvalues(k, topAst->u.binop.r,0);
			if (a->nodetype == STORE){
				topQ->opcode = STORE;
				topQ->right = a->q->right;
				topQ->prevQuad = linkQuads(a->q->prevQuad,k);
			}else{
				topQ->target = a;
				topQ->opcode = MOV;
				topQ->prevQuad = linkQuads(q,k);
			}
			topAst->q = topQ;
		} 	
	}else if (topAst->nodetype == SIMPLE_BNOP) {
		struct quad * k = malloc(sizeof(struct quad));
		resolveRvalues(q, topAst->u.binop.r,0);
		resolveRvalues(k, topAst->u.binop.l,0);
		topAst->q = linkQuads(q,k);
	}else if(topAst->nodetype == SIMPLE_UNOP) {
		if (topAst->u.unop.op == SIZEOF){
			q->opcode = MOV;
			q->left = mySizeOf(topAst->u.unop.c);
			q->prevQuad = NULL;
		}else {
			struct quad * j = malloc(sizeof(struct quad));
			q->opcode = topAst->u.unop.op;
			q->left = resolveRvalues(j, topAst->u.unop.c,0);
			q->prevQuad = j;
		}	
		topAst->q = q;
	}else if (topAst->nodetype == TERNARY_OP) {
		// DO I NEED TO DO THIS? 
	}else if (topAst->nodetype == IFNODE){
		// need to tack on expression to current basic block.
		// then need to create a true and after expression block
		// if there is any for of else go to also create an else block

		// first evaluate expression
		currentBlock = resolveComp(topAst->u.ifNode.exp, q, 0);
		struct basicBlock * old = currentBlock;
		// we are currently in the true block...
		currentBlock= createNewBlock(currentBlock);
		struct astnode * tbA = malloc(sizeof(struct astnode));
		tbA->nodetype = BASIC;
		tbA->u.b = currentBlock;
		old->q->target = NULL;
		struct astnode * topAstIf = topAst->u.ifNode.statement;
		if (topAstIf->nodetype == COMPOUND){
			topAstIf= topAstIf->u.compound.statements;
			while(topAstIf) { 
				// this does not have a its relevant parameters
				buildQuads(topAstIf);	
				if (topAstIf->q) {
					currentBlock->q = linkQuads(topAstIf->q, currentBlock->q);
				}
				currentBlock->q = topAstIf->q;
				topAstIf = topAstIf->next;
			}
		}else{

			buildQuads(topAstIf);
			currentBlock->q = topAstIf->q;
		}
		// ADD A break (need to break to end of the if statment block) but make it incomplete.
		struct quad * brQ = malloc(sizeof(struct quad));
		brQ->opcode =	BR;
	        brQ->prevQuad = currentBlock->q;
		currentBlock->q = brQ;		
		struct basicBlock * fBlock = NULL;
		struct astnode * bbe = NULL;
	       	if (topAst->u.ifNode.elseStatement){
			struct astnode * topAstElse = topAst->u.ifNode.elseStatement; 
			struct quad * breQ = malloc(sizeof(struct quad));
			bbe = malloc(sizeof(struct astnode));
			bbe->nodetype = BASIC;
			breQ->opcode = BR;
			breQ->left  = bbe;
			currentBlock = createNewBlock(currentBlock);
			fBlock = currentBlock;
			if(topAstElse->nodetype == COMPOUND){
				topAstElse = topAstElse->u.compound.statements;
				while(topAstElse) { 
				// this does not have a its relevant parameters
					buildQuads(topAstElse);	
					if (topAstElse->q) {
						currentBlock->q = linkQuads(topAstElse->q, currentBlock->q);
					}
					currentBlock->q = topAstElse->q;
					topAstElse = topAstElse->next;
				}
				breQ->prevQuad = currentBlock->q;
				currentBlock->q = breQ;
				currentBlock = createNewBlock(currentBlock);
				
			}else{
				buildQuads(topAstElse);
				breQ->prevQuad = topAstElse->q;
				currentBlock->q = breQ;
				if (topAstElse->q !=NULL){
					currentBlock = createNewBlock(currentBlock);
				}

			}
		}else{
			currentBlock = createNewBlock(currentBlock);
		}
		if(bbe)
			bbe->u.b = currentBlock;
		struct astnode *bb = malloc(sizeof(struct astnode));
		bb->nodetype=BASIC;
		bb->u.b = currentBlock;
		brQ->opcode = BR;
		brQ->left = bb;
		struct astnode * fbA = malloc(sizeof(struct astnode));
		fbA->nodetype = BASIC;
		fbA->u.b = fBlock ? fBlock : currentBlock;
		if(topAst->u.ifNode.exp->u.binop.op== LOGOR){
			old->q->left = fbA;	
			old->q->right->u.b = tbA->u.b;
		}else if(topAst->u.ifNode.exp->u.binop.op == LOGAND) {
			old->q->left->u.b = fbA->u.b;
			old->q->right= tbA;
		}else{
			old->q->left = tbA;
			old->q->right = fbA;
		}
		topAst->q = NULL;
		//if (fBlock)
		//	fBlock->next  = currentBlock;	
	}else if (topAst->nodetype == FORNODE){
		// evaluate first part..
		struct astnode * initialClause  = topAst->u.forNode.initial_clause;
		buildQuads(initialClause);
		initialClause->q->prevQuad = currentBlock->q;
		currentBlock->q = initialClause->q;
		// now need to build break to the last compare;
		struct quad * l = malloc(sizeof(struct quad));
		l->opcode = BR;
		l->left = NULL;
		l->target = NULL;
		l->prevQuad = currentBlock->q;
		currentBlock->q = l;
		struct basicBlock * initial = currentBlock;
		currentBlock = createNewBlock(currentBlock);
		//evaluate body of the for loop
		struct basicBlock * bodyBlock = currentBlock;
		if(topAst->u.forNode.statement->nodetype == COMPOUND){
				struct astnode * topAstFor = topAst->u.forNode.statement->u.compound.statements;
				struct quad * q;
				while(topAstFor) {
				       	buildQuads(topAstFor);
					// NOT ALL THINGS IN BUILDQUADS EDIT THE INPUT!!!! FIX THIS JESUS CHRIST! 
					q= topAstFor->q;
					if(q){
						while(q->prevQuad && q->prevQuad->opcode !=-1){
							q = q->prevQuad;
						}
						q->prevQuad = currentBlock->q;	
						currentBlock->q = topAstFor->q;
					}
					topAstFor = topAstFor->next;
				}
		}else{
			buildQuads(topAst->u.forNode.statement);
			currentBlock->q = topAst->u.forNode.statement->q;
		}
		// now evaluate expression 2, this is also the continue point
		currentBlock = createNewBlock(currentBlock);
		struct basicBlock * contBlock  = currentBlock;
		struct astnode * expression2 = topAst->u.forNode.expression2;
		buildQuads(expression2);
		currentBlock->q = expression2->q;
		// now need to do expression 1, the compare	
		currentBlock = createNewBlock(currentBlock);
		//create an astnode for the BB
		struct astnode * tbA = malloc(sizeof(struct astnode));
		tbA->nodetype = BASIC;
		tbA->u.b = bodyBlock;
		initial->q->left = tbA;	
		resolveComp(topAst->u.forNode.expression1,q,0);
		// need to edit the break...
		struct astnode * tbB=  malloc(sizeof(struct astnode));
		tbB->nodetype = BASIC;
		tbB->u.b= bodyBlock;
		currentBlock->q->left = tbB;
		struct basicBlock * cmpBlock = currentBlock;
		// now back to other normal basic blocks
		currentBlock = createNewBlock(currentBlock);
		struct astnode * tbC = malloc(sizeof(struct astnode));
		tbC->nodetype = BASIC;
		tbC->u.b = currentBlock;
		cmpBlock->q->right = tbC;
		if(topAst->u.forNode.expression1->nodetype == LOGOR){
			cmpBlock->q->left = tbC;
			cmpBlock->q->right->u.b = tbA->u.b;
		}else if(topAst->u.forNode.expression1->nodetype == LOGAND){
			cmpBlock->q->left->u.b = currentBlock;
			cmpBlock->q->right = tbA;

		}	
		addBreakContinue(initial, contBlock);
		topAst->q = currentBlock->q;
	}else if (topAst->nodetype == FNCALL_OP){
		// loop through all the arguments and load them in...
		struct listarg  * args= topAst->u.fn.r->start;
		int count = 0;
		struct quad * prev= NULL;
		struct quad * q, * j, *p;
		while (args){
			q = malloc(sizeof(struct quad));
			q->opcode = ARG;
			NUMS v;
			v.i = count;
			q->left = newNum(CONST_INT_OP,0,v);
			j = malloc(sizeof(struct quad));
			q->right = resolveRvalues(j, args->ast, 0);
			q->prevQuad = linkQuads(prev, j);
			count+=1;
			prev = q;
			args= args->next;
		}
		// do the call here.
		struct quad * l = malloc(sizeof(struct quad));
	        l->opcode = CALL;
		l->target = generateTarget(); 
		NUMS v;
		v.i = count;
		l->right = newNum(CONST_INT_OP,0,v);
		struct quad *n = malloc(sizeof(struct quad));
		l->left = resolveRvalues(n,topAst->u.fn.l ,0);
		l->prevQuad = linkQuads(q,n);
		currentBlock->q = l;
		topAst->q = l;
	}else if (topAst->nodetype == SIMPLESTMT){
		// check if it is a break or continue...
		// because we are only using a for loop we 
		// can have some other simplifications here.
		int simpleType = topAst->u.simple.type;
		if (simpleType == CONTINUE){
			q->opcode  = CONQUAD;
		}else if (simpleType == BREAK){
			q->opcode = BREAKQUAD;
		}else if (simpleType == RETURN){
			struct quad * j  = malloc(sizeof(struct quad));
			q->opcode = RET;
			q->left = resolveRvalues(j, topAst->u.simple.exp,0);
			q->prevQuad = j;
		}
		topAst->q = q;
	} 
}
 

struct basicBlock * 
resolveComp(struct astnode * a, struct quad * j, int p)
{
		// we want to move the intersection of 
		// functionality of the compare and the resolve
		// Expression into one function here..
		// resolve left resolve right
		if(a->u.binop.op == LOGAND){
			// compare with zero...
			struct quad * l = malloc(sizeof(struct quad));
			l->opcode = CMP;
			l->left = resolveRvalues(j, a->u.binop.l,p);
			NUMS v;
			v.i = 0;
			l->right= newNum(CONST_INT_OP,0,v);
			l->target = NULL;
			l->prevQuad = j;
			// BREQ..
			struct quad * m = malloc(sizeof(struct quad));
			m->opcode = BREQ;
			m->prevQuad = l;
			currentBlock->q  = m;
			currentBlock = createNewBlock(currentBlock);
			struct astnode * dright = malloc(sizeof(struct astnode));
			dright->nodetype = BASIC;
			dright->u.b = currentBlock;
			m->right = dright;
			struct quad * k = malloc(sizeof(struct quad));
			struct quad * n = malloc(sizeof(struct quad));
			struct astnode * dleft = malloc(sizeof(struct quad));
			struct quad * o = malloc(sizeof(struct quad));
			dleft->nodetype = BASIC;
			k->opcode = CMP;
			k->left = resolveRvalues(n, a->u.binop.r,p);
			k->right =  newNum(CONST_INT_OP,0,v);
			k->target = NULL;
			k->prevQuad = n;
			m->left = dleft;
			o->opcode = BREQ;
			o->prevQuad = k;
			o->left = dleft;
			currentBlock->q = o;
		}else if(a->u.binop.op == LOGOR){
			// compare with zero...
			struct quad * l = malloc(sizeof(struct quad));
			l->opcode = CMP;
			l->left = resolveRvalues(j, a->u.binop.l,p);
			NUMS v;
			v.i = 0;
			l->right= newNum(CONST_INT_OP,0,v);
			l->target = NULL;
			l->prevQuad = j;
			// BREQ..
			struct quad * m = malloc(sizeof(struct quad));
			m->opcode = BREQ;
			m->prevQuad = l;
			currentBlock->q  = m;
			currentBlock = createNewBlock(currentBlock);
			struct astnode *dleft = malloc(sizeof(struct astnode));
			dleft->nodetype = BASIC;
			dleft->u.b = currentBlock;
			m->left = dleft;
			struct quad * k = malloc(sizeof(struct quad));
			struct quad * n = malloc(sizeof(struct quad));
			struct astnode * dright = malloc(sizeof(struct quad));
			struct quad * o = malloc(sizeof(struct quad));
			dright->nodetype = BASIC;
			k->opcode = CMP;
			k->left = resolveRvalues(n, a->u.binop.r,p);
			k->right =  newNum(CONST_INT_OP,0,v);
			k->target = NULL;
			k->prevQuad = n;
			m->right = dright;
			o->opcode = BREQ;
			o->prevQuad = k;
			o->right = dright;
			currentBlock->q = o;
		
		}else{
			struct quad * k = malloc(sizeof(struct quad));
			struct quad * l = malloc(sizeof(struct quad));
			l->opcode = CMP;
			l->left = resolveRvalues(j, a->u.binop.l,p);
			l->right = resolveRvalues(k,a->u.binop.r,p);	
			l->target = NULL;
			l->prevQuad = linkQuads(k, j);
			// break
			// we have to break dependent on what the nodetype is 
			int ty = a->u.binop.op;
			if(ty == EQEQ){
				ty = BREQ;
			}else if(ty == NOTEQ){
				ty = BRNEQ;
			}
			struct quad * m = malloc(sizeof(struct quad));
			m->opcode = ty;
			m->prevQuad = l;
			m->left = NULL;
			m->right = NULL;
			// MAY NEED TO CHANGE THIS BACK>>>
			currentBlock->q = linkQuads(m, currentBlock->q);
		}
		return currentBlock;
}



void 
emitQuads(struct astnode * compoundAst) 
{
	// emit Quads expects a compound ast to be passed in.
	struct astnode * topAst = compoundAst->u.compound.statements;
	while(topAst) { 
		// now with each compound ast we got to print
		// out the corresponding quad
		buildQuads(topAst);	
		// not sure what is going on here....
		if (topAst->q && topAst->nodetype != FNCALL_OP) {
			currentBlock->q = linkQuads(topAst->q, currentBlock->q);
		}
		topAst = topAst->next;
	}
	addReturn();
	// now that we have all the blocks set up, lets reverse the order and then 
	// re traverse and print out all the relevant info...	
	struct basicBlock * topBlock = currentBlock;
	struct basicBlock * temp = topBlock;
	while(topBlock->prev){
		temp = topBlock;
		topBlock= topBlock->prev;
		topBlock->next = temp;
	}
	while(topBlock){
		printf("BB%d: \n", topBlock->num, topBlock->q);
		if (topBlock->q)
			topBlock->q = printQuad(topBlock->q, 1);
		topBlock = topBlock->next;
	}
}


int
getType(struct astnode * symbol)
{	
	// loop through all of the pointers...
	if (!symbol->u.symbol)
		return -1;
	struct astnode * types = symbol->u.symbol->type;
	while(types->u.spec.next && types->u.spec.val == PNTRTYPE){
		types = types->u.spec.next;

	}
	return types->u.spec.val;
}
int
getTypeSym(struct symbol * symbol)
{	
	// same function as above but made for direct symbol
	struct astnode * types = symbol->type;
	while(types->u.spec.next && types->u.spec.val == PNTRTYPE){
		types = types->u.spec.next;

	}
	return types->u.spec.val;
}

int 
isPointer(struct astnode *a) 
{
	if (a->nodetype == TMP && a->u.tmp.p == 1)
		return 1;
	// just got to peak for the first one
	if (a->nodetype != SYMBOL)
		return 0;
	struct astnode * types = a->u.symbol->type;
	if (types->u.spec.val == PNTRTYPE || types->u.spec.val == ARRAYTYPE)
		return 1;
	return 0;
}

int typeSwitch(struct astnode * type)
{
	// printf("size %d", type->u.spec.val);
	switch(type->u.spec.val){
		default: 
			return 4;
		case SHORT:
		       	return 2;
		case  INTEGER:
		      	return 4;
		case  LONG:
		      	return 8;
		case LONGLONG:
		     	return 8;
		case CHR:
		    	return 1;
		case FLT:
	 	     	return 4;
		case DBLE:
		     	return 8;
		case PNTRTYPE:
			return 4;
		case ARRAYTYPE:
			return type->u.spec.size * size(type, 0);	
	
	}
}

int 
size(struct astnode * types, int j)
{
	// set j to zero unless you want to automatically set type to types
	if(!types)
		return 1;
	struct astnode * type = types->u.spec.next;
	if(!type || j){
		type = types;
		if (!type)
			return 1;
	}
	return typeSwitch(type);
}

struct astnode * 
mySizeOf(struct astnode * a)
{
	struct astnode * s = malloc(sizeof(struct astnode));
	if(a->nodetype == SYMBOL){
		// just do normal size;
		NUMS v;
		v.i = size(a->u.symbol->type, 0);
		s = newNum(CONST_INT_OP,0,v);
	}else if (a->nodetype == TYPE) {
		NUMS v;
		v.i = typeSwitch(a);
		s = newNum(CONST_INT_OP,0,v);	
	}else{
		yyerror("Invalid input to sizeof");
	} 
	return s;
}
struct quad * 
adjustedArithmetic(struct astnode *a, int sizeType)
{
	struct quad * q = malloc(sizeof(struct quad));
	// have to figure out why/when we need to divide
	NUMS v;
	v.i = sizeType;	
	struct astnode * size =newNum(CONST_INT_OP,0,v);
	// mul
	q->opcode = '*';	
	q->left = a;
	q->right = size;
	q->prevQuad = NULL;
	q->target = generateTarget();
	q->target->u.tmp.size = getTmpSize(a);
	return q;
}

int midSize(struct astnode *a)
{
	if (a->nodetype == TMP)
		return a->u.tmp.size;
	else if (a->nodetype == SYMBOL)
		return size(a->u.symbol->type->u.spec.next, 0);
	return 1;	
}

struct quad * 
pointerArithmetic(struct quad * q, struct astnode * a) 
{
	
	// check if it is addition
	if (a->u.binop.op != '+' && a->u.binop.op != '-'){
		// do nothing
		return NULL;
	}else if (isPointer(a->u.binop.l) && !isPointer(a->u.binop.r)) { 		
		 // check if left operand is a pointer and right is not
		 int sizeA; 
		 if (a->u.binop.l->nodetype == SYMBOL)
		 	sizeA = size(a->u.binop.l->u.symbol->type, 0);
		 else 
			 sizeA = a->u.binop.l->u.tmp.size;
		 struct quad * adjust = adjustedArithmetic(a->u.binop.r,sizeA);
		 q->right = adjust->target;
		 q->target->u.tmp.p = 1;
		 q->target->u.tmp.size = midSize(a->u.binop.l);
		 return adjust;
	}else if(isPointer(a->u.binop.r) && !isPointer(a->u.binop.l)) {		
		// check if right operand is a pointer and left is not
		int sizeA;
		if(a->u.binop.r->nodetype == SYMBOL)
			sizeA = size(a->u.binop.r->u.symbol->type, 0);
		else
			sizeA = a->u.binop.r->u.tmp.size;

		struct quad * adjust = adjustedArithmetic(a->u.binop.l,sizeA);
		q->left = adjust->target;
		q->target->u.tmp.p = 1;
		q->target->u.tmp.size =midSize(a->u.binop.r);
		return adjust;
	}else if (isPointer(a->u.binop.r) && isPointer(a->u.binop.l)){
		// pointer pointer arithmetic
		if(a->u.binop.op == '+'){
			yyerror("YOU ARE NOT ALLOWED TO ADD TWO POINTERS, ONLY SUBTRACT");
			return NULL;
		}else{
			// now need to do the division by size of pointer.
			struct quad * adjust = malloc(sizeof(struct quad));
			memcpy(adjust, q, sizeof(struct quad));
			q->opcode ='/';
			q->target = generateTarget();
			q->target->u.tmp.p = 0;
			q->left = adjust->target;
			q->prevQuad = adjust;
			NUMS v;
			v.i = size(a->u.binop.l ,0);
			q->right = newNum(CONST_INT_OP,0,v);
			return adjust;
		}
	}else{
		// normal addition
		return NULL;
	}

}


struct basicBlock *  
createNewBlock(struct basicBlock * currentBlock)
{
	struct basicBlock * b = malloc(sizeof(struct basicBlock));
	if (currentBlock) {
		if(currentBlock->q == NULL){
			free(b);
			return currentBlock;
		}else{
			currentBlock->next = b;
			b->num = currentBlock->num + 1;
			b->prev = currentBlock;	
		}
	}else{
		b->num = 1;
	}
	b->q = NULL;
	return b;
}

void 
addReturn()
{
	if (currentBlock->q->opcode != RET){
		struct quad * q = malloc(sizeof(struct quad));
		q->opcode = RET;
		q->left = NULL;
		q->prevQuad = currentBlock->q;
		currentBlock->q = q;
	}
}


void
addBreakContinue(struct basicBlock * i , struct basicBlock * c)
{
	// loop basicBlocks until you get to c
	struct quad *q;
	while(i !=c){
		// loop through each quad.
		q = i->q;
		while(q){
			if(q->opcode == CONQUAD || q->opcode == BREAKQUAD){
				struct astnode * bb = malloc(sizeof(struct astnode));
				bb->nodetype = BASIC;
				bb->u.b = q->opcode == CONQUAD ? c : currentBlock;
				q->opcode = BR;
				q->left = bb;
			}
			q = q->prevQuad;
		}
		i = i->next;
	}
}

void emitTargetCode()
{
	// first thing is to go through the symbol table and get all the global variables...
		// I need to figure out how to do this only once for a file...
	FILE *target_file = fopen("target.s", "w");
	fprintf(target_file,".file \"%s\"\n", file_name);
	buildPreamble(target_file);	
	// now I need to convert quads into targetcode
	convertQuads(target_file);
	fclose(target_file);
}
void 
buildPreamble(FILE * f)
{
	char * fnDecls[100][100];
	char * strings[100][100];
	int loc = 0;
	int locS = 0;
	int lco = 0;
	struct scope * globalScope = currentScope;
	//verify that it is a global scope...
	if (globalScope->scopeType == GLOBALSCOPE){
		// iterate through all the symbols in the global scope. 
		struct symbol * syms = globalScope->last;
		while (syms){
			if (syms->type && syms->type->u.spec.val == FNTYPE){
				sprintf(fnDecls[loc], ".globl %s\n",syms->name );
				loc++;
				sprintf(fnDecls[loc], ".type %s, @function\n", syms->name);
				loc++;
			}else if (syms->type && getTypeSym(syms)!=CHR){
				// technically should havec a different align function, but for the types im using it is the same as size
				fprintf(f,".comm  %s, %d, %d\n", syms->name, size(syms->type, 1), size(syms->type,1));	
			}else if (syms->type && getTypeSym(syms)==CHR){
				// First need to make sure that this was assigned something, if it was not just skip it.
				// Need to save it to strings
				char buff[1000];
				int resp = getStringValue(syms,buff,lco);	
				if (resp) { 
					sprintf(strings[locS++],".LC%d:\n", lco);
					sprintf(strings[locS++],"\t .string \"%s\"\n", buff);
					sprintf(strings[locS++],"\t .data\n");
					sprintf(strings[locS++],"\t .align 4\n");
					sprintf(strings[locS++],"\t .type %s,@object\n", syms->name);
					sprintf(strings[locS++],"\t .size %s, %d\n",syms->name,strlen(buff));
					sprintf(strings[locS++],"%s:\n\t.long .LC%d\n", syms->name, locS);
					locS++;
					lco++
				}
				
			}	
			syms = syms->previous;
		}
		// get the rest of the strings here..
		buildLocalStrings(strings,lco,locS);
		// iterate through the fndecls: 
		fputs(".rodata\n", f);
		for (int i =0; i<=locS; i++){
			fprintf(f,strings[i]);
		}
       		fputs(".text\n", f);
		for(int i = 0; i<=loc; i++){
			fprintf(f,fnDecls[i]);	
		}	
	}else{
		yyerror("Dont support nested functions buddy");	
	}		
}

void 
convertQuads(FILE * f)
{
	// add the name of fn
	fprintf(f, "%s:\n",currentScope->last->name);
	// have to add the pushl/movl
	fprintf(f,"\tpushl %%ebp\n");
	fprintf(f,"\tmovl %%esp, %%ebp\n");	
	// get the subl and set the offset for the corresponding elements
	
	// now go through the quads and do the actual conversion...
	struct basicBlock * topBlock = currentBlock;
	struct basicBlock * temp = topBlock;
	while(topBlock->prev){
		temp = topBlock;
		topBlock= topBlock->prev;
		topBlock->next = temp;
	}
	int offset = setOffset(currentScope, topBlock);
	if (offset>0)
		fprintf(f, "\tsubl $%d, %%ebp\n",offset);
	while(topBlock){
		if (topBlock->q){
			fprintf(f,"BB%d:\n",topBlock->num);
			conversion(topBlock->q, f);	
		}
		topBlock = topBlock->next;
	}
}

void
conversion(struct quad * q, FILE *f)
{
	// TODO: Fix up this function after changing everything to offsets...
	char buff[100];
	char buff1[100];
	char buff2[100];
	char buff3[100];
	char t[2];
	while(q) {
		switch(q->opcode) 
			{
				case '*':
					basicArithmetic(f, q);	
				break;
				case '/':
					// mov the dividend ( q->left) to %eax
					resolveType(q->left,t);
					resolveSource(q->left, buff1, f);
					fprintf(f, "\tmov%s %s, %%eax\n", t, buff1);
					// mov the divisor  ( q->right) into %ecx
					resolveSource(q->right, buff2,f);
					fprintf(f, "\tmov%s %s, %%ecx\n", t, buff2);
					// do div on %ecx
					fprintf(f, "\tdiv %%ecx\n");
					// move the result (%eax) into the target location
					resolveDest(q->target, buff1);
					fprintf(f, "\tmov%s %%eax, %s\n",t,buff1);
				break;
				case '%':
					// mov the dividend ( q->left) to %eax
					resolveType(q->left,t);
					resolveSource(q->left, buff1, f);
					fprintf(f, "\tmov%s %s, %%eax\n", t, buff1);
					// mov the divisor  ( q->right) into %ecx
					resolveSource(q->right, buff2,f);
					fprintf(f, "\tmov%s %s, %%ecx\n", t, buff2);
					// do div on %ecx
					fprintf(f, "\tdiv %%ecx\n");
					// move the result (%edx) into the target location
					resolveDest(q->target, buff1);
					fprintf(f, "\tmov%s %%edx, %s\n",t,buff1);
				break;
				case '+':
					basicArithmetic(f,q);
				break;
				case '-':
					basicArithmetic(f,q);
				break;
				case SHL:
					basicArithmetic(f,q);
				break;
				case SHR:
					basicArithmetic(f,q);
				break;
				case '&':
					basicArithmetic(f,q);
				break;
				case '^':
					basicArithmetic(f,q);
				break;
				case '|':
					basicArithmetic(f,q);
				break;
				case LOAD:
					// Both load and store and just modified movl commands.
					resolveSource(q->left,buff1,f);
					resolveDest(q->target,buff2);
					resolveType(q->left,t);
					fprintf(f,"\tmov%s (%s), %%edx\n", t, buff1);
					fprintf(f,"\tmov%s %%edx, %s\n", t, buff2);
				break;	
				case STORE:
					resolveSource(q->left,buff1,f);
					resolveSource(q->right,buff2,f);
					resolveType(q->left,t);
					fprintf(f,"\tmov%s %s, %%edx\n", t, buff1);
					fprintf(f,"\tmov%s %%edx, (%s)\n", t,buff2);
				break;
				case ADDR:
					resolveSource(q->left, buff1,f);
					resolveDest(q->target,buff2);
					resolveType(q->left,t);
					fprintf(f,"\tlea%s %s, %s\n", t,buff1,buff2);
				break;
				case LEA:
					resolveSource(q->left, buff1,f);
					resolveDest(q->target,buff2);
					resolveType(q->left,t);
					fprintf(f, "\tlea%s %s, %s\n",t, buff1,buff2);
				break;
				case CMP:
					resolveSource(q->left,buff1,f);
					resolveDest(q->target,buff2);
					fprintf(f,"\tcmp %s, %s\n",buff1, buff2);
				break;
				case '<':
					//BRLT
					resolveQ(q->left, buff1, q->opcode);
					resolveQ(q->right, buff2, q->opcode);
					fprintf(f,"\tjl %s\n", buff1);
					fprintf(f,"\tjmp %s\n", buff2);
				break;
				case '>':
					//BRGT
					resolveQ(q->left, buff1, q->opcode);
					resolveQ(q->right, buff2, q->opcode);
					fprintf(f,"\tjg %s\n", buff1);
					fprintf(f,"\tjmp %s\n", buff2);
				break;
				case BR:
					resolveQ(q->left, buff1, q->opcode);
					fprintf(f,"\tjmp %s\n", buff1);
				break;
				case BREQ:
					resolveQ(q->left, buff1, q->opcode);
					resolveQ(q->right, buff2, q->opcode);
					fprintf(f,"\tje %s\n", buff1);
					fprintf(f,"\tjmp %s\n", buff2);
				break;
				case BRNEQ:
					resolveQ(q->left, buff1, q->opcode);
					resolveQ(q->right, buff2, q->opcode);
					fprintf(f, "\tjne %s\n", buff1);
					fprintf(f, "\tjmp %s\n",buff2);
				break;
				case PLUSPLUS:
					resolveDest(q->left,buff1);
					fprintf(f,"\tinc %s\n", buff1);
				break;
				case MINUSMINUS:
					resolveDest(q->left,buff1);
					fprintf(f,"\tdec %s\n", buff1);
				break;
				case ARG:
					resolveType(q->right,t);
					resolveSource(q->right,buff1,f);
					fprintf(f,"\tpush%s %s\n", t, buff1);
				break;
				case CALL:
					resolveTarget(q->left,buff1);
					fprintf(f,"\tcall %s\n",buff1);	
					// have to add back to the esp here
					fprintf(f,"\taddl $%d, %%esp\n",q->right->u.num.val.i*4);
				break;
				case '=':
					resolveSource(q->left,buff1,f);
					resolveDest(q->target,buff2);
					resolveType(q->left,t);
					fprintf(f,"\tmov%s %s,%s\n",t,buff1,buff2);					
				break;
				case MOV:
					resolveSource(q->left,buff1,f);
					resolveDest(q->target,buff2);
					resolveType(q->left,t);
					fprintf(f,"\tmov%s %s,%s\n",t,buff1,buff2);
				break;	
				case RET:
					if (q->left) {
						resolveType(q->left, buff);
 						resolveSource(q->left, buff1, f); 
						fprintf(f,"\tmov%s %s, %%eax\n",buff, buff1);
					}
					fprintf(f,"\tret\n");		
					fprintf(f,"\tleave\n");
					fprintf(f,"\t.size %s,.-%s\n", currentScope->last->name, currentScope->last->name);
				break;
				case CONQUAD:
					printf("THIS should not be here\n");
				break;


			}
		q = q->prevQuad;
	}
}

void 
resolveType(struct astnode * a, char * buff)
{
	int s = 0;
	if (a->nodetype == TMP)
		s = a->u.tmp.size;
	else if (a->nodetype == SYMBOL)
		s = size(a->u.symbol->type, 1);
	switch(s){
		default:
			sprintf(buff, "l");
		break;
		case 1:
			sprintf(buff, "b");
		break;
		case 2:
			sprintf(buff, "w");
		break;
		case 4:
			sprintf(buff, "l");
		break;
		case 8:
			sprintf(buff, "q");
		break;
	}
	
}

void
resolveLocal(struct astnode *a, char * buff)
{
	int offset = 0;
	if (a->nodetype == TMP)
		offset = a->u.tmp.offset;
	else if (a->nodetype == SYMBOL)
		offset = a->u.symbol->offset;
	sprintf(buff, "-%d(%%esp)", offset);
}

void
resolveSource(struct astnode * a, char * buff, FILE *f)
{
	// In general we have the following possibilities: 
	// 		1. CONST (Only source)
	// 		2. Global Var
	// 		3. Local Var (Can be TMP or SYMBOL)
	//		4. ARRAY 
	if (a->nodetype == CONST_INT_OP){
		sprintf(buff, "$%d", a->u.num.val.i);
	}else if(a->nodetype == CONST_CHAR_OP){
		sprintf(buff,"$%d", a->u.ident.name);
	}else if(a->nodetype == SYMBOL && a->u.symbol->definedScope->scopeType == GLOBALSCOPE) {
		//GLOBAL VAR
		char o[2];
		sprintf(buff, "%s", a->u.symbol->name);
		resolveType(a,o);
		fprintf(f, "\tmov%s %s, %%edx\n",o,buff);
		sprintf(buff, "%%edx");
	}else if (a->nodetype == SYMBOL || a->nodetype == TMP){
		//LOCAL VAR
		// If we are a source and in a two operation op code 
		// we may face the issue of too many mem references
		// so preemtively we will just move it into edx
		resolveLocal(a,buff);
		char o[2];
		resolveType(a,o);
		fprintf(f, "\tmov%s %s, %%edx\n",o,buff);
		sprintf(buff, "%%edx");
	}
}

void
resolveDest(struct astnode * a, char * buff)
{
	if(a->nodetype == SYMBOL && a->u.symbol->definedScope->scopeType == GLOBALSCOPE) {
		//GLOBAL VAR
		sprintf(buff, "%s", a->u.symbol->name);
	}else if (a->nodetype == SYMBOL || a->nodetype == TMP){
		//LOCAL VAR
		resolveLocal(a,buff);
	}


}

void
basicArithmetic(FILE *f, struct quad * q)
{
	char buff1[100];
	char buff2[100];
	char o[10];
	char t[2];
	char j[2];
	resolveOp(q->opcode, o);
	resolveType(q->target,j);
	resolveType(q->left, t);
	resolveSource(q->left, buff1, f);
	resolveDest(q->target, buff2);
	// Two steps:	1. Mov into the correct place
	// 		2. Do the actual arithmetic operation
	fprintf(f, "\tmov%s %s, %s\n",j, buff1, buff2);
	resolveSource(q->right, buff1, f);
	fprintf(f,  "\t%s%s %s, %s\n",o,t,buff1,buff2);
}

void 
resolveOp(int opcode, char * op)
{
	switch(opcode){
		default:
			printf("default\n");

		break;
		case '*': 
			sprintf(op, "mul");
		break;
		case '+':
			sprintf(op, "add");
		break;
		case '-':
			sprintf(op, "sub");
		break;
		case SHL:
			sprintf(op, "shl");	
		break;
		case SHR:
			sprintf(op,"shr");
		break;
		case '&':
			sprintf(op, "and");
		break;
		case '^':
			sprintf(op, "xor");
		break;
		case '|':
			sprintf(op, "or");
		break;
		
	}
}

int 
setOffset(struct scope * s, struct basicBlock * top)
{
	// go through the local variables first
		// where is this symbol table? 
	while(s->scopeType!=FUNCTIONSCOPE){ 
		s = s->next;
	} 	
	struct symbol * syms = s->last;
	int offset = 0;
	while(syms){
		if(syms->type->u.spec.storageClass!= STATIC){
			offset += size(syms->type ,1);
			//printf("var:%s symbol:%p offset %d\n",syms->name,syms, offset);
			syms->offset = offset;
		}		
		syms = syms->previous;
	} 	
	// then go through all the quads and add for the tmp variables..	
	while(top){
		if (top->q){
			struct quad * q= top->q;
			while(q) { 
				if (q->target && q->target->nodetype == TMP){
					offset+=q->target->u.tmp.size;
					q->target->u.tmp.offset = offset;
				
				}
				q = q->prevQuad;
			}
		}
		top = top->next;
	}	
	//printf("offset %d\n", offset);
	return offset;

}
int
getStringValue(struct symbol * symb, char * buff, int lnum)
{
	int resp = 0;
	struct basicBlock * top = currentBlock;
	struct basicBlock * temp = top;
	while(top->prev){
		temp = top;
		top= top->prev;
		top->next = temp;
	}
	while(top){
		if (top->q){
			struct quad * q= top->q;
			while(q) { 
				if (q->target && q->target->nodetype == SYMBOL && q->target->u.symbol == symb){
					sprintf(buff, q->left->u.ident.name);
					q->left->u.ident.num = lnum;
					resp = 1;
					goto end;
				}
				q = q->prevQuad;
			}
		}
		top = top->next;
	}
	end:
		return resp;
}

void
buildLocalStrings(char * strings,int lco, int locS)
{
	struct basicBlock * top = currentBlock;
	struct basicBlock * temp = top;
	while(top->prev){
		temp = top;
		top= top->prev;
		top->next = temp;
	}
	while(top){
		if (top->q){
			struct quad * q= top->q;
			while(q) { 
				if (q->left && q->left->nodetype == CONST_STR_OP){
					printf("found local const str_op)
				}
				q = q->prevQuad;
			}
		}
		top = top->next;
	}
}

