#include "pch.h"
#include <iostream>
#include <windows.h>
#include "ipc.h"
#include "function.h"

using namespace std;

//设置日期时间
void Datetime::setDatetime(tm datetime)
{
	this->year = datetime.tm_year;
	this->month = datetime.tm_mon;
	this->day = datetime.tm_mday;
	this->hour = datetime.tm_hour;
	this->minute = datetime.tm_min;
	this->second = datetime.tm_sec;
}

////获取日期时间
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

//设置用户信息
void User::setUser(int userid, const char *username, const char *password, userType usertype)
{
	this->userid = userid;
	strcpy(this->username, username);
	strcpy(this->password, password);
	this->usertype = usertype;
}
//获取用户账号
int User::getUserid()
{
	return userid;
}
//获取用户姓名
char* User::getUsername()
{
	return username;
}
//获取用户密码
char* User::getPassword()
{
	return password;
}
//获取用户类型
userType User::getUsertype()
{
	return usertype;
}

//判断目录下是否有同名文件或目录
bool Directory::samename(const char *fdname)
{
	for (int i = 2; i < subsize; i++) {
		if (strcmp(fdname, inode_table[this->subinode[i]].fdname) == 0) return true;
	}
	return false;
}

//删除子目录
void Directory::remove_dir(Directory rd_dir, int rd_index)
{
	int i = 0;
	for (i = 2; i < rd_dir.subsize; i++) {
		if (inode_table[rd_dir.subinode[i]].filetype == FT_DIR) { //为目录则递归删除子文件和子目录
			Directory rd_subdir;
			virtualDisk.open("vfs", ios::in | ios::binary);
			if (!virtualDisk.is_open()) {
				strcpy(result, "写入文件系统失败");
				Sleep(2000);
				exit(0);
			}
			virtualDisk.seekg(inode_table[rd_dir.subinode[i]].block_address, ios::beg);
			virtualDisk.read((char*)(&rd_subdir), sizeof(Directory));
			virtualDisk.close();
			//删除子文件和子目录
			rd_dir.remove_dir(rd_subdir, i);
		}
		else { //为文件则直接删除
			rd_dir.delete_file(inode_table[rd_dir.subinode[i]].fdname);
		}
	}
	//删除相关信息
	block_free(inode_table[rd_dir.inode_no].blocknum, ((inode_table[rd_dir.inode_no].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE)); //释放块
	inode_free(rd_dir.inode_no); //释放i节点
	//更新父目录，后面的文件及目录补空
	for (i = rd_index; i < this->subsize; i++) {
		this->subinode[i] = this->subinode[i + 1];
	}
	this->subsize--;
	if (this->inode_no == current_dir.inode_no) {
		current_dir = *this; //更新当前目录
	}

	//写回磁盘
	virtualDisk.open("vfs", ios::out | ios::binary | ios::_Nocreate);
	if (!virtualDisk.is_open()) {
		strcpy(result, "写入文件系统失败");
		Sleep(2000);
		exit(0);
	}
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //写数据块组
		virtualDisk.write((char*)(&blockgroup[i]), sizeof(Blockgroup));
	}

	//写i节点位图
	virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + rd_dir.inode_no * sizeof(Bitmap), ios::beg);
	virtualDisk.write((char*)(&inode_bitmap[rd_dir.inode_no]), sizeof(Bitmap));

	//写块位图
	virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + TOTAL_INODE_NUM * (sizeof(Bitmap) + sizeof(Inode)) + ((inode_table[rd_dir.inode_no].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE) * sizeof(Bitmap), ios::beg);
	for (i = 0; i < inode_table[rd_dir.inode_no].blocknum; i++) {
		virtualDisk.write((char*)(&block_bitmap[(inode_table[rd_dir.inode_no].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE]), sizeof(Bitmap));
	}
	//写父目录
	virtualDisk.seekp(inode_table[this->inode_no].block_address, ios::beg);
	virtualDisk.write((char*)(this), sizeof(Directory));
	virtualDisk.close();
}

