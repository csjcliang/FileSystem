#pragma once

#include "define.h"

#define COMMAND_NUM 13

//文件系统命令
static const char *command[COMMAND_NUM] = { "info", "cd", "dir", "md", "rd", "newfile", "cat", "copy", "del", "check", "help", "exit", "sync"};

//info 显示整个系统信息
void info();

//cd 改变目录
void cd(const char *path);

//dir 显示目录
void dir(const char *path, int is_subdir);

//md 创建目录
void md(const char *path, fileAccess fileaccess);

//rd 删除目录
void rd(const char *path);

//newfile 建立文件
void newfile(const char *path, fileAccess fileaccess);

//cat 打开文件
void cat(const char *path);

//copy 拷贝文件
void copy(const char *srcpath, const char *despath);

//del 删除文件
void del(const char *path);

//check 检测并恢复文件系统
void check();

//help 获取帮助
void help(const char *command);

//exit 退出系统
void exit();

//sync 同步系统
void sync();