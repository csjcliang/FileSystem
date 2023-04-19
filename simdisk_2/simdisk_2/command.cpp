#include "pch.h"
#include <iostream>
#include <windows.h>
#include "command.h"
#include "function.h"
#include "ipc.h"

using namespace std;

//info 显示整个系统信息
void info()
{
	stringstream tempsstream;
	tempsstream << "-------- 模拟Linux文件系统信息 --------" << endl;
	tempsstream << right << setw(15) << "块组数:" << setw(15) << BLOCK_GROUP_NUM << " 个" << endl;
	tempsstream << right << setw(15) << "每组盘块数:" << setw(15) << BLOCK_NUM_PER_GROUP << " 个" << endl;
	tempsstream << right << setw(15) << "盘块数:" << setw(15) << TOTAL_BLOCK_NUM << " 个" << endl;
	tempsstream << right << setw(15) << "盘块大小:" << setw(15) << BLOCK_SIZE << " 字节" << endl;
	tempsstream << right << setw(15) << "磁盘容量:" << setw(15) << TOTAL_BLOCK_NUM * BLOCK_SIZE << " 字节" << endl;
	tempsstream << right << setw(15) << "磁盘已用空间:" << setw(15) << (TOTAL_BLOCK_NUM - blockgroup[0].superblock.free_block_num) * BLOCK_SIZE << " 字节" << endl;
	tempsstream << right << setw(15) << "磁盘可用空间:" << setw(15) << blockgroup[0].superblock.free_block_num * BLOCK_SIZE << " 字节" << endl;
	strcpy(result, tempsstream.str().c_str()); //复制输出到返回结果中
	tempsstream.clear();
}

//cd 改变目录
void cd(const char *path)
{
	stringstream tempsstream;
	Directory tempdir;
	int pathlen = strlen(path);
	if (pathlen == 0) {
		tempsstream << "当前路径为: " << current_path << endl;
		strcpy(result, tempsstream.str().c_str()); //复制输出到返回结果中
		tempsstream.clear();
		return;
	}
	//从路径中提取出目录
	if (get_dir(path, pathlen, tempdir)) {
		//判断用户权限
		if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
			strcpy(result, "没有权限访问该目录");
			return;
		}
		//判断属性权限
		if (inode_table[tempdir.inode_no].fileaccess == WRITE_ONLY && current_usertype != ROOT_USER) {
			strcpy(result, "该目录为只写, 无法读取, 切换目录失败");
			return;
		}
		current_dir = tempdir;
		show_cur_path(current_dir);
		tempsstream << "切换目录成功，当前目录为：" << current_path;
		strcpy(result, tempsstream.str().c_str()); //复制输出到返回结果中
		tempsstream.clear();
	}
	else {
		strcpy(result, "目录不存在, 切换目录失败");
		return;
	}
}

