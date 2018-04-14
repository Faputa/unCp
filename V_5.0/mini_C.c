/**
 *    > Author:   UncP
 *    > Mail:     770778010@qq.com
 *    > Description:
 *			mini C compiler
 *				unCp
 *			Version 5.0
 *          added features
 *				1. function with arguments
 *				2. more arguments in printf
 *				3. negative number
 *				4. ...
 *			adjusted features
 *				1. ...
 *				2. ...
 *    > Created Time: 2016-3-12 20:34:56
**/
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

#include "macro.c"
#include "lexical.c"
#include "symbol.c"
#include "syntax_h.h"
#include "ast.c"
#include "assembly.c"
#include "syntax.c"
#include "vm.c"

p_stack ps;					// program related stack
stack stk;					// stack to store AST
symbol_table_list *stl;		// symbol table list

void init(Token *tk, const char *argv)
{// initialize compiler
    FILE *fp;
    int size;
    if (!argv) { printf("no source code file\n"); exit(-1); }
    if (!(fp = fopen(argv, "r")))
    	{ printf("unable to open file %s\n", argv); exit(-1); }
	if (!(tk->pos = (char *)malloc(CODE_SIZE * sizeof(char))))
		{ printf("unable to allocate source code memory\n"); exit(-1); }
	if (!(size = fread(tk->pos, sizeof(char), CODE_SIZE - 1, fp)))
		{ printf("unable to read file %s\n", argv); exit(-1); }
	tk->pos[size] = '\0';
	fclose(fp);
}

int main(int argc, char **argv)
{
	Token *tk = (Token *)malloc(sizeof(Token));		// Token
    init(tk, argv[1]);	// initialize compiler
    init_stk();			// initialize AST stack
    init_stl();			// initialize symbol table list
    init_p_stack(); 	// initialize program stack
    int *beg = ps.pc;

	tk->line = 1;
	next(tk);

	syntax(tk);			// syntax analyzer

	vm(beg, ps.pc, ps.ebp, ps.esp);
						// virtual machine
    return 0;
}
