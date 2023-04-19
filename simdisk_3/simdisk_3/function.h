#pragma once

#include "define.h"

//加载初始化信息
void sys_load();
//初始化文件系统
void sys_init();
//初始化用户信息
void init_user();
//登录
bool login();
//分配盘块
long block_alloc(int b_len, int &b_index);
//释放盘块
void block_free(int b_len, int b_index);
//分配i节点
int inode_alloc();
//释放i节点
void inode_free(int f_inode);
//显示当前路径
void show_cur_path(Directory cur_dir);
//从路径中提取目录名
bool get_dirname(const char *path, int len, int pos, char *filename);
//根据路径寻找对应目录
bool find_path_dir(const char *path, int len, int pos, char *dirname, Directory &tempdir);
//根据路径提取出最终目录及目录名文件名
bool get_dir_and_fdname(const char *path, int len, Directory &tempdir, char *filename);
//根据路径提取出最终目录
bool get_dir(const char *path, int len, Directory &tempdir);
//判断路径是否为host文件系统路径
bool host_path(char *path);
