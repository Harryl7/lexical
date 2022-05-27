/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"

/* 将 token 打印到清单文件 */
void printToken(TokenType token, const char* tokenString)
{
    switch (token)
    {
    case IF:
    case THEN:
    case ELSE:
    case END:
    case REPEAT:
    case UNTIL:
    case READ:
    case WRITE:
    case FUNC:
    case RETURN:
    case WHILE:
    case TYPE:       fprintf(listing, "key  : %s\n", tokenString); break;
    case ASSIGN:     fprintf(listing, ":=\n"); break;
    case LT:         fprintf(listing, "<\n"); break;
    case EQ:         fprintf(listing, "=\n"); break;
    case LPAREN:     fprintf(listing, "(\n"); break;
    case RPAREN:     fprintf(listing, ")\n"); break;
    case SEMI:       fprintf(listing, ";\n"); break;
    case PLUS:       fprintf(listing, "+\n"); break;
    case MINUS:      fprintf(listing, "-\n"); break;
    case TIMES:      fprintf(listing, "*\n"); break;
    case DIV:        fprintf(listing, "/\n"); break;
    case LSQUARE:    fprintf(listing, "[\n"); break;
    case RSQUARE:    fprintf(listing, "]\n"); break;
    case COMMA:      fprintf(listing, ",\n"); break;
    case ENDFILE:    fprintf(listing, "EOF\n"); break;
    case INT:        fprintf(listing, "INT  , val= %s\n", tokenString); break;
    case FLOAT:      fprintf(listing, "FLOAT, val= %s\n", tokenString); break;
    case ID:         fprintf(listing, "ID   , name= %s\n", tokenString); break;
    case ERROR:      fprintf(listing, "ERROR: %s\n", tokenString); break;
    default: /* should never happen */
        fprintf(listing, "Unknown token: %d\n", token);
    }
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode* newStmtNode(StmtKind kind)
{
    TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else {
        for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = StmtK;
        t->kind.stmt = kind;
        t->lineno = lineno;
        t->attr.val = 0;
    }
    return t;
}

/* Function newExpNode creates a new expression 
 * node for syntax tree construction
 */
TreeNode* newExpNode(ExpKind kind)
{
    TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else {
        for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = ExpK;
        t->kind.exp = kind;
        t->lineno = lineno;
        t->type = Void;
        t->attr.val = 0;
    }
    return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char* copyString(char* src)
{
    if (src == NULL) return NULL;
    int n = strlen(src) + 1;
    char* dst = (char*)malloc(n * sizeof(char));
    if (dst == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    //else strcpy(dst, src);
    else strncpy(dst, src, n);
    return dst;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=2
#define UNINDENT indentno-=2

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{
    for (int i = 0; i < indentno; i++)
        fprintf(listing, " ");
}

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode* tree)
{
    int i;
    INDENT;
    while (tree != NULL) {
        printSpaces();
        switch (tree->nodekind)
        {
        case StmtK:
            switch (tree->kind.stmt) {
            case IfK:
                fprintf(listing, "If\n");
                break;
            case RepeatK:
                fprintf(listing, "Repeat\n");
                break;
            case AssignK:
                fprintf(listing, "Assign to: %s\n", tree->attr.name);
                break;
            case ReadK:
                fprintf(listing, "Read: %s\n", tree->attr.name);
                break;
            case WriteK:
                fprintf(listing, "Write\n");
                break;
            case WhileK:
                fprintf(listing, "While\n");
                break;
            case DeclareK:
                fprintf(listing, "Declare: %s\n", typeString[tree->type]);
                break;
            case FuncK:
                fprintf(listing, "Function: %s -> %s\n", tree->attr.name, typeString[tree->type]);
                break;
            case ReturnK:
                fprintf(listing, "Return:\n");
                break;
            default:
                fprintf(listing, "Unknown StmtNode kind\n");
                break;
            }
            break;
        case ExpK:
            switch (tree->kind.exp) {
            case OpK:
                fprintf(listing, "Op: ");
                printToken(tree->attr.op, "\0");
                break;
            case ConstK:
                if (tree->type == Float) fprintf(listing, "Const: %f\n", tree->attr.fval);
                else fprintf(listing, "Const: %d\n", tree->attr.val);
                break;
            case IdK:
                if(tree->attr.val == 0) fprintf(listing, "Id: %s\n", tree->attr.name);
                else fprintf(listing, "Array: %s[%d]\n", tree->attr.name, tree->attr.val);
                break;
            case ArrayK:
                fprintf(listing, "Array: %s\n", tree->attr.name);
                break;
            case ParamK:
                fprintf(listing, "Param: %s -> %s\n", tree->attr.name, typeString[tree->type]);
                break;
            case CallK:
                fprintf(listing, "Call: %s\n", tree->attr.name);
                break;
            default:
                fprintf(listing, "Unknown ExpNode kind\n");
                break;
            }
            break;
        default:
            fprintf(listing, "Unknown node kind\n");
        }
        for (i = 0; i < MAXCHILDREN; i++)
            printTree(tree->child[i]);
        tree = tree->sibling;
    }
    UNINDENT;
}
