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
 * @brief  ���������
 **/

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "minify.h"
using namespace std;

#define VERSION "1.0.0"

struct conf_opts conf = {
    ".",          //Ĭ��ͳ�Ƶ�ǰĿ¼
    "php",        //���������ļ�
    0,            //������Ŀ¼
    0             //�����ļ�
};

void print_usage(const char *pname)
{/*{{{*/
    cout<<pname<<" "<<VERSION<<endl;
    cout<<"[usage]: "<<pname<<" [-d directory] [-f filetype] [-r] [-w]\n";
    exit(0);
}/*}}}*/

/** ���������в�������ȡ���ò���ʼ�� */
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

/* �ж�str�Ƿ���suffix��β */
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

/* �ж�str�Ƿ���prefix��ͷ */
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

/* ��ȡ�ļ����ԣ� 0-�����ڣ� 1-Ŀ¼�ļ��� 2-��ͨ�ļ�*/
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

/* ����һ���ļ��еĴ���������ע���������Լ������������ƿո� */ 
void count(const char *fname, int &tcl, int &tal, int &tl)
{/*{{{*/
     tcl = tal = tl = 0; 
     FILE *fp;
     char c;
     int state = 1;
     fp = fopen(fname, "r"); // �ԡ�ֻ������ʽ�򿪼�
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
     fclose(fp); // �ر��ļ�
     tl = tcl+tal;
}/*}}}*/

/* ��ȡ�ļ���С */
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

    //2. ��ȡ�ļ�����
    file_get_contents(fname,buf,size);
    //3. ɾ��PHP����ע��
    clean_php_annotation(buf);
    //4. ɾ�����м��ļ�ĩβ����Ļ��з�
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
    //1. ��Ŀ¼
    DIR * p_dir=0;
    p_dir = opendir(root);
    if(NULL == p_dir){
        fprintf(stderr, ">>> [Error]: can not open directory. [%s]\n", root);
        return;
    }

    //2. ɨ��Ŀ¼�µ��ļ�
    dirent * p_dirent=0;
    char filename[1024];
    while((p_dirent=readdir(p_dir))){
        sprintf(filename, "%s/%s", root, p_dirent->d_name);
        switch(get_file_attribute(filename)){
        case 1:
             //printf("%s:Ŀ¼\n", filename);
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

    //3. �ر�Ŀ¼
    closedir(p_dir);
    p_dir = 0;
}

/** ������ */
int main(int argc, char *argv[])
{
    //1. ���������в�������ȡ���ò���ʼ��
    int ret = parse_args(argc, argv);
    if(0 != ret){
        printf("parse_args fail.");
    }

    do_clear(conf.directory.c_str(), conf.filetype.c_str(), conf.recursive);

    cout<<"!!!THE END!!!"<<endl;
    return 0;
}
// vim:fdm=marker:nu:ts=4:sw=4:expandtab
