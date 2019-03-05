
// header for parser
extern int yylineno;
extern char file_name[300];
extern int line;

struct listarg {
	int size;
	int nodetype;
	struct listarg * previous;
	struct listarg * next;
	struct listarg * start;
	struct astnode * ast;
};

struct astnode_terop{
	int op;
	struct astnode *l;
	struct astnode *c;
	struct astnode *r;
};

struct astnode_fn{
	struct astnode *l;
	struct listarg *r;
};

struct astnode_binop {
		int op; 
		struct astnode *l;
		struct astnode *r;
};
struct astnode_unop {
		int op;
		struct astnode *c;
};


struct astnode_ident {
		char* name;
};

typedef union{
	int i;
	double d;
	float f;
} NUMS;

struct astnode_num {
		short type;
		NUMS val;
};

struct astnode_string {
		short stringtype;
		char* value;

};

struct astnode_spec {
		int val;
		struct astnode * next; 
};


struct astnode {
	int nodetype;
	union astnodes {
		struct astnode_fn fn;
		struct astnode_binop binop;
		struct astnode_terop terop;
		struct astnode_unop unop;
		struct astnode_ident ident;
		struct astnode_num num;
		struct astnode_string string;
		struct astnode_spec spec;
	} u;
	
};




struct scope{
	struct scope * previous;
	struct scope * next;
	struct symbol * last;
};

struct symbol{
	int nameSpace;
	int storageClass;
	struct  astnode * type;
	int type_qualifier;
	int sign;
	char* name;
	struct astnode_fn * parameters;
	struct symbol * previous;
	struct scope * definedScope;


};

struct superSpec{
	// doing this so that we can seperate
	// the type portion of speceifiers from
	// the other stuff...
	struct astnode * s;
	struct initializedTypes * initialType;
	struct init * i;
	struct astnode * generalType;
};


struct init{
	char * value;
	struct init * next;
};

struct initializedTypes{
	struct astnode *t;
	struct initializedTypes * next; 
};

struct smallSpec{
	char * value;
	struct initializedTypes * next;
};

struct astnode *newBinop(int nodetype, int op, struct astnode *l, struct astnode *r);
struct astnode *newTerop(int nodetype, int op, struct astnode *l,struct astnode *c, struct astnode *r);
struct astnode *newUnop(int nodetype, int  op, struct astnode *c);
struct astnode *newIdent(int nodetype, char *name);
struct astnode *newString(int nodetype, char *value);
struct astnode *newNum(int nodetype,short type, NUMS value);
struct astnode *newFN(int nodetype, struct astnode *l, struct listarg * r);
struct listarg *newArg(int nodetype, struct listarg * current, struct astnode *next);
struct listarg *newList(int nodetype, struct astnode *a);
void printast(struct astnode *a,int level);
void printarg(struct listarg *l, int level);
char * token2name(int token);
struct scope *newSymbolTable(struct scope * currentScope);
void destroySymbolTable(struct scope *s);
struct symbol *findSymbol(struct scope *lookingScope, char * name,int nameSpace);
void enterNewVariable(struct scope *enteringScope,int nameSpace, struct superSpec * super);
void printVariable(struct scope *enteringScope, int line, char * filenm);


