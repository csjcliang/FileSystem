#include <iostream>
#include <windows.h>
#include "command.h"
#include "function.h"
using namespace std;

//info ��ʾ����ϵͳ��Ϣ
void info()
{
	cout << "-------- ģ��Linux�ļ�ϵͳ��Ϣ --------" << endl;
	cout << right << setw(15) << "������:" << setw(15) << BLOCK_GROUP_NUM << " ��" << endl;
	cout << right << setw(15) << "ÿ���̿���:" << setw(15) << BLOCK_NUM_PER_GROUP << " ��" << endl;
	cout << right << setw(15) << "�̿���:" << setw(15) << TOTAL_BLOCK_NUM << " ��" << endl;
	cout << right << setw(15) << "�̿��С:" << setw(15) << BLOCK_SIZE << " �ֽ�"<< endl;
	cout << right << setw(15) << "��������:" << setw(15) << TOTAL_BLOCK_NUM * BLOCK_SIZE << " �ֽ�"<< endl;
	cout << right << setw(15) << "�������ÿռ�:" << setw(15) << (TOTAL_BLOCK_NUM - blockgroup[0].superblock.free_block_num) * BLOCK_SIZE << " �ֽ�" << endl;
	cout << right << setw(15) << "���̿��ÿռ�:" << setw(15) << blockgroup[0].superblock.free_block_num * BLOCK_SIZE << " �ֽ�" << endl;
}

//cd �ı�Ŀ¼
void cd(const char *path)
{
	Directory tempdir;
	int pathlen = strlen(path);
	if (pathlen == 0) {
		cout << "��ǰ·��Ϊ: " << current_path << endl;
		return;
	}
	//��·������ȡ��Ŀ¼
	if (get_dir(path, pathlen, tempdir)) {
		//�ж��û�Ȩ��
		if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
			cout << "û��Ȩ�޷��ʸ�Ŀ¼" << endl;
			return;
		}
		//�ж�����Ȩ��
		if (inode_table[tempdir.inode_no].fileaccess == WRITE_ONLY && current_usertype != ROOT_USER) {
			cout << "��Ŀ¼Ϊֻд, �޷���ȡ, �л�Ŀ¼ʧ��" << endl;
			return;
		}
		current_dir = tempdir;
		show_cur_path(current_dir);
	}
	else{
		cout << "Ŀ¼������, �л�Ŀ¼ʧ��" << endl;
		return;
	}
}

//dir ��ʾĿ¼
void dir(const char *path, int is_subdir)
{
	Directory tempdir;
	int pathlen = strlen(path);
	if (get_dir(path, pathlen, tempdir)) { //����·����ȡ��Ҫ��ʾ��Ŀ¼
		//�ж��û�Ȩ��
		if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
			cout << "û��Ȩ�޷��ʸ�Ŀ¼" << endl;
			return;
		}
		//�ж�����Ȩ��
		if (inode_table[tempdir.inode_no].fileaccess == WRITE_ONLY && current_usertype != ROOT_USER) {
			cout << "��Ŀ¼Ϊֻд, �޷���ȡ, ��ʾĿ¼ʧ��" << endl;
			return;
		}
		cout << endl;
		if (is_subdir == 1) {
			cout << "Ŀ¼" << inode_table[tempdir.inode_no].fdname << "����Ŀ¼����: " << endl;
			cout << left << setw(15) << "��Ŀ¼" << setw(25) << "�޸�ʱ��" << setw(10) << "����" << setw(10) << "��С(B)" << setw(15) << "������" << setw(10) << "����" << setw(10) << "�����ַ" << endl;
		}
		else {
			cout << "Ŀ¼" << inode_table[tempdir.inode_no].fdname << "����Ϣ����: " << endl;
			cout << left << setw(15) << "Ŀ¼/�ļ���" << setw(25) << "�޸�ʱ��" << setw(10) << "����" << setw(10) << "��С(B)" << setw(15) << "������" << setw(10) << "����" << setw(10) << "�����ַ" << endl;
		}
		
		for (int i = 0; i < tempdir.subsize; i++) {
			if (is_subdir == 1) {
				if (inode_table[tempdir.subinode[i]].filetype == FT_FILE || i == 0 || i == 1) { //�����Բ���Ϊ/s��ֻ��ʾ��Ŀ¼������ʾ�ļ�����ǰĿ¼���ϼ�Ŀ¼
					continue;
				}
			}
			//Ŀ¼/�ļ���
			if (i == 0) {
				cout << setw(15) << ".";
			}
			else if (i == 1) {
				cout << setw(15) << "..";
			}	
			else {
				cout << setw(15) << inode_table[tempdir.subinode[i]].fdname;
			}

			//�޸�ʱ��
			tm temptime= inode_table[tempdir.subinode[i]].mtime.getDatetime();
			cout << setfill('0') << setw(4) << temptime.tm_year + 1900 << "/" << right << setw(2) << temptime.tm_mon + 1 << "/" << setw(2) << temptime.tm_mday << " " << setw(2) << temptime.tm_hour << ":" << setw(2) << temptime.tm_min << ":" << setw(2) << temptime.tm_sec << setfill(' ') << setw(25-19) << "";
			cout << left;
			//����
			if (inode_table[tempdir.subinode[i]].filetype == FT_DIR) {
				cout << setw(10) << "Ŀ¼";
			}
			else {
				cout << setw(10) << "�ļ�";
			}

			//��С
			if (inode_table[tempdir.subinode[i]].filetype == FT_DIR) {
				cout << setw(10) << "-";
			}
			else {
				cout << setw(10) << inode_table[tempdir.subinode[i]].filesize;
			}

			//������
			int j = 0;
			for (j = 0; j < MAX_USER_NUM; j++) {
				if (inode_table[tempdir.subinode[i]].uid == user_group[j].getUserid()) break;
			}
			cout<< setw(15) << user_group[j].getUsername();

			//����
			switch (inode_table[tempdir.subinode[i]].fileaccess){
			case READ_ONLY:
				cout << setw(10) << "ֻ��";
				break;
			case WRITE_ONLY:
				cout << setw(10) << "ֻд";
				break;
			case READ_WRITE:
				cout << setw(10) << "��д";
				break;
			}

			//�����ַ
			cout << setw(10) << inode_table[tempdir.subinode[i]].block_address;
			cout << endl;
		}
	}
	else {
		cout << "��ʾĿ¼ʧ��" << endl;
	}
}

