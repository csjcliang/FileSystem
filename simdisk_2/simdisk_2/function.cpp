#include "pch.h"
#include <iostream>
#include <windows.h>
#include "ipc.h"
#include "function.h"

using namespace std;

//��������ʱ��
void Datetime::setDatetime(tm datetime)
{
	this->year = datetime.tm_year;
	this->month = datetime.tm_mon;
	this->day = datetime.tm_mday;
	this->hour = datetime.tm_hour;
	this->minute = datetime.tm_min;
	this->second = datetime.tm_sec;
}

////��ȡ����ʱ��
tm Datetime::getDatetime()
{
	tm datetime;
	datetime.tm_year = this->year;
	datetime.tm_mon = this->month;
	datetime.tm_mday = this->day;
	datetime.tm_hour = this->hour;
	datetime.tm_min = this->minute;
	datetime.tm_sec = this->second;
	return datetime;
}

//�����û���Ϣ
void User::setUser(int userid, const char *username, const char *password, userType usertype)
{
	this->userid = userid;
	strcpy(this->username, username);
	strcpy(this->password, password);
	this->usertype = usertype;
}
//��ȡ�û��˺�
int User::getUserid()
{
	return userid;
}
//��ȡ�û�����
char* User::getUsername()
{
	return username;
}
//��ȡ�û�����
char* User::getPassword()
{
	return password;
}
//��ȡ�û�����
userType User::getUsertype()
{
	return usertype;
}

//�ж�Ŀ¼���Ƿ���ͬ���ļ���Ŀ¼
bool Directory::samename(const char *fdname)
{
	for (int i = 2; i < subsize; i++) {
		if (strcmp(fdname, inode_table[this->subinode[i]].fdname) == 0) return true;
	}
	return false;
}

//ɾ����Ŀ¼
void Directory::remove_dir(Directory rd_dir, int rd_index)
{
	int i = 0;
	for (i = 2; i < rd_dir.subsize; i++) {
		if (inode_table[rd_dir.subinode[i]].filetype == FT_DIR) { //ΪĿ¼��ݹ�ɾ�����ļ�����Ŀ¼
			Directory rd_subdir;
			virtualDisk.open("vfs", ios::in | ios::binary);
			if (!virtualDisk.is_open()) {
				strcpy(result, "д���ļ�ϵͳʧ��");
				Sleep(2000);
				exit(0);
			}
			virtualDisk.seekg(inode_table[rd_dir.subinode[i]].block_address, ios::beg);
			virtualDisk.read((char*)(&rd_subdir), sizeof(Directory));
			virtualDisk.close();
			//ɾ�����ļ�����Ŀ¼
			rd_dir.remove_dir(rd_subdir, i);
		}
		else { //Ϊ�ļ���ֱ��ɾ��
			rd_dir.delete_file(inode_table[rd_dir.subinode[i]].fdname);
		}
	}
	//ɾ�������Ϣ
	block_free(inode_table[rd_dir.inode_no].blocknum, ((inode_table[rd_dir.inode_no].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE)); //�ͷſ�
	inode_free(rd_dir.inode_no); //�ͷ�i�ڵ�
	//���¸�Ŀ¼��������ļ���Ŀ¼����
	for (i = rd_index; i < this->subsize; i++) {
		this->subinode[i] = this->subinode[i + 1];
	}
	this->subsize--;
	if (this->inode_no == current_dir.inode_no) {
		current_dir = *this; //���µ�ǰĿ¼
	}

	//д�ش���
	virtualDisk.open("vfs", ios::out | ios::binary | ios::_Nocreate);
	if (!virtualDisk.is_open()) {
		strcpy(result, "д���ļ�ϵͳʧ��");
		Sleep(2000);
		exit(0);
	}
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //д���ݿ���
		virtualDisk.write((char*)(&blockgroup[i]), sizeof(Blockgroup));
	}

	//дi�ڵ�λͼ
	virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + rd_dir.inode_no * sizeof(Bitmap), ios::beg);
	virtualDisk.write((char*)(&inode_bitmap[rd_dir.inode_no]), sizeof(Bitmap));

	//д��λͼ
	virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + TOTAL_INODE_NUM * (sizeof(Bitmap) + sizeof(Inode)) + ((inode_table[rd_dir.inode_no].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE) * sizeof(Bitmap), ios::beg);
	for (i = 0; i < inode_table[rd_dir.inode_no].blocknum; i++) {
		virtualDisk.write((char*)(&block_bitmap[(inode_table[rd_dir.inode_no].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE]), sizeof(Bitmap));
	}
	//д��Ŀ¼
	virtualDisk.seekp(inode_table[this->inode_no].block_address, ios::beg);
	virtualDisk.write((char*)(this), sizeof(Directory));
	virtualDisk.close();
}

