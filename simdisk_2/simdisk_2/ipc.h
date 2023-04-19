#include "pch.h"
#include <iostream>
#include <windows.h>
#include <process.h>
using namespace std;

#define BUF_SIZE 102400
extern wstring m_cstrSzName[2]; //共享内存名，分别是shell传数据给simdisk和simdisk传数据给shell
extern wstring m_ReadSemaphoreName[2]; //读数据的信号量
extern wstring m_WriteSemaphoreName[2]; //写数据的信号量
extern HANDLE m_hMapFile[2]; //共享内存句柄
extern char *m_pBuf[2]; //数据缓存，m_pBuf[0]为shell向simdisk传递的数据，m_pBuf[1]为simdisk向shell传递的数据
extern HANDLE m_Read[2]; //读数据的信号量句柄
extern HANDLE m_Write[2]; //写数据的信号量句柄
extern char result[BUF_SIZE]; //返回结果
extern bool isLogin; //是否登录

void CloseFileMap(); //关闭文件映射

void CreateFileMap(); //创建文件映射

unsigned int __stdcall WriteSharedData(void *pPM); //写共享内存

unsigned int __stdcall ReadSharedData(void *pPM); //读共享内存