//删除文件
void Directory::delete_file(const char* filename)
{
	int i = 0;
	int df_pos = 0; //删除文件的位置
	int df_inode = -1; //删除文件的i节点

	//查找文件
	for (i = 2; i < this->subsize; i++) {
		if (strcmp(inode_table[subinode[i]].fdname, filename) == 0 && inode_table[subinode[i]].filetype == FT_FILE) {
			df_pos = i;
			df_inode = this->subinode[i];
			break;
		}
	}
	if (i == this->subsize) { //找不到文件
		strcpy(result, "找不到文件");
		return;
	}

	//判断用户权限
	if (current_usertype == USER && inode_table[df_inode].uid != current_user || inode_table[df_inode].uid == 0) {
		strcpy(result, "没有权限删除文件");
		return;
	}

	//释放块和i节点
	block_free(inode_table[df_inode].blocknum, ((inode_table[df_inode].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE));
	inode_free(df_inode);
	//更新父目录信息
	for (i = df_pos; i < this->subsize; i++) {
		this->subinode[i] = this->subinode[i + 1]; //后面的文件补空
	}
	this->subsize--;
	if (this->inode_no == current_dir.inode_no) {
		current_dir = *this;
	}
	//写回磁盘
	virtualDisk.open("vfs", ios::out | ios::binary | ios::_Nocreate);
	if (!virtualDisk.is_open()) {
		strcpy(result, "写入文件系统失败");
		Sleep(2000);
		exit(0);
	}
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //写数据块组
		virtualDisk.write((char*)(&blockgroup[i]), sizeof(Blockgroup));
	}
	//写i节点位图
	virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + df_inode * sizeof(Bitmap), ios::beg);
	virtualDisk.write((char*)(&inode_bitmap[df_inode]), sizeof(Bitmap));
	//写块位图
	virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + TOTAL_INODE_NUM * (sizeof(Bitmap) + sizeof(Inode)) + ((inode_table[df_inode].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE) * sizeof(Bitmap), ios::beg);
	for (i = 0; i < inode_table[df_inode].blocknum; i++) {
		virtualDisk.write((char*)(&block_bitmap[(inode_table[df_inode].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE]), sizeof(Bitmap));
	}
	//写父目录
	virtualDisk.seekp(inode_table[this->inode_no].block_address, ios::beg);
	virtualDisk.write((char*)(this), sizeof(Directory));
	virtualDisk.close();
	strcpy(result, "删除文件成功");
}

