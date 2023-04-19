#include "pch.h"
#include "ipc.h"
#include <string>
#include "define.h"
#include "function.h"
#include "command.h"
using namespace std;

//�ر��ļ�ӳ��
void CloseFileMap()
{
	for (int i = 0; i < 2; i++) {
		//4.�����ļ���ͼUnMapViewOfFile()
		UnmapViewOfFile(m_pBuf[i]);
		//5.�ر�ӳ���ļ����CloseHandle()
		CloseHandle(m_hMapFile[i]);
	}
}

//�����ļ�ӳ��
void CreateFileMap()
{
	m_cstrSzName[0] = L"NameOfMappingObject0"; //�����ڴ�0
	m_cstrSzName[1] = L"NameOfMappingObject1"; //�����ڴ�1
	m_ReadSemaphoreName[0] = L"ReadSemaphoreName0"; //���ź���0
	m_ReadSemaphoreName[1] = L"ReadSemaphoreName1"; //���ź���1
	m_WriteSemaphoreName[0] = L"WriteSemaphoreName0"; //д�ź���0
	m_WriteSemaphoreName[1] = L"WriteSemaphoreName1"; //д�ź���1
	for (int i = 0; i < 2; i++) {
		//���������ļ����
		//����һ���ļ�ӳ���ں˶���
		m_hMapFile[i] = CreateFileMapping(
			INVALID_HANDLE_VALUE,     //�����ļ����������һ�����̼乲��Ķ���
			NULL,				      //Ĭ�ϰ�ȫ����
			PAGE_READWRITE,           //Ȩ�޿ɶ���д
			0,						  //��λ�ļ���С
			BUF_SIZE,				  //��λ�ļ���С
			m_cstrSzName[i].c_str()	  //�����ڴ���
		);
		//��ȡָ���ļ���ͼ��ָ��
		//���ļ�����ӳ�䵽���̵ĵ�ַ�ռ�
		m_pBuf[i] = (char*)MapViewOfFile(
			m_hMapFile[i],			  //�����ڴ�ľ��
			FILE_MAP_ALL_ACCESS,      //�ɶ�д
			0,
			0,
			BUF_SIZE
		);
	}
	//��ʼ���ź�������ʼ��m_Write[0]Ϊ1��ֻ����shell�����ڴ�����ݣ����ȴ�shell������
	m_Read[0] = CreateSemaphore(NULL, 0, 1, m_ReadSemaphoreName[0].c_str());
	m_Write[0] = CreateSemaphore(NULL, 1, 1, m_WriteSemaphoreName[0].c_str());
	m_Read[1] = CreateSemaphore(NULL, 0, 1, m_ReadSemaphoreName[1].c_str());
	m_Write[1] = CreateSemaphore(NULL, 0, 1, m_WriteSemaphoreName[1].c_str());
}

//д�����ڴ�
unsigned int __stdcall WriteSharedData(void *pPM)
{
	//�����ݷŵ������ڴ�
	while (true) {
		//�ȴ�����simdisk������
		WaitForSingleObject(m_Write[1], INFINITE);
		memcpy(m_pBuf[1], result, BUF_SIZE - 1);
		//����shell������
		ReleaseSemaphore(m_Read[1], 1, NULL);
	}
	return true;
}

