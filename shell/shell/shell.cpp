#include "pch.h"
#include <iostream>
#include <windows.h>
#include <process.h>
using namespace std;

#define BUF_SIZE 102400 //缓存大小
wstring m_cstrSzName[2]; //共享内存，分别是shell传数据给simdisk和simdisk传数据给shell
wstring m_ReadSemaphoreName[2]; //读数据的信号量
wstring m_WriteSemaphoreName[2]; //写数据的信号量
HANDLE m_hMapFile[2]; //共享内存句柄
char *m_pBuf[2]; //数据缓存，m_pBuf[0]为shell向simdisk传递的数据，m_pBuf[1]为simdisk向shell传递的数据
HANDLE m_Read[2]; //读数据的信号量句柄
HANDLE m_Write[2]; //写数据的信号量句柄
bool isLogin; //是否登录

//关闭文件映射
void CloseFileMap()
{
	for (int i = 0; i < 2; i++) {
		//撤销文件视图
		UnmapViewOfFile(m_pBuf[i]);
		//关闭映射文件句柄
		CloseHandle(m_hMapFile[i]);
	}
}

//创建文件映射
void CreateFileMap()
{
	m_cstrSzName[0] = L"NameOfMappingObject0"; //共享内存0
	m_cstrSzName[1] = L"NameOfMappingObject1"; //共享内存1
	m_ReadSemaphoreName[0] = L"ReadSemaphoreName0"; //读信号量0
	m_ReadSemaphoreName[1] = L"ReadSemaphoreName1"; //读信号量1
	m_WriteSemaphoreName[0] = L"WriteSemaphoreName0"; //写信号量0
	m_WriteSemaphoreName[1] = L"WriteSemaphoreName1"; //写信号量1
	for (int i = 0; i < 2; i++) {
		//创建共享文件句柄
		//创建一个文件映射内核对象
		m_hMapFile[i] = CreateFileMapping(
			INVALID_HANDLE_VALUE,     //物理文件句柄，创建一个进程间共享的对象
			NULL,				      //默认安全级别
			PAGE_READWRITE,           //权限可读可写
			0,						  //高位文件大小
			BUF_SIZE,				  //低位文件大小
			m_cstrSzName[i].c_str()	  //共享内存名
		);
		//获取指向文件视图的指针
		//把文件数据映射到进程的地址空间
		m_pBuf[i] = (char*)MapViewOfFile(
			m_hMapFile[i],			  //共享内存的句柄
			FILE_MAP_ALL_ACCESS,      //可读写
			0,
			0,
			BUF_SIZE
		);
	}
	//初始化信号量，初始置m_Write[0]为1，只允许shell向共享内存放数据，即等待shell的输入
	m_Read[0] = CreateSemaphore(NULL, 0, 1, m_ReadSemaphoreName[0].c_str());
	m_Write[0] = CreateSemaphore(NULL, 1, 1, m_WriteSemaphoreName[0].c_str());
	m_Read[1] = CreateSemaphore(NULL, 0, 1, m_ReadSemaphoreName[1].c_str());
	m_Write[1] = CreateSemaphore(NULL, 0, 1, m_WriteSemaphoreName[1].c_str());
}

//写共享内存
unsigned int __stdcall WriteSharedData(void *pPM)
{
	//将数据放到共享内存
	while (true) {
		//等待允许shell放数据
		WaitForSingleObject(m_Write[0], INFINITE);
		if (!isLogin) {
			cout << "请输入账号: ";
			char userid[BUF_SIZE] = { 0 };
			gets_s(userid, BUF_SIZE);
			cout << "请输入密码: ";
			char password[BUF_SIZE] = { 0 };
			gets_s(password, BUF_SIZE);
			char *temp = strcat(userid, " ");
			char szInfo[BUF_SIZE] = { 0 };
			strcpy(szInfo, strcat(temp, password));
			memcpy(m_pBuf[0], szInfo, BUF_SIZE - 1);
		}
		else {
			cout << "请输入指令" << endl;
			char szInfo[BUF_SIZE] = { 0 };
			gets_s(szInfo, BUF_SIZE);
			memcpy(m_pBuf[0], szInfo, BUF_SIZE - 1);
		}
		//允许simdisk读数据
		ReleaseSemaphore(m_Read[0], 1, NULL);
	}
	return true;
}

//读共享内存
unsigned int __stdcall ReadSharedData(void *pPM)
{
	//从共享内存中读取数据
	while (true) {
		//等待允许shell读取simdisk返回的数据
		WaitForSingleObject(m_Read[1], INFINITE);
		if (!isLogin) {
			cout << m_pBuf[1] << endl << endl;
			if (strcmp(m_pBuf[1], "登录成功!") == 0) {
				isLogin = true;
				cout << "正在进入shell命令解释器……" << endl;
				Sleep(1000);
				system("cls");
				cout<< "-------- 欢迎使用模拟Linux文件系统shell命令解释器 --------" << endl;
			}
		}
		else {
			//返回结果不为空
			if (m_pBuf[1][0] != '\0') {
				cout << "simdisk返回结果如下: " << endl;
				cout << m_pBuf[1] << endl << endl;
			}
			if (strcmp(m_pBuf[1], "后台文件系统已退出，shell进程即将退出……") == 0) { //接收到simdisk的exit指令
				CloseFileMap();
				Sleep(2000);
				exit(0);
			}
		}
		//允许shell放数据
		ReleaseSemaphore(m_Write[0], 1, NULL);
	}
	return true;
}

int main()
{
	cout << "-------- 欢迎进入模拟Linux文件系统shell命令解释器 --------" << endl;
	isLogin = false;
	cout << "请进行登录" << endl;
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
	Sleep(1000000);
	CloseFileMap();
	return 0;
}
