/**
 *    > Author:   UncP
 *    > Mail:     770778010@qq.com
 *    > Description:
 *			supported statements
 *    > Created Time: 2016-02-17 12:55:34
**/

// expression
void expr(Token*);

// return statement
void return_node(Token*);

// function statement
void function_node(Token*);

// printf statement
void printf_node(Token*);

// if statement
void if_node(Token*);

// while statement
void while_node(Token*);

// for statement
void for_node(Token*);

// array node
void array_node(Token*);

// struct node
void struct_node(Token*);

// array declaration
void array_dec(Token*, Array*);

// struct declaration
void struct_dec(Token*);

// definition statement
void int_node(Token*);

// structure definition
void struct_def(Token*);

// statement
void stmt(Token*);

// block
void block(Token*);

// syntax analyzer
void syntax(Token*);
