%option noyywrap

%{
#include <string.h>
#define OP 257
#define FILEN 258
#define IDENT 259
#define KEYWD 260
#define CHAR  261
#define STRING 262
#define ARROW 263
#define INT 265
#define FLOAT 266
#define DOUBLE 267


char string[4096];
int pos = 0;
typedef union {
	int integer;
	char value[9];
	char *string_literal;
	float f;
	double d;
} YYSTYPE;

YYSTYPE  yylval;
char file_name[300];
int line=1;
%}

DIGIT [0-9]
OCT 0[0-7]*
HEX 0[Xx][a-fA-F0-9]*
%x STRESC
%x CHRESC
%x SIGN
%X ENDCHR
%%

!	|
%	|
\^	|
&	|
\*	|
\-	|
\+	|
= 	|
\~	|
\|	|
\.	|
\< 	|
\>	|
\?	|
\(	|
\)	|
\[	|
\]	|
\{	|
\}	|
,	|
;	|
\/	|
:	{strcpy(yylval.value,yytext);return OP;}
\.\.\.	{strcpy(yylval.value,"ELLIPSIS");return OP;}
\+=	{strcpy(yylval.value,"PLUSEQ");return OP;}
\-=	{strcpy(yylval.value,"MINUSEQ");return OP;}
\*=	{strcpy(yylval.value,"TIMESEQ");return OP;}
\/= {strcpy(yylval.value,"DIVEQ");return OP;}
%=	{strcpy(yylval.value,"MODEQ");return OP;}
\<\<=   {strcpy(yylval.value,"SHLEQ");return OP;}
\>\>=   {strcpy(yylval.value,"SHREQ");return OP;}
&=	{strcpy(yylval.value,"ANDEQ");return OP;}
\^=     {strcpy(yylval.value,"XOREQ");return OP;}
\|=     {strcpy(yylval.value,"OREQ");return OP;}
\-\>	{return ARROW;}
\+\+    {return PLUSPLUS;}
\-\-    {strcpy(yylval.value,"MINUSMINUS");return OP;}
\<\<	{strcpy(yylval.value,"SHL");return OP;}
\>\>	{strcpy(yylval.value,"SHR");return OP;}
\<=	{strcpy(yylval.value,"LTEQ");return OP;}
\>=     {strcpy(yylval.value,"GTEQ");return OP;}
==	{strcpy(yylval.value,"EQEQ");return OP;}
!=	{strcpy(yylval.value,"NOTEQ");return OP;}
&&	{strcpy(yylval.value,"LOGAND");return OP;}
\|\|    {strcpy(yylval.value,"LOGOR");return OP;}
\<%	{strcpy(yylval.value,"LTMOD");return OP;}
%\>	{strcpy(yylval.value,"GTMOD");return OP;}
\<:     {strcpy(yylval.value,"LTCOL");return OP;}
:\>     {strcpy(yylval.value,"COLGT");return OP;}
%:	{strcpy(yylval.value,"MODCOL");return OP;}
%:%:	{strcpy(yylval.value,"MODCOLMOD");return OP;}
auto	|
char	|
default	|
else	|
for	|
inline	|
return	|
static	|
union	|
while	|
_Bool	|
_Complex |
restrict |
enum	|
goto	|
int	|
short	|
struct	|
unsigned |
break	|
const	|
do	|
extern	|
if	|
long	|
signed	|
switch	|
void	|
case	|
continue |
double	|
float	|
_Imaginary |
register  |
sizeof	|
volatile |
typedef	{yylval.string_literal=yytext;return KEYWD;}
\' {yylval.string_literal = string;pos=0;printf("%s %d CHARLIT ",file_name,line);BEGIN(CHRESC);}
\" {yylval.string_literal = string;pos=0;printf("%s %d STRING ",file_name,line);BEGIN(STRESC);}
1[eE][-+]?[0-9]+[fF] {yylval.f=strtof(yytext,NULL);return FLOAT;}
[0-9]*[eE][-+][0-9]+ {yylval.d=strtod(yytext,NULL);return DOUBLE;}
[0-9]*\.[0-9]* {yylval.d=strtod(yytext,NULL);return DOUBLE;}
{OCT} {yylval.integer=strtol(yytext,NULL,8);printf("%s %d NUMBER INTEGER %d INT\n",file_name,line,yylval.integer);return INT;}
{OCT}[Uu] {yylval.integer=strtol(yytext,NULL,8);printf("%s %d NUMBER INTEGER %d UNSIGNED,INT\n",file_name,line,yylval.integer);return INT;}
{OCT}[lL] {yylval.integer=strtol(yytext,NULL,8);printf("%s %d NUMBER INTEGER %d LONG\n",file_name,line,yylval.integer);return INT;}
{OCT}(LL|ll) {yylval.integer=strtol(yytext,NULL,8);printf("%s %d NUMBER INTEGER %d LONGLONG\n",file_name,line,yylval.integer);return INT;}
{OCT}([Ll][Uu]|[Uu][Ll]) {yylval.integer=strtol(yytext,NULL,8);printf("%s %d NUMBER INTEGER %d UNSIGNED,LONG\n",file_name,line,yylval.integer);return INT;}
{OCT}([Uu](LL|ll)|(LL|ll)[Uu]) {yylval.integer=strtol(yytext,NULL,8);printf("%s %d NUMBER INTEGER %d UNSIGNED,LONGLONG\n",file_name,line,yylval.integer);return INT;}
{HEX} {yylval.integer=strtol(yytext,NULL,16);printf("%s %d NUMBER INTEGER %d INT\n",file_name,line,yylval.integer);return INT;}
{HEX}[Uu] {yylval.integer=strtol(yytext,NULL,16);printf("%s %d NUMBER INTEGER %d UNSIGNED,INT\n",file_name,line,yylval.integer);return INT;}
{HEX}[lL] {yylval.integer=strtol(yytext,NULL,16);printf("%s %d NUMBER INTEGER %d LONG\n",file_name,line,yylval.integer);return INT;}
{HEX}(LL|ll) {yylval.integer=strtol(yytext,NULL,16);printf("%s %d NUMBER INTEGER %d LONGLONG\n",file_name,line,yylval.integer);return INT;}
{HEX}([Ll][Uu]|[Uu][Ll]) {yylval.integer=strtol(yytext,NULL,16);printf("%s %d NUMBER INTEGER %d UNSIGNED,LONG\n",file_name,line,yylval.integer);return INT;}
{HEX}([Uu](LL|ll)|(LL|ll)[Uu]) {yylval.integer=strtol(yytext,NULL,16);printf("%s %d NUMBER INTEGER %d UNSIGNED,LONGLONG\n",file_name,line,yylval.integer);return INT;}
[1-9]{DIGIT}* {yylval.integer=atoi(yytext);printf("%s %d NUMBER INTEGER %d INT\n",file_name,line,yylval.integer);return INT;}
[1-9]{DIGIT}*[Uu] {yylval.integer=atoi(yytext);printf("%s %d NUMBER INTEGER %d UNSIGNED,INT\n",file_name,line,yylval.integer);return INT;}
[1-9]{DIGIT}*[lL] {yylval.integer=atoi(yytext);printf("%s %d NUMBER INTEGER %d LONG\n",file_name,line,yylval.integer);return INT;}
[1-9]{DIGIT}*(LL|ll) {yylval.integer=atoi(yytext);printf("%s %d NUMBER INTEGER %d LONGLONG\n",file_name,line,yylval.integer);return INT;}
[1-9]{DIGIT}*([Ll][Uu]|[Uu][Ll]) {yylval.integer=atoi(yytext);printf("%s %d NUMBER INTEGER %d UNSIGNED,LONG\n",file_name,line,yylval.integer);return INT;}
[1-9]{DIGIT}*([Uu](LL|ll)|(LL|ll)[Uu]) {yylval.integer=atoi(yytext);printf("%s %d NUMBER INTEGER %d UNSIGNED,LONGLONG\n",file_name,line,yylval.integer);return INT;}
[A-Za-z\_]+[A-Za-z0-9\_]* {yylval.string_literal=yytext;return IDENT;}
#.*?\.c\" {strcpy(yylval.value, yytext);return FILEN;}
#.*
\n      {line++;}
. 	{fprintf(stderr, "ERROR: unrecognized character `%s`\n",yytext);}