//dir 显示目录
void dir(const char *path, int is_subdir)
{
	stringstream tempsstream;
	Directory tempdir;
	int pathlen = strlen(path);
	if (get_dir(path, pathlen, tempdir)) { //根据路径提取出要显示的目录
		//判断用户权限
		if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
			strcpy(result, "没有权限访问该目录");
			return;
		}
		//判断属性权限
		if (inode_table[tempdir.inode_no].fileaccess == WRITE_ONLY && current_usertype != ROOT_USER) {
			strcpy(result, "该目录为只写, 无法读取, 显示目录失败");
			return;
		}
		tempsstream << endl;
		if (is_subdir == 1) {
			tempsstream << "目录" << inode_table[tempdir.inode_no].fdname << "的子目录如下: " << endl;
			tempsstream << left << setw(15) << "子目录" << setw(25) << "修改时间" << setw(10) << "类型" << setw(10) << "大小(B)" << setw(15) << "所有者" << setw(10) << "属性" << setw(10) << "物理地址" << endl;
		}
		else {
			tempsstream << "目录" << inode_table[tempdir.inode_no].fdname << "的信息如下: " << endl;
			tempsstream << left << setw(15) << "目录/文件名" << setw(25) << "修改时间" << setw(10) << "类型" << setw(10) << "大小(B)" << setw(15) << "所有者" << setw(10) << "属性" << setw(10) << "物理地址" << endl;
		}

		for (int i = 0; i < tempdir.subsize; i++) {
			if (is_subdir == 1) {
				if (inode_table[tempdir.subinode[i]].filetype == FT_FILE || i == 0 || i == 1) { //若属性参数为/s则只显示子目录，不显示文件及当前目录和上级目录
					continue;
				}
			}
			//目录/文件名
			if (i == 0) {
				tempsstream << setw(15) << ".";
			}
			else if (i == 1) {
				tempsstream << setw(15) << "..";
			}
			else {
				tempsstream << setw(15) << inode_table[tempdir.subinode[i]].fdname;
			}

			//修改时间
			tm temptime = inode_table[tempdir.subinode[i]].mtime.getDatetime();
			tempsstream << setfill('0') << setw(4) << temptime.tm_year + 1900 << "/" << right << setw(2) << temptime.tm_mon + 1 << "/" << setw(2) << temptime.tm_mday << " " << setw(2) << temptime.tm_hour << ":" << setw(2) << temptime.tm_min << ":" << setw(2) << temptime.tm_sec << setfill(' ') << setw(25 - 19) << "";
			tempsstream << left;
			//类型
			if (inode_table[tempdir.subinode[i]].filetype == FT_DIR) {
				tempsstream << setw(10) << "目录";
			}
			else {
				tempsstream << setw(10) << "文件";
			}

			//大小
			if (inode_table[tempdir.subinode[i]].filetype == FT_DIR) {
				tempsstream << setw(10) << "-";
			}
			else {
				tempsstream << setw(10) << inode_table[tempdir.subinode[i]].filesize;
			}

			//所有者
			int j = 0;
			for (j = 0; j < MAX_USER_NUM; j++) {
				if (inode_table[tempdir.subinode[i]].uid == user_group[j].getUserid()) break;
			}
			tempsstream << setw(15) << user_group[j].getUsername();

			//属性
			switch (inode_table[tempdir.subinode[i]].fileaccess) {
			case READ_ONLY:
				tempsstream << setw(10) << "只读";
				break;
			case WRITE_ONLY:
				tempsstream << setw(10) << "只写";
				break;
			case READ_WRITE:
				tempsstream << setw(10) << "读写";
				break;
			}

			//物理地址
			tempsstream << setw(10) << inode_table[tempdir.subinode[i]].block_address;
			tempsstream << endl;
		}
		strcpy(result, tempsstream.str().c_str()); //复制输出到返回结果中
		tempsstream.clear();
	}
	else {
		strcpy(result, "显示目录失败");
	}
}

//md 创建目录
void md(const char *path, fileAccess fileaccess)
{
	Directory tempdir;
	int pathlen = strlen(path);
	char md_dirname[MAX_FILENAME_SIZE];
	if (get_dir_and_fdname(path, pathlen, tempdir, md_dirname)) { //根据路径提取出要创建的目录的上级目录及创建的目录名
		//判断用户权限
		if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
			strcpy(result, "没有权限访问该目录");
			return;
		}
		//判断属性权限
		if (inode_table[tempdir.inode_no].fileaccess == READ_ONLY && current_usertype != ROOT_USER) { //只读则不可在目录下面创建
			strcpy(result, "该目录为只读, 无法在此目录下创建新目录");
			return;
		}

		//检查是否存在同名目录
		if (tempdir.samename(md_dirname)) {
			strcpy(result, "该目录下已存在同名的目录, 创建目录失败");
			return;
		}

		long md_addr = -1; //分配目录在数据区的首地址
		int md_inode = -1; //分配目录的i节点
		int b_index; //分配块在块位图的起始位置
		//为目录分配空间
		md_addr = block_alloc(DIR_SIZE, b_index);
		if (md_addr < 0) { //空间不足
			strcpy(result, "磁盘空间不足，创建目录失败");
			return;
		}
		else {
			//分配i节点
			md_inode = inode_alloc();
			if (md_inode < 0) {
				strcpy(result, "i节点分配失败，创建目录失败");
				return;
			}

			//创建目录
			Directory newdir;
			newdir.inode_no = md_inode; //i节点号
			newdir.subsize = 2; //一个为当前目录，一个为父目录
			strcpy(inode_table[newdir.inode_no].fdname, md_dirname); //目录名
			newdir.subinode[0] = md_inode; //当前目录
			newdir.subinode[1] = tempdir.inode_no; //父目录
			//修改i节点信息
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

			//修改父目录信息
			tempdir.subinode[tempdir.subsize] = md_inode;
			tempdir.subsize++;
			if (tempdir.inode_no == current_dir.inode_no) {
				current_dir = tempdir;
			}

			//写入磁盘
			int i = 0;
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
			virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + md_inode * sizeof(Bitmap), ios::beg);
			virtualDisk.write((char*)(&inode_bitmap[md_inode]), sizeof(Bitmap));
			//写i节点表
			virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + TOTAL_INODE_NUM * sizeof(Bitmap) + md_inode * sizeof(Inode), ios::beg);
			virtualDisk.write((char*)(&inode_table[md_inode]), sizeof(Inode));
			//写块位图
			virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + TOTAL_INODE_NUM * sizeof(Bitmap) + TOTAL_INODE_NUM * sizeof(Inode) + b_index * sizeof(Bitmap), ios::beg);
			for (i = 0; i < DIR_SIZE; i++) {
				virtualDisk.write((char*)(&block_bitmap[b_index]), sizeof(Bitmap));
			}
			//写回当前目录和父目录
			virtualDisk.seekp(md_addr, ios::beg);
			virtualDisk.write((char*)(&newdir), sizeof(Directory));
			virtualDisk.seekp(inode_table[tempdir.inode_no].block_address, ios::beg);
			virtualDisk.write((char*)(&tempdir), sizeof(Directory));
			virtualDisk.close();
			strcpy(result, "创建目录成功");
		}
	}
	else {
		strcpy(result, "创建目录失败");
	}
}