//创建文件
void Directory::create_file(const char* filename, char *content, int filelen, fileAccess fileaccess)
{
	int i = 0;
	int cf_blocknum; //文件占块数
	int cf_addr = -1; //保存文件盘块的首地址
	int cf_index; //保存文件盘块在块位图的位置
	int cf_inode = -1; //文件i节点

	//计算文件占盘块数
	if ((filelen + 1) % BLOCK_SIZE == 0) { //整除
		cf_blocknum = (filelen + 1) / BLOCK_SIZE;
	}
	else {
		cf_blocknum = (filelen + 1) / BLOCK_SIZE + 1;
	}

	//分配块和i节点
	cf_addr = block_alloc(cf_blocknum, cf_index);
	if (cf_addr < 0) {
		strcpy(result, "磁盘空间不足，创建文件失败");
		return;
	}
	cf_inode = inode_alloc();
	if (cf_inode < 0) {
		strcpy(result, "i节点分配失败，创建文件失败");
		return;
	}

	//修改i节点信息
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

	//修改父目录信息
	this->subinode[this->subsize] = cf_inode;
	this->subsize++;
	if (this->inode_no == current_dir.inode_no) {
		current_dir = *this;
	}

	//写回磁盘
	virtualDisk.open("vfs", ios::out | ios::binary | ios::_Nocreate);
	if (!virtualDisk.is_open()) {
		strcpy(result, "写入文件系统失败");
		Sleep(2000);
		exit(0);
	}
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //写数据块组
		virtualDisk.write((char*)(&blockgroup[i]), sizeof(Blockgroup));
	}
	//写i节点位图
	virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + cf_inode * sizeof(Bitmap), ios::beg);
	virtualDisk.write((char*)(&inode_bitmap[cf_inode]), sizeof(Bitmap));
	//写i节点表
	virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + TOTAL_INODE_NUM * sizeof(Bitmap) + cf_inode * sizeof(Inode), ios::beg);
	virtualDisk.write((char*)(&inode_table[cf_inode]), sizeof(Inode));
	//写块位图
	virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + TOTAL_INODE_NUM * (sizeof(Bitmap) + sizeof(Inode)) + cf_index * sizeof(Bitmap), ios::beg);
	for (i = 0; i < cf_blocknum; i++) {
		virtualDisk.write((char*)(&block_bitmap[cf_index]), sizeof(Bitmap));
	}
	//写文件
	virtualDisk.seekp(cf_addr, ios::beg);
	virtualDisk.write((char*)(content), filelen);
	//写父目录
	virtualDisk.seekp(inode_table[this->inode_no].block_address, ios::beg);
	virtualDisk.write((char*)(this), sizeof(Directory));
	virtualDisk.close();
	strcpy(result, "创建文件成功");
}

//加载初始化信息
void sys_load()
{
	cout << "-------- 欢迎使用模拟Linux文件系统 --------" << endl;
	virtualDisk.open("vfs", ios::in | ios::binary);
	if (!virtualDisk.is_open()) { //文件系统未初始化
		char input = '\0'; //用户输入字符
		while (1) {
			if (input != '\n') cout << "模拟Linux文件系统未初始化, 是否进行初始化?（Y/N）";
			input = getchar();
			if (input == 'Y' || input == 'y' || input == 'N' || input == 'n') {
				break;
			}
		}
		if (input == 'Y' || input == 'y') { //初始化文件系统
			virtualDisk.clear();
			sys_init();
			return;
		}
		else {
			cout << "文件系统初始化失败" << endl;
			Sleep(2000);
			exit(0);
		}
	}
	//若已经初始化则读入信息
	int i = 0;
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //读入数据块组
		virtualDisk.read((char*)(&blockgroup[i]), sizeof(Blockgroup));
	}
	for (i = 0; i < TOTAL_INODE_NUM; i++) { //读入i节点位图
		virtualDisk.read((char*)(&inode_bitmap[i]), sizeof(Bitmap));
	}
	for (i = 0; i < TOTAL_INODE_NUM; i++) { //读入i节点表
		virtualDisk.read((char*)(&inode_table[i]), sizeof(Inode));
	}
	for (i = 0; i < TOTAL_BLOCK_NUM; i++) { //读入块位图
		virtualDisk.read((char*)(&block_bitmap[i]), sizeof(Bitmap));
	}
	virtualDisk.read((char*)(&current_dir), sizeof(Directory));
	virtualDisk.close();
	strcpy(current_path, inode_table[current_dir.inode_no].fdname); //复制当前路径

	//初始化用户信息
	init_user();
}

