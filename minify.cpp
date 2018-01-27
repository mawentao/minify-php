/***************************************************************************
 *
 * Copyright (c) 2018 mawentao, Inc. All Rights Reserved
 * $Id: minify.cpp, v1.0.0, 2018-01-27 09:15:05, mawentao Exp $
 *
 **************************************************************************/

/**
 * @file   minify.cpp
 * @author mawentao
 * @date   2018-01-27 09:15:05
 * @brief  minify.cpp
 **/

#include "minify.h"
#include <cstdio>

/* 读取文件内容 */
void file_get_contents(const char *filepath,char *buf,int bufsize)
{/*{{{*/
    FILE *fi = fopen(filepath, "r");
    char c;
    char *p=buf;
    int t = 0;
    while ((c=getc(fi)) != EOF) {
        *p++ = c;
        ++t;
        if (t>=bufsize-1) {
            break;
        }
    }
    fclose(fi);
    *p='\0';
}/*}}}*/

/* 写入文件 */
void file_put_contents(const char *filepath,const char *buf)
{/*{{{*/
    FILE *fo = fopen(filepath, "w");
    if (fo==NULL) {
        fprintf(stderr, "FATAL: fopen %s fail\n", filepath);
        exit(0);
    }
    const char *p = buf;
    while (*p) {
        fprintf(fo, "%c", *p);
        ++p;
    }
    fclose(fo);
}/*}}}*/

/*  删除PHP代码注释 */
void clean_php_annotation(char *buf)
{/*{{{*/
    const char *pb = buf;
    char *p = buf;
    int state = 0;
    while (*pb) {
        char c = *pb;
        switch (state) {
            case 0:
                if (c=='/') {state=1;}
                else {
                    *p++ = c;
                    if (c=='\'') state=5;
                    else if (c=='"') state = 7;
                }
                break;
            case 1:
                if (c=='/') {state=2;}
                else if (c=='*') {state=3;}
                else {
                    *p++ = '/';
                    *p++ = c;
                    state = 0;
                }
                break;
            case 2:
                if (c=='\n') {
                    *p++ = c;
                    state=0;
                }
                break;
            case 3:
                if (c=='*') state=4;
                break;
            case 4:
                if (c=='/') state=0;
                else if (c=='*') state=4;
                else state=3;
                break;
            case 5:
                *p++ = c;
                if (c =='\\') {state=6;}
                else if (c=='\'') {state=0;}
                break;
            case 6:
                *p++ = c;
                state=5;
                break;
            case 7:
                *p++ = c;
                if (c =='\\') {state=8;}
                else if (c=='"') {state=0;}
                break;
            case 8:
                *p++ = c;
                state=7;
                break;
        }
        ++pb;
    }
    *p = '\0';
}/*}}}*/

/* 删除空行,包括文件末尾的换行符 */
void clean_blank_line(char *buf)
{/*{{{*/
    char *p = buf;
    char *pf = buf;
    char *pb = buf;
    int state = 0;
    int hh = 0;
    while(*pf) {
        char c = *pf;
        switch (state) {
            case 0:
                if (c=='\r' || c=='\n') {
                }
                else if (c==' ' || c=='\t') {
                    pb = p;
                    if (hh) *p++ = '\n';
                    *p++ = c;
                    state = 1;
                }
                else {
                    if (hh) *p++ = '\n';
                    *p++ = c;
                    state = 2;
                }
                break;
            case 1:
                if (c=='\r' || c=='\n') {
                    p = pb;
                    state = 0;
                }
                else if (c==' ' || c=='\t') {
                    *p++ = c;
                }
                else {
                    *p++ = c;
                    state = 2;
                }
                break;
            case 2:
                if (*pf=='\r' || *pf=='\n') {
                    hh = 1;
                    state = 0;
                } else {
                    *p++ = *pf;
                }
                break;
        }
        ++pf;
    }
    *p = '\0';
}/*}}}*/


// vim:fdm=marker:nu:ts=4:sw=4:expandtab
