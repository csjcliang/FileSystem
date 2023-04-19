#pragma once

#include <fstream>
#include <sstream>
#include <iomanip>
#include <time.h>
using namespace std;

#define BLOCK_GROUP_NUM 100 //块组数100
#define BLOCK_NUM_PER_GROUP 1024 //每个块组含盘块数
#define INODE_NUM_PER_GROUP 1024 //每个块组含i节点数
#define BLOCK_SIZE 1024 //盘块大小1k
#define DIR_SIZE (sizeof(Directory)/BLOCK_SIZE+1) //一个目录占盘块数

#define TOTAL_BLOCK_NUM (BLOCK_GROUP_NUM*BLOCK_NUM_PER_GROUP) //盘块总数
#define TOTAL_INODE_NUM (BLOCK_GROUP_NUM*INODE_NUM_PER_GROUP) //i节点总数

#define DATA_BLOCK_BEGIN (BLOCK_GROUP_NUM*sizeof(Blockgroup)+(TOTAL_BLOCK_NUM + TOTAL_INODE_NUM)*sizeof(Bitmap)+TOTAL_INODE_NUM*sizeof(Inode)) //第一个数据块的首地址（数据区位于超级块+组描述符+块位图+i节点位图+i节点之后）

#define MAX_FILE_NUM 256 //一个目录下可含文件(或子目录)的最大数目
#define MAX_PATH_SIZE 256 //路径最大长度
#define MAX_FILENAME_SIZE 256 //文件或目录名最大长度
#define MAX_USERNAME_SIZE 32 //用户名最大长度
#define MAX_PASSWORD_SIZE 16 //密码最大长度
#define MAX_COMMAND_SIZE 128 //命令最大长度
#define MAX_USER_NUM 5 //最大用户数

#define MAX_WHOLE_COMMAND_SIZE MAX_COMMAND_SIZE + MAX_PATH_SIZE * 2 + 2 //总输入命令最大长度（命令最大长度+2*路径最大长度+2*空格）

//类型（文件或目录）
enum fileType
{
	FT_FILE, //文件
	FT_DIR //目录
};

//文件权限
enum fileAccess
{
	READ_ONLY, //只读
	WRITE_ONLY, //只写
	READ_WRITE, //可读写
};

//位图（0或1）
enum Bitmap
{
	NOT_USED, //未被使用
	USED //已被使用
};

//用户类型
enum userType
{
	ROOT_USER, //根用户（无视用户保护和权限限制）
	ADMIN, //管理员（无视用户保护但受权限限制）
	USER, //普通用户（遵守用户保护和权限限制）
};

//用户类
class User
{
private:
	int userid;	//账号
	char username[MAX_USERNAME_SIZE]; //姓名
	char password[MAX_PASSWORD_SIZE]; //密码
	userType usertype; //用户类型
public:
	void setUser(int userid, const char *username, const char *password, userType usertype); //设置用户信息
	int getUserid(); //获取用户账号
	char* getUsername(); //获取用户姓名
	char* getPassword(); //获取用户密码
	userType getUsertype(); //获取用户类型
};

//时间类
class Datetime
{
private:
	int year; //年
	int month; //月
	int day; //日
	int hour; //时
	int minute;	//分
	int second;	//秒
public:
	void setDatetime(tm datetime); //设置日期时间
	tm getDatetime(); //获取日期时间
};

//超级块类
class Superblock 
{
public:
	int block_num; //盘块总数
	int inode_num; //i节点总数
	int block_size; //盘块大小
	int free_block_num; //空闲块数
	int free_inode_num; //空闲i节点数
	int data_block_address; //第一个数据块起始地址
};

//组描述符
class groupDescription
{
public:
	long group_address; //组在数据区的起始地址
	int group_block_bitmap; //组中数据块位图所在块号
	int group_inode_bitmap; //组中i节点位图所在块号
	int group_inode_table; //组中i节点表所在块号
	int group_free_blocks_num; //组中空闲块数
	int group_free_inodes_num; //组中空闲i节点数
};

//块组类（超级块和组描述符）
class Blockgroup
{
public:
	Superblock superblock; //超级块
	groupDescription groupdescription; //组描述符
};

//i节点类，每个文件或目录对应一个i节点
class Inode
{
public:
	char fdname[MAX_FILENAME_SIZE]; //文件或目录名
	Datetime mtime; //修改时间
	fileType filetype; //类型
	long filesize; //大小
	int uid; //拥有者账号
	fileAccess fileaccess; //文件权限
	long block_address; //磁盘块起始地址
	int blocknum; //分配的磁盘块数
};

//目录类（目录及文件）
class Directory
{
public:
	int inode_no; //i节点号
	int subsize; //子文件或子目录数
	int subinode[MAX_FILE_NUM]; //子文件或子目录i节点组
	//目录类操作函数
	bool samename(const char *fdname); //判断目录下是否有同名文件或目录
	void remove_dir(Directory rd_dir, int rd_index); //删除子目录
	//文件类操作函数
	void delete_file(const char *filename); //删除文件
	void create_file(const char *filename, char *content, int filelen, fileAccess fileaccess); //创建文件
};

extern fstream virtualDisk; //虚拟磁盘文件

extern Blockgroup blockgroup[BLOCK_GROUP_NUM]; //数据块组
extern Bitmap block_bitmap[TOTAL_BLOCK_NUM]; //块位图
extern Bitmap inode_bitmap[TOTAL_INODE_NUM]; //i节点位图
extern Inode inode_table[TOTAL_INODE_NUM]; //i节点表

extern Directory current_dir; //当前目录
extern char current_path[MAX_PATH_SIZE]; //当前路径

extern User user_group[MAX_USER_NUM]; //用户组
extern int current_user; //当前用户
extern userType current_usertype; //当前用户类型