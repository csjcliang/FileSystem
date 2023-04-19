#include "pch.h"
#include <iostream>
#include "function.h"
#include "command.h"
#include "sync.h"
using namespace std;

wstring m_cstrSzName; //�����ڴ���
wstring m_mutexName; //��������
HANDLE m_hMapFile; //���������
HANDLE m_hMutex; //����ͬ���Ļ�����
Shared_Memory *m_pBuf; //������ָ��

//�ر��ļ�ӳ�䣨��������
void CloseFileMap()
{
	//�ر�ӳ���ļ����
	//�����ļ���ͼ
	if (m_hMutex) {
		CloseHandle(m_hMutex);
	}
	UnmapViewOfFile(m_pBuf);
	CloseHandle(m_hMapFile);
}

//�����ļ�ӳ�䣨��������
bool CreateFileMap()
{
	m_cstrSzName = L"NameOfSharedMemory"; //�����ڴ�
	m_mutexName = L"NameOfMutex"; //������
	bool is_init = false;

	//����������
	m_hMapFile = CreateFileMapping(
		(HANDLE)0xFFFFFFFF, 
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(Shared_Memory),
		m_cstrSzName.c_str()
	);

	if (GetLastError() == ERROR_ALREADY_EXISTS) { //�Ѿ�����������
		m_hMapFile = OpenFileMapping(FILE_MAP_WRITE, FALSE, m_cstrSzName.c_str());
	}
	else { //δ��ʼ��������
		is_init = true;
	}

	//����������
	m_hMutex = CreateMutex(NULL, FALSE, m_mutexName.c_str());

	//��ȡָ���ļ���ͼ��ָ��
	m_pBuf = (Shared_Memory*)MapViewOfFile(
		m_hMapFile,
		FILE_MAP_WRITE, //��д
		0,
		0,
		sizeof(Shared_Memory)
	);

	if (is_init) { //δ��ʼ������������г�ʼ��
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

//д�빲�����û���¼��Ϣ
bool set_logined_user(int userid, bool user_logined)
{
	if (user_logined == true) { //�����û���¼��Ϣ
		//д���û���¼��Ϣ
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
		//���û���¼��Ϣ��ɾ��
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

//��ȡ�������û���¼��Ϣ
bool get_logined_user(int userid)
{
	//��ȡ�û���¼��Ϣ
	if (WaitForSingleObject(m_hMutex, 200) == WAIT_OBJECT_0) {
		//�ж��û��Ƿ��ѵ�¼
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

//���乲����i�ڵ�ʹ�ñ�����
int sharedm_alloc(int inode_no)
{
	int index = -1; //i�ڵ��ڹ�����λ��
	//д�빲����
	if (WaitForSingleObject(m_hMutex, 200) == WAIT_OBJECT_0) { //�ȴ������Ϊ���ź�״̬
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

//�ͷŹ�����i�ڵ�ʹ�ñ�����
bool sharedm_free(int index)
{
	if (index < 0) return false;
	//�ӹ����������
	if (WaitForSingleObject(m_hMutex, 200) == WAIT_OBJECT_0) { //�ȴ������Ϊ���ź�״̬
		m_pBuf->using_inode[index] = -1;
		ReleaseMutex(m_hMutex);
		return true;
	}
	return false;
}

//��ȡ������i�ڵ�ʹ�ñ�
bool sharedm_read(int inode_no)
{
	//��ȡ��Ϣ
	if (WaitForSingleObject(m_hMutex, 200) == WAIT_OBJECT_0) {
		//�ж�i�ڵ��Ƿ�����д��
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

