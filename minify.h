/***************************************************************************
 *
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 * $Id: clear_annotation.h, v1.0.0, 2015-12-31 17:35:08, mawentao(mawt@youzu.com) Exp $
 *
 **************************************************************************/

/**
 * @file   clear_annotation.h
 * @author mawentao(mawt@youzu.com)
 * @date   2015-12-31 17:35:08
 * @brief  clear_annotation.h
 **/

#ifndef CLEAR_ANNOTATION_H
#define CLEAR_ANNOTATION_H


#include <cstdlib>
#include <iostream>
#include <cstring>
#include <getopt.h>
#include <sys/stat.h>
#include <dirent.h>

using std::string;

struct conf_opts{
    string directory;     /* 统计的文件目录(.) */
    string filetype;      /* 统计的文件类型 (*)*/
    int recursive;        /* 查找子目录(默认true) */
    int writeback;        /* 写回文件 */
};

/* 读取文件内容 */
void file_get_contents(const char *filepath,char *buf,int bufsize);

/* 写入文件 */
void file_put_contents(const char *filepath,const char *buf);

/* 删除PHP代码注释 */
void clean_php_annotation(char *buf);

/* 删除空行,包括文件末尾的换行符 */
void clean_blank_line(char *);


#endif

// vim:fdm=marker:nu:ts=4:sw=4:expandtab
