/**
 *    > Author:   UncP
 *    > Mail:     770778010@qq.com
 *    > Description:
 *			symbol table
 *    > Created Time: 2016-02-02 12:32:12
**/
typedef struct Array         // array information
{
    int dim;            // dimension of array
    int len[DIM];       // length of each dimension
}Array;

typedef struct element       // structure element information
{
    int   tag;          // type of the element, Int for imm, Arr for array
    char *name;         // element's name
    int   offset;       // offset
    Array *arr;         // array
}element;

typedef struct Struct        // structure information
{
    element e[ELE_NUM]; // element information
    int size;           // number of the elements
}Struct;

typedef struct Func          // function
{
    int ret;            // return type
    int argc;           // argument count
    element e[ELE_NUM]; // argument information
}Func;

typedef struct symbol
{
	bool  empty;            // used to check whether this position is empty
	int   type;	  	        // Int Arr Stru Fun
	int   offset;	        // position in ebp
    char *name;             // name of variable
    union {
        Array   *arr;       // array
        Struct  *stru;      // structure
        Func    *fun;       // function
    };
}symbol;

typedef struct symbol_table
{
    symbol *sym;    // symbol table
}symbol_table;

typedef struct symbol_table_list
{
    symbol_table  tab;                  // table of the symbol table list node
    struct symbol_table_list *pre;      // previous symbol list
    struct symbol_table_list *next;     // next symbol list
}symbol_table_list;

extern symbol_table_list *stl;          // symbol table list

extern p_stack ps;                      // program stack

int hash_number(char *beg)
{// get hash number
    int hash = 0;
    char *end = beg;
    while (*end != '\0') hash += *end++;
    hash %= TABLE_SIZE;
    return hash;
}

int insert(char *beg, int type)
{// insert name and type into symbol table
    int hash = hash_number(beg);
    while (!stl->tab.sym[hash].empty) HASH(hash);
    stl->tab.sym[hash].empty = false;
    stl->tab.sym[hash].type  = type;
    stl->tab.sym[hash].name  = (char *)malloc((strlen(beg) + 1) * sizeof(char));
    strcpy(stl->tab.sym[hash].name, beg);
    return hash;
}

void int_insert(char *beg)
{// insert integer into symbol table
    int hash = insert(beg, Int);
    stl->tab.sym[hash].offset = ps.offset++ - ps.ebp;
}

void array_insert(char *beg, Array *arr)
{// insert array into symbol table
    int hash = insert(beg, Arr);
    stl->tab.sym[hash].arr = (Array *)malloc(sizeof(Array));
    int offset = 1;
    int dim = arr->dim;
    stl->tab.sym[hash].arr->dim = dim;
    for (int i = 0; i < dim; ++i) {
        int len = arr->len[i];
        stl->tab.sym[hash].arr->len[i] = len;
        offset *= len;
    }
    stl->tab.sym[hash].offset = ps.offset - ps.ebp;
    ps.offset += offset;
}

void arg_int_insert(char *beg)
{// insert integer into symbol table
    int hash = insert(beg, Int);
    stl->tab.sym[hash].offset = --ps.arg - ps.ebp;
}

void arg_array_insert(char *beg, Array *arr)
{// argument array insert
    // TODO
}

void fun_info_insert(const char *beg, const int hash, Func *fun)
{
    symbol_table_list *curr = stl;
    while (1) {
        if (!curr->tab.sym[hash].empty &&
            !strcmp(beg, curr->tab.sym[hash].name)) break;
        else curr = curr->pre;
    }
    curr->tab.sym[hash].fun = (Func *)malloc(sizeof(Func));
    Func *p = curr->tab.sym[hash].fun;
    p->ret = fun->ret;
    p->argc = fun->argc;
    for (int i = 0; i < fun->argc; ++i)
        p->e[i].tag = fun->e[i].tag;
}

int fun_insert(char *beg)
{// insert function into symbol table
    int hash = insert(beg, Fun);
    stl->tab.sym[hash].offset = ps.pc - ps.ebp;
    return hash;
}

void struct_var_insert(const int hash, char *stru, char *beg)
{// insert struct variable
    symbol_table_list *curr = stl;
    while (1) {
        if (!curr->tab.sym[hash].empty &&
            !strcmp(stru, curr->tab.sym[hash].name)) break;
        else curr = curr->pre;
    }
    int h = insert(beg, Stru);
    Struct *p = curr->tab.sym[hash].stru;
    stl->tab.sym[h].stru = p;
    stl->tab.sym[h].offset = ps.offset - ps.ebp;
    ps.offset += p->e[p->size].offset;
}

int struct_insert(char *beg)
{// insert struct name into symbol table
    int hash = insert(beg, Stru);
    stl->tab.sym[hash].stru = (Struct *)malloc(sizeof(Struct));
    Struct *p = stl->tab.sym[hash].stru;
    p->size = 0;
    p->e[0].offset = 0;
    return hash;
}

int ele_lookup(const int hash, const char *stru, char *ele, const int line)
{// lookup struct element in symbol table
    symbol_table_list *curr = stl;
    while (1) {
        if (!strcmp(stru, curr->tab.sym[hash].name)) break;
        else curr = curr->pre;
    }
    Struct *p = curr->tab.sym[hash].stru;
    for(int i = 0; i < p->size; ++i)
        if (!strcmp(ele, p->e[i].name)) return 0;
    return -1;
}