//md ����Ŀ¼
void md(const char *path, fileAccess fileaccess)
{
	Directory tempdir;
	int pathlen = strlen(path);
	char md_dirname[MAX_FILENAME_SIZE];
	if (get_dir_and_fdname(path, pathlen, tempdir, md_dirname)) { //����·����ȡ��Ҫ������Ŀ¼���ϼ�Ŀ¼��������Ŀ¼��
		//�ж��û�Ȩ��
		if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0){
			cout << "û��Ȩ�޷��ʸ�Ŀ¼" << endl;
			return;
		}
		//�ж�����Ȩ��
		if (inode_table[tempdir.inode_no].fileaccess == READ_ONLY && current_usertype != ROOT_USER){ //ֻ���򲻿���Ŀ¼���洴��
			cout << "��Ŀ¼Ϊֻ��, �޷��ڴ�Ŀ¼�´�����Ŀ¼" << endl;
			return;
		}
		
		//����Ƿ����ͬ��Ŀ¼
		if (tempdir.samename(md_dirname)){
			cout << "��Ŀ¼���Ѵ���ͬ����Ŀ¼, ����Ŀ¼ʧ��" << endl;
			return;
		}

		long md_addr = -1; //����Ŀ¼�����������׵�ַ
		int md_inode = -1; //����Ŀ¼��i�ڵ�
		int b_index; //������ڿ�λͼ����ʼλ��
		//ΪĿ¼����ռ�
		md_addr = block_alloc(DIR_SIZE, b_index);
		if (md_addr < 0) { //�ռ䲻��
			cout << "���̿ռ䲻�㣬����Ŀ¼ʧ��" << endl;
			return;
		}
		else {
			//����i�ڵ�
			md_inode = inode_alloc();
			if (md_inode < 0) {
				cout << "i�ڵ����ʧ�ܣ�����Ŀ¼ʧ��" << endl;
				return;
			}

			//����Ŀ¼
			Directory newdir;
			newdir.inode_no = md_inode; //i�ڵ��
			newdir.subsize = 2; //һ��Ϊ��ǰĿ¼��һ��Ϊ��Ŀ¼
			strcpy(inode_table[newdir.inode_no].fdname, md_dirname); //Ŀ¼��
			newdir.subinode[0] = md_inode; //��ǰĿ¼
			newdir.subinode[1] = tempdir.inode_no; //��Ŀ¼
			//�޸�i�ڵ���Ϣ
			inode_table[md_inode].blocknum = DIR_SIZE;
			inode_table[md_inode].block_address = md_addr;
			inode_table[md_inode].fileaccess = fileaccess;
			inode_table[md_inode].filesize = sizeof(Directory);
			inode_table[md_inode].filetype = FT_DIR;
			inode_table[md_inode].uid = current_user;
			time_t t;
			time(&t);
			tm local = *localtime(&t);
			inode_table[md_inode].mtime.setDatetime(local);

			//�޸ĸ�Ŀ¼��Ϣ
			tempdir.subinode[tempdir.subsize] = md_inode;
			tempdir.subsize++;
			if (tempdir.inode_no == current_dir.inode_no) {
				current_dir = tempdir;
			}	

			//д�����
			int i = 0;
			virtualDisk.open("vfs", ios::out | ios::binary | ios::_Nocreate);
			if (!virtualDisk.is_open()) {
				cout << "д���ļ�ϵͳʧ��" << endl;
				Sleep(2000);
				exit(0);
			}
			for (i = 0; i < BLOCK_GROUP_NUM; i++) { //д���ݿ���
				virtualDisk.write((char*)(&blockgroup[i]), sizeof(Blockgroup));
			}
			//дi�ڵ�λͼ
			virtualDisk.seekp(BLOCK_GROUP_NUM*sizeof(Blockgroup) + md_inode*sizeof(Bitmap), ios::beg);
			virtualDisk.write((char*)(&inode_bitmap[md_inode]), sizeof(Bitmap));
			//дi�ڵ��
			virtualDisk.seekp(BLOCK_GROUP_NUM*sizeof(Blockgroup) + TOTAL_INODE_NUM*sizeof(Bitmap) + md_inode*sizeof(Inode), ios::beg);
			virtualDisk.write((char*)(&inode_table[md_inode]), sizeof(Inode));
			//д��λͼ
			virtualDisk.seekp(BLOCK_GROUP_NUM*sizeof(Blockgroup) + TOTAL_INODE_NUM * sizeof(Bitmap) + TOTAL_INODE_NUM*sizeof(Inode) + b_index*sizeof(Bitmap), ios::beg);
			for (i = 0; i < DIR_SIZE; i++) {
				virtualDisk.write((char*)(&block_bitmap[b_index]), sizeof(Bitmap));
			}
			//д�ص�ǰĿ¼�͸�Ŀ¼
			virtualDisk.seekp(md_addr, ios::beg);
			virtualDisk.write((char*)(&newdir), sizeof(Directory));
			virtualDisk.seekp(inode_table[tempdir.inode_no].block_address, ios::beg);
			virtualDisk.write((char*)(&tempdir), sizeof(Directory));
			virtualDisk.close();
			cout << "����Ŀ¼�ɹ�" << endl;
		}
	}
	else{
		cout << "����Ŀ¼ʧ��" << endl;
	}
}