//ɾ���ļ�
void Directory::delete_file(const char* filename)
{
	int i = 0;
	int df_pos = 0; //ɾ���ļ���λ��
	int df_inode = -1; //ɾ���ļ���i�ڵ�

	//�����ļ�
	for (i = 2; i < this->subsize; i++) {
		if (strcmp(inode_table[subinode[i]].fdname, filename) == 0 && inode_table[subinode[i]].filetype == FT_FILE) {
			df_pos = i;
			df_inode = this->subinode[i];
			break;
		}
	}
	if (i == this->subsize) { //�Ҳ����ļ�
		strcpy(result, "�Ҳ����ļ�");
		return;
	}

	//�ж��û�Ȩ��
	if (current_usertype == USER && inode_table[df_inode].uid != current_user || inode_table[df_inode].uid == 0) {
		strcpy(result, "û��Ȩ��ɾ���ļ�");
		return;
	}

	//�ͷſ��i�ڵ�
	block_free(inode_table[df_inode].blocknum, ((inode_table[df_inode].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE));
	inode_free(df_inode);
	//���¸�Ŀ¼��Ϣ
	for (i = df_pos; i < this->subsize; i++) {
		this->subinode[i] = this->subinode[i + 1]; //������ļ�����
	}
	this->subsize--;
	if (this->inode_no == current_dir.inode_no) {
		current_dir = *this;
	}
	//д�ش���
	virtualDisk.open("vfs", ios::out | ios::binary | ios::_Nocreate);
	if (!virtualDisk.is_open()) {
		strcpy(result, "д���ļ�ϵͳʧ��");
		Sleep(2000);
		exit(0);
	}
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //д���ݿ���
		virtualDisk.write((char*)(&blockgroup[i]), sizeof(Blockgroup));
	}
	//дi�ڵ�λͼ
	virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + df_inode * sizeof(Bitmap), ios::beg);
	virtualDisk.write((char*)(&inode_bitmap[df_inode]), sizeof(Bitmap));
	//д��λͼ
	virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + TOTAL_INODE_NUM * (sizeof(Bitmap) + sizeof(Inode)) + ((inode_table[df_inode].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE) * sizeof(Bitmap), ios::beg);
	for (i = 0; i < inode_table[df_inode].blocknum; i++) {
		virtualDisk.write((char*)(&block_bitmap[(inode_table[df_inode].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE]), sizeof(Bitmap));
	}
	//д��Ŀ¼
	virtualDisk.seekp(inode_table[this->inode_no].block_address, ios::beg);
	virtualDisk.write((char*)(this), sizeof(Directory));
	virtualDisk.close();
	strcpy(result, "ɾ���ļ��ɹ�");
}

