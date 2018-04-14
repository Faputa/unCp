/**
 *    > Author:   UncP
 *    > Mail:     770778010@qq.com
 *    > Description:
 *			syntax analyzer
 *    > Created Time: 2016-02-16 16:53:15
**/

extern p_stack ps;

void expr(Token *tk)
{// expression
	int *opbase, *optop;
	opbase = (int *)malloc(STACK_SIZE * sizeof(int));
	optop = opbase;
	bool opnd = true, oper = false, con = false, var = false;
	int bra = 0;
	while (1) {
		int id = tk->id;
		if        (id == Id) {
		 	if (opnd) {
		 		int c = peek(tk);
		 		if (c == '(') {
		 			function_node(tk);
		 			return ;
		 		} else if (c == '[') {
		 			array_node(tk);
		 		} else if (c == '.') {
		 			struct_node(tk);
		 		} else if (lookup(tk->beg, Int, tk->line, false) >= 0) {
		 			gen(tk);
		 		} else PANIC("undefined variable", tk->line);
		 		opnd = false; oper = true; var = true;
		 	} else PANIC("expected operator", tk->line);
		} else if (id == Num) {
			if (opnd) { gen(tk); opnd = false; oper = true; }
			else PANIC("expected operator", tk->line);
		} else if (strchr("=+-*/<>.[", id) || id == Eq || id == And || id == Or) {
			if (oper) { oper = false; opnd = true; }
			else PANIC("expected operand", tk->line);
		} else if (id == '(') {
			if (oper) PANIC("expected operator", tk->line);
			++bra;
			oper = false; opnd = true; con = true;
		} else if (id == ')') {
			if (!bra) {
				if (tk->stype == For || tk->stype == Arg) {
					connect(tk, &opbase, &optop);
					return ;
				} else PANIC("unmatched left parenthesis", tk->line);
			}
			if (!oper) PANIC("expected operand", tk->line);
			--bra;
			oper = true; opnd = false; con = true;
		} else if (id == Inc || id == Dec) {
			if (!oper) { oper = false; opnd = true; }
			else PANIC("expected operand", tk->line);
		} else if (id == ']') {
			if (oper) { connect(tk, &opbase, &optop); return ;
			} else PANIC("expected operand", tk->line);
		} else if (id == ';') {
			if (tk->stype == Arg) PANIC("illegal semicolon", tk->line);
			if (opnd && var) PANIC("expected operand", tk->line);
			if (bra > 0) PANIC("unmatched right brace", tk->line);
			connect(tk, &opbase, &optop);
			return ;
		} else if (id == ',') {
			if (oper) {
				if (tk->stype != Int && tk->stype != Arg)
					PANIC("illegal comma", tk->line);
				return ;
			} else PANIC("expected operand", tk->line);
		} else if (id == Str) {
			if (lookup(tk->beg, Str, tk->line, false) < 0)
				insert(tk->beg, Str);
			gen(tk);
			opnd = false; oper = true; var = true;
		} else PANIC("wrong input", tk->line);
		if (id != Id && id != Num && id != Str) {
			if (opbase == optop) { *optop++ = id; }
			else connect(tk, &opbase, &optop);
		}
		next(tk);
	}
}

void return_node(Token *tk)
{// return statement
	next(tk);
	Node node;
	node.id = Ret;
	node.rtag = 0;
	if(tk->id == ';') {
		node.ltag = 0;
	} else {
		expr(tk);
		Node l = pop();
		if (l.id) {
			node.ltag = 1;
			node.left.n = (Node *)malloc(sizeof(Node));
			*(node.left.n) = l;
		} else {
			node.ltag = 2;
			node.left.l = (Leaf *)malloc(sizeof(Leaf));
			*(node.left.l) = *(l.left.l);
		}
	}
	assembly(node);
}