//rd ɾ��Ŀ¼
void rd(const char *path)
{
	Directory tempdir;
	int pathlen = strlen(path);
	char rd_dirname[MAX_FILENAME_SIZE];
	if (get_dir_and_fdname(path, pathlen, tempdir, rd_dirname)) { //����·����ȡ��Ҫɾ����Ŀ¼���ϼ�Ŀ¼��ɾ����Ŀ¼��
		//�ж��û�Ȩ��
		if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
			cout << "û��Ȩ�޷��ʸ�Ŀ¼" << endl;
			return;
		}

		int i = 0;
		int rd_inode = -1; //ɾ��Ŀ¼��i�ڵ�
		int rd_pos = 0; //ɾ��Ŀ¼��λ��

		//������Ҫɾ����Ŀ¼
		for (i = 2; i < tempdir.subsize; i++) {
			if (strcmp(inode_table[tempdir.subinode[i]].fdname, rd_dirname) == 0 && inode_table[tempdir.subinode[i]].filetype == FT_DIR){
				rd_inode = tempdir.subinode[i];
				rd_pos = i;
				break;
			}
		}
		if (i == tempdir.subsize) { //���Ҳ���Ŀ¼
			cout << "Ŀ¼������, ɾ��Ŀ¼ʧ��" << endl;
			return;
		}
		else { //�ҵ�Ŀ¼
			Directory rd_dir; //Ҫɾ����Ŀ¼
			virtualDisk.open("vfs", ios::in | ios::binary);
			if (!virtualDisk.is_open()) {
				cout << "�����ļ�ϵͳʧ��" << endl;
				Sleep(2000);
				exit(0);
			}
			virtualDisk.seekg(inode_table[rd_inode].block_address, ios::beg);
			virtualDisk.read((char*)(&rd_dir), sizeof(Directory));
			virtualDisk.close();
			if (rd_dir.inode_no == current_dir.inode_no) { //����ɾ����ǰĿ¼
				cout << "����ɾ����ǰĿ¼" << endl;
				return;
			}
			if (strstr(current_path, path)) {
				cout << "����ɾ����ǰĿ¼����·���ϵ�Ŀ¼" << endl;
				return;
			}

			//�ж��û�Ȩ��
			if ((current_usertype == USER && inode_table[rd_dir.inode_no].uid != current_user) || inode_table[rd_dir.inode_no].uid == 0) {
				cout << "û��Ȩ�޷��ʸ�Ŀ¼" << endl;
				return;
			}

			if (rd_dir.subsize > 2) { //Ŀ¼��������
				char s = '\0';
				while (1) {
					if (s != '\n') cout << "Ŀ¼ " << inode_table[rd_dir.inode_no].fdname << " ������Ŀ¼���ļ�, �Ƿ���Ҫɾ��? (Y/N) ";
					s = getchar();
					cin.clear();
					cin.ignore();
					if (s == 'Y' || s == 'y' || s == 'N' || s == 'n') {
						break;
					}
				}
				if (s == 'Y' || s == 'y') { //Ŀ¼��ͬ���µ���Ŀ¼�����ļ���ɾ��
					tempdir.remove_dir(rd_dir, rd_pos);
					cout << "ɾ��Ŀ¼�ɹ�" << endl;
					return;
				}
				else {
					cout << "ɾ��Ŀ¼ʧ��" << endl;
					return;
				}
				cout << endl;
			}
			else { //Ŀ¼Ϊ����ֱ��ɾ��
				//�ͷſ��i�ڵ�
				block_free(inode_table[rd_inode].blocknum, ((inode_table[rd_inode].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE));
				inode_free(rd_inode);
				//���¸�Ŀ¼��Ϣ
				for (i = rd_pos; i < tempdir.subsize; i++) {
					tempdir.subinode[i] = tempdir.subinode[i+1]; //������ļ����� 
				}
				tempdir.subsize--;
				if (tempdir.inode_no == current_dir.inode_no) {
					current_dir = tempdir;
				}

				//д�ش���
				virtualDisk.open("vfs", ios::out | ios::binary | ios::_Nocreate);
				if (!virtualDisk.is_open()) {
					cout << "д���ļ�ϵͳʧ��" << endl;
					Sleep(2000);
					exit(0);
				}
				for (i = 0; i < BLOCK_GROUP_NUM; i++) { //д���ݿ���
					virtualDisk.write((char*)(&blockgroup[i]), sizeof(Blockgroup));
				}
				//дi�ڵ�λͼ
				virtualDisk.seekp(BLOCK_GROUP_NUM*sizeof(Blockgroup) + rd_inode*sizeof(Bitmap), ios::beg);
				virtualDisk.write((char*)(&inode_bitmap[rd_inode]), sizeof(Bitmap));
				//д��λͼ
				virtualDisk.seekp(BLOCK_GROUP_NUM*sizeof(Blockgroup) + TOTAL_INODE_NUM*(sizeof(Bitmap) + sizeof(Inode)) + ((inode_table[rd_inode].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE)*sizeof(Bitmap), ios::beg);
				for (i = 0; i < inode_table[rd_inode].blocknum; i++) {
					virtualDisk.write((char*)(&block_bitmap[(inode_table[rd_inode].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE]), sizeof(Bitmap));
				}
				//д��Ŀ¼
				virtualDisk.seekp(inode_table[tempdir.inode_no].block_address, ios::beg);
				virtualDisk.write((char*)(&tempdir), sizeof(Directory));
				virtualDisk.close();
				cout << "ɾ��Ŀ¼�ɹ�" << endl;
			}
		}
	}
	else {
		cout << "ɾ��Ŀ¼ʧ��" << endl;
	}
}

//newfile �����ļ�
void newfile(const char*path, fileAccess fileaccess)
{
	Directory tempdir;
	int pathlen = strlen(path);
	char nf_filename[MAX_FILENAME_SIZE];
	if (get_dir_and_fdname(path, pathlen, tempdir, nf_filename)) { //����·����ȡ��Ҫ�����ļ�������Ŀ¼���������ļ���
		//�ж��û�Ȩ��
		if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
			cout << "û��Ȩ�޷��ʸ�Ŀ¼" << endl;
			return;
		}
		//�ж�����Ȩ��
		if (inode_table[tempdir.inode_no].fileaccess == READ_ONLY && current_usertype != ROOT_USER) {
			cout << "��Ŀ¼Ϊֻ��, �޷��ڴ�Ŀ¼�´����ļ�" << endl;
			return;
		}

		//����Ƿ����ͬ���ļ�
		if (tempdir.samename(nf_filename)) {
			cout << "Ŀ¼���Ѵ���ͬ�����ļ�, �����ļ�ʧ��" << endl;
			return;
		}

		int i = 0;
		int nf_len = 0; //�ļ��ַ���
		int nf_size = 16;  //�ļ���С
		char *content = new char[nf_size]; //�ļ�����
		for (i = 0; i < nf_size; i++) {
			content[i] = 0;
		}
		
		cout << "�������ļ�����, ��#��������" << endl;
		char *temp;
		char s = '\0';
		//��ʼ�����ļ�����
		while ((s = cin.get()) != '#') {
			content[nf_len++] = s;
			if (nf_len >= nf_size - 1) { //�������ݳ���Ԥ���ļ���С
				temp = new char[nf_size];
				strcpy(temp, content); //���Ƶ���ʱ������
				delete(content);
				nf_size *= 2; //�ļ���С����������
				content = new char[nf_size];
				for (i = 0; i < nf_size; i++) {
					content[i] = 0;
				}
				strcpy(content, temp);
				delete(temp);
			}
		}
		cin.ignore();
		tempdir.create_file(nf_filename, content, nf_len, fileaccess); //�����½����ļ�
		delete(content);
	}
	else {
		cout << "�����ļ�ʧ��" << endl;
	}
}

//cat ���ļ�
void cat(const char *path)
{
	Directory tempdir;
	int pathlen = strlen(path);
	char cat_filename[MAX_FILENAME_SIZE];
	if (get_dir_and_fdname(path, pathlen, tempdir, cat_filename)) { //����·����ȡ��Ҫ���ļ�������Ŀ¼���򿪵��ļ���
		//�ж��û�Ȩ��
		if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
			cout << "û��Ȩ�޷��ʸ��ļ�" << endl;
			return;
		}
		//�ж�����Ȩ��
		if (inode_table[tempdir.inode_no].fileaccess == WRITE_ONLY && current_usertype != ROOT_USER) {
			cout << "��Ŀ¼Ϊֻд, �޷��ڴ�Ŀ¼�´��ļ�" << endl;
			return;
		}

		int i = 0;
		int cat_inode = -1; //�ļ���i�ڵ�
		//����Ҫ�򿪵��ļ�
		for (i = 2; i < tempdir.subsize; i++) {
			if (strcmp(inode_table[tempdir.subinode[i]].fdname, cat_filename) == 0 && inode_table[tempdir.subinode[i]].filetype == FT_FILE) {
				cat_inode = tempdir.subinode[i];
				break;
			}
		}
		if (i == tempdir.subsize) { //�Ҳ����ļ�
			cout << "�ļ�������, ���ļ�ʧ��" << endl;
		}
		else {
			//�ж��û�Ȩ��
			if (current_usertype == USER && inode_table[cat_inode].uid != current_user && inode_table[cat_inode].uid != 0) {
				cout << "û��Ȩ�޷��ʸ��ļ�" << endl;
				return;
			}
			//�ж�����Ȩ��
			if (inode_table[cat_inode].fileaccess == WRITE_ONLY && current_usertype != ROOT_USER) {
				cout << "���ļ�Ϊֻд, �޷����ļ�" << endl;
				return;
			}

			//���ļ����������ݵ�content��
			char *content = new char[inode_table[cat_inode].filesize];
			virtualDisk.open("vfs", ios::in | ios::binary);
			if (!virtualDisk.is_open()) {
				cout << "�����ļ�ϵͳʧ��" << endl;
				Sleep(2000);
				exit(0);
			}
			virtualDisk.seekg(inode_table[cat_inode].block_address, ios::beg);
			virtualDisk.read((char*)(content), inode_table[cat_inode].filesize);
			virtualDisk.close();
			content[inode_table[cat_inode].filesize - 1] = 0;
			//����ļ�����
			cout << "�ļ�" << cat_filename << "����������: " << endl;
			cout << content << endl;
			delete(content);
		}
	}
	else {
		cout << "���ļ�ʧ��" << endl;
	}
}