//初始化文件系统
void sys_init()
{
	virtualDisk.clear(); //清除流状态
	cout << "正在进行初始化……" << endl;

	int i = 0;
	//初始化数据块组
	for (i = 0; i < BLOCK_GROUP_NUM; i++) {
		//初始化超级块
		blockgroup[i].superblock.block_num = TOTAL_BLOCK_NUM;
		blockgroup[i].superblock.inode_num = TOTAL_INODE_NUM;
		blockgroup[i].superblock.block_size = BLOCK_SIZE;
		blockgroup[i].superblock.free_block_num = TOTAL_BLOCK_NUM - DIR_SIZE; //其中一个块用于根目录
		blockgroup[i].superblock.free_inode_num = TOTAL_INODE_NUM - 1; //其中一个i节点用于根目录
		blockgroup[i].superblock.data_block_address = DATA_BLOCK_BEGIN;
		//初始化组描述符
		blockgroup[i].groupdescription.group_address = DATA_BLOCK_BEGIN + i * BLOCK_NUM_PER_GROUP*BLOCK_SIZE;
		blockgroup[i].groupdescription.group_block_bitmap = i * BLOCK_NUM_PER_GROUP;
		blockgroup[i].groupdescription.group_free_blocks_num = BLOCK_NUM_PER_GROUP;
		blockgroup[i].groupdescription.group_free_inodes_num = INODE_NUM_PER_GROUP;
		blockgroup[i].groupdescription.group_inode_bitmap = i * INODE_NUM_PER_GROUP;
		blockgroup[i].groupdescription.group_inode_table = i * INODE_NUM_PER_GROUP;
	}
	//初始化i节点位图
	for (i = 0; i < TOTAL_INODE_NUM; i++) {
		inode_bitmap[i] = NOT_USED;
	}
	//初始化i节点表
	for (i = 0; i < TOTAL_INODE_NUM; i++) {
		inode_table[i].block_address = -1;
		inode_table[i].blocknum = 0;
		inode_table[i].fileaccess = READ_WRITE;
		inode_table[i].filesize = 0;
		inode_table[i].uid = 0;
		inode_table[i].filetype = FT_DIR;
	}
	//初始化块位图
	for (i = 0; i < TOTAL_BLOCK_NUM; i++) {
		block_bitmap[i] = NOT_USED;
	}
	//初始化根目录信息
	//初始化根目录所在数据块组
	blockgroup[0].groupdescription.group_free_blocks_num = BLOCK_NUM_PER_GROUP - DIR_SIZE;
	blockgroup[0].groupdescription.group_free_inodes_num = INODE_NUM_PER_GROUP - 1;
	//初始化根目录的数据块位图
	for (i = 0; i < DIR_SIZE; i++) {
		block_bitmap[i] = USED;
	}
	//初始化根目录的i节点位图
	inode_bitmap[0] = USED;
	//初始化根目录的i节点表
	strcpy(inode_table[0].fdname, "root");
	time_t t;
	time(&t);
	tm local = *localtime(&t);
	inode_table[0].mtime.setDatetime(local);
	inode_table[0].block_address = DATA_BLOCK_BEGIN; //根目录位于起始位置
	inode_table[0].blocknum = DIR_SIZE;
	inode_table[0].fileaccess = READ_WRITE;
	inode_table[0].filesize = sizeof(Directory); //根目录大小
	inode_table[0].uid = 0;
	inode_table[0].filetype = FT_DIR;
	//设置根目录信息
	current_dir.inode_no = 0; //根目录i节点号为0
	current_dir.subsize = 2; //当前目录和上一级目录
	current_dir.subinode[0] = 0; //当前目录为根目录
	current_dir.subinode[1] = 0; //根目录上一级目录仍为根目录
	//设置当前路径
	strcpy(current_path, "root");

	//写入文件
	virtualDisk.open("vfs", ios::out | ios::binary);
	if (!virtualDisk.is_open()) {
		cout << "写入文件系统失败" << endl;
		Sleep(2000);
		exit(0);
	}
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //写数据块组
		virtualDisk.write((char*)(&blockgroup[i]), sizeof(Blockgroup)); //**************
	}
	for (i = 0; i < TOTAL_INODE_NUM; i++) { //写i节点位图
		virtualDisk.write((char*)(&inode_bitmap[i]), sizeof(Bitmap));
	}
	for (i = 0; i < TOTAL_INODE_NUM; i++) { //写i节点表
		virtualDisk.write((char*)(&inode_table[i]), sizeof(Inode));
	}
	for (i = 0; i < TOTAL_BLOCK_NUM; i++) { //写块位图
		virtualDisk.write((char*)(&block_bitmap[i]), sizeof(Bitmap));
	}
	//写根目录
	virtualDisk.seekp(inode_table[current_dir.inode_no].block_address, ios::beg);
	virtualDisk.write((char*)(&current_dir), sizeof(Directory));

	//分配100M的空间
	long filesize = BLOCK_SIZE * TOTAL_BLOCK_NUM;
	char *buf = new char[filesize];
	virtualDisk.write((char*)(buf), filesize * sizeof(char));
	cout << "初始化完成,正在进入登录界面……" << endl;
	virtualDisk.close();
	delete(buf);
	Sleep(1000);
	//初始化用户信息
	init_user();

	system("cls");
	cout << "-------- 欢迎来到登录界面 --------" << endl;
}

