# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>
# include "parser.h"
# include "cparser.tab.h"
# include "types.h"
	
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
	int val;
	struct astnode * specs = super->s;
	struct init * i = super->i;
	struct initializedTypes * currentType = super->initialType;
	while (i) {
		// look up variable name in scope right here
		struct symbol * sameName = findSymbol(enteringScope, i->value, nameSpace);
		if (sameName && sameName->definedScope == enteringScope){
			//RAISE ERROR
			yyerror("CONFLICTING SCOPES WITH VARIABLE: %s", i->value);
			goto FINISHED;
		}
		struct symbol * newSymbol = malloc(sizeof(struct symbol));
		newSymbol->name = strdup(i->value);
		printf("new init name: %s\n", i->value);
		newSymbol->previous = enteringScope->last;
		newSymbol->nameSpace = nameSpace;
		newSymbol->definedScope = enteringScope;
		newSymbol->sign = 1;
		while(specs) {
			val = specs->u.spec.val;
			//printf("SPEC %d",val);
			if (val == AUTO || val == EXTERN || val == REGISTER || val == STATIC){
			 	printf("storage class");
				newSymbol->storageClass = val;
			}else if (val == CONST || val == VOLATILE || val == RESTRICT){
				newSymbol->type_qualifier = val;
			}else if (val == UNSIGNED) {
				newSymbol->sign = 0;
			}
			if ((val == UNSIGNED || val == SIGNED) && super->generalType == NULL) {
				// here we have to assign the type here to int.
				struct astnode * ty = malloc(sizeof(struct astnode));
				ty->nodetype = TYPE;
			       	ty->u.spec.val = INTEGER;	
				currentType->t = ty;
			}
			specs = specs->u.spec.next;
		}
		// now got to resolve the type here.
		// basically, we need to take the current 
		// pointer to type use it and then move to the 
		// next one 
		newSymbol->type = currentType->t;
	       	currentType = currentType->next;	
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
		printf("YO! FILENAME:%s line: %d \n\t NameSpace:%d, StorageClass:%d, type:%d, type_qualifier:%d, sign:%d,\n\t name:%s\n",filenm, line, last->nameSpace, last->storageClass, last->type->u.spec.val, last->type_qualifier, last->sign, last->name);
	} 
}