//copy �����ļ�
void copy(const char *srcpath, const char *despath)
{
	Directory tempdir;
	char cp_filename[MAX_FILENAME_SIZE];
	char *content; //�ļ�����
	char divide;
	int len = 0, i = 0;
	int src_len = strlen(srcpath), des_len = strlen(despath);
	char *f_srcpath = new char[src_len];
	char *f_despath = new char[des_len];
	strcpy(f_srcpath, srcpath);
	strcpy(f_despath, despath);
	//��ʼ�����ļ�
	if (host_path(f_srcpath)) { //��host�ļ�ϵͳ�����ļ���ģ���ļ�ϵͳ��
		if (host_path(f_despath)) { //����·����Ϊhost·��
			cout << "��֧�ִ�host�ļ�ϵͳ�����ļ���host�ļ�ϵͳ" << endl;
			return;
		}

		//��ȡhost�ļ�
		fstream fsHost;
		fsHost.open(f_srcpath, ios::in | ios::binary);
		if (!fsHost.is_open()) {
			cout << "host�ļ�ϵͳ�в����ڸ��ļ�" << endl;
			return;
		}
		fsHost.seekg(0, ios::end);
		len = fsHost.tellg(); //�ļ�����
		//����洢�ռ�
		content = new char[len];
		content[len - 1] = 0;
		fsHost.seekg(0, ios::beg);
		fsHost.read((char*)(content), len - 1);
		fsHost.close();
		//��ȡ�ļ���
		divide = '\\';
		strcpy(cp_filename, strrchr(f_srcpath, divide) + 1);
		if (get_dir(f_despath, des_len, tempdir)) { //��Ŀ��·������ȡ��Ŀ¼����
			//�ж��û�Ȩ��
			if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
				cout << "û��Ȩ�޷���Ŀ��·����Ŀ¼" << endl;
				delete(content);
				return;
			}
			//�ж�����Ȩ��
			if (inode_table[tempdir.inode_no].fileaccess == READ_ONLY && current_usertype != ROOT_USER) {
				cout << "Ŀ��·����Ŀ¼Ϊֻ��, �޷��ڴ�Ŀ¼�´����ļ�" << endl;
				delete(content);
				return;
			}

			//����Ƿ����ͬ���ļ�
			if (tempdir.samename(cp_filename)) {
				cout << "��Ŀ¼���Ѵ���ͬ�����ļ�, �����ļ�ʧ��" << endl;
				delete(content);
				return;
			}
			//�����ļ���ģ���ļ�ϵͳ
			tempdir.create_file(cp_filename, content, len, READ_WRITE);
			delete(content);
			cout << "�ļ������ɹ�" << endl;
		}
		else {
			cout << "�ļ�����ʧ��" << endl;
		}
	}
	else { //��ģ���ļ�ϵͳ�����ļ�
		if (host_path(f_despath)) { //ģ���ļ�ϵͳ�����ļ���host�ļ�ϵͳ��
			if (get_dir_and_fdname(f_srcpath, src_len, tempdir, cp_filename)) { //��Դ·������ȡ��Ŀ¼�����ļ���
				int cp_inode;
				for (i = 2; i < tempdir.subsize; i++) {
					if (strcmp(inode_table[tempdir.subinode[i]].fdname, cp_filename) == 0 && inode_table[tempdir.subinode[i]].filetype == FT_FILE) { //�ҵ���Ӧ�ļ�
						cp_inode = tempdir.subinode[i];
						break;
					}
				}
				if (i == tempdir.subsize) { //�Ҳ����ļ�
					cout << "���ļ�������" << endl;
				}
				else { //�ҵ��ļ�
					//�ж��û�Ȩ��
					if (current_usertype == USER && inode_table[cp_inode].uid != current_user) {
						cout << "û��Ȩ�޷���Դ·�����ļ�" << endl;
						return;
					}
					//�ж�����Ȩ��
					if (inode_table[cp_inode].fileaccess == WRITE_ONLY && current_usertype != ROOT_USER) {
						cout << "Դ·�����ļ�Ϊֻд, �޷��ڴ�Ŀ¼�¶�ȡ�ļ�" << endl;
						return;
					}

					//���ļ����������ݵ�content��
					content = new char[inode_table[cp_inode].filesize];
					virtualDisk.open("vfs", ios::in | ios::binary);
					if (!virtualDisk.is_open()) {
						cout << "�����ļ�ϵͳʧ��" << endl;
						Sleep(2000);
						exit(0);
					}
					virtualDisk.seekg(inode_table[cp_inode].block_address, ios::beg);
					virtualDisk.read((char*)(content), inode_table[cp_inode].filesize);
					virtualDisk.close();
					content[inode_table[cp_inode].filesize - 1] = 0;
					len = inode_table[cp_inode].filesize;

					//�ϲ�·��
					char *complete_path = new char[src_len + des_len + 2];
					stringstream sstream;
					sstream << f_despath;
					if (f_despath[des_len - 1] != '\\') sstream << "\\";
					sstream << cp_filename;
					sstream >> complete_path;
					sstream.clear();

					//�����ļ���host�ļ�ϵͳ
					fstream fsHost;
					fsHost.open(complete_path, ios::out | ios::binary);
					if (!fsHost.is_open()) {
						cout << "д���ļ���host�ļ�ϵͳʧ��" << endl;
						delete(content);
						delete(complete_path);
						return;
					}
					fsHost.write((char*)(content), len);
					fsHost.close();
					delete(content);
					delete(complete_path);
					cout << "�ļ������ɹ�" << endl;
				}
			}
			else {
				cout << "�ļ�����ʧ��" << endl;
			}
		}
		else { //ģ���ļ�ϵͳ�ڲ�����
			if (get_dir_and_fdname(f_srcpath, src_len, tempdir, cp_filename)) { //��Դ·������ȡ��Ŀ¼�����ļ���
				int cp_inode;
				for (i = 2; i < tempdir.subsize; i++) {
					if (strcmp(inode_table[tempdir.subinode[i]].fdname, cp_filename) == 0 && inode_table[tempdir.subinode[i]].filetype == FT_FILE) { //�ҵ���Ӧ�ļ�
						cp_inode = tempdir.subinode[i];
						break;
					}
				}
				if (i == tempdir.subsize) { //�Ҳ����ļ�
					cout << "���ļ�������" << endl;
				}
				else //�ҵ��ļ�
				{
					//�ж��û�Ȩ��
					if (current_usertype == USER && inode_table[cp_inode].uid != current_user) {
						cout << "û��Ȩ�޷���Դ·�����ļ�" << endl;
						return;
					}
					//�ж�����Ȩ��
					if (inode_table[cp_inode].fileaccess == WRITE_ONLY && current_usertype != ROOT_USER) {
						cout << "Դ·�����ļ�Ϊֻд, �޷��ڴ�Ŀ¼�¶�ȡ�ļ�" << endl;
						return;
					}

					//���ļ����������ݵ�content��
					fileAccess fileaccess = inode_table[cp_inode].fileaccess;
					content = new char[inode_table[cp_inode].filesize];
					virtualDisk.open("vfs", ios::in | ios::binary);
					if (!virtualDisk.is_open()) {
						cout << "�����ļ�ϵͳʧ��" << endl;
						Sleep(2000);
						exit(0);
					}
					virtualDisk.seekg(inode_table[cp_inode].block_address, ios::beg);
					virtualDisk.read((char*)(content), inode_table[cp_inode].filesize);
					virtualDisk.close();
					content[inode_table[cp_inode].filesize - 1] = 0;
					len = inode_table[cp_inode].filesize;

					//�ϲ�·��
					char *complete_path = new char[src_len + des_len + 2];
					stringstream sstream;
					sstream << f_despath;
					if (f_despath[des_len - 1] != '/') sstream << "/";
					sstream << cp_filename;
					sstream >> complete_path;
					sstream.clear();

					if (get_dir_and_fdname(complete_path, src_len + des_len + 2, tempdir, cp_filename)) { //��·������ȡ��Ŀ¼�����ļ���
						//�ж��û�Ȩ��
						if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
							cout << "û��Ȩ�޷���Ŀ��·����Ŀ¼" << endl;
							delete(content);
							delete(complete_path);
							return;
						}
						//�ж�����Ȩ��
						if (inode_table[tempdir.inode_no].fileaccess == READ_ONLY && current_usertype != ROOT_USER) {
							cout << "Ŀ��·����Ŀ¼Ϊֻ��, �޷��ڴ�Ŀ¼�´����ļ�" << endl;
							delete(content);
							delete(complete_path);
							return;
						}

						//����Ƿ����ͬ���ļ�
						if (tempdir.samename(cp_filename)) {
							cout << "��Ŀ¼���Ѵ���ͬ�����ļ�, �����ļ�ʧ��" << endl;
							delete(content);
							delete(complete_path);
							return;
						}
						//�����ļ�
						tempdir.create_file(cp_filename, content, len, fileaccess);
						cout << "�ļ������ɹ�" << endl;
					}
					else {
						cout << "�ļ�����ʧ��" << endl;
					}
					delete(content);
					delete(complete_path);
				}
			}
			else {
				cout << "�ļ�����ʧ��" << endl;
			}
		}
	}
}