//初始化用户信息
void init_user()
{
	user_group[0].setUser(0, "root_user", "root3373", ROOT_USER); //设置0号为超级用户（根用户）
	user_group[1].setUser(100, "ljc", "ljc3373", ADMIN); //设置1号为管理员
	//以下为普通用户
	user_group[2].setUser(101, "xiaoming", "xiaoming101", USER);
	user_group[3].setUser(102, "xiaohong", "xiaohong102", USER);
	user_group[4].setUser(103, "xiaogang", "xiaogang103", USER);
}

//登录
bool login(char *userid, char *password)
{
	int i = 0;
	//在用户表中查看账号密码是否正确
	for (i = 0; i < MAX_USER_NUM; i++) {
		if (atoi(userid) == user_group[i].getUserid() && strcmp(password, user_group[i].getPassword()) == 0) {
			current_user = atoi(userid);
			current_usertype = user_group[i].getUsertype();
			strcpy(result, "登录成功!");
			Sleep(500);
			return true;
		}
	}
	strcpy(result, "账号或密码错误，请重新输入!");
	return false;
}

//分配盘块
//b_index为分配块在块位图中的起始位置
long block_alloc(int b_len, int &b_index)
{
	int i = 0, j = 0;
	long addr = -1; //用于返回分配块的首地址
	//空闲块不足
	if (b_len > blockgroup[0].superblock.free_block_num) return addr;

	int count = 0; //连续数据块数
	int avl_block_index = 0; //第一个可用数据块的位置索引
	int blockgroup_index = 0; //第一个用到的数据块组的位置索引
	bool blockgroup_used[BLOCK_GROUP_NUM];	//记录用到的数据块组
	int blockgroup_count[BLOCK_GROUP_NUM]; //用到的数据块组的数据块数目

	for (i = 0; i < BLOCK_GROUP_NUM; i++) {
		blockgroup_used[i] = false;
		blockgroup_count[i] = 0;
	}
	for (i = 0; i < BLOCK_GROUP_NUM; i++) {
		if (count == 0) { //判断组中是否有足够的块 **********
			if (b_len > blockgroup[i].groupdescription.group_free_blocks_num) continue;
		}
		for (j = 0; j < BLOCK_NUM_PER_GROUP; j++) { //查找并统计连续空闲块
			if (block_bitmap[(blockgroup[i].groupdescription.group_block_bitmap + j)] == NOT_USED) {  //空闲块
				count++;
				blockgroup_used[i] = true;
				blockgroup_count[i]++;
				if (count == 1) { //第一个数据块
					addr = blockgroup[i].groupdescription.group_address + j * BLOCK_SIZE;
					avl_block_index = i * BLOCK_NUM_PER_GROUP + j;
					b_index = avl_block_index;
					blockgroup_index = i;
				}
			}
			else { //中间存在已使用的块，即空闲块不连续
				count = 0; //重新计数
				if (j == 0 && (i - 1) >= 0 && blockgroup_used[i - 1]) { //恰好到了下一个块组则需要复位上一个块组的信息
					blockgroup_used[i - 1] = false;
					blockgroup_count[i - 1] = 0;
				}
				blockgroup_used[i] = false;
				blockgroup_count[i] = 0;
			}
			if (count == b_len)break; //块数够了
		}
		if (count == b_len)break; //块数够了
	}
	if (count != b_len) { //不存在足够的连续块
		addr = -1;
		return addr;
	}
	//分配成功后更新信息
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //更新超级块
		blockgroup[i].superblock.free_block_num -= b_len;
	}
	for (i = blockgroup_index; i < BLOCK_GROUP_NUM; i++) { //更新组描述符
		if (blockgroup_used[i]) blockgroup[i].groupdescription.group_free_blocks_num -= blockgroup_count[i];
	}
	j = avl_block_index + b_len;
	for (i = avl_block_index; i < j; i++) { //更新块位图
		block_bitmap[i] = USED;
	}
	return addr;
}