void function_node(Token *tk)
{// function statement
	adjust_arg();
	gen(tk);
	next(tk);		// get token (
	Func *fun = (Func *)malloc(sizeof(Func));
	if ((fun = function_info(tk->beg, tk->line))) {
		tk->stype = Arg;
		while (tk->id != ')' && tk->id) {
			next(tk);
			expr(tk);
		}
		if(!tk->id) PANIC("illegal function call", tk->line);
		int offset = function(fun, tk->line);
		assembly_with_offset(pop(), offset);
		next(tk);
		if (tk->id != ';')
			PANIC("expected semicolon after function call", tk->line);
	} else PANIC("undefined function", tk->line);
}

void printf_node(Token *tk)
{// printf statement
	next(tk);
	if (!(tk->id == '(')) PANIC("expected left brace after printf call", tk->line);
	tk->stype = Arg;
	while (tk->id != ')' && tk->id) {
		next(tk);
		expr(tk);
	}
	if (!tk->id) PANIC("expected right brace after printf call", tk->line);
	if (empty()) PANIC("illegal printf function call", tk->line);
	int offset = print(tk->line);
	assembly_with_offset(pop(), offset);
	next(tk);
	if (!(tk->id == ';'))
		PANIC("expected semicolon after printf function call", tk->line);
	tk->stype = 0;
}

void if_node(Token *tk)
{// if statement
	int *gap;
	next(tk);
	if (tk->id != '(') PANIC("expected left parenthesis after if", tk->line);
	tk->stype = Arg;
	next(tk);
	expr(tk);
	if (tk->id != ')')
		PANIC("expected right parenthesis in if statement", tk->line);
	tk->stype = 0;
	assembly(pop());
	*ps.pc++ = BZ;
	gap = ps.pc++;
	next(tk);
	if (tk->id == '{') block(tk);
	else stmt(tk);
    if (peek(tk) == Else) {
		next(tk);
		*ps.pc++ = JMP;
		*gap = (ps.pc - gap + 1);
		gap = ps.pc++;
		next(tk);
		if (tk->id == If) {
			if_node(tk);
		} else {
			if (tk->id == '{') block(tk);
			else stmt(tk);
		}
	}
	*gap = (ps.pc - gap);
}

void while_node(Token *tk)
{// while statement
	int *back, *gap;
	back = ps.pc;
	next(tk);
	if (!(tk->id == '(')) PANIC("expected left parenthesis after while", tk->line);
	tk->stype = Arg;
	next(tk);
	expr(tk);
	if (!(tk->id == ')'))
		PANIC("expected right parenthesis in while statement", tk->line);
	tk->stype = 0;
	assembly(pop());
	*ps.pc++ = BZ;
	gap = ps.pc++;
	next(tk);
	if (tk->id == '{') block(tk);
	else stmt(tk);
	*ps.pc++ = JMP;
	*ps.pc = -(ps.pc - back);
	++ps.pc;
	*gap = (ps.pc - gap);
}

void for_node(Token *tk)
{// for statement
	int *back, *gap;
	tk->stype = For;
	next(tk);
	if (!(tk->id == '(')) PANIC("expected left parenthesis after for", tk->line);
	next(tk);				// the first argument
	expr(tk);
	if (!empty()) assembly(pop());
	back = ps.pc;			// set the jump back position
	next(tk);				// the second argument
	expr(tk);
	assembly(pop());
	*ps.pc++ = BZ;			// set the test position
	gap = ps.pc++;
	next(tk);				// the third argument
	expr(tk);
	bool last = true;
	Node third;
	if (!empty())
		third = pop();		// pop out the last argument
	else last = false;
	if (!(tk->id == ')'))
		PANIC("expected right parenthesis in for statement", tk->line);
	tk->stype = 0;
	next(tk);
	if (tk->id == '{') block(tk);
	else stmt(tk);
	if (last) assembly(third);
	*ps.pc++ = JMP;
	*ps.pc = -(ps.pc - back);
	++ps.pc;
	*gap = (ps.pc - gap);
}