//rd 删除目录
void rd(const char *path)
{
	Directory tempdir;
	int pathlen = strlen(path);
	char rd_dirname[MAX_FILENAME_SIZE];
	if (get_dir_and_fdname(path, pathlen, tempdir, rd_dirname)) { //根据路径提取出要删除的目录的上级目录及删除的目录名
		//判断用户权限
		if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
			strcpy(result, "没有权限访问该目录");
			return;
		}

		int i = 0;
		int rd_inode = -1; //删除目录的i节点
		int rd_pos = 0; //删除目录的位置

		//查找需要删除的目录
		for (i = 2; i < tempdir.subsize; i++) {
			if (strcmp(inode_table[tempdir.subinode[i]].fdname, rd_dirname) == 0 && inode_table[tempdir.subinode[i]].filetype == FT_DIR) {
				rd_inode = tempdir.subinode[i];
				rd_pos = i;
				break;
			}
		}
		if (i == tempdir.subsize) { //查找不到目录
			strcpy(result, "目录不存在, 删除目录失败");
			return;
		}
		else { //找到目录
			Directory rd_dir; //要删除的目录
			virtualDisk.open("vfs", ios::in | ios::binary);
			if (!virtualDisk.is_open()) {
				strcpy(result, "读入文件系统失败");
				Sleep(2000);
				exit(0);
			}
			virtualDisk.seekg(inode_table[rd_inode].block_address, ios::beg);
			virtualDisk.read((char*)(&rd_dir), sizeof(Directory));
			virtualDisk.close();
			if (rd_dir.inode_no == current_dir.inode_no) { //不能删除当前目录
				strcpy(result, "不能删除当前目录");
				return;
			}
			if (strstr(current_path, path)) {
				strcpy(result, "不能删除当前目录所在路径上的目录");
				return;
			}

			//判断用户权限
			if ((current_usertype == USER && inode_table[rd_dir.inode_no].uid != current_user) || inode_table[rd_dir.inode_no].uid == 0) {
				strcpy(result, "没有权限访问该目录");
				return;
			}

			if (rd_dir.subsize > 2) { //目录存在子项
				tempdir.remove_dir(rd_dir, rd_pos);
				strcpy(result, "删除目录成功");
				return;
			}
			else { //目录为空则直接删除
				//释放块和i节点
				block_free(inode_table[rd_inode].blocknum, ((inode_table[rd_inode].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE));
				inode_free(rd_inode);
				//更新父目录信息
				for (i = rd_pos; i < tempdir.subsize; i++) {
					tempdir.subinode[i] = tempdir.subinode[i + 1]; //后面的文件补空 
				}
				tempdir.subsize--;
				if (tempdir.inode_no == current_dir.inode_no) {
					current_dir = tempdir;
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
				virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + rd_inode * sizeof(Bitmap), ios::beg);
				virtualDisk.write((char*)(&inode_bitmap[rd_inode]), sizeof(Bitmap));
				//写块位图
				virtualDisk.seekp(BLOCK_GROUP_NUM * sizeof(Blockgroup) + TOTAL_INODE_NUM * (sizeof(Bitmap) + sizeof(Inode)) + ((inode_table[rd_inode].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE) * sizeof(Bitmap), ios::beg);
				for (i = 0; i < inode_table[rd_inode].blocknum; i++) {
					virtualDisk.write((char*)(&block_bitmap[(inode_table[rd_inode].block_address - DATA_BLOCK_BEGIN) / BLOCK_SIZE]), sizeof(Bitmap));
				}
				//写父目录
				virtualDisk.seekp(inode_table[tempdir.inode_no].block_address, ios::beg);
				virtualDisk.write((char*)(&tempdir), sizeof(Directory));
				virtualDisk.close();
				strcpy(result, "删除目录成功");
			}
		}
	}
	else {
		strcpy(result, "删除目录失败");
	}
}

//newfile 建立文件
void newfile(const char*path, fileAccess fileaccess)
{
	Directory tempdir;
	int pathlen = strlen(path);
	char nf_filename[MAX_FILENAME_SIZE];
	if (get_dir_and_fdname(path, pathlen, tempdir, nf_filename)) { //根据路径提取出要创建文件的所在目录及创建的文件名
		//判断用户权限
		if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
			cout << "没有权限访问该目录" << endl;
			return;
		}
		//判断属性权限
		if (inode_table[tempdir.inode_no].fileaccess == READ_ONLY && current_usertype != ROOT_USER) {
			cout << "该目录为只读, 无法在此目录下创建文件" << endl;
			return;
		}

		//检查是否存在同名文件
		if (tempdir.samename(nf_filename)) {
			cout << "目录下已存在同名的文件, 创建文件失败" << endl;
			return;
		}

		int i = 0;
		int nf_len = 0; //文件字符数
		int nf_size = 16;  //文件大小
		char *content = new char[nf_size]; //文件内容
		for (i = 0; i < nf_size; i++) {
			content[i] = 0;
		}

		cout << "请输入文件内容, 以#结束输入" << endl;
		char *temp;
		char s = '\0';
		//开始输入文件内容
		while ((s = cin.get()) != '#') {
			content[nf_len++] = s;
			if (nf_len >= nf_size - 1) { //输入内容超过预定文件大小
				temp = new char[nf_size];
				strcpy(temp, content); //复制到临时数组中
				delete(content);
				nf_size *= 2; //文件大小增加至两倍
				content = new char[nf_size];
				for (i = 0; i < nf_size; i++) {
					content[i] = 0;
				}
				strcpy(content, temp);
				delete(temp);
			}
		}
		cin.ignore();
		tempdir.create_file(nf_filename, content, nf_len, fileaccess); //保存新建的文件
		delete(content);
	}
	else {
		cout << "创建文件失败" << endl;
	}
}