//释放盘块
//b_index为需要删除的块在块位图中的起始位置
void block_free(int b_len, int b_index)
{
	int i = 0;
	int block_end = b_len + b_index; //块末尾地址
	int free_blockgroup_count[BLOCK_GROUP_NUM]; //记录块位图对应的数据块组中需更新的数据块数目 
	//更新信息
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //更新超级块
		blockgroup[i].superblock.free_block_num += b_len;
		free_blockgroup_count[i] = 0;
	}
	for (i = b_index; i < block_end; i++) { //更新块位图
		block_bitmap[i] = NOT_USED;
		free_blockgroup_count[i / BLOCK_NUM_PER_GROUP]++; //第i个块所对应块组
	}
	for (i = b_index / BLOCK_NUM_PER_GROUP; i < BLOCK_NUM_PER_GROUP; i++) { //更新组描述符
		if (free_blockgroup_count[i] != 0) { //有需要释放的数据块
			blockgroup[i].groupdescription.group_free_blocks_num += free_blockgroup_count[i];
		}
		else { //无数据块释放
			break;
		}
	}
}

//分配i节点
int inode_alloc()
{
	int b_index, i_index = -1; //i节点所在的数据块组,i节点索引
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
	if (i_index != -1) { //分配成功后更新信息
		for (i = 0; i < BLOCK_GROUP_NUM; i++) {	//更新超级块
			blockgroup[i].superblock.free_inode_num -= 1;
		}
		blockgroup[b_index].groupdescription.group_free_inodes_num -= 1; //更新组描述符
		inode_bitmap[i_index] = USED; //更新i节点位图
	}
	return i_index;
}

//释放i节点
void inode_free(int f_inode)
{
	int i = 0;
	//更新信息
	for (i = 0; i < BLOCK_GROUP_NUM; i++) { //更新超级块
		blockgroup[i].superblock.free_inode_num += 1;
	}
	blockgroup[f_inode / BLOCK_NUM_PER_GROUP].groupdescription.group_free_inodes_num += 1; //更新组描述符
	inode_bitmap[f_inode] = NOT_USED; //更新inode位图
}

//显示当前路径
void show_cur_path(Directory cur_dir)
{
	Directory temp_dir = cur_dir;
	//递归直到找到根目录
	if (cur_dir.inode_no != 0) {
		//查找父目录
		virtualDisk.open("vfs", ios::in | ios::binary);
		if (!virtualDisk.is_open()) {
			strcpy(result, "读取文件系统失败");
			Sleep(2000);
			exit(0);
		}
		virtualDisk.seekg(inode_table[cur_dir.subinode[1]].block_address, ios::beg);
		virtualDisk.read((char*)(&cur_dir), sizeof(Directory));
		virtualDisk.close();
		show_cur_path(cur_dir);
	}
	//设置当前路径字符串
	if (temp_dir.inode_no == 0) {
		strcpy(current_path, "root");
	}
	else {
		stringstream sstream;
		sstream << current_path << "/" << inode_table[temp_dir.inode_no].fdname; //合并目录
		sstream >> current_path;
		sstream.clear();
	}
}

