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

struct quad * 
linkQuads(struct quad  * l, struct quad * r) 
{
	struct quad * tmpR;
	if (l){ 
		tmpR = l;
		while(tmpR->prevQuad) { 
			tmpR = tmpR->prevQuad;
		} 
		tmpR->prevQuad = r;
	} else { 
		l = r;
	}
	return l; 	
}

struct astnode *
augSymbol(struct astnode * symbolNode, int op)
{
	// this function serves as a general augmentation function
	// to a symbol. For example it can be used to Move a variable
	// or load a pointer to a variable.
	struct quad *  aug        = malloc(sizeof(struct quad));
	aug->opcode = op;
	aug->target = generateTarget(op);
	aug->right = symbolNode;
	aug->left = NULL;
	symbolNode->q = aug;
	return symbolNode;
}


struct quad * 
generateQuad(struct astnode * left, struct astnode * right, struct astnode * target, int op) 
{
	// got to check if left or right is a symbol...
	struct quad * q = malloc(sizeof(struct quad));
	q->opcode = op;
	switch(op) { 
		default: 
			q->left	  = left->nodetype == SYMBOL ? augSymbol(left, MOV) :left; 
		break;
		case '=': 
			// this is placed in order to get the ident to be a lvalue in an equal 
			// statement
			q->left = left;
		break;	
		
	} 
	q->right  = right->nodetype == SYMBOL? augSymbol(right, MOV) :right;
	q->prevQuad = linkQuads(left->q, right->q);
	q-> target= target;
	// got to link the two quads together.
	return q;
}
	
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
generateTarget(int nodetype) 
{
	struct astnode * target = NULL; 
	if (nodetype != '='){
		target  = malloc(sizeof(struct astnode));
		target->nodetype = TMP;
		target->u.tmp.num = tmpCounter;
		tmpCounter +=1;
	}
	return target;
}

struct astnode *
newBinop(int nodetype, int op, struct astnode *l, struct astnode *r)
{
	struct astnode *a = malloc(sizeof(struct astnode));
	a->nodetype= nodetype;
	a->u.binop.op = op;
	a->u.binop.l = l;
	a->u.binop.r = r;
	struct astnode * target = generateTarget(op);
	struct quad * q = generateQuad(l,r,target,op); 
	a->q = q;
	return a;
}

struct astnode * 
newUnop(int nodetype, int op, struct astnode *c)
{

	struct astnode *a = malloc(sizeof(struct astnode));
	a->nodetype = nodetype;
	a->u.unop.op = op;
	a->u.unop.c = c;
	struct quad * q = NULL;
	switch(op){
		case '*':
			if (c->nodetype == SYMBOL && c->u.symbol->type->u.spec.val == ARRAYTYPE){
				q = augSymbol(c, LEA)->q;
				struct quad * load = malloc(sizeof(struct quad));
				load->opcode = LOAD;
				load->right = q->target;
				load->target = generateTarget(LOAD);
				load->prevQuad = q;
				q = load;
			} else { 
				q = augSymbol(c, LOAD)->q;
			}
		break;
	}
	a->q = q;
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
	while(q) {
			next = q->prevQuad;
			q->prevQuad = prev;
			prev  = q;
			if (next == NULL) {
				break;
			} 	
			q = next;
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
					printf("%s =	MOV %s\n", resolveTarget(q->left,buf), resolveQ(q->right, buf1,q->opcode));
				break;
				case MOV:
					printf("%s =	MOV %s\n", resolveTarget(q->target, buf), resolveQ(q->right, buf1,q->opcode));
				break;
				case LOAD:
					printf("%s =	LOAD %s\n", resolveTarget(q->target, buf), resolveQ(q->right, buf1,q->opcode));
				break;	
				case STORE:
					printf("	STORE %s %s\n", resolveTarget(q->right,buf), resolveQ(q->left, buf1, q->opcode));
				break;
				case LEA:
					printf("%s =	LEA %s\n", resolveTarget(q->target, buf), resolveQ(q->right, buf1,q->opcode));
				break;
			}
		q =q->prevQuad;
	} 
}

struct quad * 
fixPointer(struct quad * bad, struct quad * tQ) 
{
	// basically need to move around things in bad and then make it point to 
	// next thing in tq.
	bad->opcode = STORE;
	bad->target = NULL;
	// will now search tQ until we find the current quad.
	struct quad * prev = NULL;
	struct quad * top = tQ;
	while(tQ) {
		if (tQ->opcode == '=' && tQ->right->nodetype == CONST_INT_OP){
				bad->left = tQ->right;
		}
		if (tQ == bad)
			break;
		// got to get rid of that extra move....
		else if (tQ->opcode == '=' && tQ->left->q == bad) {
			struct quad * delete = top;
			top = top->prevQuad;
			free(delete);
			// allow for constants too
			
		} else { 
			prev = tQ;  
		} 
		

		tQ = tQ->prevQuad;       
	}
	if (tQ->prevQuad && !bad->left) {
		bad->left = tQ->prevQuad->target; 
	}	
	return top;
}


struct quad *
checkLnode(struct astnode *a, struct quad * tQ) 
{
	if (a->nodetype == SIMPLE_BNOP){
		
		checkLnode(a->u.binop.l, tQ);
		checkLnode(a->u.binop.r, tQ);
	}else if (a->nodetype == SIMPLE_UNOP) { 
		if(a->u.unop.op == '*'){ 
			// fix up the * here
			// it it is simple just call fixPointer
			// however if it is some arithmetic need 
			// to call another function
			if (a->u.unop.c->nodetype == SIMPLE_BNOP){
				// Do some arithmetic...
				printf("doing some arithmetic");
			}else{
				tQ = fixPointer(a->q, tQ);
			}
		}
		checkLnode(a->u.unop.c, tQ);
	}else if (a->nodetype == TERNARY_OP){
		checkLnode(a->u.terop.l, tQ);
		checkLnode(a->u.terop.c, tQ);
		checkLnode(a->u.terop.r, tQ);
	
	} 
	return tQ;
}


void 
fixQuads(struct astnode * a) 
{ 
	// first thing I need to check that the head is an assignment
	if (a->nodetype !=0 || a->u.binop.op != '=') { 
		return;
	} 
	// now we must to a search of the left side of this tree and find all 
	// the relevant nodes that need to be fixed.
	

	struct quad * topQuads = a->q;

	a->q = checkLnode(a->u.binop.l, topQuads);

} 

struct basicBlock * 
emitQuads(struct astnode * compoundAst) 
{
	// emit Quads expects a compound ast to be passed in.
	struct astnode * topAst = compoundAst->u.compound.statements;

	// There are several operators that change their action based on whether they are 
	// lvalues or rvalues. I have chosen to just treat them as an lvalue and right this 
	// function to clean it up after the whole ast is build.
	
	fixQuads(topAst);	
		
	while(topAst) { 
		// now with each compound ast we got to print
		// out the corresponding quad
		printQuad(topAst); 	
		topAst = topAst->next;
	}	

}
