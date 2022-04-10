/****************************************************/
/* File: scan.h                                     */
/* The scanner interface for the TINY compiler      */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SCAN_H_
#define _SCAN_H_

/* token的最大长度 */
#define MAXTOKENLEN 40

/* 暂存每个token串 */
extern char tokenString[MAXTOKENLEN+1];

/* 返回源代码中下一个 token */
TokenType getToken(void);

#endif
