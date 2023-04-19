#include "pch.h"
#include <iostream>
#include "function.h"
#include "command.h"
#include "sync.h"
using namespace std;

wstring m_cstrSzName; //共享内存名
wstring m_mutexName; //互斥量名
HANDLE m_hMapFile; //共享区句柄
HANDLE m_hMutex; //用于同步的互斥量
Shared_Memory *m_pBuf; //共享区指针

//关闭文件映射（共享区）
void CloseFileMap()
{
	//关闭映射文件句柄
	//撤销文件视图
	if (m_hMutex) {
		CloseHandle(m_hMutex);
	}
	UnmapViewOfFile(m_pBuf);
	CloseHandle(m_hMapFile);
}

//创建文件映射（共享区）
bool CreateFileMap()
{
	m_cstrSzName = L"NameOfSharedMemory"; //共享内存
	m_mutexName = L"NameOfMutex"; //互斥量
	bool is_init = false;

	//创建共享区
	m_hMapFile = CreateFileMapping(
		(HANDLE)0xFFFFFFFF, 
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(Shared_Memory),
		m_cstrSzName.c_str()
	);

	if (GetLastError() == ERROR_ALREADY_EXISTS) { //已经创建共享区
		m_hMapFile = OpenFileMapping(FILE_MAP_WRITE, FALSE, m_cstrSzName.c_str());
	}
	else { //未初始化共享区
		is_init = true;
	}

	//创建互斥量
	m_hMutex = CreateMutex(NULL, FALSE, m_mutexName.c_str());

	//获取指向文件视图的指针
	m_pBuf = (Shared_Memory*)MapViewOfFile(
		m_hMapFile,
		FILE_MAP_WRITE, //可写
		0,
		0,
		sizeof(Shared_Memory)
	);

	if (is_init) { //未初始化共享区则进行初始化
		int i = 0;
		for (i = 0; i < MAX_USER_NUM * 2; i++) {
			m_pBuf->using_inode[i] = -1;
		}
		for (i = 0; i < MAX_USER_NUM; i++) {
			m_pBuf->logined_user[i] = -1;
		}
	}
	return is_init;
}

//写入共享区用户登录信息
bool set_logined_user(int userid, bool user_logined)
{
	if (user_logined == true) { //增加用户登录信息
		//写入用户登录信息
		if (WaitForSingleObject(m_hMutex, 200) == WAIT_OBJECT_0) {
			for (int i = 0; i < MAX_USER_NUM; i++) {
				if (m_pBuf->logined_user[i] == -1) {
					m_pBuf->logined_user[i] = userid;
					ReleaseMutex(m_hMutex);
					return true;
				}
			}
			ReleaseMutex(m_hMutex);
			return false;
		}
		else {
			return false;
		}
	}
	else {
		//从用户登录信息中删除
		if (WaitForSingleObject(m_hMutex, 200) == WAIT_OBJECT_0) {
			for (int i = 0; i < MAX_USER_NUM; i++) {
				if (m_pBuf->logined_user[i] == userid) {
					m_pBuf->logined_user[i] = -1;
					ReleaseMutex(m_hMutex);
					return true;
				}
			}
			ReleaseMutex(m_hMutex);
			return false;
		}
		else {
			return false;
		}
	}
}

//获取共享区用户登录信息
bool get_logined_user(int userid)
{
	//读取用户登录信息
	if (WaitForSingleObject(m_hMutex, 200) == WAIT_OBJECT_0) {
		//判断用户是否已登录
		for (int i = 0; i < MAX_USER_NUM; i++) {
			if (m_pBuf->logined_user[i] == userid) {
				ReleaseMutex(m_hMutex);
				return true;
			}
		}
		ReleaseMutex(m_hMutex);
		return false;
	}
	else {
		return true;
	}
}

//分配共享区i节点使用表内容
int sharedm_alloc(int inode_no)
{
	int index = -1; //i节点在共享区位置
	//写入共享区
	if (WaitForSingleObject(m_hMutex, 200) == WAIT_OBJECT_0) { //等待对象变为有信号状态
		for (int i = 0; i < MAX_USER_NUM * 2; i++) {
			if (m_pBuf->using_inode[i] == -1) {
				m_pBuf->using_inode[i] = inode_no;
				index = i;
				break;
			}
		}
		ReleaseMutex(m_hMutex);
	}
	return index;
}

//释放共享区i节点使用表内容
bool sharedm_free(int index)
{
	if (index < 0) return false;
	//从共享区中清除
	if (WaitForSingleObject(m_hMutex, 200) == WAIT_OBJECT_0) { //等待对象变为有信号状态
		m_pBuf->using_inode[index] = -1;
		ReleaseMutex(m_hMutex);
		return true;
	}
	return false;
}

//读取共享区i节点使用表
bool sharedm_read(int inode_no)
{
	//读取信息
	if (WaitForSingleObject(m_hMutex, 200) == WAIT_OBJECT_0) {
		//判断i节点是否正在写入
		for (int i = 0; i < MAX_USER_NUM * 2; i++) {
			if (m_pBuf->using_inode[i] == inode_no) {
				ReleaseMutex(m_hMutex);
				return true;
			}
		}
		ReleaseMutex(m_hMutex);
		return false;
	}
	else {
		return true;
	}
}

