/****************************************************/
/* File: scan.h                                     */
/* The scanner interface for the TINY compiler      */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SCAN_H_
#define _SCAN_H_

/* token����󳤶� */
#define MAXTOKENLEN 40

/* �ݴ�ÿ��token�� */
extern char tokenString[MAXTOKENLEN+1];

/* ����Դ��������һ�� token */
TokenType getToken(void);

#endif
