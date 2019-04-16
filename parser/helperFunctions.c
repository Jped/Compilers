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
		printf("\t");
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
	}else{
		yyerror("TARGET IS NOT A PROPER LVALUE");
		return NULL;
	}	
}

char *
resolveQ(struct astnode * node, char * buf, int op)
{
	// at this point there are two options
	// either a tmp or a contstant
	if(op != MOV && op !=LEA && op != LOAD && node->q && node->q->target->nodetype) {
		return resolveTarget(node->q->target, buf);
	}else if(node->nodetype == CONST_INT_OP) {
		sprintf(buf, "%d", node->u.num.val.i);
		return buf;
	}else if (node->nodetype == SYMBOL) { 
		sprintf(buf, "%s", node->u.symbol->name);
		return buf;
	}else{
		return resolveTarget(node, buf);
	}  
}
void 
printQuad(struct astnode * head)
{
	// need to reverse the order and start from the bottom.
	// we do not need to re-traverse the tree, everything we need 
	// is at the head node, just in reverse order. 
	
	
	struct quad * q = head->q;
	struct quad * prev = NULL;
	struct quad * next = NULL;
	char buf[10];
	char buf1[10];
	char buf2[10];
	while(q->prevQuad && q->prevQuad->opcode!=-1){
		next = q->prevQuad;
		q->prevQuad = prev;
		prev  = q;
		if (next == NULL) {
			break;
		} 	
		q = next;
	}
	if(q){
		q->prevQuad = prev;
	}
	
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
			}
		q =q->prevQuad;
	} 
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
	if (l && l->opcode!=-1){ 
		tmpR = l;
		while(tmpR && tmpR->opcode !=-1 && tmpR->prevQuad) { 
			tmpR = tmpR->prevQuad;
		} 
		tmpR->prevQuad = r;
	} else { 
		l = r;
	}
	return l; 	
}

struct astnode *
resolveLvalues(struct quad * q, struct astnode * a)
{
	struct quad * j  = malloc(sizeof(struct quad));
	if(a->nodetype == SYMBOL || a->nodetype == CONST_INT_OP) {
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
			q->prevQuad = j;
			return q->target;
		} 
	     
	}
	a->nodetype = TMP;
	a->u.tmp.num = q->target->u.tmp.num;
       	a->q = q;
	return a;	

}


struct astnode * 
resolveRvalues(struct quad* q, struct astnode * a,int p) 
{
	struct quad * j  = malloc(sizeof(struct quad));
	if(a->nodetype == SYMBOL || a->nodetype == CONST_INT_OP) {
	 	if (p == 1 && a->nodetype == SYMBOL && getType(a) == ARRAYTYPE){
			q->opcode = LEA;
			q->left = a;
			q->target = generateTarget();
			q->target->u.tmp.p = p;
			struct astnode * b = malloc(sizeof(struct astnode));
			b->nodetype = TMP;
			b->u.tmp.num = q->target->u.tmp.num;
			b->u.tmp.p = p;
			b->q = q;	
			return b;
		}else{
			q->opcode = -1;
			return a;
		}
	} else if(a->nodetype == SIMPLE_BNOP) {
		// need to check here for pointer arithmetic,
		// I can do this by looking down either side and seeing what
		// the type is ...
		struct quad * k  = malloc(sizeof(struct quad));
		q->opcode = a->u.binop.op;
		q->left = resolveRvalues(j, a->u.binop.l,p);
		q->right = resolveRvalues(k, a->u.binop.r,p);
		q->target = generateTarget();	
		struct quad * newQ  = pointerArithmetic(q,a);
		q->prevQuad = linkQuads(j,k);
		if (newQ) {
			newQ->prevQuad = q->prevQuad;
			q->prevQuad  = newQ;
		} 
	} else if(a->nodetype == SIMPLE_UNOP) {
		int p = a->u.unop.op == '*' ? 1: 0;
		struct astnode * left = resolveRvalues(j,a->u.unop.c,p);
		// got to also verify that we do not do double load on multidim array
		//TODO: figure out what to check the opcode, cause store does not have target
		if(p  && (a->u.unop.c->nodetype==SYMBOL || a->u.unop.c->q->target->u.tmp.p!=1)){
			q->opcode = LOAD;
		}else if (a->u.unop.op == '&') {
			q->opcode = ADDR;
		}else {
			q->opcode = a->u.unop.op;
		}
		
		if(p && a->u.unop.c->nodetype!=SYMBOL &&a->u.unop.c->q->target->nodetype == TMP && a->u.unop.c->q->target->u.tmp.p ==1){	
			memcpy(q,j, sizeof(struct quad));
		} else {
	       		q->left = left;
			q->target = generateTarget();
			q->prevQuad = j;
		}
	} else if(a->nodetype == TERNARY_OP) {
		// TODO.
	}
	a->nodetype = TMP;
	a->u.tmp.num = q->target->u.tmp.num;
	a->u.tmp.p = q->target->u.tmp.p;
       	a->u.tmp.size = q->target->u.tmp.size;
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
		resolveRvalues(q, topAst->u.unop.c,0);
		topAst->q = q;
	}else if(topAst->nodetype == TERNARY_OP) {
	} 
}
 



struct basicBlock * 
emitQuads(struct astnode * compoundAst) 
{
	// emit Quads expects a compound ast to be passed in.
	struct astnode * topAst = compoundAst->u.compound.statements;

	while(topAst) { 
		// now with each compound ast we got to print
		// out the corresponding quad
		buildQuads(topAst);	
		printQuad(topAst); 	
		topAst = topAst->next;
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

int 
size(struct astnode * types)
{
	if(!types)
		return 1;
	struct astnode * type = types->u.spec.next;
	if(!type){
		type = types;
		if (!type)
			return 1;
	}
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
			return 8;
		case ARRAYTYPE:
			return type->u.spec.size * size(type);	
	
	}
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
	return q;
}

int midSize(struct astnode *a)
{
	if (a->nodetype == TMP)
		return a->u.tmp.size;
	else if (a->nodetype == SYMBOL)
		return size(a->u.symbol->type->u.spec.next);
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
		 	sizeA = size(a->u.binop.l->u.symbol->type);
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
			sizeA = size(a->u.binop.r->u.symbol->type);
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
			v.i = size(a->u.binop.l);
			q->right = newNum(CONST_INT_OP,0,v);
			return adjust;
		}
	}else{
		// normal addition
		return NULL;
	}

}

