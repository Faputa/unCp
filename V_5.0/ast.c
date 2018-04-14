/**
 *    > Author:   UncP
 *    > Mail:     770778010@qq.com
 *    > Description:
 *			AST
 *    > Created Time: 2016-02-14 14:29:52
**/

#include "priority.c"

typedef struct Leaf 		// leaf of the AST
{
	int tag;				// type of the leaf, 1 for v, 2 for i
	union {
		char *v;			// variable
		int   i;			// immediate number
	};
}Leaf;

typedef struct Node			// node of the AST
{
	int id;					// operator or reserve word
	int ltag, rtag;			// type of the left and right node  1 for node,  2 for leaf
	union{
		struct Node *n;		// left node
		struct Leaf *l;		// left leaf
	}left;
	union{
		struct Node *n; 	// right node
		struct Leaf *l;		// right leaf
	}right;
}Node;

typedef struct stack 		// stack of the AST
{
	Node *base;		// bottom of the stack
	Node *top;		// top of the stack
}stack;

extern stack stk;			// stack to store AST

void init_stk()
{
	if (!(stk.base = (Node *)malloc(STACK_SIZE * sizeof(Node))))
		printf("\nallocate stack memory failed\n");
    stk.top = stk.base;
}

Node*get_base()	  { return stk.base; }
Node pop()    	  { return *--stk.top; }
Node pull()		  { return *stk.base++; }
void push(Node n) { *stk.top++ = n; }
bool empty() 	  { return stk.top == stk.base; }
void make_empty() { stk.top = stk.base; }
void adjust(Node *n) { stk.top = stk.base = n; }

static void leaf_leaf(const int op, Node l, Node r)
{
	l.id      = op;
	l.rtag    = 2;
	l.right.l = (Leaf *)malloc(sizeof(Leaf));
	*(l.right.l) = *(r.left.l);
	push(l);
}

static void node_leaf(const int op, Node l, Node r)
{
	Node node;
	node.id      = op;
	node.ltag    = 1;
	node.rtag    = 2;
	node.left.n  = (Node *)malloc(sizeof(Node));
	node.right.l = (Leaf *)malloc(sizeof(Leaf));
	*(node.left.n)  = l;
	*(node.right.l) = *(r.left.l);
	push(node);
}

static void leaf_node(const int op, Node l, Node r)
{
	l.id      = op;
	l.rtag    = 1;
	l.right.n = (Node *)malloc(sizeof(Node));
	*(l.right.n) = r;
	push(l);
}

static void node_node(const int op, Node l, Node r)
{
	Node node;
	node.id      = op;
	node.ltag    = node.rtag = 1;
	node.left.n  = (Node *)malloc(sizeof(Node));
	node.right.n = (Node *)malloc(sizeof(Node));
	*(node.left.n)  = l;
	*(node.right.n) = r;
	push(node);
}

static void self(const int op, Node node)
{// Inc Dec
	if (node.id) {
		Node N;
		N.id = op;
		N.ltag = 1;
		N.rtag = 0;
		N.left.n = (Node *)malloc(sizeof(Node));
		*(N.left.n) = node;
		push(N);
	} else {
		node.id = op;
		push(node);
	}
}

void offset_l(Node node, int base)
{// generate offset for array
	node.id = '*';
	node.rtag = 2;
	node.right.l = (Leaf *)malloc(sizeof(Leaf));
	node.right.l->tag = 2;
	node.right.l->i = base;
	push(node);
}

void offset_n(Node r, int base)
{// generate offset for array
	Node node;
	node.id = '*';
	node.ltag = 1;
	node.left.n = (Node *)malloc(sizeof(Node));
	*(node.left.n) = r;
	node.rtag = 2;
	node.right.l = (Leaf *)malloc(sizeof(Leaf));
	node.right.l->tag = 2;
	node.right.l->i = base;
	push(node);
}

int function(Func *fun, const int line)
{// function tree
	Node l = pop();
	int curr_argc = 0;
	while (!empty()) {
		++curr_argc;
		Node r = pop();
		if (!l.id && !r.id) leaf_leaf(Arg, l, r);
		else if (!r.id) 	node_leaf(Arg, l, r);
		else if (!l.id) 	leaf_node(Arg, l, r);
		else 				node_node(Arg, l, r);
		l = pop();
	}
	if (curr_argc != fun->argc) PANIC("unmatched arguments", line);
	l.id = Fun;
	if (!curr_argc) {
		l.rtag    = 2;
		l.right.l = l.left.l;
		l.ltag    = 0;
	}
	push(l);
	return curr_argc;
}

int print(const int line)
{// printf tree
	Node l = pop();
	int flag = 0;
	while (!empty()) {
		++flag;
		Node r = pop();
		if (!l.id && !r.id) leaf_leaf(Arg, l, r);
		else if (!r.id) 	node_leaf(Arg, l, r);
		else if (!l.id) 	leaf_node(Arg, l, r);
		else 				node_node(Arg, l, r);
		l = pop();
	}
	if (l.rtag == 1 || (l.rtag == 2 && l.right.l->tag == 2))
		PANIC("illegal printf call", line);
	if (!l.rtag && l.left.l->tag == 2) PANIC("illegal printf call", line);
	l.id = Prtf;
	if (!flag) {
		l.rtag    = 2;
		l.right.l = l.left.l;
		l.ltag    = 0;
	}
	push(l);
	return flag;
}

void array()
{// array tree
	Node r = pop(), l = pop();
	if (!l.id && !r.id) leaf_leaf(Arr, l, r);
	else if (!r.id) 	node_leaf(Arr, l, r);
	else if (!l.id) 	leaf_node(Arr, l, r);
	else 				node_node(Arr, l, r);
}

void connect(Token *tk, int **opbase, int **optop)
{// connect two AST nodes
	int flag = true;
	while (flag && *opbase != *optop) {
		int top = *(*optop - 1);
		// printf("\n%c %c\n", top, tk->id);
		switch (priority(top, tk->id)) {
			case '>' :
				--*optop;
				if (top == Inc || top == Dec) {
					Node node = pop();
					self(top, node);
				} else {
					Node r = pop(), l = pop();
					if (!r.id && !l.id) leaf_leaf(top, l, r);
					else if (!r.id)		node_leaf(top, l, r);
					else if (!l.id) 	leaf_node(top, l, r);
					else 				node_node(top, l, r);
				}
				break;
			case '<' : **optop = tk->id; ++*optop; flag = false; break;
			case '=' :
				--*optop; flag = false;
				break;
			default  :
				printf("\n%c %c error in line %2d\n", top, tk->id, tk->line); exit(-1);
				break;
		}
	}
}

void gen(Token *tk)
{// generate an AST node
	Node node;
	node.id     = 0;
	node.ltag   = 2;
	node.rtag   = 0;
	Leaf l;
	if (tk->id == Id || tk->id == Str) {
		l.tag = 1;
		l.v   = (char *)malloc((strlen(tk->beg) + 1) * sizeof(char));
		strcpy(l.v, tk->beg);
	} else {
		l.tag = 2;
		l.i   = tk->val;
	}
	node.left.l = (Leaf *)malloc(sizeof(Leaf));
	*(node.left.l) = l;
	push(node);
}