void ele_int_insert(const int hash, char *beg)
{// insert struct element integer into symbol table
    Struct *p = stl->tab.sym[hash].stru;
    int s = p->size++;
    p->e[s].tag = Int;
    p->e[s].name = (char *)malloc((strlen(beg) + 1) * sizeof(char));
    strcpy(p->e[s].name, beg);
    p->e[s + 1].offset = p->e[s].offset + 1;
}

void ele_array_insert(const int hash, char *beg, Array *arr)
{// insert struct element array into symbol table
    Struct *p = stl->tab.sym[hash].stru;
    int s = p->size++;
    p->e[s].tag = Arr;
    p->e[s].name = (char *)malloc((strlen(beg) + 1) * sizeof(char));
    strcpy(p->e[s].name, beg);
    p->e[s].arr = (Array *)malloc(sizeof(Array));
    int offset = 1;
    int dim = arr->dim;
    p->e[s].arr->dim = dim;
    for (int i = 0; i < dim; ++i) {
        int len = arr->len[i];
        p->e[s].arr->len[i] = len;
        offset *= len;
    }
    p->e[s + 1].offset = p->e[s].offset + offset;
}

int lookup(char *beg, const int type, const int line, bool flag)
{// lookup variable in symbol table
	int hash = hash_number(beg);
    int pre_hash = hash;
    symbol_table_list *curr = stl;
    while (curr) {
        while (!curr->tab.sym[hash].empty && strcmp(beg, curr->tab.sym[hash].name))
            HASH(hash);
        if (curr->tab.sym[hash].empty) {
            if (flag) return -1;
            else curr = curr->pre;
        } else if (curr->tab.sym[hash].type != type) {
            PANIC("conflicting types", line);
        } else return hash;
        hash = pre_hash;
    }
    return -1;
}

Array* array_info(char *beg, const int line)
{// get array's dimension
    int hash = hash_number(beg);
    int pre_hash = hash;
    symbol_table_list *curr = stl;
    while (curr) {
        while (!curr->tab.sym[hash].empty && strcmp(beg, curr->tab.sym[hash].name))
            HASH(hash);
        if (curr->tab.sym[hash].empty) { curr = curr->pre;
        } else if (curr->tab.sym[hash].type != Arr) { PANIC("conflicting types", line);
        } else return curr->tab.sym[hash].arr;
        hash = pre_hash;
    }
    return 0;
}

Struct* struct_info(char *beg, const int line)
{// get array's dimension
    int hash = hash_number(beg);
    int pre_hash = hash;
    symbol_table_list *curr = stl;
    while (curr) {
        while (!curr->tab.sym[hash].empty && strcmp(beg, curr->tab.sym[hash].name))
            HASH(hash);
        if (curr->tab.sym[hash].empty) { curr = curr->pre;
        } else if (curr->tab.sym[hash].type != Stru) { PANIC("conflicting types", line);
        } else return curr->tab.sym[hash].stru;
        hash = pre_hash;
    }
    return 0;
}

Func* function_info(char *beg, const int line)
{
    int hash = hash_number(beg);
    int pre_hash = hash;
    symbol_table_list *curr = stl;
    while (curr) {
        while (!curr->tab.sym[hash].empty && strcmp(beg, curr->tab.sym[hash].name))
            HASH(hash);
        if (curr->tab.sym[hash].empty) { curr = curr->pre;
        } else if (curr->tab.sym[hash].type != Fun) { PANIC("conflicting types", line);
        } else return curr->tab.sym[hash].fun;
        hash = pre_hash;
    }
    return 0;
}

void init_stl()
{// initialize symbol table list
    stl = (symbol_table_list *)malloc(sizeof(symbol_table_list));
    stl->tab.sym = (symbol *)malloc(TABLE_SIZE * sizeof(symbol));
    for (int i = 0; i < TABLE_SIZE; ++i) stl->tab.sym[i].empty = true;
    stl->pre = stl->next = NULL;
}

void ent()
{// enter a scope
    // puts("enter");
    symbol_table_list *temp = stl;
    stl->next = (symbol_table_list *)malloc(sizeof(symbol_table_list));
    stl = stl->next;
    stl->pre = temp;
    stl->tab.sym = (symbol *)malloc(TABLE_SIZE * sizeof(symbol));
    for (int i = 0; i < TABLE_SIZE; ++i) stl->tab.sym[i].empty = true;
    stl->next = NULL;
}

void lev()
{// leave a scope
    // puts("leave");
    stl = stl->pre;
    free(stl->next->tab.sym);
    stl->next->tab.sym = NULL;
    free(stl->next);
    stl->next = NULL;
}

void show_symbol_table()
{
    printf("\nsymbol");
    symbol_table_list *curr = stl;
    // while (curr) {
        int hash = 0;
        while (hash < TABLE_SIZE) {
            if (!curr->tab.sym[hash].empty)
                puts(curr->tab.sym[hash].name);
            ++hash;
        }
        // curr = curr->pre;
    // }
}
