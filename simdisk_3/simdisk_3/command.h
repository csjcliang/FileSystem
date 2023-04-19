#pragma once

#include "define.h"

#define COMMAND_NUM 13

//�ļ�ϵͳ����
static const char *command[COMMAND_NUM] = { "info", "cd", "dir", "md", "rd", "newfile", "cat", "copy", "del", "check", "help", "exit", "sync"};

//info ��ʾ����ϵͳ��Ϣ
void info();

//cd �ı�Ŀ¼
void cd(const char *path);

//dir ��ʾĿ¼
void dir(const char *path, int is_subdir);

//md ����Ŀ¼
void md(const char *path, fileAccess fileaccess);

//rd ɾ��Ŀ¼
void rd(const char *path);

//newfile �����ļ�
void newfile(const char *path, fileAccess fileaccess);

//cat ���ļ�
void cat(const char *path);

//copy �����ļ�
void copy(const char *srcpath, const char *despath);

//del ɾ���ļ�
void del(const char *path);

//check ��Ⲣ�ָ��ļ�ϵͳ
void check();

//help ��ȡ����
void help(const char *command);

//exit �˳�ϵͳ
void exit();

//sync ͬ��ϵͳ
void sync();