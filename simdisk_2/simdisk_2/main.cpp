#include "pch.h"
#include <iostream>
#include <string>
#include "define.h"
#include "function.h"
#include "command.h"
#include "ipc.h"

using namespace std;

fstream virtualDisk; //虚拟磁盘文件

Blockgroup blockgroup[BLOCK_GROUP_NUM]; //数据块组
Bitmap block_bitmap[TOTAL_BLOCK_NUM]; //块位图
Bitmap inode_bitmap[TOTAL_INODE_NUM]; //i节点位图
Inode inode_table[TOTAL_INODE_NUM]; //i节点表

Directory current_dir; //当前目录
char current_path[MAX_PATH_SIZE]; //当前路径

User user_group[MAX_USER_NUM]; //用户组
int current_user = 0; //当前用户
userType current_usertype; //当前用户类型

wstring m_cstrSzName[2]; //共享内存名，分别是shell传数据给simdisk和simdisk传数据给shell
wstring m_ReadSemaphoreName[2]; //读数据的信号量
wstring m_WriteSemaphoreName[2]; //写数据的信号量
HANDLE m_hMapFile[2]; //共享内存句柄
char *m_pBuf[2]; //数据缓存，m_pBuf[0]为shell向simdisk传递的数据，m_pBuf[1]为simdisk向shell传递的数据
HANDLE m_Read[2]; //读数据的信号量句柄
HANDLE m_Write[2]; //写数据的信号量句柄
char result[BUF_SIZE]; //返回结果
bool isLogin; //是否登录

int main()
{
	sys_load(); //加载初始化信息
	isLogin = false;
	
	CreateFileMap();//创建共享内存
	UINT threadId;
	//创建线程
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, WriteSharedData, 0, 0, &threadId);//创建写数据的线程
	if (hThread == NULL) {
		cout << "Starting WriteSharedData Thread  Failed!" << endl;
	}
	HANDLE hThread2 = (HANDLE)_beginthreadex(NULL, 0, ReadSharedData, 0, 0, &threadId);//创建读数据的线程
	if (hThread2 == NULL) {
		cout << "Starting ReadSharedData Thread  Failed!" << endl;
	}
	if (isLogin) {
		system("cls");
		cin.clear();
		cin.ignore();
		cout << "-------- 欢迎进入模拟Linux文件系统 --------" << endl;
	}
	Sleep(1000000);
	CloseFileMap();
}

