/****************************************************/
/* File: parse.c                                    */
/* The parser implementation for the TINY compiler  */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

static TokenType token; /* holds current token */

/* function prototypes for recursive calls */
static TreeNode* func_sequence(void);
static TreeNode* function(void);
static TreeNode* param_declare(void);
static TreeNode* stmt_sequence(void);
static TreeNode* statement(void);
static TreeNode* if_stmt(void);
static TreeNode* repeat_stmt(void);
static TreeNode* while_stmt(void);
static TreeNode* assign_stmt(void);
static TreeNode* read_stmt(void);
static TreeNode* write_stmt(void);
static TreeNode* declare_stmt(void);
static TreeNode* return_stmt(void);
static TreeNode* variable(void);
static TreeNode* exp(void);
static TreeNode* simple_exp(void);
static TreeNode* term(void);
static TreeNode* factor(void);
static TreeNode* params(void);

static void syntaxError(char* message)
{
    fprintf(listing, "\n>>> Syntax error at line %d: %s", lineno, message);
    Error = TRUE;
}

static void match(TokenType expected)
{
    if (token == expected) token = getToken();
    else {
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        fprintf(listing, "      ");
    }
}

static void getType(TreeNode* t)
{
    switch (tokenString[0])
    {
    case 'v': t->type = Void; break;
    case 'i': t->type = Integer; break;
    case 'b': t->type = Boolean; break;
    case 'f': t->type = Float; break;
    default: t->type = Unknown; syntaxError("unknown type");
    }
}

