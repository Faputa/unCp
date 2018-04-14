/**
 *    > Author:   UncP
 *    > Mail:     770778010@qq.com
 *    > Description:
 *			error information
 *			marco
 *    > Created Time: 2016-02-14 14:54:27
**/
#define  Test printf("\n12345\n");

#define  CODE_SIZE  4096
#define  VAR_LENGTH  16
#define  ARG_LENGTH  32
#define  TABLE_SIZE  50
#define  ELE_NUM     5
#define  STACK_SIZE  16
#define  PC_NUMBER   1024
#define  OFFSET		 64
#define  ARG_SIZE	 16
#define  DIM		 3

typedef int bool;
#define  true   1
#define  false  0

#define HASH(hash) (hash = ((((hash) * 2) + 1) % TABLE_SIZE))

#define PANIC(p, line) { printf("\n%s in line %2d\n", p, line); exit(-1); }
