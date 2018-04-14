/**
 *    > Author:   UncP
 *    > Mail:     770778010@qq.com
 *    > Description:
 *			assembly code generation
 *    > Created Time: 2016-02-06 14:54:45
**/

enum {
	LI , LA, LV, SI,
	ADD, SUB, MUL, DIV,
	LT, GT, EQ, INC, DEC, BZ, BNZ, AND, OR, PSH, JMP, SAVE, CALL, RET, PRTF, ADJ
};

extern p_stack ps;

void init_p_stack()
{// initialize program stack
	ps.pc  = (int *)malloc(PC_NUMBER * sizeof(int));
	memset(ps.pc, 0, PC_NUMBER);
	ps.esp = (int *)malloc((STACK_SIZE + OFFSET) * sizeof(int));
	memset(ps.esp, 0, STACK_SIZE + OFFSET);
	ps.esp += STACK_SIZE;
	ps.ebp = ps.esp;
	ps.arg = ps.offset = ps.ebp;
}

void adjust_ebp()
{// adjust offset
    ps.offset = ps.ebp;
}

void adjust_arg()
{// adjust argument offset
	ps.arg = ps.ebp;
}

int position(char *beg)
{// get offset of the variable
	int hash = hash_number(beg);
	int pre_hash = hash;
    symbol_table_list *curr = stl;
	while (curr) {
        while (!curr->tab.sym[hash].empty && strcmp(beg, curr->tab.sym[hash].name))
        	HASH(hash);
	    if (curr->tab.sym[hash].empty) curr = curr->pre;
	    else return curr->tab.sym[hash].offset;
		hash = pre_hash;
    }
    return -1;
}

void assembly(Node node)
{// generate assembly code
	if (node.ltag == 1) {
		assembly(*(node.left.n));
		if (node.left.n->id == Arr) {
			if(node.id == Inc || node.id == Dec) *ps.pc++ = PSH;
			if (node.id != '=' && node.id != Arr) *ps.pc++ = LV;
		}
		if(node.id != Inc && node.id != Dec)
			*ps.pc++ = PSH;
	} else if (node.ltag == 2) {
		Leaf leaf = *(node.left.l);
		if (leaf.tag == 1) {
			*ps.pc++ = LA; *ps.pc++ = position(leaf.v);
			if (node.id == Inc || node.id == Dec) { *ps.pc++ = PSH; }
			if (node.id != '=' && node.id != Arr)
				*ps.pc++ = LV;
		} else { *ps.pc++ = LI; *ps.pc++ = leaf.i; }
		if (node.id != Inc && node.id != Dec) *ps.pc++ = PSH;
	}
	if (node.rtag == 1) {
		assembly(*(node.right.n));
		if (node.right.n->id == Arr) *ps.pc++ = LV;
	} else if (node.rtag == 2) {
		Leaf leaf = *(node.right.l);
		if (leaf.tag == 1) {
			if (node.id == Fun) *ps.pc++ = SAVE;
			if (node.id != Prtf) {
				*ps.pc++ = LA; *ps.pc++ = position(leaf.v);
				if (node.id != Fun) *ps.pc++ = LV;
				else *ps.pc++ = PSH;
			} else {
				*ps.pc++ = LI; *ps.pc++ = (int)leaf.v;
			}
		} else {
			*ps.pc++ = LI;
			*ps.pc++ = leaf.i;
		}
	}
	if      (node.id == '=') *ps.pc++ = SI;
	else if (node.id == '+') *ps.pc++ = ADD;
	else if (node.id == '-') *ps.pc++ = SUB;
	else if (node.id == '*') *ps.pc++ = MUL;
	else if (node.id == '/') *ps.pc++ = DIV;
	else if (node.id == '<') *ps.pc++ = LT;
	else if (node.id == '>') *ps.pc++ = GT;
	else if (node.id == Eq)  *ps.pc++ = EQ;
	else if (node.id == Arr) *ps.pc++ = ADD;
	else if (node.id == Inc) { *ps.pc++ = INC; *ps.pc++ = SI; }
	else if (node.id == Dec) { *ps.pc++ = DEC; *ps.pc++ = SI; }
	else if (node.id == And) *ps.pc++ = AND;
	else if (node.id == Or)  *ps.pc++ = OR;
	else if (node.id == Prtf)*ps.pc++ = PRTF;
	else if (node.id == Arg) ;
	else if (node.id == Fun) *ps.pc++ = CALL;
	else if (node.id == Ret) *ps.pc++ = RET;
}

void assembly_with_offset(Node node, int offset)
{
	assembly(node);
	if (offset) {
		*ps.pc++ = ADJ;
		*ps.pc++ = offset;
	}
}
