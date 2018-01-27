/***************************************************************************
 *
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 * $Id: main.cpp, v1.0.0, 2015-12-31 17:29:31, mawentao(mawt@youzu.com) Exp $
 *
 **************************************************************************/

/**
 * @file   main.cpp
 * @author mawentao(mawt@youzu.com)
 * @date   2015-12-31 17:29:31
 * @brief  主程序入口
 **/

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "minify.h"
using namespace std;

#define VERSION "1.0.0"

struct conf_opts conf = {
    ".",          //默认统计当前目录
    "php",        //所有类型文件
    0,            //包括子目录
    0             //覆盖文件
};

void print_usage(const char *pname)
{/*{{{*/
    cout<<pname<<" "<<VERSION<<endl;
    cout<<"[usage]: "<<pname<<" [-d directory] [-f filetype] [-r] [-w]\n";
    exit(0);
}/*}}}*/

/** 解析命令行参数，读取配置并初始化 */
int parse_args(int argc, char *argv[])
{/*{{{*/
    if (argc<2) print_usage(argv[0]);
    const char *shortopts = "d:f:rwh";
    struct option longopts[] = {
        {"directory",  required_argument, NULL, 'd'},
        {"filetype",   required_argument, NULL, 'f'},
        {"writeback",  no_argument, NULL, 'w'},
        {"recursive",  no_argument, NULL, 'r'},
        {"help",       no_argument, NULL, 'h'}
    };
    int c;
    while ((c = getopt_long (argc, argv, shortopts, longopts, NULL)) != -1)
    {
        switch (c)
        {       
            case 'd':
                conf.directory = optarg; 
                break;  
            case 'f':
                conf.filetype = optarg; 
                break;  
            case 'r':
                conf.recursive = 1;
                break;
            case 'w':
                conf.writeback = 1;
                break;
            case '?':
            case 'h':
                print_usage(argv[0]);
                exit(1);
                break;  
        }       
    }
    return 0;
}/*}}}*/

/* 判断str是否以suffix结尾 */
bool endwith(const char *str, const char *suffix)
{/*{{{*/
    size_t n1 = strlen(str);
    size_t n2 = strlen(suffix);
    if(n1<n2){return false;} 
    const char *ps = str+(n1-n2);
    const char *pd = suffix; 
    while(*ps && *ps==*pd){
        ++ps;   
        ++pd;   
    }
    return !(*ps); 
}/*}}}*/

/* 判断str是否以prefix开头 */
bool beginwith(const char *str, const char *prefix)
{/*{{{*/
    const char *ps = str;
    const char *pf = prefix; 
    while(*ps && *pf && *ps==*pf){
        ++ps;   
        ++pf;   
    }
    return !(*pf); 
}/*}}}*/

/* 获取文件属性： 0-不存在； 1-目录文件； 2-普通文件*/
int get_file_attribute(const char *str)
{/*{{{*/
     struct stat buf;
     if((stat(str, &buf)!=-1)){
         return S_ISDIR(buf.st_mode)?1:2;
     }
     else{   
         return 0;
     }
}/*}}}*/

/* 计算一个文件中的代码行数，注释行数，以及总行数（不计空格） */ 
void count(const char *fname, int &tcl, int &tal, int &tl)
{/*{{{*/
     tcl = tal = tl = 0; 
     FILE *fp;
     char c;
     int state = 1;
     fp = fopen(fname, "r"); // 以“只读”形式打开件
     if(fp == NULL) return;
     while((c=getc(fp)) != EOF){
        switch(state){
        case 1:
             if(c==' '||c=='\t'||c=='\n') state = 1;
             else if(c=='/') state=3;
             else state = 2;
             break;
        case 2:
             if(c == '\n') {
                 state = 1;
                 tcl++;
             }
             break;
        case 3:
             if(c == '\n'){
                 tcl++;
                 state = 1;
             }
             else if(c == '/') state = 4;
             else if(c == '*') state = 5;
             else state=2;
             break;
        case 4:
             if(c == '\n'){
                 state = 1;
                 tal++;
             }
             break;
        case 5:
             if(c == '\n') tal++;
             else if(c == '*') state = 6;
             break;
        case 6:
             if(c == '\n') {
                 tal++;
                 state = 5;
             }
             else if(c == '/') state = 7;
             else if(c == '*') state = 6;
             else state = 5;
             break;
        case 7:
             if(c == '\n'){
                 state = 1;
                 tal++;
             }
             else if(c == '/') state = 8;
             break;
        case 8:
             if(c == '\n'){
                 state = 1;
                 tal++;
             }
             else if(c == '*') state = 5;
             break;
        }
     }
     if(state == 2 || state == 3) tcl++;
     else if(state != 1)tal++;
     fclose(fp); // 关闭文件
     tl = tcl+tal;
}/*}}}*/

/* 获取文件大小 */
int get_size(const char *fname)
{/*{{{*/
    int size=0;
    FILE *fi;
    char c;
    fi = fopen(fname, "r");
    if(fi == NULL) return 0;
    while ((c=getc(fi)) != EOF) {
        ++size;
    }
    fclose(fi);
    return size;
}/*}}}*/

void process_file(const char *fname)
{
    //1. create memory
    int size = get_size(fname);
    char *buf = new char[size+1024];
    if (!buf) {
        fprintf(stderr, "FATAL: new char[%d] fail\n", size);
        exit(0);
    }
    printf("minify-php: %s (%d B)\n", fname, size);

    //2. 读取文件内容
    file_get_contents(fname,buf,size);
    //3. 删除PHP代码注释
    clean_php_annotation(buf);
    //4. 删除空行及文件末尾多余的换行符
    clean_blank_line(buf);
    //5. write back
    if (!conf.writeback) {
        printf("%s\n",buf);
    } else {
        file_put_contents(fname,buf);
    }
    delete []buf;
}

void do_clear(const char *root, const char *ftype, int recursive)
{
    //1. 打开目录
    DIR * p_dir=0;
    p_dir = opendir(root);
    if(NULL == p_dir){
        fprintf(stderr, ">>> [Error]: can not open directory. [%s]\n", root);
        return;
    }

    //2. 扫描目录下的文件
    dirent * p_dirent=0;
    char filename[1024];
    while((p_dirent=readdir(p_dir))){
        sprintf(filename, "%s/%s", root, p_dirent->d_name);
        switch(get_file_attribute(filename)){
        case 1:
             //printf("%s:目录\n", filename);
             if(strcmp(p_dirent->d_name, ".") != 0 &&
                strcmp(p_dirent->d_name, "..") != 0 &&
                recursive
             ){
                 do_clear(filename, ftype, recursive);
             }
             break;
        case 2:
             if( endwith(filename, ftype) ) {
                 process_file(filename);
             }
             break;
        }
    }

    //3. 关闭目录
    closedir(p_dir);
    p_dir = 0;
}

/** 主程序 */
int main(int argc, char *argv[])
{
    //1. 解析命令行参数，读取配置并初始化
    int ret = parse_args(argc, argv);
    if(0 != ret){
        printf("parse_args fail.");
    }

    do_clear(conf.directory.c_str(), conf.filetype.c_str(), conf.recursive);

    cout<<"!!!THE END!!!"<<endl;
    return 0;
}
// vim:fdm=marker:nu:ts=4:sw=4:expandtab
