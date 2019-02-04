
// header for parser
extern int yylineno;
void yyerror(char *s,...);

struct astnode_binop {
		int op; 
		struct astnode *l;
		struct astnode *r;
};

struct astnode_ident {
		char* name;
};


struct astnode_num {
		short numtype;
		int value;
};

struct astnode_string {
		short stringtype;
		char* value;

};



struct astnode {
	int nodetype;
	union astnodes {
		struct astnode_binop binop;
		struct astnode_ident ident;
		struct astnode_num num;
		struct astnode_string string;
	} u;
	
};

struct astnode *newBinop(int nodetype, int op, struct astnode *l, struct astnode *r);
struct astnode *newIdent(int nodetype, char *name);
struct astnode *newString(int nodetype, char *value);
struct astnode *newNum(int nodetype, short numtype, int value);
void printast(struct astnode *a,int level);






