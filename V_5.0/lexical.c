/**
 *    > Author:   UncP
 *    > Mail:     770778010@qq.com
 *    > Description:
 *			lexical analyzer
 *    > Created Time: 2016-02-14 14:37:47
**/

typedef struct Token
{
	char *pos;		// position in source code
	int   line;		// line number
	int   id;		// token id
	int   stype;	// statement type
	int   val;		// immediate number value
	char *beg;		// variable
}Token;

typedef struct p_stack
{
	int	*pc;		// program counter
	int *arg;		// argument offset
    int *ebp;       // heap
    int *offset;    // offset of the variable
    int *esp;		// stack
}p_stack;

enum {
	Num = 128, Id,															// immediate number, variable
	Void, Int, Str, Arr, If, Else, While, For, Ret, Fun, Prtf, Arg, Stru,   // reserve word, node id
	Eq, And, Or, Inc, Dec 													// operation
};

void next(Token *tk)
{// get next token
	while (tk->id = *tk->pos++) {
		// printf("%c ", tk->id);
		if ((tk->id >= 'a' && tk->id <= 'z') || (tk->id >= 'A' && tk->id <= 'Z')) {
			tk->beg = (char *)malloc(VAR_LENGTH * sizeof(char));
			char *end = tk->beg;
			*end++ = tk->id;
			while ((*tk->pos >= 'a' && *tk->pos <= 'z') || (*tk->pos >= 'A' && *tk->pos <= 'Z')
				|| (*tk->pos >= '0' && *tk->pos <= '9'))
				*end++ = *tk->pos++;
			*end = '\0';
			if (!strcmp(tk->beg, "int"))   { tk->id = Int;   return ; }
			if (!strcmp(tk->beg, "if"))    { tk->id = If;    return ; }
			if (!strcmp(tk->beg, "else"))  { tk->id = Else;  return ; }
			if (!strcmp(tk->beg, "while")) { tk->id = While; return ; }
			if (!strcmp(tk->beg, "for"))   { tk->id = For;   return ; }
			if (!strcmp(tk->beg, "printf")){ tk->id = Prtf;  return ; }
			if (!strcmp(tk->beg, "void"))  { tk->id = Void;  return ; }
			if (!strcmp(tk->beg, "return")){ tk->id = Ret;   return ; }
			if (!strcmp(tk->beg, "struct")){ tk->id = Stru;  return ; }
			tk->id = Id; return ;
		}
		if (tk->id >= '0' && tk->id <= '9') {
			tk->val = tk->id - '0';
			while (*tk->pos >= '0' && *tk->pos <= '9')
				tk->val = tk->val * 10 + (*tk->pos++ - '0');
			tk->id = Num; return ;
		}
		if (tk->id == '"') {
			tk->beg = (char *)malloc(ARG_LENGTH * sizeof(char));
			char *end = tk->beg;
			while (*tk->pos != '"') {
				if (*tk->pos == '\\') {
					if (*(tk->pos + 1) == 'n') { *end++ = '\n'; tk->pos += 2; }
					else if (*(tk->pos + 1) == 't') { *end++ = '\t'; tk->pos += 2; }
					else { *end++ = *tk->pos++; }
				} else { *end++ = *tk->pos++; }
			}
			*end = '\0';
			++tk->pos;
			tk->id = Str; return ;
		}
		if (tk->id == '>') return ;
		if (tk->id == '<') return ;
		if (tk->id == '=') { if (*(tk->pos) == '=') { tk->id = Eq;  ++tk->pos; } return ; }
		if (tk->id == '+') {
			if (*(tk->pos) == '+') {
				tk->id = Inc; ++tk->pos;
			} else if (*tk->pos >= '0' && *tk->pos <= '9') {
				tk->val = *tk->pos++ - '0';
				while (*tk->pos >= '0' && *tk->pos <= '9')
					tk->val = tk->val * 10 + (*tk->pos++ - '0');
				tk->id = Num;
			}
			return ;
		}
		if (tk->id == '-') {
			if (*(tk->pos) == '-') {
				tk->id = Dec; ++tk->pos;
			} else if (*tk->pos >= '0' && *tk->pos <= '9') {
				tk->val = *tk->pos++ - '0';
				while (*tk->pos >= '0' && *tk->pos <= '9')
					tk->val = tk->val * 10 + (*tk->pos++ - '0');
				tk->val = -tk->val; tk->id = Num;
			}
			return ;
		}
		if (tk->id == '&') {
			if (*(tk->pos) == '&') {
				tk->id = And; ++tk->pos; return ;
			} else PANIC("illegal symbol", tk->line);
		}
		if (tk->id == '|') {
			if (*(tk->pos) == '|') {
				tk->id = Or;  ++tk->pos; return ;
			} else PANIC("illegal symbol", tk->line);
		}
		if (tk->id == '*') { return ; }
		if (tk->id == '[') { return ; }
		if (tk->id == ']') { return ; }
		if (tk->id == '/') {
			if (*(tk->pos) == '/') {
				while (*++tk->pos != '\n') ;
				++tk->pos;
				++tk->line;
				continue;
			} else if (*tk->pos == '*') {
				while (*tk->pos != '/') {
					while(*++tk->pos != '*')
						if (*tk->pos == '\n') ++tk->line;
					++tk->pos;
				}
				++tk->pos; continue;
			} else { return ; }
		}
		if (tk->id == '{' || tk->id == '}' || tk->id == '(' || tk->id == ')'
	        || tk->id == ';' || tk->id == ',' || tk->id == '.') return ;
		if (tk->id == '\n') ++tk->line;
		else if (tk->id == ' ' || tk->id == '\t') ;
		else PANIC("illegal symbol", tk->line);
	}
}

int peek(Token *tk)
{// get lookahead symbol
	char *ppos  = tk->pos;
	int   pline = tk->line;
	int   pid   = tk->id;
	char *pbeg  = tk->beg;
	int   pval  = tk->val;
	next(tk);
	int ret = tk->id;
	tk->pos  = ppos;
	tk->line = pline;
	tk->id   = pid;
	tk->beg  = pbeg;
	tk->val  = pval;
	return ret;
}