//�����ļ�
void Directory::create_file(const char* filename, char *content, int filelen, fileAccess fileaccess)
{
	int i = 0;
	int cf_blocknum; //�ļ�ռ����
	int cf_addr = -1; //�����ļ��̿���׵�ַ
	int cf_index; //�����ļ��̿��ڿ�λͼ��λ��
	int cf_inode = -1; //�ļ�i�ڵ�

	//�����ļ�ռ�̿���
	if ((filelen + 1) % BLOCK_SIZE == 0) { //����
		cf_blocknum = (filelen + 1) / BLOCK_SIZE;
	}
	else {
		cf_blocknum = (filelen + 1) / BLOCK_SIZE + 1;
	}

	//������i�ڵ�
	cf_addr = block_alloc(cf_blocknum, cf_index);
	if (cf_addr < 0) {
		strcpy(result, "���̿ռ䲻�㣬�����ļ�ʧ��");
		return;
	}
	cf_inode = inode_alloc();
	if (cf_inode < 0) {
		strcpy(result, "i�ڵ����ʧ�ܣ������ļ�ʧ��");
		return;
	}

	//�޸�i�ڵ���Ϣ
	inode_table[cf_inode].blocknum = cf_blocknum;
	inode_table[cf_inode].block_address = cf_addr;
	inode_table[cf_inode].fileaccess = fileaccess;
	inode_table[cf_inode].filesize = filelen + 1;
	inode_table[cf_inode].filetype = FT_FILE;
	inode_table[cf_inode].uid = current_user;
	strcpy(inode_table[cf_inode].fdname, filename);
	time_t t;
	time(&t);
	tm local = *localtime(&t);
	inode_table[cf_inode].mtime.setDatetime(local);

	//�޸ĸ�Ŀ¼��Ϣ
	this->subinode[this->subsize] = cf_inode;
	this->subsize++;
	if (this->inode_no == current_dir.inode_no) {
		current_dir = *this;
	}

	//д�ش���
	virtualDisk.open("vfs", ios::out | ios::binary | ios::_Nocreate);
	if (!virtualDisk.is_open()) {
		strcpy(result, "д���ļ�ϵͳʧ��");
		Sleep(2000);
		exit(0);
	}
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //д���ݿ���
		virtualDisk.write((char*)(&blockgroup[i]), sizeof(Blockgroup));
	}
	//дi�ڵ�λͼ
	virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + cf_inode * sizeof(Bitmap), ios::beg);
	virtualDisk.write((char*)(&inode_bitmap[cf_inode]), sizeof(Bitmap));
	//дi�ڵ��
	virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + TOTAL_INODE_NUM * sizeof(Bitmap) + cf_inode * sizeof(Inode), ios::beg);
	virtualDisk.write((char*)(&inode_table[cf_inode]), sizeof(Inode));
	//д��λͼ
	virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + TOTAL_INODE_NUM * (sizeof(Bitmap) + sizeof(Inode)) + cf_index * sizeof(Bitmap), ios::beg);
	for (i = 0; i < cf_blocknum; i++) {
		virtualDisk.write((char*)(&block_bitmap[cf_index]), sizeof(Bitmap));
	}
	//д�ļ�
	virtualDisk.seekp(cf_addr, ios::beg);
	virtualDisk.write((char*)(content), filelen);
	//д��Ŀ¼
	virtualDisk.seekp(inode_table[this->inode_no].block_address, ios::beg);
	virtualDisk.write((char*)(this), sizeof(Directory));
	virtualDisk.close();
	strcpy(result, "�����ļ��ɹ�");
}

//���س�ʼ����Ϣ
void sys_load()
{
	cout << "-------- ��ӭʹ��ģ��Linux�ļ�ϵͳ --------" << endl;
	virtualDisk.open("vfs", ios::in | ios::binary);
	if (!virtualDisk.is_open()) { //�ļ�ϵͳδ��ʼ��
		char input = '\0'; //�û������ַ�
		while (1) {
			if (input != '\n') cout << "ģ��Linux�ļ�ϵͳδ��ʼ��, �Ƿ���г�ʼ��?��Y/N��";
			input = getchar();
			if (input == 'Y' || input == 'y' || input == 'N' || input == 'n') {
				break;
			}
		}
		if (input == 'Y' || input == 'y') { //��ʼ���ļ�ϵͳ
			virtualDisk.clear();
			sys_init();
			return;
		}
		else {
			cout << "�ļ�ϵͳ��ʼ��ʧ��" << endl;
			Sleep(2000);
			exit(0);
		}
	}
	//���Ѿ���ʼ���������Ϣ
	int i = 0;
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //�������ݿ���
		virtualDisk.read((char*)(&blockgroup[i]), sizeof(Blockgroup));
	}
	for (i = 0; i < TOTAL_INODE_NUM; i++) { //����i�ڵ�λͼ
		virtualDisk.read((char*)(&inode_bitmap[i]), sizeof(Bitmap));
	}
	for (i = 0; i < TOTAL_INODE_NUM; i++) { //����i�ڵ��
		virtualDisk.read((char*)(&inode_table[i]), sizeof(Inode));
	}
	for (i = 0; i < TOTAL_BLOCK_NUM; i++) { //�����λͼ
		virtualDisk.read((char*)(&block_bitmap[i]), sizeof(Bitmap));
	}
	virtualDisk.read((char*)(&current_dir), sizeof(Directory));
	virtualDisk.close();
	strcpy(current_path, inode_table[current_dir.inode_no].fdname); //���Ƶ�ǰ·��

	//��ʼ���û���Ϣ
	init_user();
}