//从路径中提取目录名
bool get_dirname(const char *path, int len, int pos, char *filename)
{
	int i = 0;
	char *temp = new char[len];
	char dirname[MAX_FILENAME_SIZE];
	char *q; //保存切割后的子串
	//按/进行切割
	strcpy(temp, path);
	q = strtok(temp, "/");
	if (q == NULL) return false;
	strcpy(dirname, q);
	q = strtok(NULL, "/");
	for (i = 1; i < pos; i++) {
		if (q) {
			if (q == NULL) return false;
			strcpy(dirname, q);
			//返回指向被分割出片段的指针
			q = strtok(NULL, "/");
		}
		else {
			return false;
		}
	}
	strcpy(filename, dirname);
	return true;
}

//根据路径寻找对应目录
bool find_path_dir(const char *path, int len, int pos, char *dirname, Directory &tempdir)
{
	//提取每一层目录
	while (get_dirname(path, len, pos, dirname)) {
		int i = 0;
		int dirsize = tempdir.subsize;
		for (i = 2; i < dirsize; i++) { //目录项的前两个为当前目录和上一级目录
			if (strcmp(dirname, inode_table[tempdir.subinode[i]].fdname) == 0 && inode_table[tempdir.subinode[i]].filetype == FT_DIR) { //匹配子目录
				virtualDisk.seekg(inode_table[tempdir.subinode[i]].block_address, ios::beg);
				virtualDisk.read((char*)(&tempdir), sizeof(Directory));
				break;
			}
		}
		if (i < dirsize) {	//匹配到则继续在目录向下寻找
			pos++;
		}
		else { //匹配不到
			strcpy(result, "指定路径不存在");
			return false;
		}
	}
	return true;
}

//根据路径提取出最终目录及目录名文件名
bool get_dir_and_fdname(const char *path, int len, Directory &tempdir, char *filename)
{
	tempdir = current_dir; //保存路径中的目录
	const char *divide = strrchr(path, '/'); //分解路径
	if (divide) {
		int i = 0;
		int div_pos = int(divide - path); //最后一个/的位置
		int plen = div_pos + 1; //除最右文件名或目录名的路径的长度
		char *newpath = new char[plen]; //除最右文件名或目录名的路径

		//提取最右的文件名或目录名
		for (i = 1; i < len - div_pos; i++) {
			filename[i - 1] = divide[i];
		}
		filename[i - 1] = 0;
		//提取除最右文件名或目录名的路径
		if (div_pos > 0) {
			for (i = 0; i < div_pos; i++) {
				newpath[i] = path[i];
			}
			newpath[i] = 0;
		}
		else //只有一层目录
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

		//拆分路径
		char gdirname[MAX_FILENAME_SIZE];
		int ppos = 1; //路径中目录的位置(第几个目录)
		get_dirname(newpath, plen, ppos, gdirname); //从路径中提取第一个目录名
		virtualDisk.open("vfs", ios::in | ios::binary);
		if (!virtualDisk.is_open()) {
			strcpy(result, "读取文件系统失败");
			Sleep(2000);
			exit(0);
		}
		virtualDisk.seekg(inode_table[0].block_address, ios::beg);
		virtualDisk.read((char*)(&tempdir), sizeof(Directory));

		//查找对应的目录
		if (newpath[0] == '/' || strcmp(gdirname, "root") == 0) { //根目录
			if (newpath[0] != '/') ppos++;
			if (find_path_dir(newpath, plen, ppos, gdirname, tempdir)) { //找到
				virtualDisk.close();
				delete(newpath);
				return true;
			}
			else { //没找到
				virtualDisk.close();
				delete(newpath);
				return false;
			}
		}
		else if (strcmp(gdirname, "..") == 0) { //转到上级目录
			virtualDisk.seekg(inode_table[tempdir.inode_no].block_address, ios::beg);
			virtualDisk.read((char*)(&tempdir), sizeof(Directory));
			ppos++;
			if (find_path_dir(newpath, plen, ppos, gdirname, tempdir)) { //找到
				virtualDisk.close();
				delete(newpath);
				return true;
			}
			else { //没找到
				virtualDisk.close();
				delete(newpath);
				return false;
			}
		}
		else { //在当前目录查找
			tempdir = current_dir;
			if (strcmp(gdirname, ".") == 0) ppos++;
			if (find_path_dir(newpath, plen, ppos, gdirname, tempdir)) { //找到
				virtualDisk.close();
				delete(newpath);
				return true;
			}
			else { //没找到
				virtualDisk.close();
				delete(newpath);
				return false;
			}
		}
	}
	else { //默认为当前目录
		strcpy(filename, path);	//复制文件名
		return true;
	}
}