void array_node(Token *tk)
{// array node
	int pre_stype = tk->stype;
	tk->stype = Arg;
	int curr_dim = 0;	// current dimension
	gen(tk);
	next(tk);			// get token [
	Array *p;
	if (p = array_info(tk->beg, tk->line)) {
		next(tk);
		if (!tk->id) PANIC("expected operand", tk->line);
		while (tk->id) {
			if (tk->id == Num || tk->id == Id)
				expr(tk);
			else PANIC("expected operand", tk->line);
			int offset = 4;
			if (++curr_dim > p->dim) PANIC("wrong array dimension", tk->line);
			for (int i = curr_dim; i < p->dim; ++i)
				offset *= p->len[i];
			Node l = pop();
			if (!l.id) offset_l(l, offset);
			else offset_n(l, offset);
			array();
			if (peek(tk) == '[') {
				next(tk);		// get token [
				next(tk);
			} else break;
		}
		if (!tk->id) PANIC("expected operand", tk->line);
		if (curr_dim != p->dim) PANIC("wrong array dimension", tk->line);
	} else PANIC("undefined array", tk->line);
	tk->stype = pre_stype;
}

void struct_node(Token *tk)
{// struct node
	int pre_stype = tk->stype;
	tk->stype = Arg;
	gen(tk);
	next(tk);		// get token .
	Struct *p;
	if (p = struct_info(tk->beg, tk->line)) {
		char *stru = (char *)malloc(VAR_LENGTH * sizeof(char));
		strcpy(stru, tk->beg);
		next(tk);
		if (tk->id == Id) {
			int i;
			for (i = 0; i < p->size; ++i)
				if (!strcmp(tk->beg, p->e[i].name))
					break;
			if (i == p->size) {
				PANIC("undefined struct element", tk->line);
			} else if (p->e[i].tag == Int) {
				Node l = pop();
				l.id = Arr;
				l.rtag = 2;
				l.right.l = (Leaf *)malloc(sizeof(Leaf));
				l.right.l->tag = 2;
				l.right.l->i = p->e[i].offset << 2;
				push(l);
			} else {
				int curr_dim = 0;
				int off = p->e[i].offset;
				next(tk);
				if (!(tk->id == '['))
				  PANIC("expected left square bra in struct", tk->line);
				next(tk);
				if (!tk->id) PANIC("expected operand", tk->line);
				Array *arr = p->e[i].arr;
				while (tk->id) {
					if (tk->id == Id || tk->id == Num)
						expr(tk);
					else PANIC("expected operand", tk->line);
					if (++curr_dim > arr->dim)
						PANIC("wrong array dimension", tk->line);
					int offset = 4;
					for (int i = curr_dim; i < arr->dim; ++i)
						offset *= arr->len[i];
					Node l = pop();
					if (!l.id) offset_l(l, offset + off);
					else offset_n(l, offset + off);
					array();
					off = 0;
					if (peek(tk) == '[') {
						next(tk);		// get token [
						next(tk);
					} else break;
				}
				if (!tk->id) PANIC("expected operand", tk->line);
				if (curr_dim != arr->dim)
					PANIC("wrong array dimension", tk->line);
			}
		} else PANIC("illegal struct element", tk->line);
	} else PANIC("undefined struct", tk->line);
	tk->stype = pre_stype;
}

