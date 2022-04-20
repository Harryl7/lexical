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

/* �����ֻ� id �� lexeme */
char tokenString[MAXTOKENLEN + 1];

/* BUFLEN = Դ����ÿһ�����뻺�����ĳ��� */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* �����л����� */
static int linepos = 0; /* ָ�� lineBuf ��һ������ȡ�ַ� */
static int bufsize = 0; /* ��ǰ�����ַ�����ʵ�ʴ�С */
static int EOF_flag = FALSE; /* corrects ungetNextChar behavior on EOF */

/* �� lineBuf �л�ȡ��һ���ǿ��ַ�
��� lineBuf �ľ������ȡ�����ٷ���
��������ļ�ĩβ������ EOF ���� EOF ��־��λ*/
static int getNextChar(void)
{
    if (!(linepos < bufsize)) // ������
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
    else return lineBuf[linepos++]; // ֱ�ӷ�����һ���ַ�
}

/* ָ����˵���һ���ַ������û�е����ļ�ĩβ�� */
static void ungetNextChar(void)
{
    if (!EOF_flag) linepos--;
}

/* ������ӳ��� */
static struct
{
    char* str;
    TokenType tok;
} reservedWords[MAXRESERVED] = {
    {"if",IF}, {"then",THEN}, {"else",ELSE}, {"end",END},
    {"repeat",REPEAT}, {"until",UNTIL}, {"read",READ},
    {"write",WRITE}
};

/* ��ѯһ�� ID �� token �Ƿ�Ϊ�����֣����Բ��ң����������� */
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

/* ����Դ��������һ�� token */
TokenType getToken(void)
{  
    /* tokenString �д����һ���ַ����±� */
    int tokenStringIndex = 0;
    /* �ݴ浱ǰҪ���ص� token */
    TokenType currentToken;
    /* current state - always begins at START */
    StateType state = START;
    /* �ɱ��浽 tokenString �ı�־ */
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
            else // �ǣ����֡���ĸ��':' ��'{' ��' ' ��'\t' ��'\n'��'/'
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
            //{ // ��������ո񡢷ֺŻ�ע�͵Ŀ�ͷ
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
        fprintf(listing, "\t%2d: ", lineno); // ��ӡ token �����к�
        printToken(currentToken, tokenString); // ��ӡ token
    }
    return currentToken;
} /* end getToken */
