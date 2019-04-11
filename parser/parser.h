
// header for parser
extern int yylineno;
extern char file_name[300];
extern int line;
extern struct scope * currentScope;
extern int withinLoop;
extern int tmpCounter;

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

struct astnode_if {
	struct astnode * exp;
	struct astnode * statement;
	struct astnode * elseStatement;
};

struct adnode_for {
	struct astnode * initial_clause;
	struct astnode * expression1;
	struct astnode * expression2;
	struct astnode * statement;
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

struct astnode_cmpnd {
		struct astnode * statements; 
};

struct astnode_simpleStmt {
		int type;
		struct astnode * exp;
};

struct astnode_for {
		struct astnode * initial_clause;
		struct astnode * expression1;
		struct astnode * expression2;
		struct astnode * statement;

};

struct astnode_label {
		struct astnode * label;
		struct astnode * statement;
};

struct astnode_spec {
		int val;
		int size;
		struct astnode * next; 
		int storageClass;
		int type_qualifier;
		int sign;
		char * incompleteName;
		// we create a new symbol table for 
		// parameters to the function...
		struct symbol * params;
		//only assign this for a function...
		struct scope * functionScope;
};

struct astnode_tmp {
		int num;
};

struct symbol{

	int nameSpace;
	struct  astnode * type;
	char* name;
	struct symbol * previous;
	struct scope * definedScope;

};

struct quad { 
	int opcode; 
	struct astnode * left; 
	struct astnode * right;
	struct astnode * target; 
	struct quad * prevQuad;

};

struct basicBlock { 
	struct quad * firstQuad;
	struct quad * lastQuad; 
	int num;
	struct basicBlock * nextBlock; 

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
		struct astnode_if ifNode;
		struct astnode_cmpnd  compound; 
		struct astnode_for forNode;
		struct astnode_simpleStmt simple;
		struct astnode_label label;
		struct astnode_tmp tmp;
		struct symbol * symbol;
	} u;
	struct quad * q; 
	struct astnode * next;	
};

struct scope{
	int scopeType;
	struct scope * previous;
	struct scope * next;
	struct symbol * last;
	int done;
};


struct superSpec{
	// doing this so that we can seperate
	// the type portion of speceifiers from
	// the other stuff...
	struct astnode *s;
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
	struct initializedTypes * types;
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
struct scope *newSymbolTable(struct scope * currentScope, int scopeType);
struct scope *destroySymbolTable(struct scope *s);
struct symbol *findSymbol(struct scope *lookingScope, char * name,int nameSpace);
void enterNewVariable(struct scope *enteringScope,int nameSpace, struct superSpec * super);
void printVariable(struct scope *enteringScope, int line, char * filenm);
struct astnode * addInitType(int nodeType, int newVal, struct astnode * next); 
struct astnode * addCondStatement(int conditionType,struct astnode * exp, struct astnode * statement, struct astnode * elseStatement);
struct astnode * createStruct(struct scope * currentScope, struct symbol * structSymbol, struct scope * field_list, int isUnion);
struct astnode * createCompoundAst(int type, struct astnode * decl_or_stmt);
struct astnode * addForNode(struct astnode * initial_clause, struct astnode * expression1, struct astnode * expression2, struct astnode * statement);
struct astnode * addSimpleStmt(int type, struct astnode * exp);
struct astnode * addLabelStatement(struct astnode * label, struct astnode * statement);
struct astnode * addCaseStmt(int type, int val);
struct astnode * addBreakStmt();
struct astnode * addContStmt();
void notInGlobalScope();
struct basicBlock * emitQuads(struct astnode * f);
void printQuad(struct astnode * head); 
struct quad * linkQuads(struct quad * l, struct quad *r);
struct quad * generateQuad(struct astnode * l, struct astnode * r, struct astnode * target, int op);
struct astnode * augSymbol(struct astnode * symbol, int op);
struct astnode * generateTarget(int nodetype);
struct quad * fixPointer(struct quad * bad, struct quad * tQ);
struct quad *  checkLnode(struct astnode * a, struct quad *tQ);
void fixQuads(struct astnode * a);
char * resolveQ(struct astnode * node, char * buf, int op);
void yyerror(const char *s,...);