void argument_dec(Token *tk, Func *fun)
{// function argument declaration
	// copy function name
	next(tk);		// get int
	if (tk->id == ')') return ;
	if (tk->id != Int) PANIC("illegal function declaration", tk->line);
	next(tk);
	if (tk->id == Id) {
		int num = fun->argc;
		++fun->argc;
		fun->e[num].name = (char *)malloc((strlen(tk->beg) + 1) * sizeof(char));
		strcpy(fun->e[num].name, tk->beg);
		if (peek(tk) == '[') {
			fun->e[num].tag = Arr;
			fun->e[num].arr = (Array *)malloc(sizeof(Array));
			array_dec(tk, fun->e[num].arr);
			next(tk);
			if (tk->id == ',')
				argument_dec(tk, fun);
			arg_array_insert(tk->beg, fun->e[num].arr);
		} else {
			fun->e[num].tag = Int;
			next(tk);
			if (tk->id == ',')
				argument_dec(tk, fun);
			arg_int_insert(fun->e[num].name);
		}
		return ;
	} else PANIC("illegal function declaration", tk->line);
}

void array_dec(Token *tk, Array *arr)
{// integer array declaration
	arr->dim = 0;		//initialize array's dimension
	while (peek(tk) == '[') {
		next(tk);		// get token [
		next(tk);
		if (tk->id == Num) {
			next(tk);
			if (tk->id == ']') {
				arr->len[arr->dim++] = tk->val;
				if (arr->dim > DIM)
					PANIC("unsupported array dimension", tk->line);
			} else PANIC("expected right square bra", tk->line);
		} else PANIC("illegal array declaration", tk->line);
	}
}

void struct_dec(Token *tk)
{// struct declaration
	next(tk);
	if (tk->id == Id) {
		int nu = 0;
		if ((nu = lookup(tk->beg, Stru, tk->line, false)) >= 0) {
			char *stru = (char *)malloc(VAR_LENGTH * sizeof(char));
			strcpy(stru, tk->beg);
			next(tk);
			if (tk->id == ';' || !tk->id)
				PANIC("useless typename in empty struct declaration", tk->line);
			while (tk->id) {
				if (tk->id == Id) {
					if (lookup(tk->beg, Stru, tk->line, true) < 0)
						struct_var_insert(nu, stru, tk->beg);
					else PANIC("struct redefinition", tk->line);
					next(tk);
					if (tk->id == ';') break;
					else if (tk->id == ',') next(tk);
				} else PANIC("illegal struct declaration", tk->line);
			}
			if (!tk->id) PANIC("expected semicolon", tk->line);
		} else PANIC("undefined struct", tk->line);
	}
}

void int_node(Token *tk)
{// integer data type declaration statement without initialization
	tk->stype = Int;
	next(tk);
	if (tk->id == ';' || !tk->id)
		PANIC("useless typename in empty declaration", tk->line);
	while (tk->id) {
		if (tk->id == Id) {
			if (peek(tk) != '[') {
				if (lookup(tk->beg, Int, tk->line, true) < 0)
					int_insert(tk->beg);
				else PANIC("redefinition", tk->line);
			} else {
				char *beg = (char *)malloc(VAR_LENGTH * sizeof(char));
				strcpy(beg, tk->beg);
				Array *arr = (Array *)malloc(sizeof(Array));
				array_dec(tk, arr);
				if (lookup(beg, Arr, tk->line, true) < 0)
					array_insert(beg, arr);
				else PANIC("redefinition", tk->line);
				free(beg);
				free(arr);
			}
			next(tk);
			if (tk->id == ';') break;
			else if (tk->id == ',') next(tk);
		} else PANIC("illegal declaration", tk->line);
	}
	if (!tk->id) PANIC("expected semicolon", tk->line);
	tk->stype = 0;
}

