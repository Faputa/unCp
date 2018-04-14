/**
 *    > Author:   UncP
 *    > Mail:     770778010@qq.com
 *    > Description:
 *			output assembly code
 *			output machine code
 *			run virtual machine
 *    > Created Time: 2016-02-14 16:02:49
**/

void show_op(int op)
{
	if (op == LI) 		printf("LI   ");
	else if (op == LA)  printf("LA   ");
	else if (op == LV)  printf("LV   ");
	else if (op == SI)  printf("SI   ");
	else if (op == ADD) printf("ADD  ");
	else if (op == SUB) printf("SUB  ");
	else if (op == MUL) printf("MUL  ");
	else if (op == DIV) printf("DIV  ");
	else if (op == LT)  printf("LT   ");
	else if (op == GT)  printf("GT   ");
	else if (op == EQ)  printf("EQ   ");
	else if (op == INC) printf("INC  ");
	else if (op == DEC) printf("DEC  ");
	else if (op == AND) printf("AND  ");
	else if (op == OR)  printf("OR   ");
	else if (op == BZ)  printf("BZ   ");
	else if (op == BNZ) printf("BNZ  ");
	else if (op == PSH) printf("PSH  ");
	else if (op == ADJ) printf("ADJ  ");
	else if (op == JMP) printf("JMP  ");
	else if (op == SAVE)printf("SAVE ");
	else if (op == CALL)printf("CALL ");
	else if (op == RET) printf("RET");
	else if (op == PRTF)printf("PRTF ");
	else printf("%d\n", op);
	putchar('\n');
}


void assembly_code(int *pc, int *nop)
{
	FILE *out;
	int *start = pc;
	if (!(out = fopen("assembly_code.txt", "w")))
		{ printf("\nunable to open assembly code file\n"); exit(-1); }
	while(start != nop) {
		int op = *start++;
		if (op == LI) 		fprintf(out, "%7d   LI   %d\n", (start - 1), *start++);
		else if (op == LA)  fprintf(out, "%7d   LA   %d\n", (start - 1), *start++);
		else if (op == LV)  fprintf(out, "%7d   LV\n",  (start - 1));
		else if (op == SI)  fprintf(out, "%7d   SI\n",  (start - 1));
		else if (op == ADD) fprintf(out, "%7d   ADD\n", (start - 1));
		else if (op == SUB) fprintf(out, "%7d   SUB\n", (start - 1));
		else if (op == MUL) fprintf(out, "%7d   MUL\n", (start - 1));
		else if (op == DIV) fprintf(out, "%7d   DIV\n", (start - 1));
		else if (op == LT)  fprintf(out, "%7d   LT\n",  (start - 1));
		else if (op == GT)  fprintf(out, "%7d   GT\n",  (start - 1));
		else if (op == EQ)  fprintf(out, "%7d   EQ\n",  (start - 1));
		else if (op == INC) fprintf(out, "%7d   INC\n", (start - 1));
		else if (op == DEC) fprintf(out, "%7d   DEC\n", (start - 1));
		else if (op == AND) fprintf(out, "%7d   AND\n", (start - 1));
		else if (op == OR)  fprintf(out, "%7d   OR\n",  (start - 1));
		else if (op == BZ)  fprintf(out, "%7d   BZ   %d\n", (start - 1), *start++);
		else if (op == BNZ) fprintf(out, "%7d   BNZ\n",  (start - 1));
		else if (op == PSH) fprintf(out, "%7d   PSH\n",  (start - 1));
		else if (op == ADJ) fprintf(out, "%7d   ADJ  %d\n", (start - 1), *start++);
		else if (op == JMP) fprintf(out, "%7d   JMP  %d\n", (start - 1), *start);
		else if (op == SAVE)fprintf(out, "%7d   SAVE\n", (start - 1));
		else if (op == CALL)fprintf(out, "%7d   CALL\n", (start - 1));
		else if (op == RET) fprintf(out, "%7d   RET\n",  (start - 1));
		else if (op == PRTF)fprintf(out, "%7d   PRTF\n", (start - 1));
	}
	fclose(out);
}

void machine_code(int *pc, int *nop)
{
	FILE *out;
	int *start = pc;
	if (!(out = fopen("machine_code.txt", "w")))
		{ printf("\nunable to open machine code file\n"); exit(-1); }
	while (start != nop) fprintf(out, "%d", *start++);
	fclose(out);
}

void vm(int *pc, int *nop, int *ebp, int *esp)
{// virtual machine
	assembly_code(pc, nop);
	machine_code(pc, nop);
	int a;
	char *M = "main";
	a = position(M);
	if (a == -1) { printf("\nno main function\n"); exit(-1); }
	pc  = (int *)(a + ebp);
	while (pc != nop) {
		int i = *pc++;
		// printf("%2d ", i); show_op(i);
		if      (i == LI)  { a = *pc++; }				// load immediate number
		else if (i == LA)  { a = (int)(ebp + *pc++);}	// load variable address
		else if (i == LV)  { a = *(int *)a; }			// load variable value
		else if (i == SI)  { *(int *)(*esp++) = a; }	// store int
		else if (i == PSH) { *--esp = a; } 				// push stack
		else if (i == ADJ) { esp = esp + *pc++; }		// adjust stack
		else if (i == ADD) { a = *esp++ + a; }
		else if (i == SUB) { a = *esp++ - a; }
		else if (i == MUL) { a = *esp++ * a; }
		else if (i == DIV) { a = *esp++ / a; }
		else if (i == LT)  { a = *esp++ < a; }
		else if (i == GT)  { a = *esp++ > a; }
		else if (i == EQ)  { a = *esp++ == a; }
		else if (i == AND) { a = *esp++ && a; }
		else if (i == OR)  { a = *esp++ || a; }
		else if (i == INC) { ++a; }
		else if (i == DEC) { --a; }
		else if (i == BZ)  { pc = a ? pc + 1: (pc + *pc); }
		else if (i == BNZ) { pc = a ? (pc + *pc) : pc; }
		else if (i == JMP) { pc = pc + *pc; }
		else if (i == SAVE){ *--esp = (int)pc; }
		else if (i == CALL){ pc = (int *)*esp++; }
		else if (i == RET) { pc = (int *)*esp++; pc += 4; }
		else if (i == PRTF){
			printf((char *)a, *esp, *(esp+1), *(esp+2), *(esp+3), *(esp+4));
		}
	}
}
