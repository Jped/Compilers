%option noyywrap nodefault yylineno

%{
#include <string.h>
#include "parser.h"
#include "simple.tab.h"

char string[4096];
int pos = 0;
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
:	{return *yytext;}
\.\.\.	{return ELLIPSIS;}
\+=	{return PLUSEQ;}
\-=	{return MINUSEQ;}
\*=	{return TIMESEQ;}
\/= 	{return DIVEQ;}
%=	{return MODEQ;}
\<\<=   {return SHLEQ;}
\>\>=   {return SHREQ;}
&=	{return ANDEQ;}
\^=     {return XOREQ;}
\|=     {return OREQ;}
\-\>	{return ARROW;}
\+\+    {return PLUSPLUS;}
\-\-    {return MINUSMINUS;}
\<\<	{return SHL;}
\>\>	{return SHR;}
\<=	{return LTEQ;}
\>=     {return GTEQ;}
==	{return EQEQ;}
!=	{return NOTEQ;}
&&	{return LOGAND;}
\|\|    {return LOGOR;}
\<%	{return LTMOD;}
%\>	{return GTMOD;}
\<:     {return LTCOL;}
:\>     {return COLGT;}
%:	{return MODCOL;}
%:%:	{return MODCOLMOD;}
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
typedef	{return *yytext;}
\' {yylval.string_literal = string;pos=0;BEGIN(CHRESC);}
\" {yylval.string_literal = string;pos=0;BEGIN(STRESC);}
1[eE][-+]?[0-9]+[fF] {yylval.f=strtof(yytext,NULL);return FLOAT;}
[0-9]*[eE][-+][0-9]+ {yylval.d=strtod(yytext,NULL);return DOUBLE;}
[0-9]*\.[0-9]* {yylval.d=strtod(yytext,NULL);return DOUBLE;}
{OCT} {yylval.integer=strtol(yytext,NULL,8);return INT;}
{OCT}[Uu] {yylval.integer=strtol(yytext,NULL,8);return INT;}
{OCT}[lL] {yylval.integer=strtol(yytext,NULL,8);return INT;}
{OCT}(LL|ll) {yylval.integer=strtol(yytext,NULL,8);return INT;}
{OCT}([Ll][Uu]|[Uu][Ll]) {yylval.integer=strtol(yytext,NULL,8);return INT;}
{OCT}([Uu](LL|ll)|(LL|ll)[Uu]) {yylval.integer=strtol(yytext,NULL,8);return INT;}
{HEX} {yylval.integer=strtol(yytext,NULL,16);return INT;}
{HEX}[Uu] {yylval.integer=strtol(yytext,NULL,16);return INT;}
{HEX}[lL] {yylval.integer=strtol(yytext,NULL,16);return INT;}
{HEX}(LL|ll) {yylval.integer=strtol(yytext,NULL,16);return INT;}
{HEX}([Ll][Uu]|[Uu][Ll]) {yylval.integer=strtol(yytext,NULL,16);return INT;}
{HEX}([Uu](LL|ll)|(LL|ll)[Uu]) {yylval.integer=strtol(yytext,NULL,16);return INT;}
[1-9]{DIGIT}* {yylval.integer=atoi(yytext);return INT;}
[1-9]{DIGIT}*[Uu] {yylval.integer=atoi(yytext);return INT;}
[1-9]{DIGIT}*[lL] {yylval.integer=atoi(yytext);return INT;}
[1-9]{DIGIT}*(LL|ll) {yylval.integer=atoi(yytext);return INT;}
[1-9]{DIGIT}*([Ll][Uu]|[Uu][Ll]) {yylval.integer=atoi(yytext);return INT;}
[1-9]{DIGIT}*([Uu](LL|ll)|(LL|ll)[Uu]) {yylval.integer=atoi(yytext);return INT;}
[A-Za-z\_]+[A-Za-z0-9\_]* {yylval.string_literal = string;pos=0;strcpy(yylval.string_literal,yytext);return IDENT;}
#.*?\.c\" {strcpy(yylval.value, yytext);return FILEN;}
#.*
\n      {line++;return EOL;}
[ \t]	{/*ignore whitespace */}
. 	{ yyerror("Error, unknown character %c\n", *yytext);}


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