void struct_ele_def(Token *tk, const char* stru, const int nu)
{// struct element definition
	while (tk->id) {
		if (tk->id == Int) {
			next(tk);
			if (tk->id == ';' || !tk->id)
				PANIC("useless typename in empty declaration", tk->line);
			while (tk->id) {
				if (tk->id == Id) {
					if (peek(tk) != '[') {
						if (ele_lookup(nu, stru, tk->beg, tk->line) < 0)
							ele_int_insert(nu, tk->beg);
						else PANIC("struct element redefinition", tk->line);
					} else {
						char *ele =
							(char *)malloc(VAR_LENGTH * sizeof(char));
						strcpy(ele, tk->beg);
						Array *arr = (Array *)malloc(sizeof(Array));
						array_dec(tk, arr);
						if (ele_lookup(nu, stru, ele, tk->line) < 0)
							ele_array_insert(nu, ele, arr);
						else PANIC("struct element redefinition", tk->line);
						free(ele);
						free(arr);
					}
					next(tk);
					if (tk->id == ';') break;
					else if (tk->id == ',') next(tk);
				} else PANIC("illegal struct definition", tk->line);
			}
			if (!tk->id) PANIC("expected semicolon", tk->line);
			next(tk);
		} else PANIC("illegal struct definition", tk->line);
		if (tk->id == '}') return ;
		else if (tk->id == ';') next(tk);
	}
	if (!tk->id) PANIC("expected right brace in strcut definition", tk->line);
}

void struct_def(Token *tk)
{// structure definition
	next(tk);
	if (tk->id == Id) {
		if (lookup(tk->beg, Stru, tk->line, true) < 0) {
			char *stru = (char *)malloc(VAR_LENGTH * sizeof(char));
			strcpy(stru, tk->beg);
			int nu = struct_insert(stru);		// insert struct
			next(tk);
			if (tk->id == '{') {
				next(tk);
				struct_ele_def(tk, stru, nu);
			} else PANIC("expected left brace in struct definition", tk->line);
			free(stru);
		} else PANIC("struct redefinition", tk->line);
	} else PANIC("illegal struct definition", tk->line);
	next(tk);
	if (!(tk->id == ';')) PANIC("expected semicolon after struct definition", tk->line);
}

void stmt(Token *tk)
{// statement
	make_empty();
	if 		(tk->id == If) 	  if_node(tk);
	else if (tk->id == While) while_node(tk);
	else if (tk->id == For)	  for_node(tk);
	else if (tk->id == Int)   int_node(tk);
	else if (tk->id == Ret)   return_node(tk);
	else if (tk->id == Prtf)  printf_node(tk);
	else if (tk->id == Stru)  struct_dec(tk);
	else if (tk->id == ';')   next(tk);
	else if (tk->id == '}')   return ;
	else { expr(tk); assembly(pop()); }
}

void block(Token *tk)
{// block
	ent();
	next(tk);
	if (tk->id == '}') PANIC("expected statement", tk->line);
	while (tk->id != '}' && tk->id) {
		stmt(tk);
		next(tk);
	}
	lev();
}

void syntax(Token *tk)
{// syntax analyzer
	char *fun_name;
	int fun_pos;
	Func *fun = (Func *)malloc(sizeof(Func));
	while (tk->id) {
		make_empty();
		if (tk->id == Void || tk->id == Int) {
			fun->ret = tk->id;
			next(tk);
			if (tk->id == Id) {
				adjust_ebp();
				if (lookup(tk->beg, Fun, tk->line, false) < 0) {
					fun_pos = fun_insert(tk->beg);
					fun_name = (char *)malloc((strlen(tk->beg) + 1) * sizeof(char));
					strcpy(fun_name, tk->beg);
				} else PANIC("function redefinition", tk->line);
			} else PANIC("missing function name", tk->line);
		} else if (tk->id == Stru) { struct_def(tk);
		} else if (tk->id == '(') {
			ent();
			fun->argc = 0;
			argument_dec(tk, fun);
			if (tk->id == ')') fun_info_insert(fun_name, fun_pos, fun);
			else PANIC("illegal function declaration", tk->line);
		} else if (tk->id == '{') {
			next(tk);
			if (tk->id == '}') PANIC("expected statement", tk->line);
			while (tk->id != '}' && tk->id) {
				stmt(tk);
				next(tk);
			}
			lev();
			if (!tk->id) PANIC("expected right brace", tk->line);
		}
		next(tk);
	}
}
