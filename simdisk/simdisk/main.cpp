#include <iostream>
#include <string>
#include "define.h"
#include "function.h"
#include "command.h"

using namespace std;

fstream virtualDisk; //��������ļ�

Blockgroup blockgroup[BLOCK_GROUP_NUM]; //���ݿ���
Bitmap block_bitmap[TOTAL_BLOCK_NUM]; //��λͼ
Bitmap inode_bitmap[TOTAL_INODE_NUM]; //i�ڵ�λͼ
Inode inode_table[TOTAL_INODE_NUM]; //i�ڵ��

Directory current_dir; //��ǰĿ¼
char current_path[MAX_PATH_SIZE]; //��ǰ·��

User user_group[MAX_USER_NUM]; //�û���
int current_user = 0; //��ǰ�û�
userType current_usertype; //��ǰ�û�����

int main()
{
	sys_load(); //���س�ʼ����Ϣ
	while (!login()); //���е�¼
	system("cls");
	cin.clear();
	cin.ignore();
	cout << "-------- ��ӭ����ģ��Linux�ļ�ϵͳ --------" << endl;
	int i = 0;
	stringstream sstream;
	char whole_command[MAX_WHOLE_COMMAND_SIZE], input_command[MAX_COMMAND_SIZE], input_para1[MAX_PATH_SIZE], input_para2[MAX_PATH_SIZE]; //����Ͳ���
	//ѭ����������
	while (1) {
		cout << endl;
		cout << current_path << ">"; //��ǰ·��
		//��ȡ��������Ͳ���
		cin.getline(whole_command, MAX_WHOLE_COMMAND_SIZE); 
		sstream << whole_command;
		sstream >> input_command >> input_para1 >> input_para2;
		if (input_command[0] == '\0') { //���Իس�
			sstream.clear();
			continue;
		}
		
		if (!sstream.eof()) { //���������������
			cout << "���������Ĳ������ܳ�������" << endl;
			sstream.str("");
			sstream.clear();
			continue;
		}
		sstream.clear();

		//ƥ������
		for (i = 0; i < COMMAND_NUM; i++) {
			if (strcmp(input_command, command[i]) == 0) break; //ƥ�䵽����
		}
		switch (i)
		{
		case 0: //info ��ʾ����ϵͳ��Ϣ
			if (input_para1[0] != '\0') {
				cout << "info����������࣬����������" << endl;
			}
			else{
				info();
			}
			break;
		case 1: //cd �ı�Ŀ¼
			if (input_para2[0] != '\0') {
				cout << "cd����������࣬����������" << endl;
			}
			else{
				cd(input_para1);
			}
			break;
		case 2: //dir ��ʾĿ¼
			if (strcmp(input_para2,"/s") == 0) {
				dir(input_para1, 1);
			}
			else if (input_para2[0] != '\0' && strcmp(input_para2, "/s") != 0) {
				cout << "dir���Բ�����������������" << endl;
			}
			else if (input_para1[0] == '\0') { //Ĭ�ϲ���Ϊ��ǰĿ¼
				dir(".", 0);
			}
			else {
				dir(input_para1, 0);
			}
			break;
		case 3: //md ����Ŀ¼
			if (input_para1[0] == '\0') {
				cout << "md����������٣�����������" << endl;
			}
			else if (input_para2[0] != '\0') {
				switch (atoi(input_para2)) //���Բ���
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
					cout << "md�������Բ�����������������" << endl;
					break;
				}
			}
			else { //Ĭ�ϲ���Ϊ�ɶ�д
				md(input_para1, READ_WRITE);
			}
			break;
		case 4: //rd ɾ��Ŀ¼
			if (input_para1[0] == '\0') {
				cout << "rd����������٣�����������" << endl;
			}
			else if (input_para2[0] != '\0') {
				cout << "rd����������࣬����������" << endl;
			}
			else {
				rd(input_para1);
			}
			break;
		case 5: //newfile �����ļ�
			if (input_para1[0] == '\0') {
				cout << "newfile����������٣�����������" << endl;
			}
			else if (input_para2[0] != '\0') {
				switch (atoi(input_para2)) //���Բ���
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
					cout << "newfile�������Բ�����������������" << endl;
					break;
				}
			}
			else { //Ĭ�ϲ���Ϊ�ɶ�д
				newfile(input_para1, READ_WRITE);
			}
			break;
		case 6: //cat ���ļ�
			if (input_para1[0] == '\0') {
				cout << "cat����������٣�����������" << endl;
			}
			else if (input_para2[0] != '\0') {
				cout << "cat����������࣬����������" << endl;
			}
			else {
				cat(input_para1);
			}
			break;
		case 7: //copy �����ļ�
			if (input_para1[0] == '\0' || input_para2[0] == '\0') {
				cout << "copy����������٣�����������" << endl;
			}
			else {
				copy(input_para1, input_para2);
			}
			break;
		case 8: //del ɾ���ļ�
			if (input_para1[0] == '\0') {
				cout << "del����������٣�����������" << endl;
			}
			else if (input_para2[0] != '\0') {
				cout << "del����������࣬����������" << endl;
			}
			else {
				del(input_para1);
			}
			break;
		case 9: //check ��Ⲣ�ָ��ļ�ϵͳ
			if (input_para1[0] != '\0') {
				cout << "check����������࣬����������" << endl;
			}
			else {
				check();
			}
			break;
		case 10: //help ��ȡ����
			if (input_para2[0] != '\0') {
				cout << "help����������࣬����������" << endl;
			}
			else {
				help(input_para1);
			}
			break;
		case 11: //exit �˳�ϵͳ
			if (input_para1[0] != '\0') {
				cout << "exit����������࣬����������" << endl;
			}
			else {
				exit();
			}
			break;
		default:
			cout << "����������������ʽ����ȷ������������" << endl;
		}
	}
}