//根据路径提取出最终目录
bool get_dir(const char *path, int len, Directory &tempdir)
{
	tempdir = current_dir; //保存路径中的目录
	if (strcmp(path, "..") == 0) { //上级目录
		virtualDisk.open("vfs", ios::in | ios::binary);
		if (!virtualDisk.is_open()) {
			strcpy(result, "读取文件系统失败");
			Sleep(2000);
			exit(0);
		}
		virtualDisk.seekg(inode_table[tempdir.subinode[1]].block_address, ios::beg); //1为上级目录
		virtualDisk.read((char*)(&tempdir), sizeof(Directory));
		virtualDisk.close();
		return true;
	}
	else if (strcmp(path, ".") == 0) { //当前目录
		return true;
	}
	else {
		//拆分路径
		char gdirname[MAX_FILENAME_SIZE];
		int ppos = 1; //路径中目录的位置(第几个目录)
		get_dirname(path, len, ppos, gdirname); //从路径中提取第一个目录名
		virtualDisk.open("vfs", ios::in | ios::binary);
		if (!virtualDisk.is_open()) {
			strcpy(result, "读取文件系统失败");
			Sleep(2000);
			exit(0);
		}
		virtualDisk.seekg(inode_table[0].block_address, ios::beg);
		virtualDisk.read((char*)(&tempdir), sizeof(Directory));

		//查找对应的目录
		if (path[0] == '/' || strcmp(gdirname, "root") == 0) { //根目录
			if (path[0] != '/') ppos++;
			if (find_path_dir(path, len, ppos, gdirname, tempdir)) { //找到
				virtualDisk.close();
				return true;
			}
			else { //没找到
				virtualDisk.close();
				return false;
			}
		}
		else if (strcmp(gdirname, "..") == 0) { //转到上级目录
			virtualDisk.seekg(inode_table[tempdir.inode_no].block_address, ios::beg);
			virtualDisk.read((char*)(&tempdir), sizeof(Directory));
			ppos++;
			if (find_path_dir(path, len, ppos, gdirname, tempdir)) { //找到
				virtualDisk.close();
				return true;
			}
			else { //没找到
				virtualDisk.close();
				return false;
			}
		}
		else { //在当前目录查找
			tempdir = current_dir;
			if (strcmp(gdirname, ".") == 0) ppos++;
			if (find_path_dir(path, len, ppos, gdirname, tempdir)) { //找到
				virtualDisk.close();
				return true;
			}
			else { //没找到
				virtualDisk.close();
				return false;
			}
		}
	}
}

//判断路径是否为host文件系统路径
bool host_path(char *path)
{
	char *temp;
	//判断path中是否含<host>
	temp = strstr(path, "<host>");
	if (temp == path) {
		strcpy(path, temp + 6);
		return true;
	}
	return false;
}