//��ʼ���ļ�ϵͳ
void sys_init()
{
	virtualDisk.clear(); //�����״̬
	cout << "���ڽ��г�ʼ������" << endl;

	int i = 0;
	//��ʼ�����ݿ���
	for (i = 0; i < BLOCK_GROUP_NUM; i++) {
		//��ʼ��������
		blockgroup[i].superblock.block_num = TOTAL_BLOCK_NUM;
		blockgroup[i].superblock.inode_num = TOTAL_INODE_NUM;
		blockgroup[i].superblock.block_size = BLOCK_SIZE;
		blockgroup[i].superblock.free_block_num = TOTAL_BLOCK_NUM - DIR_SIZE; //����һ�������ڸ�Ŀ¼
		blockgroup[i].superblock.free_inode_num = TOTAL_INODE_NUM - 1; //����һ��i�ڵ����ڸ�Ŀ¼
		blockgroup[i].superblock.data_block_address = DATA_BLOCK_BEGIN;
		//��ʼ����������
		blockgroup[i].groupdescription.group_address = DATA_BLOCK_BEGIN + i * BLOCK_NUM_PER_GROUP*BLOCK_SIZE;
		blockgroup[i].groupdescription.group_block_bitmap = i * BLOCK_NUM_PER_GROUP;
		blockgroup[i].groupdescription.group_free_blocks_num = BLOCK_NUM_PER_GROUP;
		blockgroup[i].groupdescription.group_free_inodes_num = INODE_NUM_PER_GROUP;
		blockgroup[i].groupdescription.group_inode_bitmap = i * INODE_NUM_PER_GROUP;
		blockgroup[i].groupdescription.group_inode_table = i * INODE_NUM_PER_GROUP;
	}
	//��ʼ��i�ڵ�λͼ
	for (i = 0; i < TOTAL_INODE_NUM; i++) {
		inode_bitmap[i] = NOT_USED;
	}
	//��ʼ��i�ڵ��
	for (i = 0; i < TOTAL_INODE_NUM; i++) {
		inode_table[i].block_address = -1;
		inode_table[i].blocknum = 0;
		inode_table[i].fileaccess = READ_WRITE;
		inode_table[i].filesize = 0;
		inode_table[i].uid = 0;
		inode_table[i].filetype = FT_DIR;
	}
	//��ʼ����λͼ
	for (i = 0; i < TOTAL_BLOCK_NUM; i++) {
		block_bitmap[i] = NOT_USED;
	}
	//��ʼ����Ŀ¼��Ϣ
	//��ʼ����Ŀ¼�������ݿ���
	blockgroup[0].groupdescription.group_free_blocks_num = BLOCK_NUM_PER_GROUP - DIR_SIZE;
	blockgroup[0].groupdescription.group_free_inodes_num = INODE_NUM_PER_GROUP - 1;
	//��ʼ����Ŀ¼�����ݿ�λͼ
	for (i = 0; i < DIR_SIZE; i++) {
		block_bitmap[i] = USED;
	}
	//��ʼ����Ŀ¼��i�ڵ�λͼ
	inode_bitmap[0] = USED;
	//��ʼ����Ŀ¼��i�ڵ��
	strcpy(inode_table[0].fdname, "root");
	time_t t;
	time(&t);
	tm local = *localtime(&t);
	inode_table[0].mtime.setDatetime(local);
	inode_table[0].block_address = DATA_BLOCK_BEGIN; //��Ŀ¼λ����ʼλ��
	inode_table[0].blocknum = DIR_SIZE;
	inode_table[0].fileaccess = READ_WRITE;
	inode_table[0].filesize = sizeof(Directory); //��Ŀ¼��С
	inode_table[0].uid = 0;
	inode_table[0].filetype = FT_DIR;
	//���ø�Ŀ¼��Ϣ
	current_dir.inode_no = 0; //��Ŀ¼i�ڵ��Ϊ0
	current_dir.subsize = 2; //��ǰĿ¼����һ��Ŀ¼
	current_dir.subinode[0] = 0; //��ǰĿ¼Ϊ��Ŀ¼
	current_dir.subinode[1] = 0; //��Ŀ¼��һ��Ŀ¼��Ϊ��Ŀ¼
	//���õ�ǰ·��
	strcpy(current_path, "root");

	//д���ļ�
	virtualDisk.open("vfs", ios::out | ios::binary);
	if (!virtualDisk.is_open()) {
		cout << "д���ļ�ϵͳʧ��" << endl;
		Sleep(2000);
		exit(0);
	}
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //д���ݿ���
		virtualDisk.write((char*)(&blockgroup[i]), sizeof(Blockgroup)); //**************
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
	//д��Ŀ¼
	virtualDisk.seekp(inode_table[current_dir.inode_no].block_address, ios::beg);
	virtualDisk.write((char*)(&current_dir), sizeof(Directory));

	//����100M�Ŀռ�
	long filesize = BLOCK_SIZE * TOTAL_BLOCK_NUM;
	char *buf = new char[filesize];
	virtualDisk.write((char*)(buf), filesize * sizeof(char));
	cout << "��ʼ�����,���ڽ����¼���桭��" << endl;
	virtualDisk.close();
	delete(buf);
	Sleep(1000);
	//��ʼ���û���Ϣ
	init_user();

	system("cls");
	cout << "-------- ��ӭ������¼���� --------" << endl;
}

//��ʼ���û���Ϣ
void init_user()
{
	user_group[0].setUser(0, "root_user", "root3373", ROOT_USER); //����0��Ϊ�����û������û���
	user_group[1].setUser(100, "ljc", "ljc3373", ADMIN); //����1��Ϊ����Ա
	//����Ϊ��ͨ�û�
	user_group[2].setUser(101, "xiaoming", "xiaoming101", USER);
	user_group[3].setUser(102, "xiaohong", "xiaohong102", USER);
	user_group[4].setUser(103, "xiaogang", "xiaogang103", USER);
}

//��¼
bool login(char *userid, char *password)
{
	int i = 0;
	//���û����в鿴�˺������Ƿ���ȷ
	for (i = 0; i < MAX_USER_NUM; i++) {
		if (atoi(userid) == user_group[i].getUserid() && strcmp(password, user_group[i].getPassword()) == 0) {
			current_user = atoi(userid);
			current_usertype = user_group[i].getUsertype();
			strcpy(result, "��¼�ɹ�!");
			Sleep(500);
			return true;
		}
	}
	strcpy(result, "�˺Ż������������������!");
	return false;
}

//�����̿�
//b_indexΪ������ڿ�λͼ�е���ʼλ��
long block_alloc(int b_len, int &b_index)
{
	int i = 0, j = 0;
	long addr = -1; //���ڷ��ط������׵�ַ
	//���п鲻��
	if (b_len > blockgroup[0].superblock.free_block_num) return addr;

	int count = 0; //�������ݿ���
	int avl_block_index = 0; //��һ���������ݿ��λ������
	int blockgroup_index = 0; //��һ���õ������ݿ����λ������
	bool blockgroup_used[BLOCK_GROUP_NUM];	//��¼�õ������ݿ���
	int blockgroup_count[BLOCK_GROUP_NUM]; //�õ������ݿ�������ݿ���Ŀ

	for (i = 0; i < BLOCK_GROUP_NUM; i++) {
		blockgroup_used[i] = false;
		blockgroup_count[i] = 0;
	}
	for (i = 0; i < BLOCK_GROUP_NUM; i++) {
		if (count == 0) { //�ж������Ƿ����㹻�Ŀ� **********
			if (b_len > blockgroup[i].groupdescription.group_free_blocks_num) continue;
		}
		for (j = 0; j < BLOCK_NUM_PER_GROUP; j++) { //���Ҳ�ͳ���������п�
			if (block_bitmap[(blockgroup[i].groupdescription.group_block_bitmap + j)] == NOT_USED) {  //���п�
				count++;
				blockgroup_used[i] = true;
				blockgroup_count[i]++;
				if (count == 1) { //��һ�����ݿ�
					addr = blockgroup[i].groupdescription.group_address + j * BLOCK_SIZE;
					avl_block_index = i * BLOCK_NUM_PER_GROUP + j;
					b_index = avl_block_index;
					blockgroup_index = i;
				}
			}
			else { //�м������ʹ�õĿ飬�����п鲻����
				count = 0; //���¼���
				if (j == 0 && (i - 1) >= 0 && blockgroup_used[i - 1]) { //ǡ�õ�����һ����������Ҫ��λ��һ���������Ϣ
					blockgroup_used[i - 1] = false;
					blockgroup_count[i - 1] = 0;
				}
				blockgroup_used[i] = false;
				blockgroup_count[i] = 0;
			}
			if (count == b_len)break; //��������
		}
		if (count == b_len)break; //��������
	}
	if (count != b_len) { //�������㹻��������
		addr = -1;
		return addr;
	}
	//����ɹ��������Ϣ
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //���³�����
		blockgroup[i].superblock.free_block_num -= b_len;
	}
	for (i = blockgroup_index; i < BLOCK_GROUP_NUM; i++) { //������������
		if (blockgroup_used[i]) blockgroup[i].groupdescription.group_free_blocks_num -= blockgroup_count[i];
	}
	j = avl_block_index + b_len;
	for (i = avl_block_index; i < j; i++) { //���¿�λͼ
		block_bitmap[i] = USED;
	}
	return addr;
}

//�ͷ��̿�
//b_indexΪ��Ҫɾ���Ŀ��ڿ�λͼ�е���ʼλ��
void block_free(int b_len, int b_index)
{
	int i = 0;
	int block_end = b_len + b_index; //��ĩβ��ַ
	int free_blockgroup_count[BLOCK_GROUP_NUM]; //��¼��λͼ��Ӧ�����ݿ���������µ����ݿ���Ŀ 
	//������Ϣ
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //���³�����
		blockgroup[i].superblock.free_block_num += b_len;
		free_blockgroup_count[i] = 0;
	}
	for (i = b_index; i < block_end; i++) { //���¿�λͼ
		block_bitmap[i] = NOT_USED;
		free_blockgroup_count[i / BLOCK_NUM_PER_GROUP]++; //��i��������Ӧ����
	}
	for (i = b_index / BLOCK_NUM_PER_GROUP; i < BLOCK_NUM_PER_GROUP; i++) { //������������
		if (free_blockgroup_count[i] != 0) { //����Ҫ�ͷŵ����ݿ�
			blockgroup[i].groupdescription.group_free_blocks_num += free_blockgroup_count[i];
		}
		else { //�����ݿ��ͷ�
			break;
		}
	}
}

//����i�ڵ�
int inode_alloc()
{
	int b_index, i_index = -1; //i�ڵ����ڵ����ݿ���,i�ڵ�����
	int i = 0, j = 0;
	int temp;
	for (i = 0; i < BLOCK_GROUP_NUM; i++) {
		for (j = 0; j < INODE_NUM_PER_GROUP; j++) {
			temp = blockgroup[i].groupdescription.group_inode_bitmap + j;
			if (inode_bitmap[temp] == NOT_USED) {
				b_index = i;
				i_index = temp;
				break;
			}
		}
		if (i_index != -1) break;
	}
	if (i_index != -1) { //����ɹ��������Ϣ
		for (i = 0; i < BLOCK_GROUP_NUM; i++) {	//���³�����
			blockgroup[i].superblock.free_inode_num -= 1;
		}
		blockgroup[b_index].groupdescription.group_free_inodes_num -= 1; //������������
		inode_bitmap[i_index] = USED; //����i�ڵ�λͼ
	}
	return i_index;
}

//�ͷ�i�ڵ�
void inode_free(int f_inode)
{
	int i = 0;
	//������Ϣ
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //���³�����
		blockgroup[i].superblock.free_inode_num += 1;
	}
	blockgroup[f_inode / BLOCK_NUM_PER_GROUP].groupdescription.group_free_inodes_num += 1; //������������
	inode_bitmap[f_inode] = NOT_USED; //����inodeλͼ
}

//��ʾ��ǰ·��
void show_cur_path(Directory cur_dir)
{
	Directory temp_dir = cur_dir;
	//�ݹ�ֱ���ҵ���Ŀ¼
	if (cur_dir.inode_no != 0) {
		//���Ҹ�Ŀ¼
		virtualDisk.open("vfs", ios::in | ios::binary);
		if (!virtualDisk.is_open()) {
			strcpy(result, "��ȡ�ļ�ϵͳʧ��");
			Sleep(2000);
			exit(0);
		}
		virtualDisk.seekg(inode_table[cur_dir.subinode[1]].block_address, ios::beg);
		virtualDisk.read((char*)(&cur_dir), sizeof(Directory));
		virtualDisk.close();
		show_cur_path(cur_dir);
	}
	//���õ�ǰ·���ַ���
	if (temp_dir.inode_no == 0) {
		strcpy(current_path, "root");
	}
	else {
		stringstream sstream;
		sstream << current_path << "/" << inode_table[temp_dir.inode_no].fdname; //�ϲ�Ŀ¼
		sstream >> current_path;
		sstream.clear();
	}
}

//��·������ȡĿ¼��
bool get_dirname(const char *path, int len, int pos, char *filename)
{
	int i = 0;
	char *temp = new char[len];
	char dirname[MAX_FILENAME_SIZE];
	char *q; //�����и����Ӵ�
	//��/�����и�
	strcpy(temp, path);
	q = strtok(temp, "/");
	if (q == NULL) return false;
	strcpy(dirname, q);
	q = strtok(NULL, "/");
	for (i = 1; i < pos; i++) {
		if (q) {
			if (q == NULL) return false;
			strcpy(dirname, q);
			//����ָ�򱻷ָ��Ƭ�ε�ָ��
			q = strtok(NULL, "/");
		}
		else {
			return false;
		}
	}
	strcpy(filename, dirname);
	return true;
}

//����·��Ѱ�Ҷ�ӦĿ¼
bool find_path_dir(const char *path, int len, int pos, char *dirname, Directory &tempdir)
{
	//��ȡÿһ��Ŀ¼
	while (get_dirname(path, len, pos, dirname)) {
		int i = 0;
		int dirsize = tempdir.subsize;
		for (i = 2; i < dirsize; i++) { //Ŀ¼���ǰ����Ϊ��ǰĿ¼����һ��Ŀ¼
			if (strcmp(dirname, inode_table[tempdir.subinode[i]].fdname) == 0 && inode_table[tempdir.subinode[i]].filetype == FT_DIR) { //ƥ����Ŀ¼
				virtualDisk.seekg(inode_table[tempdir.subinode[i]].block_address, ios::beg);
				virtualDisk.read((char*)(&tempdir), sizeof(Directory));
				break;
			}
		}
		if (i < dirsize) {	//ƥ�䵽�������Ŀ¼����Ѱ��
			pos++;
		}
		else { //ƥ�䲻��
			strcpy(result, "ָ��·��������");
			return false;
		}
	}
	return true;
}

//����·����ȡ������Ŀ¼��Ŀ¼���ļ���
bool get_dir_and_fdname(const char *path, int len, Directory &tempdir, char *filename)
{
	tempdir = current_dir; //����·���е�Ŀ¼
	const char *divide = strrchr(path, '/'); //�ֽ�·��
	if (divide) {
		int i = 0;
		int div_pos = int(divide - path); //���һ��/��λ��
		int plen = div_pos + 1; //�������ļ�����Ŀ¼����·���ĳ���
		char *newpath = new char[plen]; //�������ļ�����Ŀ¼����·��

		//��ȡ���ҵ��ļ�����Ŀ¼��
		for (i = 1; i < len - div_pos; i++) {
			filename[i - 1] = divide[i];
		}
		filename[i - 1] = 0;
		//��ȡ�������ļ�����Ŀ¼����·��
		if (div_pos > 0) {
			for (i = 0; i < div_pos; i++) {
				newpath[i] = path[i];
			}
			newpath[i] = 0;
		}
		else //ֻ��һ��Ŀ¼
		{
			delete(newpath);
			if (path[0] == '/') {
				newpath = new char[2];
				newpath[0] = '/';
				newpath[1] = '\0';
			}
			else {
				return true;
			}
		}

		//���·��
		char gdirname[MAX_FILENAME_SIZE];
		int ppos = 1; //·����Ŀ¼��λ��(�ڼ���Ŀ¼)
		get_dirname(newpath, plen, ppos, gdirname); //��·������ȡ��һ��Ŀ¼��
		virtualDisk.open("vfs", ios::in | ios::binary);
		if (!virtualDisk.is_open()) {
			strcpy(result, "��ȡ�ļ�ϵͳʧ��");
			Sleep(2000);
			exit(0);
		}
		virtualDisk.seekg(inode_table[0].block_address, ios::beg);
		virtualDisk.read((char*)(&tempdir), sizeof(Directory));

		//���Ҷ�Ӧ��Ŀ¼
		if (newpath[0] == '/' || strcmp(gdirname, "root") == 0) { //��Ŀ¼
			if (newpath[0] != '/') ppos++;
			if (find_path_dir(newpath, plen, ppos, gdirname, tempdir)) { //�ҵ�
				virtualDisk.close();
				delete(newpath);
				return true;
			}
			else { //û�ҵ�
				virtualDisk.close();
				delete(newpath);
				return false;
			}
		}
		else if (strcmp(gdirname, "..") == 0) { //ת���ϼ�Ŀ¼
			virtualDisk.seekg(inode_table[tempdir.inode_no].block_address, ios::beg);
			virtualDisk.read((char*)(&tempdir), sizeof(Directory));
			ppos++;
			if (find_path_dir(newpath, plen, ppos, gdirname, tempdir)) { //�ҵ�
				virtualDisk.close();
				delete(newpath);
				return true;
			}
			else { //û�ҵ�
				virtualDisk.close();
				delete(newpath);
				return false;
			}
		}
		else { //�ڵ�ǰĿ¼����
			tempdir = current_dir;
			if (strcmp(gdirname, ".") == 0) ppos++;
			if (find_path_dir(newpath, plen, ppos, gdirname, tempdir)) { //�ҵ�
				virtualDisk.close();
				delete(newpath);
				return true;
			}
			else { //û�ҵ�
				virtualDisk.close();
				delete(newpath);
				return false;
			}
		}
	}
	else { //Ĭ��Ϊ��ǰĿ¼
		strcpy(filename, path);	//�����ļ���
		return true;
	}
}

//����·����ȡ������Ŀ¼
bool get_dir(const char *path, int len, Directory &tempdir)
{
	tempdir = current_dir; //����·���е�Ŀ¼
	if (strcmp(path, "..") == 0) { //�ϼ�Ŀ¼
		virtualDisk.open("vfs", ios::in | ios::binary);
		if (!virtualDisk.is_open()) {
			strcpy(result, "��ȡ�ļ�ϵͳʧ��");
			Sleep(2000);
			exit(0);
		}
		virtualDisk.seekg(inode_table[tempdir.subinode[1]].block_address, ios::beg); //1Ϊ�ϼ�Ŀ¼
		virtualDisk.read((char*)(&tempdir), sizeof(Directory));
		virtualDisk.close();
		return true;
	}
	else if (strcmp(path, ".") == 0) { //��ǰĿ¼
		return true;
	}
	else {
		//���·��
		char gdirname[MAX_FILENAME_SIZE];
		int ppos = 1; //·����Ŀ¼��λ��(�ڼ���Ŀ¼)
		get_dirname(path, len, ppos, gdirname); //��·������ȡ��һ��Ŀ¼��
		virtualDisk.open("vfs", ios::in | ios::binary);
		if (!virtualDisk.is_open()) {
			strcpy(result, "��ȡ�ļ�ϵͳʧ��");
			Sleep(2000);
			exit(0);
		}
		virtualDisk.seekg(inode_table[0].block_address, ios::beg);
		virtualDisk.read((char*)(&tempdir), sizeof(Directory));

		//���Ҷ�Ӧ��Ŀ¼
		if (path[0] == '/' || strcmp(gdirname, "root") == 0) { //��Ŀ¼
			if (path[0] != '/') ppos++;
			if (find_path_dir(path, len, ppos, gdirname, tempdir)) { //�ҵ�
				virtualDisk.close();
				return true;
			}
			else { //û�ҵ�
				virtualDisk.close();
				return false;
			}
		}
		else if (strcmp(gdirname, "..") == 0) { //ת���ϼ�Ŀ¼
			virtualDisk.seekg(inode_table[tempdir.inode_no].block_address, ios::beg);
			virtualDisk.read((char*)(&tempdir), sizeof(Directory));
			ppos++;
			if (find_path_dir(path, len, ppos, gdirname, tempdir)) { //�ҵ�
				virtualDisk.close();
				return true;
			}
			else { //û�ҵ�
				virtualDisk.close();
				return false;
			}
		}
		else { //�ڵ�ǰĿ¼����
			tempdir = current_dir;
			if (strcmp(gdirname, ".") == 0) ppos++;
			if (find_path_dir(path, len, ppos, gdirname, tempdir)) { //�ҵ�
				virtualDisk.close();
				return true;
			}
			else { //û�ҵ�
				virtualDisk.close();
				return false;
			}
		}
	}
}

//�ж�·���Ƿ�Ϊhost�ļ�ϵͳ·��
bool host_path(char *path)
{
	char *temp;
	//�ж�path���Ƿ�<host>
	temp = strstr(path, "<host>");
	if (temp == path) {
		strcpy(path, temp + 6);
		return true;
	}
	return false;
}