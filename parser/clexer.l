%option noyywrap nodefault yylineno

%{
#include <string.h>
#include "parser.h"
#include "cparser.tab.h"

char string[4096];
int pos = 0;
char file_name[300];
int line=1;
%}

DIGIT [0-9]
OCT 0[0-7]*
HEX 0[Xx][a-fA-F0-9]*
SIGN [+-]*
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
auto	{return AUTO;}
char	{return CHR;}
default	{return DEFAULT;}
else	{return ELSE;}
for	{return FOR;}
inline	{return INLINE;}
return	{return RETURN;}
static	{return STATIC;}
union	{return UNION;}
while	{return WHILE;}
_Bool	{return BOOL;}
_Complex {return COMPLEX;}
restrict {return RESTRICT;}
enum	{return ENUM;}
goto	{return GOTO;}
int	{return INTEGER;}
short	{return SHORT;}
struct	{return STRUCT;}
unsigned {return UNSIGNED;}
break	{return BREAK;}
const	{return CONST;}
do	{return DO;}
extern	{return EXTERN;}
if	{return IF;}
long	{return LONG;}
signed	{return SIGNED;}
switch	{return SWITCH;}
void	{return VOID;}
case	{return CASE;}
continue {return CONTINUE;}
double	{return DBLE;}
float	{return FLT;}
_Imaginary {return IMAGINARY;}
register  {return REGISTER;}
sizeof	{return SIZEOF;}
volatile {return VOLATILE;}
typedef	{return TYPEDEF;}
\' {yylval.string_literal = string;pos=0;BEGIN(CHRESC);}
\" {yylval.string_literal = string;pos=0;BEGIN(STRESC);}
[eE][-+]?[0-9]+[fF] {yylval.f=strtof(yytext,NULL);return FLOAT;}
[0-9]*[eE][-+][0-9]+ {yylval.d=strtod(yytext,NULL);return DOUBLE;}
[0-9]*\.[0-9]* {yylval.d=strtod(yytext,NULL);return DOUBLE;}
{OCT} {yylval.integer=strtoull(yytext,NULL,8);return INT;}
{OCT}[Uu] {yylval.integer=strtoull(yytext,NULL,8);return INT;}
{OCT}[lL] {yylval.integer=strtoull(yytext,NULL,8);return INT;}
{OCT}(LL|ll) {yylval.integer=strtoull(yytext,NULL,8);return INT;}
{OCT}([Ll][Uu]|[Uu][Ll]) {yylval.integer=strtoull(yytext,NULL,8);return INT;}
{OCT}([Uu](LL|ll)|(LL|ll)[Uu]) {yylval.integer=strtoull(yytext,NULL,8);return INT;}
{HEX} {yylval.integer=strtoull(yytext,NULL,16);return INT;}
{HEX}[Uu] {yylval.integer=strtoull(yytext,NULL,16);return INT;}
{HEX}[lL] {yylval.integer=strtoull(yytext,NULL,16);return INT;}
{HEX}(LL|ll) {yylval.integer=strtoull(yytext,NULL,16);return INT;}
{HEX}([Ll][Uu]|[Uu][Ll]) {yylval.integer=strtoull(yytext,NULL,16);return INT;}
{HEX}([Uu](LL|ll)|(LL|ll)[Uu]) {yylval.integer=strtoull(yytext,NULL,16);return INT;}
[1-9]{DIGIT}* {yylval.integer=strtoull(yytext,NULL,10);return INT;}
[1-9]{DIGIT}*[Uu] {yylval.integer=strtoull(yytext,NULL,10);return INT;}
[1-9]{DIGIT}*[lL] {yylval.integer=strtoull(yytext,NULL,10);return INT;}
[1-9]{DIGIT}*(LL|ll) {yylval.integer=strtoull(yytext,NULL,10);return INT;}
[1-9]{DIGIT}*([Ll][Uu]|[Uu][Ll]) {yylval.integer=strtoull(yytext,NULL,10);return INT;}
[1-9]{DIGIT}*([Uu](LL|ll)|(LL|ll)[Uu]) {yylval.integer=strtoull(yytext,NULL,10);return INT;}
[A-Za-z\_]+[A-Za-z0-9\_]* {yylval.string_literal = string;pos=0;strcpy(yylval.string_literal,yytext);return IDENT;}
#.*?\.c\" {strcpy(yylval.value, yytext);return FILEN;}
#.*
\n      {line++;}
[ \t]	{/*ignore whitespace */}
. 	{ yyerror("Error, unknown character %c\n", *yytext);}


<CHRESC,STRESC>\\[0-7]{1,3} {
			if(YYSTATE==STRESC || YYSTATE==CHRESC){
				yytext++;
				int val = strtoull(yytext,NULL,8);
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
			int val = strtoull(yytext,NULL,16);
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
					BEGIN(ENDCHR);
				}
			}
			}


<STRESC>\"L? {
		yylval.string_literal[pos] = '\0';
		BEGIN(INITIAL);
		return STRING;
		}

<CHRESC,ENDCHR>\'L? {
		BEGIN(INITIAL);
		return CHAR;
		}
<CHRESC,STRESC>. {
	   if(YYSTATE==STRESC || YYSTATE==CHRESC) {
		yylval.string_literal[pos]= *yytext;
		pos++;
	 	if(YYSTATE==CHRESC){
			yylval.string_literal[pos] = '\0';
			BEGIN(ENDCHR);
		}
	 }
	 }


<ENDCHR>.

%%