//cat 打开文件
void cat(const char *path)
{
	stringstream tempsstream;
	Directory tempdir;
	int pathlen = strlen(path);
	char cat_filename[MAX_FILENAME_SIZE];
	if (get_dir_and_fdname(path, pathlen, tempdir, cat_filename)) { //根据路径提取出要打开文件的所在目录及打开的文件名
		//判断用户权限
		if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
			strcpy(result, "没有权限访问该文件");
			return;
		}
		//判断属性权限
		if (inode_table[tempdir.inode_no].fileaccess == WRITE_ONLY && current_usertype != ROOT_USER) {
			strcpy(result, "该目录为只写, 无法在此目录下打开文件");
			return;
		}

		int i = 0;
		int cat_inode = -1; //文件的i节点
		//查找要打开的文件
		for (i = 2; i < tempdir.subsize; i++) {
			if (strcmp(inode_table[tempdir.subinode[i]].fdname, cat_filename) == 0 && inode_table[tempdir.subinode[i]].filetype == FT_FILE) {
				cat_inode = tempdir.subinode[i];
				break;
			}
		}
		if (i == tempdir.subsize) { //找不到文件
			strcpy(result, "文件不存在, 打开文件失败");
		}
		else {
			//判断用户权限
			if (current_usertype == USER && inode_table[cat_inode].uid != current_user && inode_table[cat_inode].uid != 0) {
				strcpy(result, "没有权限访问该文件");
				return;
			}
			//判断属性权限
			if (inode_table[cat_inode].fileaccess == WRITE_ONLY) {
				strcpy(result, "该文件为只写, 无法打开文件");
				return;
			}

			//打开文件并读入内容到content中
			char *content = new char[inode_table[cat_inode].filesize];
			virtualDisk.open("vfs", ios::in | ios::binary);
			if (!virtualDisk.is_open()) {
				strcpy(result, "读入文件系统失败");
				Sleep(2000);
				exit(0);
			}
			virtualDisk.seekg(inode_table[cat_inode].block_address, ios::beg);
			virtualDisk.read((char*)(content), inode_table[cat_inode].filesize);
			virtualDisk.close();
			content[inode_table[cat_inode].filesize - 1] = 0;
			//输出文件内容
			tempsstream << "文件" << cat_filename << "的内容如下: " << endl;
			tempsstream << content << endl;
			strcpy(result, tempsstream.str().c_str()); //复制输出到返回结果中
			tempsstream.clear();
			delete(content);
		}
	}
	else {
		strcpy(result, "打开文件失败");
	}
}