<CHRESC,STRESC>\\[0-7]{1,3} {
			if(YYSTATE==STRESC || YYSTATE==CHRESC){
				yytext++;
				int val = strtol(yytext,NULL,8);
				if(val==0){
					yylval.string_literal[pos] = '\\';
					pos++;
					yylval.string_literal[pos] = '0';
					pos++;
				}else if(val>32 && val<128){
					yylval.string_literal[0] = val;
					pos++;
				}else if(val>255){
					yylval.string_literal[pos] = '\\';
					pos++;
					yylval.string_literal[pos] = '3';
					pos++;
					yylval.string_literal[pos] = '7';
					pos++;
					yylval.string_literal[pos] = '7';
					pos++;
				}else{
					yylval.string_literal[pos] = '\\';
					pos++;
					yylval.string_literal[pos+2] = 48 + val%8;
					val = val/8;
					yylval.string_literal[pos+1] = 48 + val%8;
					val = val/8;
					yylval.string_literal[pos] = 48 + val%8;
					pos = pos + 3;
				}
				if(YYSTATE==CHRESC){
					yylval.string_literal[pos] = '\0';
					printf("%c", yylval.string_literal[0]);
					BEGIN(ENDCHR);
				}
			}
			}
<STRESC,CHRESC>\\x[a-fA-F0-9]* {
		if(YYSTATE==STRESC || YYSTATE==CHRESC){
			yytext = yytext + 2;
			int val = strtol(yytext,NULL,16);
			if(val==0){
				yylval.string_literal[pos] = '\\';
				pos++;
				yylval.string_literal[pos] = '0';
				pos++;
			}else if(val>32 && val<128){
				yylval.string_literal[pos] = val;
				pos++;
			}else if(val>255){
				yylval.string_literal[pos] = '\\';
				pos++;
				yylval.string_literal[pos] = '3';
				pos++;
				yylval.string_literal[pos] = '7';
				pos++;
				yylval.string_literal[pos] = '7';
				pos++;
			}else{
				yylval.string_literal[pos] = '\\';
				pos++;
				yylval.string_literal[pos+2] = 48 + val%8;
				val = val/8;
				yylval.string_literal[pos+1] = 48 + val%8;
				val = val/8;
				yylval.string_literal[pos] = 48 + val%8;
				pos = pos + 3;
			}
			if(YYSTATE==CHRESC){
				yylval.string_literal[pos] = '\0';
				printf("%s", yylval.string_literal);
				BEGIN(ENDCHR);
			}
		}
		}
