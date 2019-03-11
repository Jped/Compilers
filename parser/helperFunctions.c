# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>
# include "parser.h"
# include "cparser.tab.h"
# include "types.h"
# include "typeTypes.h"	
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
	} 
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
yyerror(char*s,...)
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
	currentScope->previous = NULL;
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


struct scope *
newSymbolTable(struct scope * currentScope)
{
	//create a new scope
	struct scope * newScope = malloc(sizeof(struct scope));
	//link them
	newScope->previous = currentScope;
	currentScope->next = newScope;
	//return new current scope
	return newScope;
}

void 
destroySymbolTable(struct scope *s)
{
	struct symbol * sym = s->last;
	while (sym){
		
		struct symbol * temp = sym->previous;
		free(sym);
		sym = temp;
	}
	free(s);
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
	} else{
		return NULL;
	}
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
	// right here we will do a quick check for incomplete types.
	if (super->generalType && (super->generalType->u.spec.val == INCOMPLETETYPE || super->generalType->u.spec.val == INCOMPLETEUNION ) && (!currentType->t || currentType->t->u.spec.val != PNTRTYPE)){
		yyerror("This type (%s) is incomplete and can not be used as a forward reference",super->generalType->u.spec.incompleteName);
		goto FINISHED;
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
		newSymbol->name = strdup(i->value);
		newSymbol->type = typeNode;
		newSymbol->previous = enteringScope->last;
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
				typeNode->u.spec.storageClass = val;
			}else if (val == CONST || val == VOLATILE || val == RESTRICT){
				typeNode->u.spec.type_qualifier = val;
			}else if (val == UNSIGNED) {
				typeNode->u.spec.sign = 0;
			}
			if ((val == UNSIGNED || val == SIGNED) && super->generalType == NULL) {
				// here we have to assign the type here to int.
			       	typeNode->u.spec.val = INTEGER;	
			}
			specs = specs->u.spec.next;
			
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
			currentType = currentType->next;
				
		} 
		enteringScope->last = newSymbol;
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
		printf("YO! FILENAME:%s line: %d \n\t NameSpace:%d, StorageClass:%d, type:%d, type_qualifier:%d, sign:%d,\n\t name:%s size:%d\n",filenm, line, last->nameSpace, a->u.spec.storageClass, a->u.spec.val, a->u.spec.type_qualifier, a->u.spec.sign, last->name, a->u.spec.size);
	} 
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
