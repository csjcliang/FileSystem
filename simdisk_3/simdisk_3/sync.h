#include "pch.h"
#include "define.h"
#include <windows.h>
#pragma once

//������
class Shared_Memory
{
public:
	int logined_user[MAX_USER_NUM]; //��¼�û�
	int using_inode[MAX_USER_NUM * 2]; //���ڱ�ʹ�õ�i�ڵ�
};

//�ر��ļ�ӳ�䣨��������
void CloseFileMap();

//�����ļ�ӳ�䣨��������
bool CreateFileMap();

//д�빲�����û���¼��Ϣ
bool set_logined_user(int userid, bool user_logined);

//��ȡ�������û���¼��Ϣ
bool get_logined_user(int userid);

//���乲����i�ڵ�ʹ�ñ�����
int sharedm_alloc(int inode_no);

//�ͷŹ�����i�ڵ�ʹ�ñ�����
bool sharedm_free(int index);

//��ȡ������i�ڵ�ʹ�ñ�
bool sharedm_read(int inode_no);

extern wstring m_cstrSzName; //�����ڴ���
extern wstring m_mutexName; //��������
extern HANDLE m_hMapFile; //���������
extern HANDLE m_hMutex; //����ͬ���Ļ�����
extern Shared_Memory *m_pBuf; //������ָ��
