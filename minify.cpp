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

/* 删除空行,包括文件末尾的换行符 */
void clean_blank_line(char *buf)
{
    char *p = buf;
    char *pf = buf;
    int state = 0;
    int hh = 0;
    while(*pf) {
        switch (state) {
            case 0:
                if (*pf=='\r' || *pf=='\n') {

                } else {
                    if (hh) *p++ = '\n';
                    *p++ = *pf;
                    state = 1;
                }
                break;
            case 1:
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
}


// vim:fdm=marker:nu:ts=4:sw=4:expandtab