//copy 拷贝文件
void copy(const char *srcpath, const char *despath)
{
	Directory tempdir;
	char cp_filename[MAX_FILENAME_SIZE];
	char *content; //文件内容
	char divide;
	int len = 0, i = 0;
	int src_len = strlen(srcpath), des_len = strlen(despath);
	char *f_srcpath = new char[src_len];
	char *f_despath = new char[des_len];
	strcpy(f_srcpath, srcpath);
	strcpy(f_despath, despath);
	//开始复制文件
	if (host_path(f_srcpath)) { //从host文件系统复制文件到模拟文件系统中
		if (host_path(f_despath)) { //两个路径均为host路径
			strcpy(result, "不支持从host文件系统拷贝文件到host文件系统");
			return;
		}

		//读取host文件
		fstream fsHost;
		fsHost.open(f_srcpath, ios::in | ios::binary);
		if (!fsHost.is_open()) {
			strcpy(result, "host文件系统中不存在该文件");
			return;
		}
		fsHost.seekg(0, ios::end);
		len = fsHost.tellg(); //文件长度
		//分配存储空间
		content = new char[len];
		content[len - 1] = 0;
		fsHost.seekg(0, ios::beg);
		fsHost.read((char*)(content), len - 1);
		fsHost.close();
		//提取文件名
		divide = '\\';
		strcpy(cp_filename, strrchr(f_srcpath, divide) + 1);
		if (get_dir(f_despath, des_len, tempdir)) { //从目的路径中提取出目录对象
			//判断用户权限
			if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
				strcpy(result, "没有权限访问目的路径的目录");
				delete(content);
				return;
			}
			//判断属性权限
			if (inode_table[tempdir.inode_no].fileaccess == READ_ONLY && current_usertype != ROOT_USER) {
				strcpy(result, "目的路径的目录为只读, 无法在此目录下创建文件");
				delete(content);
				return;
			}

			//检查是否存在同名文件
			if (tempdir.samename(cp_filename)) {
				strcpy(result, "该目录下已存在同名的文件, 拷贝文件失败");
				delete(content);
				return;
			}
			//保存文件到模拟文件系统
			tempdir.create_file(cp_filename, content, len, READ_WRITE);
			delete(content);
			strcpy(result, "文件拷贝成功");
		}
		else {
			strcpy(result, "文件拷贝失败");
		}
	}
	else { //从模拟文件系统复制文件
		if (host_path(f_despath)) { //模拟文件系统复制文件到host文件系统中
			if (get_dir_and_fdname(f_srcpath, src_len, tempdir, cp_filename)) { //从源路径中提取出目录对象及文件名
				int cp_inode;
				for (i = 2; i < tempdir.subsize; i++) {
					if (strcmp(inode_table[tempdir.subinode[i]].fdname, cp_filename) == 0 && inode_table[tempdir.subinode[i]].filetype == FT_FILE) { //找到对应文件
						cp_inode = tempdir.subinode[i];
						break;
					}
				}
				if (i == tempdir.subsize) { //找不到文件
					strcpy(result, "该文件不存在");
				}
				else { //找到文件
					//判断用户权限
					if (current_usertype == USER && inode_table[cp_inode].uid != current_user) {
						strcpy(result, "没有权限访问源路径的文件");
						return;
					}
					//判断属性权限
					if (inode_table[cp_inode].fileaccess == WRITE_ONLY && current_usertype != ROOT_USER) {
						strcpy(result, "源路径的文件为只写, 无法在此目录下读取文件");
						return;
					}

					//打开文件并读入内容到content中
					content = new char[inode_table[cp_inode].filesize];
					virtualDisk.open("vfs", ios::in | ios::binary);
					if (!virtualDisk.is_open()) {
						strcpy(result, "读入文件系统失败");
						Sleep(2000);
						exit(0);
					}
					virtualDisk.seekg(inode_table[cp_inode].block_address, ios::beg);
					virtualDisk.read((char*)(content), inode_table[cp_inode].filesize);
					virtualDisk.close();
					content[inode_table[cp_inode].filesize - 1] = 0;
					len = inode_table[cp_inode].filesize;

					//合并路径
					char *complete_path = new char[src_len + des_len + 2];
					stringstream sstream;
					sstream << f_despath;
					if (f_despath[des_len - 1] != '\\') sstream << "\\";
					sstream << cp_filename;
					sstream >> complete_path;
					sstream.clear();

					//保存文件到host文件系统
					fstream fsHost;
					fsHost.open(complete_path, ios::out | ios::binary);
					if (!fsHost.is_open()) {
						strcpy(result, "写入文件到host文件系统失败");
						delete(content);
						delete(complete_path);
						return;
					}
					fsHost.write((char*)(content), len);
					fsHost.close();
					delete(content);
					delete(complete_path);
					strcpy(result, "文件拷贝成功");
				}
			}
			else {
				strcpy(result, "文件拷贝失败");
			}
		}
		else { //模拟文件系统内部复制
			if (get_dir_and_fdname(f_srcpath, src_len, tempdir, cp_filename)) { //从源路径中提取出目录对象及文件名
				int cp_inode;
				for (i = 2; i < tempdir.subsize; i++) {
					if (strcmp(inode_table[tempdir.subinode[i]].fdname, cp_filename) == 0 && inode_table[tempdir.subinode[i]].filetype == FT_FILE) { //找到对应文件
						cp_inode = tempdir.subinode[i];
						break;
					}
				}
				if (i == tempdir.subsize) { //找不到文件
					strcpy(result, "该文件不存在");
				}
				else //找到文件
				{
					//判断用户权限
					if (current_usertype == USER && inode_table[cp_inode].uid != current_user) {
						strcpy(result, "没有权限访问源路径的文件");
						return;
					}
					//判断属性权限
					if (inode_table[cp_inode].fileaccess == WRITE_ONLY && current_usertype != ROOT_USER) {
						strcpy(result, "源路径的文件为只写, 无法在此目录下读取文件");
						return;
					}

					//打开文件并读入内容到content中
					fileAccess fileaccess = inode_table[cp_inode].fileaccess;
					content = new char[inode_table[cp_inode].filesize];
					virtualDisk.open("vfs", ios::in | ios::binary);
					if (!virtualDisk.is_open()) {
						strcpy(result, "读入文件系统失败");
						Sleep(2000);
						exit(0);
					}
					virtualDisk.seekg(inode_table[cp_inode].block_address, ios::beg);
					virtualDisk.read((char*)(content), inode_table[cp_inode].filesize);
					virtualDisk.close();
					content[inode_table[cp_inode].filesize - 1] = 0;
					len = inode_table[cp_inode].filesize;

					//合并路径
					char *complete_path = new char[src_len + des_len + 2];
					stringstream sstream;
					sstream << f_despath;
					if (f_despath[des_len - 1] != '/') sstream << "/";
					sstream << cp_filename;
					sstream >> complete_path;
					sstream.clear();

					if (get_dir_and_fdname(complete_path, src_len + des_len + 2, tempdir, cp_filename)) { //从路径中提取出目录对象及文件名
						//判断用户权限
						if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
							strcpy(result, "没有权限访问目的路径的目录");
							delete(content);
							delete(complete_path);
							return;
						}
						//判断属性权限
						if (inode_table[tempdir.inode_no].fileaccess == READ_ONLY && current_usertype != ROOT_USER) {
							strcpy(result, "目的路径的目录为只读, 无法在此目录下创建文件");
							delete(content);
							delete(complete_path);
							return;
						}

						//检查是否存在同名文件
						if (tempdir.samename(cp_filename)) {
							strcpy(result, "该目录下已存在同名的文件, 拷贝文件失败");
							delete(content);
							delete(complete_path);
							return;
						}
						//保存文件
						tempdir.create_file(cp_filename, content, len, fileaccess);
						strcpy(result, "文件拷贝成功");
					}
					else {
						strcpy(result, "文件拷贝失败");
					}
					delete(content);
					delete(complete_path);
				}
			}
			else {
				strcpy(result, "文件拷贝失败");
			}
		}
	}
}

