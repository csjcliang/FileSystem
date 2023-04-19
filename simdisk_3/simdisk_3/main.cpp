#include "pch.h"
#include <iostream>
#include <string>
#include "define.h"
#include "function.h"
#include "command.h"
#include "sync.h"

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

int main()
{
	CreateFileMap(); //创建进程同步区
	sys_load(); //加载初始化信息
	while (!login()); //进行登录
	system("cls");
	cin.clear();
	cin.ignore();
	cout << "-------- 欢迎进入模拟Linux文件系统 --------" << endl;
	int i = 0;
	stringstream sstream;
	char whole_command[MAX_WHOLE_COMMAND_SIZE], input_command[MAX_COMMAND_SIZE], input_para1[MAX_PATH_SIZE], input_para2[MAX_PATH_SIZE]; //命令和参数
	//循环输入命令
	while (1) {
		cout << endl;
		cout << current_path << ">"; //当前路径
		//获取输入命令和参数
		cin.getline(whole_command, MAX_WHOLE_COMMAND_SIZE);
		sstream << whole_command;
		sstream >> input_command >> input_para1 >> input_para2;
		if (input_command[0] == '\0') { //忽略回车
			sstream.clear();
			continue;
		}

		if (!sstream.eof()) { //输入参数超过两个
			cout << "命令后输入的参数不能超过两个" << endl;
			sstream.str("");
			sstream.clear();
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
				cout << "info命令参数过多，请重新输入" << endl;
			}
			else {
				info();
			}
			break;
		case 1: //cd 改变目录
			if (input_para2[0] != '\0') {
				cout << "cd命令参数过多，请重新输入" << endl;
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
				cout << "dir属性参数错误，请重新输入" << endl;
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
				cout << "md命令参数过少，请重新输入" << endl;
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
					cout << "md命令属性参数错误，请重新输入" << endl;
					break;
				}
			}
			else { //默认参数为可读写
				md(input_para1, READ_WRITE);
			}
			break;
		case 4: //rd 删除目录
			if (input_para1[0] == '\0') {
				cout << "rd命令参数过少，请重新输入" << endl;
			}
			else if (input_para2[0] != '\0') {
				cout << "rd命令参数过多，请重新输入" << endl;
			}
			else {
				rd(input_para1);
			}
			break;
		case 5: //newfile 建立文件
			if (input_para1[0] == '\0') {
				cout << "newfile命令参数过少，请重新输入" << endl;
			}
			else if (input_para2[0] != '\0') {
				switch (atoi(input_para2)) //属性参数
				{
				case 0:
					newfile(input_para1, READ_ONLY);
					break;
				case 1:
					newfile(input_para1, WRITE_ONLY);
					break;
				case 2:
					newfile(input_para1, READ_WRITE);
					break;
				default:
					cout << "newfile命令属性参数错误，请重新输入" << endl;
					break;
				}
			}
			else { //默认参数为可读写
				newfile(input_para1, READ_WRITE);
			}
			break;
		case 6: //cat 打开文件
			if (input_para1[0] == '\0') {
				cout << "cat命令参数过少，请重新输入" << endl;
			}
			else if (input_para2[0] != '\0') {
				cout << "cat命令参数过多，请重新输入" << endl;
			}
			else {
				cat(input_para1);
			}
			break;
		case 7: //copy 拷贝文件
			if (input_para1[0] == '\0' || input_para2[0] == '\0') {
				cout << "copy命令参数过少，请重新输入" << endl;
			}
			else {
				copy(input_para1, input_para2);
			}
			break;
		case 8: //del 删除文件
			if (input_para1[0] == '\0') {
				cout << "del命令参数过少，请重新输入" << endl;
			}
			else if (input_para2[0] != '\0') {
				cout << "del命令参数过多，请重新输入" << endl;
			}
			else {
				del(input_para1);
			}
			break;
		case 9: //check 检测并恢复文件系统
			if (input_para1[0] != '\0') {
				cout << "check命令参数过多，请重新输入" << endl;
			}
			else {
				check();
			}
			break;
		case 10: //help 获取帮助
			if (input_para2[0] != '\0') {
				cout << "help命令参数过多，请重新输入" << endl;
			}
			else {
				help(input_para1);
			}
			break;
		case 11: //exit 退出系统
			if (input_para1[0] != '\0') {
				cout << "exit命令参数过多，请重新输入" << endl;
			}
			else {
				exit();
			}
			break;
		case 12: //sync 同步系统
			if (input_para1[0] != '\0') {
				cout << "sync命令参数过多，请重新输入" << endl;
			}
			else {
				sync();
			}
			break;
		default:
			cout << "不存在命令或命令格式不正确，请重新输入" << endl;
		}
	}
}