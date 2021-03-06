/****************************************************/
/* File: globals.h                                  */
/* Global types and vars for TINY compiler          */
/* must come before other include files             */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* 保留字的数量 */
#define MAXRESERVED 15

typedef enum
/* book-keeping tokens */
{
    ENDFILE, ERROR,
    /* 保留字 */
    IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE,
    /* 多字符的 token */
    ID, INT, FLOAT,
    /* 特殊符号 */
    ASSIGN, EQ, LT, PLUS, MINUS, TIMES, DIV, LPAREN, RPAREN, SEMI,
    // 新增
    FUNC, RETURN, WHILE, TYPE, LSQUARE, RSQUARE, COMMA
} TokenType;

extern FILE* source; /* 源代码txt文件 */
extern FILE* listing; /* 用于回显的清单文件 */
extern FILE* code; /* code text file for TM simulator */

extern int lineno; /* 代码回显时输出的行号 */

/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

typedef enum { StmtK, ExpK } NodeKind;
typedef enum { IfK, RepeatK, AssignK, ReadK, WriteK, FuncK, ReturnK, WhileK, DeclareK } StmtKind;
typedef enum { OpK, ConstK, IdK, ArrayK, ParamK, CallK } ExpKind;

/* 用于类型检查 */
typedef enum {
    Void, Integer, Boolean, Float, Unknown
} ExpType;

#define TYPENUM 5
static char* typeString[TYPENUM] = { "void","integer","boolean","float","unknown" };

#define MAXCHILDREN 3

typedef struct treeNode {
    struct treeNode* child[MAXCHILDREN];
    struct treeNode* sibling;
    int lineno;
    NodeKind nodekind;
    struct {
        StmtKind stmt;
        ExpKind exp;
    } kind;
    struct {
        TokenType op; // ExpKind = OpK
        int val;      // ExpKind = ConstK | IdK(array)
        float fval;
        char* name;   // ExpKind = IdK; StmtKind = AssignK | ReadK | FuncK
    } attr;
    ExpType type; /* 函数参数类型 */ /* for type checking of exps */
} TreeNode;

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

/* EchoSource = TRUE causes the source program to
 * be echoed to the listing file with line numbers
 * during parsing
 */
extern int EchoSource;

/* TraceScan = TRUE causes token information to be
 * printed to the listing file as each token is
 * recognized by the scanner
 */
extern int TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
 * printed to the listing file in linearized form
 * (using indents for children)
 */
extern int TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts
 * and lookups to be reported to the listing file
 */
extern int TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
 * to the TM code file as code is generated
 */
extern int TraceCode;

/* Error = TRUE prevents further passes if an error occurs */
extern int Error;
#endif