//del 删除文件
void del(const char *path)
{
	Directory tempdir;
	int pathlen = strlen(path);
	char del_filename[MAX_FILENAME_SIZE];
	if (get_dir_and_fdname(path, pathlen, tempdir, del_filename)) { //根据路径提取出要删除文件的所在目录及删除的文件名
		//判断用户权限
		if (current_usertype == USER && inode_table[tempdir.inode_no].uid != current_user && inode_table[tempdir.inode_no].uid != 0) {
			strcpy(result, "没有权限访问该目录");
			return;
		}
		tempdir.delete_file(del_filename);
	}
	else {
		strcpy(result, "删除文件失败");
	}
}

//check 检测并恢复文件系统
void check()
{
	stringstream tempsstream;
	int i = 0, j = 0;
	int free_block_num = 0, free_inode_num = 0, total_free_block = 0, total_free_inode = 0; //空闲块数、i节点数、空闲块总数和i节点总数
	int start; //记录位图起始位置
	bool error = false; //系统是否有异常
	tempsstream << endl;
	tempsstream << "正在检查文件系统……" << endl;
	for (i = 0; i < BLOCK_GROUP_NUM; i++) {
		free_block_num = 0, free_inode_num = 0;
		start = i * BLOCK_NUM_PER_GROUP;
		for (j = 0; j < BLOCK_NUM_PER_GROUP; j++) {
			if (block_bitmap[start + j] == NOT_USED) free_block_num++; //统计空闲块数
			if (inode_bitmap[start + j] == NOT_USED) free_inode_num++; //统计空闲i节点数
		}
		if (blockgroup[i].groupdescription.group_free_blocks_num != free_block_num) { //与磁盘记录不符
			error = true;
			blockgroup[i].groupdescription.group_free_blocks_num = free_block_num;
		}
		if (blockgroup[i].groupdescription.group_free_inodes_num != free_inode_num) { //与磁盘记录不符
			error = true;
			blockgroup[i].groupdescription.group_free_inodes_num = free_inode_num;
		}
		total_free_block += blockgroup[i].groupdescription.group_free_blocks_num;
		total_free_inode += blockgroup[i].groupdescription.group_free_inodes_num;
	}
	if (blockgroup[0].superblock.free_block_num != total_free_block) { //总数与磁盘记录不符
		error = true;
		for (i = 0; i < BLOCK_GROUP_NUM; i++) {
			blockgroup[i].superblock.free_block_num = total_free_block;
		}
	}
	if (blockgroup[0].superblock.free_inode_num != total_free_inode) { //总数与磁盘记录不符
		error = true;
		for (i = 0; i < BLOCK_GROUP_NUM; i++) {
			blockgroup[i].superblock.free_inode_num = total_free_inode;
		}
	}
	//判断是否发生异常
	if (!error) {
		tempsstream << "检查完成, 文件系统无异常" << endl;
	}
	else {
		//保存修改
		tempsstream << "检查发现异常, 正在修复文件系统……" << endl;
		virtualDisk.open("vfs", ios::out | ios::binary | ios::_Nocreate);
		if (!virtualDisk.is_open()) {
			strcpy(result, "写入文件系统失败");
			Sleep(2000);
			exit(0);
		}
		for (i = 0; i < BLOCK_GROUP_NUM; i++) { //写数据块
			virtualDisk.write((char*)(&blockgroup[i]), sizeof(Blockgroup));
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
		//写当前目录
		virtualDisk.seekp(inode_table[current_dir.inode_no].block_address, ios::beg);
		virtualDisk.write((char*)(&current_dir), sizeof(Directory));
		virtualDisk.close();

		tempsstream << "文件系统修复完成!" << endl;
	}
	strcpy(result, tempsstream.str().c_str()); //复制输出到返回结果中
	tempsstream.clear();
}

//help 获取帮助
void help(const char *command_in)
{
	stringstream tempsstream;
	if (command_in[0] == '\0') { //不含参数
		tempsstream << endl;
		tempsstream << "-------- 模拟Linux文件系统命令帮助 --------" << endl;
		tempsstream << "0.info" << "\t\t" << "显示整个系统信息" << endl;
		tempsstream << "1.cd" << "\t\t" << "切换目录" << endl;
		tempsstream << "2.dir" << "\t\t" << "显示目录" << endl;
		tempsstream << "3.md" << "\t\t" << "创建目录" << endl;
		tempsstream << "4.rd" << "\t\t" << "删除目录" << endl;
		tempsstream << "6.cat" << "\t\t" << "打开文件" << endl;
		tempsstream << "7.copy" << "\t\t" << "拷贝文件" << endl;
		tempsstream << "8.del" << "\t\t" << "删除文件" << endl;
		tempsstream << "9.check" << "\t\t" << "检测并恢复文件系统" << endl;
		tempsstream << "10.help" << "\t\t" << "获取帮助" << endl;
		tempsstream << "11.exit" << "\t\t" << "退出系统" << endl;
		tempsstream << "要获得每个命令的具体使用方法, 请用help+命令名称查询" << endl;
	}
	else { //含参数
		int i = 0;
		for (i = 0; i < COMMAND_NUM; i++) {
			if (strcmp(command_in, command[i]) == 0) break;
		}
		switch (i)
		{
		case 0: //info 显示整个系统信息
			tempsstream << "info (输入info):\n显示整个系统信息，包括盘块数、盘容量、可用空间等信息。" << endl;
			break;
		case 1: //cd 改变目录
			tempsstream << "cd (输入cd+路径):\n改变当前工作目录（路径参数中.为当前目录，..为上一级目录，默认参数为当前目录）。" << endl;
			break;
		case 2: //dir 显示目录
			tempsstream << "dir (输入dir+路径+属性):\n显示指定目录下或当前目录下的文件及子目录信息（路径参数中.为当前目录，..为上一级目录，默认参数为当前目录，属性参数可不填或填/s显示所有子目录，填/s需保证路径不为空）。" << endl;
			break;
		case 3: //md 创建目录
			tempsstream << "md (输入md+路径+属性):\n在指定路径或当前路径下创建指定目录（属性参数中0为只读、1为只写、2为可读写，默认为可读写）。" << endl;
			break;
		case 4: //rd 删除目录
			tempsstream << "rd (输入rd+路径):\n删除指定目录下所有文件和子目录。" << endl;
			break;
		case 6: //cat 打开文件
			tempsstream << "cat (输入cat+路径(含打开文件名)):\n打开指定路径下的文件（路径参数中只输入文件名则在当前目录打开文件）。" << endl;
			break;
		case 7: //copy 拷贝文件
			tempsstream << "copy (输入copy+源路径(含源文件名)+目的路径):\n拷贝文件，支持模拟文件系统内部的文件拷贝，也支持host文件系统与模拟文件系统间的文件拷贝，host文件系统的路径参数应在最前加上<host>。" << endl;
			break;
		case 8: //del 删除文件
			tempsstream << "del (输入del+路径(含删除文件名)):\n删除指定目录下的文件（路径参数中只输入文件名则在当前目录删除文件）。" << endl;
			break;
		case 9: //check 检测并恢复文件系统
			tempsstream << "check (输入check):\n检测并恢复文件系统。" << endl;
			break;
		case 10: //help 获取帮助
			tempsstream << "help (输入help+命令名):\n获取命令帮助（命令名空缺则显示所有命令帮助）。" << endl;
			break;
		case 11: //exit 退出系统
			tempsstream << "exit (输入exit):\n退出系统。" << endl;
			break;
		default:
			tempsstream << "不存在命令或命令格式不正确，请重新输入" << endl;
			break;
		}
	}
	strcpy(result, tempsstream.str().c_str()); //复制输出到返回结果中
	tempsstream.clear();
}

//exit 退出系统
void exit()
{
	strcpy(result, "后台文件系统已退出，shell进程即将退出……");
	//允许simdisk放数据
	ReleaseSemaphore(m_Write[1], 1, NULL);
	Sleep(2000);
	exit(0);
}