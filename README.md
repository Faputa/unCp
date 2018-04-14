### 迷你C语言编译器

Fork自https://github.com/UncP/unCp

####	unCp

当期最新版本是5.0

#### 实现细节
	1. 词法分析
	2. 语法分析生成抽象语法树,同时进行语义分析
	3. 后序遍历AST生成自定义指令集
	4. 虚拟机执行指令


#### 支持函数
	含参函数(不支持返回参数)


#### 支持语句
	if else
	while
	for
	printf


#### 支持数据结构
	整型数
	整型数组(多维)
	结构体(由整型数及整型数组组成)


#### 支持运算符:
    = + - * / ( ) == > <
    && ||
    ++ --(前缀)


#### 用法:
    gcc -std=c11 -o mini_C mini_C.c
    mini_C test1.txt
    mini_C test2.txt
    ...
