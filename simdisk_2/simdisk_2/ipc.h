#include "pch.h"
#include <iostream>
#include <windows.h>
#include <process.h>
using namespace std;

#define BUF_SIZE 102400
extern wstring m_cstrSzName[2]; //�����ڴ������ֱ���shell�����ݸ�simdisk��simdisk�����ݸ�shell
extern wstring m_ReadSemaphoreName[2]; //�����ݵ��ź���
extern wstring m_WriteSemaphoreName[2]; //д���ݵ��ź���
extern HANDLE m_hMapFile[2]; //�����ڴ���
extern char *m_pBuf[2]; //���ݻ��棬m_pBuf[0]Ϊshell��simdisk���ݵ����ݣ�m_pBuf[1]Ϊsimdisk��shell���ݵ�����
extern HANDLE m_Read[2]; //�����ݵ��ź������
extern HANDLE m_Write[2]; //д���ݵ��ź������
extern char result[BUF_SIZE]; //���ؽ��
extern bool isLogin; //�Ƿ��¼

void CloseFileMap(); //�ر��ļ�ӳ��

void CreateFileMap(); //�����ļ�ӳ��

unsigned int __stdcall WriteSharedData(void *pPM); //д�����ڴ�

unsigned int __stdcall ReadSharedData(void *pPM); //�������ڴ�
