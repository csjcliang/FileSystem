#pragma once

#include <fstream>
#include <sstream>
#include <iomanip>
#include <time.h>
using namespace std;

#define BLOCK_GROUP_NUM 100 //������100
#define BLOCK_NUM_PER_GROUP 1024 //ÿ�����麬�̿���
#define INODE_NUM_PER_GROUP 1024 //ÿ�����麬i�ڵ���
#define BLOCK_SIZE 1024 //�̿��С1k
#define DIR_SIZE (sizeof(Directory)/BLOCK_SIZE+1) //һ��Ŀ¼ռ�̿���

#define TOTAL_BLOCK_NUM (BLOCK_GROUP_NUM*BLOCK_NUM_PER_GROUP) //�̿�����
#define TOTAL_INODE_NUM (BLOCK_GROUP_NUM*INODE_NUM_PER_GROUP) //i�ڵ�����

#define DATA_BLOCK_BEGIN (BLOCK_GROUP_NUM*sizeof(Blockgroup)+(TOTAL_BLOCK_NUM + TOTAL_INODE_NUM)*sizeof(Bitmap)+TOTAL_INODE_NUM*sizeof(Inode)) //��һ�����ݿ���׵�ַ��������λ�ڳ�����+��������+��λͼ+i�ڵ�λͼ+i�ڵ�֮��

#define MAX_FILE_NUM 256 //һ��Ŀ¼�¿ɺ��ļ�(����Ŀ¼)�������Ŀ
#define MAX_PATH_SIZE 256 //·����󳤶�
#define MAX_FILENAME_SIZE 256 //�ļ���Ŀ¼����󳤶�
#define MAX_USERNAME_SIZE 32 //�û�����󳤶�
#define MAX_PASSWORD_SIZE 16 //������󳤶�
#define MAX_COMMAND_SIZE 128 //������󳤶�
#define MAX_USER_NUM 5 //����û���

#define MAX_WHOLE_COMMAND_SIZE MAX_COMMAND_SIZE + MAX_PATH_SIZE * 2 + 2 //������������󳤶ȣ�������󳤶�+2*·����󳤶�+2*�ո�

//���ͣ��ļ���Ŀ¼��
enum fileType
{
	FT_FILE, //�ļ�
	FT_DIR //Ŀ¼
};

//�ļ�Ȩ��
enum fileAccess
{
	READ_ONLY, //ֻ��
	WRITE_ONLY, //ֻд
	READ_WRITE, //�ɶ�д
};

//λͼ��0��1��
enum Bitmap
{
	NOT_USED, //δ��ʹ��
	USED //�ѱ�ʹ��
};

//�û�����
enum userType
{
	ROOT_USER, //���û��������û�������Ȩ�����ƣ�
	ADMIN, //����Ա�������û���������Ȩ�����ƣ�
	USER, //��ͨ�û��������û�������Ȩ�����ƣ�
};

//�û���
class User
{
private:
	int userid;	//�˺�
	char username[MAX_USERNAME_SIZE]; //����
	char password[MAX_PASSWORD_SIZE]; //����
	userType usertype; //�û�����
public:
	void setUser(int userid, const char *username, const char *password, userType usertype); //�����û���Ϣ
	int getUserid(); //��ȡ�û��˺�
	char* getUsername(); //��ȡ�û�����
	char* getPassword(); //��ȡ�û�����
	userType getUsertype(); //��ȡ�û�����
};

//ʱ����
class Datetime
{
private:
	int year; //��
	int month; //��
	int day; //��
	int hour; //ʱ
	int minute;	//��
	int second;	//��
public:
	void setDatetime(tm datetime); //��������ʱ��
	tm getDatetime(); //��ȡ����ʱ��
};

//��������
class Superblock 
{
public:
	int block_num; //�̿�����
	int inode_num; //i�ڵ�����
	int block_size; //�̿��С
	int free_block_num; //���п���
	int free_inode_num; //����i�ڵ���
	int data_block_address; //��һ�����ݿ���ʼ��ַ
};

//��������
class groupDescription
{
public:
	long group_address; //��������������ʼ��ַ
	int group_block_bitmap; //�������ݿ�λͼ���ڿ��
	int group_inode_bitmap; //����i�ڵ�λͼ���ڿ��
	int group_inode_table; //����i�ڵ�����ڿ��
	int group_free_blocks_num; //���п��п���
	int group_free_inodes_num; //���п���i�ڵ���
};

//�����ࣨ�����������������
class Blockgroup
{
public:
	Superblock superblock; //������
	groupDescription groupdescription; //��������
};

//i�ڵ��࣬ÿ���ļ���Ŀ¼��Ӧһ��i�ڵ�
class Inode
{
public:
	char fdname[MAX_FILENAME_SIZE]; //�ļ���Ŀ¼��
	Datetime mtime; //�޸�ʱ��
	fileType filetype; //����
	long filesize; //��С
	int uid; //ӵ�����˺�
	fileAccess fileaccess; //�ļ�Ȩ��
	long block_address; //���̿���ʼ��ַ
	int blocknum; //����Ĵ��̿���
};

//Ŀ¼�ࣨĿ¼���ļ���
class Directory
{
public:
	int inode_no; //i�ڵ��
	int subsize; //���ļ�����Ŀ¼��
	int subinode[MAX_FILE_NUM]; //���ļ�����Ŀ¼i�ڵ���
	//Ŀ¼���������
	bool samename(const char *fdname); //�ж�Ŀ¼���Ƿ���ͬ���ļ���Ŀ¼
	void remove_dir(Directory rd_dir, int rd_index); //ɾ����Ŀ¼
	//�ļ����������
	void delete_file(const char *filename); //ɾ���ļ�
	void create_file(const char *filename, char *content, int filelen, fileAccess fileaccess); //�����ļ�
};

extern fstream virtualDisk; //��������ļ�

extern Blockgroup blockgroup[BLOCK_GROUP_NUM]; //���ݿ���
extern Bitmap block_bitmap[TOTAL_BLOCK_NUM]; //��λͼ
extern Bitmap inode_bitmap[TOTAL_INODE_NUM]; //i�ڵ�λͼ
extern Inode inode_table[TOTAL_INODE_NUM]; //i�ڵ��

extern Directory current_dir; //��ǰĿ¼
extern char current_path[MAX_PATH_SIZE]; //��ǰ·��

extern User user_group[MAX_USER_NUM]; //�û���
extern int current_user; //��ǰ�û�
extern userType current_usertype; //��ǰ�û�����