//�������ڴ�
unsigned int __stdcall ReadSharedData(void *pPM)
{
	int i = 0;
	stringstream sstream;
	char input_command[MAX_COMMAND_SIZE], input_para1[MAX_PATH_SIZE], input_para2[MAX_PATH_SIZE]; //����Ͳ���
	char input_userid[MAX_USERNAME_SIZE], input_password[MAX_PASSWORD_SIZE]; //�˺ź�����
	//�ӹ����ڴ��ж�ȡ����
	while (true) {
		//�ȴ�����simdisk��ȡshell�ŵ�����
		WaitForSingleObject(m_Read[0], INFINITE);
		if (!isLogin) {
			sstream << m_pBuf[0];
			sstream >> input_userid >> input_password;
			result[0] = '\0';
			if (input_userid[0] == '\0') { //���Իس�
				sstream.clear();
				ReleaseSemaphore(m_Write[1], 1, NULL);
				continue;
			}
			sstream.clear();
			isLogin = login(input_userid, input_password);
		}
		else {
			cout << endl;
			cout << current_path << ">"; //��ǰ·��

			//��ȡ��������Ͳ���
			cout << m_pBuf[0];
			sstream << m_pBuf[0];
			sstream >> input_command >> input_para1 >> input_para2;
			result[0] = '\0';
			if (input_command[0] == '\0') { //���Իس�
				sstream.clear();
				ReleaseSemaphore(m_Write[1], 1, NULL);
				continue;
			}

			if (!sstream.eof()) { //���������������
				strcpy(result, "���������Ĳ������ܳ�������");
				sstream.str("");
				sstream.clear();
				ReleaseSemaphore(m_Write[1], 1, NULL);
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
					strcpy(result, "info����������࣬����������");
				}
				else {
					info();
				}
				break;
			case 1: //cd �ı�Ŀ¼
				if (input_para2[0] != '\0') {
					strcpy(result, "cd����������࣬����������");
				}
				else {
					cd(input_para1);
				}
				break;
			case 2: //dir ��ʾĿ¼
				if (strcmp(input_para2, "/s") == 0) {
					dir(input_para1, 1);
				}
				else if (input_para2[0] != '\0' && strcmp(input_para2, "/s") != 0) {
					strcpy(result, "dir���Բ�����������������");
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
					strcpy(result, "md����������٣�����������");
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
						strcpy(result, "md�������Բ�����������������");
						break;
					}
				}
				else { //Ĭ�ϲ���Ϊ�ɶ�д
					md(input_para1, READ_WRITE);
				}
				break;
			case 4: //rd ɾ��Ŀ¼
				if (input_para1[0] == '\0') {
					strcpy(result, "rd����������٣�����������");
				}
				else if (input_para2[0] != '\0') {
					strcpy(result, "rd����������࣬����������");
				}
				else {
					rd(input_para1);
				}
				break;
			case 6: //cat ���ļ�
				if (input_para1[0] == '\0') {
					strcpy(result, "cat����������٣�����������");
				}
				else if (input_para2[0] != '\0') {
					strcpy(result, "cat����������࣬����������");
				}
				else {
					cat(input_para1);
				}
				break;
			case 7: //copy �����ļ�
				if (input_para1[0] == '\0' || input_para2[0] == '\0') {
					strcpy(result, "copy����������٣�����������");
				}
				else {
					copy(input_para1, input_para2);
				}
				break;
			case 8: //del ɾ���ļ�
				if (input_para1[0] == '\0') {
					strcpy(result, "del����������٣�����������");
				}
				else if (input_para2[0] != '\0') {
					strcpy(result, "del����������࣬����������");
				}
				else {
					del(input_para1);
				}
				break;
			case 9: //check ��Ⲣ�ָ��ļ�ϵͳ
				if (input_para1[0] != '\0') {
					strcpy(result, "check����������࣬����������");
				}
				else {
					check();
				}
				break;
			case 10: //help ��ȡ����
				if (input_para2[0] != '\0') {
					strcpy(result, "help����������࣬����������");
				}
				else {
					help(input_para1);
				}
				break;
			case 11: //exit �˳�ϵͳ
				if (input_para1[0] != '\0') {
					strcpy(result, "exit����������࣬����������");
				}
				else {
					exit();
				}
				break;
			default:
				strcpy(result, "����������������ʽ����ȷ������������");
			}
			
		}
		//����simdisk������
		ReleaseSemaphore(m_Write[1], 1, NULL);
	}
	return true;
}
