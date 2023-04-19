#include "pch.h"
#include "define.h"
#include <windows.h>
#pragma once

//共享区
class Shared_Memory
{
public:
	int logined_user[MAX_USER_NUM]; //登录用户
	int using_inode[MAX_USER_NUM * 2]; //正在被使用的i节点
};

//关闭文件映射（共享区）
void CloseFileMap();

//创建文件映射（共享区）
bool CreateFileMap();

//写入共享区用户登录信息
bool set_logined_user(int userid, bool user_logined);

//获取共享区用户登录信息
bool get_logined_user(int userid);

//分配共享区i节点使用表内容
int sharedm_alloc(int inode_no);

//释放共享区i节点使用表内容
bool sharedm_free(int index);

//读取共享区i节点使用表
bool sharedm_read(int inode_no);

extern wstring m_cstrSzName; //共享内存名
extern wstring m_mutexName; //互斥量名
extern HANDLE m_hMapFile; //共享区句柄
extern HANDLE m_hMutex; //用于同步的互斥量
extern Shared_Memory *m_pBuf; //共享区指针