//del ɾ���ļ�
void del(const char *path)
{
	Directory tempdir;
	int pathlen = strlen(path);
	char del_filename[MAX_FILENAME_SIZE];
	if (get_dir_and_fdname(path, pathlen, tempdir, del_filename)) { //����·����ȡ��Ҫɾ���ļ�������Ŀ¼��ɾ�����ļ���
		//�ж��û�Ȩ��
		if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
			cout << "û��Ȩ�޷��ʸ�Ŀ¼" << endl;
			return;
		}
		tempdir.delete_file(del_filename);
	}
	else {
		cout << "ɾ���ļ�ʧ��" << endl;
	}
}

//check ��Ⲣ�ָ��ļ�ϵͳ
void check()
{
	int i = 0, j = 0;
	int free_block_num = 0, free_inode_num = 0, total_free_block = 0, total_free_inode = 0; //���п�����i�ڵ��������п�������i�ڵ�����
	int start; //��¼λͼ��ʼλ��
	bool error = false; //ϵͳ�Ƿ����쳣
	cout << endl;
	cout << "���ڼ���ļ�ϵͳ����" << endl;
	for (i = 0; i < BLOCK_GROUP_NUM; i++) {
		free_block_num = 0, free_inode_num = 0;
		start = i * BLOCK_NUM_PER_GROUP;
		for (j = 0; j < BLOCK_NUM_PER_GROUP; j++) {
			if (block_bitmap[start + j] == NOT_USED) free_block_num++; //ͳ�ƿ��п���
			if (inode_bitmap[start + j] == NOT_USED) free_inode_num++; //ͳ�ƿ���i�ڵ���
		}
		if (blockgroup[i].groupdescription.group_free_blocks_num != free_block_num) { //����̼�¼����
			error = true;
			blockgroup[i].groupdescription.group_free_blocks_num = free_block_num;
		}
		if (blockgroup[i].groupdescription.group_free_inodes_num != free_inode_num) { //����̼�¼����
			error = true;
			blockgroup[i].groupdescription.group_free_inodes_num = free_inode_num;
		}
		total_free_block += blockgroup[i].groupdescription.group_free_blocks_num;
		total_free_inode += blockgroup[i].groupdescription.group_free_inodes_num;
	}
	if (blockgroup[0].superblock.free_block_num != total_free_block) { //��������̼�¼����
		error = true;
		for (i = 0; i < BLOCK_GROUP_NUM; i++) {
			blockgroup[i].superblock.free_block_num = total_free_block;
		}
	}
	if (blockgroup[0].superblock.free_inode_num != total_free_inode) { //��������̼�¼����
		error = true;
		for (i = 0; i < BLOCK_GROUP_NUM; i++) {
			blockgroup[i].superblock.free_inode_num = total_free_inode;
		}
	}
	//�ж��Ƿ����쳣
	if (!error)	{
		cout << "������, �ļ�ϵͳ���쳣" << endl;
	}
	else {
		//�����޸�
		cout << "��鷢���쳣, �����޸��ļ�ϵͳ����" << endl;
		virtualDisk.open("vfs", ios::out | ios::binary | ios::_Nocreate);
		if (!virtualDisk.is_open()) {
			cout << "д���ļ�ϵͳʧ��" << endl;
			Sleep(2000);
			exit(0);
		}
		for (i = 0; i < BLOCK_GROUP_NUM; i++) { //д���ݿ�
			virtualDisk.write((char*)(&blockgroup[i]), sizeof(Blockgroup));
		}
		for (i = 0; i < TOTAL_INODE_NUM; i++) { //дi�ڵ�λͼ
			virtualDisk.write((char*)(&inode_bitmap[i]), sizeof(Bitmap));
		}
		for (i = 0; i < TOTAL_INODE_NUM; i++) { //дi�ڵ��
			virtualDisk.write((char*)(&inode_table[i]), sizeof(Inode));
		}
		for (i = 0; i < TOTAL_BLOCK_NUM; i++) { //д��λͼ
			virtualDisk.write((char*)(&block_bitmap[i]), sizeof(Bitmap));
		}
		//д��ǰĿ¼
		virtualDisk.seekp(inode_table[current_dir.inode_no].block_address, ios::beg);
		virtualDisk.write((char*)(&current_dir), sizeof(Directory));
		virtualDisk.close();

		cout << "�ļ�ϵͳ�޸����!" << endl;
	}
}

