#pragma once

#include "define.h"

//���س�ʼ����Ϣ
void sys_load();
//��ʼ���ļ�ϵͳ
void sys_init();
//��ʼ���û���Ϣ
void init_user();
//��¼
bool login();
//�����̿�
long block_alloc(int b_len, int &b_index);
//�ͷ��̿�
void block_free(int b_len, int b_index);
//����i�ڵ�
int inode_alloc();
//�ͷ�i�ڵ�
void inode_free(int f_inode);
//��ʾ��ǰ·��
void show_cur_path(Directory cur_dir);
//��·������ȡĿ¼��
bool get_dirname(const char *path, int len, int pos, char *filename);
//����·��Ѱ�Ҷ�ӦĿ¼
bool find_path_dir(const char *path, int len, int pos, char *dirname, Directory &tempdir);
//����·����ȡ������Ŀ¼��Ŀ¼���ļ���
bool get_dir_and_fdname(const char *path, int len, Directory &tempdir, char *filename);
//����·����ȡ������Ŀ¼
bool get_dir(const char *path, int len, Directory &tempdir);
//�ж�·���Ƿ�Ϊhost�ļ�ϵͳ·��
bool host_path(char *path);
