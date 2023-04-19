#include "pch.h"
#include "ipc.h"
#include <string>
#include "define.h"
#include "function.h"
#include "command.h"
using namespace std;

//关闭文件映射
void CloseFileMap()
{
	for (int i = 0; i < 2; i++) {
		//4.撤销文件视图UnMapViewOfFile()
		UnmapViewOfFile(m_pBuf[i]);
		//5.关闭映射文件句柄CloseHandle()
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
		//等待允许simdisk放数据
		WaitForSingleObject(m_Write[1], INFINITE);
		memcpy(m_pBuf[1], result, BUF_SIZE - 1);
		//允许shell读数据
		ReleaseSemaphore(m_Read[1], 1, NULL);
	}
	return true;
}

//读共享内存
unsigned int __stdcall ReadSharedData(void *pPM)
{
	int i = 0;
	stringstream sstream;
	char input_command[MAX_COMMAND_SIZE], input_para1[MAX_PATH_SIZE], input_para2[MAX_PATH_SIZE]; //命令和参数
	char input_userid[MAX_USERNAME_SIZE], input_password[MAX_PASSWORD_SIZE]; //账号和密码
	//从共享内存中读取数据
	while (true) {
		//等待允许simdisk读取shell放的数据
		WaitForSingleObject(m_Read[0], INFINITE);
		if (!isLogin) {
			sstream << m_pBuf[0];
			sstream >> input_userid >> input_password;
			result[0] = '\0';
			if (input_userid[0] == '\0') { //忽略回车
				sstream.clear();
				ReleaseSemaphore(m_Write[1], 1, NULL);
				continue;
			}
			sstream.clear();
			isLogin = login(input_userid, input_password);
		}
		else {
			cout << endl;
			cout << current_path << ">"; //当前路径

			//获取输入命令和参数
			cout << m_pBuf[0];
			sstream << m_pBuf[0];
			sstream >> input_command >> input_para1 >> input_para2;
			result[0] = '\0';
			if (input_command[0] == '\0') { //忽略回车
				sstream.clear();
				ReleaseSemaphore(m_Write[1], 1, NULL);
				continue;
			}

			if (!sstream.eof()) { //输入参数超过两个
				strcpy(result, "命令后输入的参数不能超过两个");
				sstream.str("");
				sstream.clear();
				ReleaseSemaphore(m_Write[1], 1, NULL);
				continue;
			}
			sstream.clear();

			//匹配命令
			for (i = 0; i < COMMAND_NUM; i++) {
				if (strcmp(input_command, command[i]) == 0) break; //匹配到命令
			}
			switch (i)
			{
			case 0: //info 显示整个系统信息
				if (input_para1[0] != '\0') {
					strcpy(result, "info命令参数过多，请重新输入");
				}
				else {
					info();
				}
				break;
			case 1: //cd 改变目录
				if (input_para2[0] != '\0') {
					strcpy(result, "cd命令参数过多，请重新输入");
				}
				else {
					cd(input_para1);
				}
				break;
			case 2: //dir 显示目录
				if (strcmp(input_para2, "/s") == 0) {
					dir(input_para1, 1);
				}
				else if (input_para2[0] != '\0' && strcmp(input_para2, "/s") != 0) {
					strcpy(result, "dir属性参数错误，请重新输入");
				}
				else if (input_para1[0] == '\0') { //默认参数为当前目录
					dir(".", 0);
				}
				else {
					dir(input_para1, 0);
				}
				break;
			case 3: //md 创建目录
				if (input_para1[0] == '\0') {
					strcpy(result, "md命令参数过少，请重新输入");
				}
				else if (input_para2[0] != '\0') {
					switch (atoi(input_para2)) //属性参数
					{
					case 0:
						md(input_para1, READ_ONLY);
						break;
					case 1:
						md(input_para1, WRITE_ONLY);
						break;
					case 2:
						md(input_para1, READ_WRITE);
						break;
					default:
						strcpy(result, "md命令属性参数错误，请重新输入");
						break;
					}
				}
				else { //默认参数为可读写
					md(input_para1, READ_WRITE);
				}
				break;
			case 4: //rd 删除目录
				if (input_para1[0] == '\0') {
					strcpy(result, "rd命令参数过少，请重新输入");
				}
				else if (input_para2[0] != '\0') {
					strcpy(result, "rd命令参数过多，请重新输入");
				}
				else {
					rd(input_para1);
				}
				break;
			case 6: //cat 打开文件
				if (input_para1[0] == '\0') {
					strcpy(result, "cat命令参数过少，请重新输入");
				}
				else if (input_para2[0] != '\0') {
					strcpy(result, "cat命令参数过多，请重新输入");
				}
				else {
					cat(input_para1);
				}
				break;
			case 7: //copy 拷贝文件
				if (input_para1[0] == '\0' || input_para2[0] == '\0') {
					strcpy(result, "copy命令参数过少，请重新输入");
				}
				else {
					copy(input_para1, input_para2);
				}
				break;
			case 8: //del 删除文件
				if (input_para1[0] == '\0') {
					strcpy(result, "del命令参数过少，请重新输入");
				}
				else if (input_para2[0] != '\0') {
					strcpy(result, "del命令参数过多，请重新输入");
				}
				else {
					del(input_para1);
				}
				break;
			case 9: //check 检测并恢复文件系统
				if (input_para1[0] != '\0') {
					strcpy(result, "check命令参数过多，请重新输入");
				}
				else {
					check();
				}
				break;
			case 10: //help 获取帮助
				if (input_para2[0] != '\0') {
					strcpy(result, "help命令参数过多，请重新输入");
				}
				else {
					help(input_para1);
				}
				break;
			case 11: //exit 退出系统
				if (input_para1[0] != '\0') {
					strcpy(result, "exit命令参数过多，请重新输入");
				}
				else {
					exit();
				}
				break;
			default:
				strcpy(result, "不存在命令或命令格式不正确，请重新输入");
			}
			
		}
		//允许simdisk放数据
		ReleaseSemaphore(m_Write[1], 1, NULL);
	}
	return true;
}