//help ��ȡ����
void help(const char *command_in)
{
	if (command_in[0] == '\0') { //��������
		cout << endl;
		cout << "-------- ģ��Linux�ļ�ϵͳ������� --------" << endl;
		cout << "0.info" << "\t\t" << "��ʾ����ϵͳ��Ϣ" << endl;
		cout << "1.cd" << "\t\t" << "�л�Ŀ¼" << endl;
		cout << "2.dir" << "\t\t" << "��ʾĿ¼" << endl;
		cout << "3.md" << "\t\t" << "����Ŀ¼" << endl;
		cout << "4.rd" << "\t\t" << "ɾ��Ŀ¼" << endl;
		cout << "5.newfile" << "\t" << "�����ļ�" << endl;
		cout << "6.cat" << "\t\t" << "���ļ�" << endl;
		cout << "7.copy" << "\t\t" << "�����ļ�" << endl;
		cout << "8.del" << "\t\t" << "ɾ���ļ�" << endl;
		cout << "9.check" << "\t\t" << "��Ⲣ�ָ��ļ�ϵͳ" << endl;
		cout << "10.help" << "\t\t" << "��ȡ����" << endl;
		cout << "11.exit" << "\t\t" << "�˳�ϵͳ" << endl;
		cout << "Ҫ���ÿ������ľ���ʹ�÷���, ����help+�������Ʋ�ѯ" << endl;
	}
	else { //������
		int i = 0;
		for (i = 0; i < COMMAND_NUM; i++) {
			if (strcmp(command_in, command[i]) == 0) break;
		}
		switch (i)
		{
		case 0: //info ��ʾ����ϵͳ��Ϣ
			cout << "info (����info):\n��ʾ����ϵͳ��Ϣ�������̿����������������ÿռ����Ϣ��" << endl;
			break;
		case 1: //cd �ı�Ŀ¼
			cout << "cd (����cd+·��):\n�ı䵱ǰ����Ŀ¼��·��������.Ϊ��ǰĿ¼��..Ϊ��һ��Ŀ¼��Ĭ�ϲ���Ϊ��ǰĿ¼����" << endl;
			break;
		case 2: //dir ��ʾĿ¼
			cout << "dir (����dir+·��+����):\n��ʾָ��Ŀ¼�»�ǰĿ¼�µ��ļ�����Ŀ¼��Ϣ��·��������.Ϊ��ǰĿ¼��..Ϊ��һ��Ŀ¼��Ĭ�ϲ���Ϊ��ǰĿ¼�����Բ����ɲ������/s��ʾ������Ŀ¼����/s�豣֤·����Ϊ�գ���" << endl;
			break;
		case 3: //md ����Ŀ¼
			cout << "md (����md+·��+����):\n��ָ��·����ǰ·���´���ָ��Ŀ¼�����Բ�����0Ϊֻ����1Ϊֻд��2Ϊ�ɶ�д��Ĭ��Ϊ�ɶ�д����" << endl;
			break;
		case 4: //rd ɾ��Ŀ¼
			cout << "rd (����rd+·��):\nɾ��ָ��Ŀ¼�������ļ�����Ŀ¼��" << endl;
			break;
		case 5: //newfile �����ļ�
			cout << "newfile (����newfile+·��(�������ļ���)+����):\n��ָ��·����ǰ·���½����ļ�����·��������ֻ�����ļ������ڵ�ǰĿ¼�����ļ������Բ�����0Ϊֻ����1Ϊֻд��2Ϊ�ɶ�д��Ĭ��Ϊ�ɶ�д����" << endl;
			break;
		case 6: //cat ���ļ�
			cout << "cat (����cat+·��(�����ļ���)):\n��ָ��·���µ��ļ���·��������ֻ�����ļ������ڵ�ǰĿ¼���ļ�����" << endl;
			break;
		case 7: //copy �����ļ�
			cout << "copy (����copy+Դ·��(��Դ�ļ���)+Ŀ��·��):\n�����ļ���֧��ģ���ļ�ϵͳ�ڲ����ļ�������Ҳ֧��host�ļ�ϵͳ��ģ���ļ�ϵͳ����ļ�������host�ļ�ϵͳ��·������Ӧ����ǰ����<host>��" << endl;
			break;
		case 8: //del ɾ���ļ�
			cout << "del (����del+·��(��ɾ���ļ���)):\nɾ��ָ��Ŀ¼�µ��ļ���·��������ֻ�����ļ������ڵ�ǰĿ¼ɾ���ļ�����" << endl;
			break;
		case 9: //check ��Ⲣ�ָ��ļ�ϵͳ
			cout << "check (����check):\n��Ⲣ�ָ��ļ�ϵͳ��" << endl;
			break;
		case 10: //help ��ȡ����
			cout << "help (����help+������):\n��ȡ�����������������ȱ����ʾ���������������" << endl;
			break;
		case 11: //exit �˳�ϵͳ
			cout << "exit (����exit):\n�˳�ϵͳ��" << endl;
			break;
		default:
			cout << "����������������ʽ����ȷ������������" << endl;
			break;
		}
	}
}

//exit �˳�ϵͳ
void exit()
{
	cout << "�����˳��ļ�ϵͳ����" << endl;
	Sleep(2000);
	exit(0);
}