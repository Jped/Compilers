parser: clexer.l simple.y parser.h
	bison -v --report=look-ahead -d simple.y
	flex -osimple.lex.c  clexer.l
	cc -o $@ simple.tab.c simple.lex.c astStructure.c
