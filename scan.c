/****************************************************/
/* File: scan.c                                     */
/* The scanner implementation for the TINY compiler */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"

typedef enum
/* states in scanner DFA */
{
    START, INASSIGN, INCOMMENT1, ININT, INID, DONE,
    DOT, INFLOAT, COMMENT2LEFT, COMMENT2RIGHT, INCOMMENT2
}
StateType;

/* 保留字或 id 的 lexeme */
char tokenString[MAXTOKENLEN + 1];

/* BUFLEN = 源代码每一行输入缓冲区的长度 */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* 代码行缓冲区 */
static int linepos = 0; /* 指向 lineBuf 下一个待读取字符 */
static int bufsize = 0; /* 当前缓冲字符串的实际大小 */
static int EOF_flag = FALSE; /* corrects ungetNextChar behavior on EOF */

/* 从 lineBuf 中获取下一个非空字符
如果 lineBuf 耗尽，则读取新行再返回
如果读到文件末尾，返回 EOF 并将 EOF 标志置位*/
static int getNextChar(void)
{
    if (!(linepos < bufsize)) // 读新行
    {
        lineno++;
        if (fgets(lineBuf, BUFLEN - 1, source))
        {
            if (EchoSource) fprintf(listing, "%4d: %s", lineno, lineBuf);
            bufsize = strlen(lineBuf);
            linepos = 0;
            return lineBuf[linepos++];
        }
        else
        {
            EOF_flag = TRUE;
            return EOF;
        }
    }
    else return lineBuf[linepos++]; // 直接返回下一个字符
}

/* 指针回退到上一个字符（如果没有到达文件末尾） */
static void ungetNextChar(void)
{
    if (!EOF_flag) linepos--;
}

/* 保留字映射表 */
static struct
{
    char* str;
    TokenType tok;
} reservedWords[MAXRESERVED] = {
    {"if",IF}, {"then",THEN}, {"else",ELSE}, {"end",END},
    {"repeat",REPEAT}, {"until",UNTIL}, {"read",READ},
    {"write",WRITE}
};

/* 查询一个 ID 型 token 是否为保留字（线性查找），返回类型 */
static TokenType reservedLookup(char* s)
{
    int i;
    for (i = 0; i < MAXRESERVED; i++)
        if (!strcmp(s, reservedWords[i].str))
            return reservedWords[i].tok;
    return ID;
}

/****************************************/
/* the primary function of the scanner  */
/****************************************/

/* 返回源代码中下一个 token */
TokenType getToken(void)
{  
    /* tokenString 中存放下一个字符的下标 */
    int tokenStringIndex = 0;
    /* 暂存当前要返回的 token */
    TokenType currentToken;
    /* current state - always begins at START */
    StateType state = START;
    /* 可保存到 tokenString 的标志 */
    int save;
    while (state != DONE)
    {
        char c = getNextChar();
        save = TRUE;
        switch (state)
        {
        case START: {
            if (isdigit(c))
                state = ININT;
            else if (isalpha(c))
                state = INID;
            else if (c == ':')
                state = INASSIGN;
            else if ((c == ' ') || (c == '\t') || (c == '\n'))
                save = FALSE;
            else if (c == '{')
            {
                save = FALSE;
                state = INCOMMENT1;
            }
            else if (c == '/')
            {
                save = FALSE;
                state = COMMENT2LEFT;
            }
            else // 非：数字、字母、':' 、'{' 、' ' 、'\t' 、'\n'、'/'
            {
                state = DONE;
                switch (c)
                {
                case EOF:
                    save = FALSE;
                    currentToken = ENDFILE;
                    break;
                case '=':
                    currentToken = EQ;
                    break;
                case '<':
                    currentToken = LT;
                    break;
                case '+':
                    currentToken = PLUS;
                    break;
                case '-':
                    currentToken = MINUS;
                    break;
                case '*':
                    currentToken = TIMES;
                    break;
                /*case '/':
                    currentToken = DIV;
                    break;*/
                case '(':
                    currentToken = LPAREN;
                    break;
                case ')':
                    currentToken = RPAREN;
                    break;
                case ';':
                    currentToken = SEMI;
                    break;
                default:
                    currentToken = ERROR;
                    break;
                }
            }
            break;
        }
        case INCOMMENT1: {
            save = FALSE;
            if (c == EOF)
            {
                state = DONE;
                currentToken = ENDFILE;
            }
            else if (c == '}')
                state = START;
            break;
        }
        case COMMENT2LEFT: {
            save = FALSE;
            if (c == '*')
                state = INCOMMENT2;
            else {
                ungetNextChar();
                state = DONE;
                currentToken = DIV;
                break;
            }
        }
        case INCOMMENT2: {
            save = FALSE;
            if (c == EOF)
            {
                state = DONE;
                currentToken = ENDFILE;
            }
            else if (c == '*')
                state = COMMENT2RIGHT;
            break;
        }
        case COMMENT2RIGHT: {
            save = FALSE;
            if (c == EOF)
            {
                state = DONE;
                currentToken = ENDFILE;
            }
            else if (c == '/')
                state = START;
            else if (c != '*')
                state = INCOMMENT2;
            break;
        }
        case INASSIGN: {
            state = DONE;
            if (c == '=')
                currentToken = ASSIGN;
            else
            { /* backup in the input */
                ungetNextChar();
                save = FALSE;
                currentToken = ERROR;
            }
            break;
        }
        case ININT: {
            if (c == '.') // float
                state = DOT;
            //else if (c == ' ' || c == '+' || c == '-' || c == '*' || c == '/'
            //    || c == ';' || c == '<' || c == '=' || c == '{')
            //{ // 运算符、空格、分号或注释的开头
            //    ungetNextChar();
            //    save = FALSE;
            //    state = DONE;
            //    currentToken = INT;
            //}
            else if (!isdigit(c))
            {
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = INT;
            }
            break;
        }
        case DOT: {
            if (isdigit(c))
                state = INFLOAT;
            else
            {
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = ERROR;
            }
            break;
        }
        case INFLOAT: {
            if (!isdigit(c))
            {
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = FLOAT;
            }
            break;
        }
        case INID: {
            if (!isalpha(c))
            { /* backup in the input */
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = ID;
            }
            break;
        }
        case DONE:
        default: /* should never happen */ {
            fprintf(listing, "Scanner Bug: state= %d\n", state);
            state = DONE;
            currentToken = ERROR;
            break;
        }
        }
        if ((save) && (tokenStringIndex <= MAXTOKENLEN))
            tokenString[tokenStringIndex++] = (char)c;
        if (state == DONE)
        {
            tokenString[tokenStringIndex] = '\0';
            if (currentToken == ID)
                currentToken = reservedLookup(tokenString);
        }
    }
    if (TraceScan) {
        fprintf(listing, "\t%2d: ", lineno); // 打印 token 所属行号
        printToken(currentToken, tokenString); // 打印 token
    }
    return currentToken;
} /* end getToken */