<STRESC,CHRESC>\\.   {
			if(YYSTATE==STRESC || YYSTATE==CHRESC){
				char *ch = yytext;
				ch++;
				if(*ch == '0' || *ch == 'a' || *ch=='b' || *ch=='f' || *ch=='n' || *ch == 'r'|| *ch=='t' || *ch=='v' || *ch=='\'' || *ch == '\\' || *ch=='\"'){
					yylval.string_literal[pos] = '\\';
					pos++;
					yylval.string_literal[pos] = *ch;
					pos++;
				}else{
					yylval.string_literal[pos] = *ch;
					pos++;
				}
				if(YYSTATE==CHRESC){
					yylval.string_literal[pos] = '\0';
					printf("%s",yylval.string_literal);
					BEGIN(ENDCHR);
				}
			}
			}


<STRESC>\"L? {
		yylval.string_literal[pos] = '\0';
		printf("%s ", yylval.string_literal);
		printf(" \n ");
		return STRING;
		}

<CHRESC,ENDCHR>\'L? {
		printf("\n");
		return CHAR;
		}
<CHRESC,STRESC>\n {printf(" \n");line++;}
<CHRESC,STRESC>. {
	   if(YYSTATE==STRESC || YYSTATE==CHRESC) {
		yylval.string_literal[pos]= *yytext;
		pos++;
	 	if(YYSTATE==CHRESC){
			yylval.string_literal[pos] = '\0';
			printf("%s",yylval.string_literal);
			BEGIN(ENDCHR);
		}
	 }
	 }


<ENDCHR>.

%%

main()
{
 int t;
 char *cptr;
 char c[4];
 char b;
	while (t=yylex())
	{
	 switch(t)
	 {
		case OP:
			printf("%s %d %s\n",file_name,line,yylval.value);
			break;
		case ARROW:
			printf("%s %d ARROW\n",file_name,line);
			break;
		case PLUSPLUS:
			printf("%s %d PLUSPLUS\n", file_name,line);
			break;
		case FILEN:
			cptr = strtok(yylval.value,"\"");
	 		cptr = strtok(NULL,"\"");
			line = 0;
			strcpy(file_name, cptr);
			break;
	 	case IDENT:
			printf("%s %d IDENT %s\n",file_name,line,yylval.string_literal);
			break;
		case KEYWD:
			cptr = yylval.string_literal;
			while(*cptr){
				*cptr = toupper((unsigned char) *cptr);
				cptr++;
			}
			printf("%s %d %s\n",file_name,line,yylval.string_literal);
			break;
		case FLOAT:
		        printf("%s %d NUMBER REAL %.e FLOAT\n",file_name,line,yylval.f);
			break;
		case DOUBLE:
			printf("%s %d NUMBER REAL %f DOUBLE\n",file_name,line,yylval.d);
		case STRING:
			BEGIN(INITIAL);
			break;
		case CHAR:
			BEGIN(INITIAL);
			break;
	 }
	}
}
