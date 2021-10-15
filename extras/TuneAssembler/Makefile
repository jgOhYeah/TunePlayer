%default: build
%phony: build clean

build:
	@echo Building tuneassembler
	yacc -d tuneassembler.y
	flex tuneassembler.l
	gcc lex.yy.c y.tab.c -lm

clean:
	rm lex.yy.c
	rm a.out
	rm y.tab.c
	rm y.tab.h
	@echo Done cleaning