TreeNode* func_sequence(void)
{
    TreeNode* t = function();
    TreeNode* p = t;
    while (token == FUNC)
    {
        TreeNode* q = function();
        if (q != NULL)
        {
            if (t == NULL) t = p = q;
            else
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

TreeNode* function(void)
{
    TreeNode* t = newStmtNode(FuncK);
    match(FUNC);
    if (t != NULL && token == ID)
        t->attr.name = copyString(tokenString);
    match(ID);
    match(LPAREN);
    if (t != NULL)
        t->child[0] = param_declare(); // 可能为空
    match(RPAREN);
    if (t != NULL && token == TYPE)
        getType(t);
    match(TYPE);
    if (t != NULL)
        t->child[1] = stmt_sequence();
    match(END);
    return t;
}

TreeNode* param_declare(void)
{
    TreeNode* t = NULL;
    if (token == TYPE)
    {
        t = newExpNode(ParamK);
        if (t != NULL) // 不用判断 token==TYPE 因为 if 条件已经判断过了
            getType(t);
        match(TYPE);
        if (t != NULL && token == ID)
            t->attr.name = copyString(tokenString);
        match(ID);
    }
    TreeNode* p = t, * q;
    while (token == COMMA)
    {
        match(COMMA);
        q = newExpNode(ParamK);
        if (q != NULL && token == TYPE)
            getType(q);
        match(TYPE);
        if (q != NULL && token == ID)
            q->attr.name = copyString(tokenString);
        match(ID);
        if (q != NULL)
        {
            if (t == NULL) t = p = q;
            else
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

TreeNode* stmt_sequence(void)
{
    TreeNode* t = statement();
    TreeNode* p = t;
    while ((token != ENDFILE) && (token != END) &&
        (token != ELSE) && (token != UNTIL))
    {
        TreeNode* q;
        match(SEMI);
        q = statement();
        if (q != NULL)
        {
            if (t == NULL) t = p = q;
            else /* now p cannot be NULL either */
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

TreeNode* statement(void)
{
    TreeNode* t = NULL;
    switch (token) {
    case IF: t = if_stmt(); break;
    case REPEAT: t = repeat_stmt(); break;
    case WHILE: t = while_stmt(); break;
    case ID: t = assign_stmt(); break;
    case READ: t = read_stmt(); break;
    case WRITE: t = write_stmt(); break;
    case TYPE: t = declare_stmt(); break;
    case RETURN: t = return_stmt(); break;
    default: syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    } /* end case */
    return t;
}

TreeNode* if_stmt(void)
{
    TreeNode* t = newStmtNode(IfK);
    match(IF);
    if (t != NULL) t->child[0] = exp();
    match(THEN);
    if (t != NULL) t->child[1] = stmt_sequence();
    if (token == ELSE) {
        match(ELSE);
        if (t != NULL) t->child[2] = stmt_sequence();
    }
    match(END);
    return t;
}

TreeNode* repeat_stmt(void)
{
    TreeNode* t = newStmtNode(RepeatK);
    match(REPEAT);
    if (t != NULL) t->child[0] = stmt_sequence();
    match(UNTIL);
    if (t != NULL) t->child[1] = exp();
    return t;
}

TreeNode* while_stmt(void)
{
    TreeNode* t = newStmtNode(WhileK);
    match(WHILE);
    match(LPAREN);
    if (t != NULL) t->child[0] = exp();
    match(RPAREN);
    if (t != NULL) t->child[1] = stmt_sequence();
    match(END);
    return t;
}

TreeNode* assign_stmt(void)
{
    TreeNode* t = newStmtNode(AssignK);
    if ((t != NULL) && (token == ID))
        t->attr.name = copyString(tokenString);
    match(ID);
    match(ASSIGN);
    if (t != NULL) t->child[0] = exp();
    return t;
}

TreeNode * read_stmt(void)
{ TreeNode * t = newStmtNode(ReadK);
  match(READ);
  if ((t!=NULL) && (token==ID))
    t->attr.name = copyString(tokenString);
  match(ID);
  return t;
}

TreeNode * write_stmt(void)
{ TreeNode * t = newStmtNode(WriteK);
  match(WRITE);
  if (t!=NULL) t->child[0] = exp();
  return t;
}

TreeNode* declare_stmt(void)
{
    TreeNode* t = newStmtNode(DeclareK);
    if (t != NULL) // 不用判断 token==TYPE 因为 statement 已经 case 过了
        getType(t);
    match(TYPE);
    if (t != NULL) t->child[0] = variable();
    TreeNode* p = (t == NULL ? NULL : t->child[0]);
    while (token == COMMA)
    {
        match(COMMA);
        TreeNode* q = variable();
        if (q != NULL)
        {
            if (t == NULL) t = p = q;
            else
            {
                q->type = t->type;
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

TreeNode* return_stmt(void)
{
    TreeNode* t = newStmtNode(ReturnK);
    match(RETURN);
    if (t != NULL)
        t->child[0] = exp();
    return t;
}

TreeNode* variable(void)
{
    TreeNode* t = newExpNode(IdK);
    if (t != NULL && token == ID)
        t->attr.name = copyString(tokenString);
    match(ID);
    if (token == LSQUARE)
    {
        match(LSQUARE);
        if (t != NULL && token == INT)
        {
            char* left;
            t->attr.val = strtol(tokenString, &left, 10);
            if (strlen(left) > 0)
                syntaxError("integer too long");
        }
        match(INT);
        match(RSQUARE);
    }
    return t;
}

TreeNode* exp(void)
{
    TreeNode* t = simple_exp();
    if ((token == LT) || (token == EQ)) {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
        }
        match(token);
        if (t != NULL)
            t->child[1] = simple_exp();
    }
    return t;
}

TreeNode* simple_exp(void)
{
    TreeNode* t = term();
    while ((token == PLUS) || (token == MINUS))
    {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = term();
        }
    }
    return t;
}

TreeNode* term(void)
{
    TreeNode* t = factor();
    while ((token == TIMES) || (token == DIV))
    {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            p->child[1] = factor();
        }
    }
    return t;
}

TreeNode* factor(void)
{
    TreeNode* t = NULL;
    switch (token) {
    case INT:
        t = newExpNode(ConstK);
        if (t != NULL && token == INT)
        {
            char* left;
            t->attr.val = strtol(tokenString, &left, 10);
            if (strlen(left) > 0)
                syntaxError("integer too long");
        }
        match(INT);
        break;
    case FLOAT:
        t = newExpNode(ConstK);
        if (t != NULL && token == FLOAT)
        {
            char* left;
            t->type = Float;
            t->attr.fval = strtof(tokenString, &left);
            if (strlen(left) > 0)
                syntaxError("float too long");
        }
        match(FLOAT);
        break;
    case ID:
        t = newExpNode(IdK);
        if ((t != NULL) && (token == ID))
            t->attr.name = copyString(tokenString);
        match(ID);
        if (token == LPAREN)
        {
            match(LPAREN);
            if (t != NULL)
            {
                t->kind.exp = CallK;
                t->child[0] = params();
            }
            match(RPAREN);
        }
        else if (token == LSQUARE)
        {
            match(LSQUARE);
            if (t != NULL)
            {
                t->kind.exp = ArrayK;
                t->child[0] = exp();
            }
            match(RSQUARE);
        }
        break;
    case LPAREN:
        match(LPAREN);
        t = exp();
        match(RPAREN);
        break;
    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    }
    return t;
}

TreeNode* params(void)
{
    TreeNode* t = factor();
    TreeNode* p = t;
    while (token == COMMA)
    {
        match(COMMA);
        TreeNode* q = exp();
        if (q != NULL)
        {
            if (t == NULL) t = p = q;
            else
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function parse returns the newly 
 * constructed syntax tree
 */
TreeNode* parse(void)
{
    TreeNode* t = NULL;
    token = getToken();
    if (token == FUNC)
    {
        t = func_sequence();
        t->sibling = stmt_sequence();
    }
    else
        t = stmt_sequence();
    if (token != ENDFILE)
        syntaxError("Code ends before file\n");
    return t;
}
