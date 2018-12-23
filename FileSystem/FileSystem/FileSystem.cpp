// FileSystem.cpp: �������̨Ӧ�ó������ڵ㡣
//
#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"

#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

const char USERNAME[] = "local";

#define FREE 0
#define DIRLEN 80
#define END 65535
#define SIZE 1024000
#define BLOCKNUM 1000
#define BLOCKSIZE 1024
#define MAXOPENFILE 10
#define ROOTBLOCKNUM 2

#define SAYERROR printf("ERROR: ")
#define max(X, Y) (((X) > (Y)) ? (X) : (Y))
#define min(X, Y) (((X) < (Y)) ? (X) : (Y))

typedef struct FAT {
	unsigned short id;
} fat;

typedef struct FCB {
	char free; // ��fcb�Ƿ��ѱ�ɾ������Ϊ��һ��fcb�Ӵ��̿���ɾ���Ǻܷ��µģ�����ѡ������fcb��free�����������Ƿ�ɾ��
	char exname[3];
	char filename[DIRLEN];
	unsigned short time;
	unsigned short date;
	unsigned short first; // �ļ���ʼ�̿��
	unsigned long length; // �ļ���ʵ�ʳ���
	unsigned char attribute; // �ļ������ֶΣ�Ϊ�����������ֻΪ�ļ��������������ԣ�ֵΪ 0 ʱ��ʾĿ¼�ļ���ֵΪ 1 ʱ��ʾ�����ļ�
} fcb;

// �����ļ���fcb����count��Զ������fcb��length��
// ֻ���ļ�fcb��count����ݴ򿪷�ʽ�Ĳ�ͬ�Ͷ�д��ʽ�Ĳ�ͬ����ͬ
typedef struct USEROPEN {
	fcb open_fcb; // �ļ��� FCB �е�����
	int count; // ��дָ�����ļ��е�λ��
	int dirno; // ��Ӧ���ļ���Ŀ¼���ڸ�Ŀ¼�ļ��е��̿��
	int diroff; // ��Ӧ���ļ���Ŀ¼���ڸ�Ŀ¼�ļ���dirno�̿��е���ʼλ��
	char fcbstate; // �Ƿ��޸����ļ��� FCB �����ݣ�����޸�����Ϊ 1������Ϊ 0
	char topenfile; // ��ʾ���û��򿪱����Ƿ�Ϊ�գ���ֵΪ 0����ʾΪ�գ������ʾ�ѱ�ĳ���ļ�ռ��
	char dir[DIRLEN]; // ���ļ��ľ���·����������������ټ���ָ���ļ��Ƿ��Ѿ���
} useropen;

typedef struct BLOCK0 {
	unsigned short root;
	char information[200];
	unsigned char *startblock;
} block0;

// utils
// ������һЩ�����ĺ���������һЩ���ظ����õĲ���

// ���������Ĳ�����fcb���г�ʼ��
void fcb_init(fcb *new_fcb, const char* filename, unsigned short first, unsigned char attribute);
// ���������Ĳ�����һ�����ļ�����г�ʼ��
void useropen_init(useropen *openfile, int dirno, int diroff, const char* dir);
// ͨ��dfs�Խ���ʹ�õ�fat���ͷ�
void fatFree(int id);
// �õ�һ�����е�fat��
int getFreeFatid();
// �õ�һ�����еĴ��ļ�����
int getFreeOpenlist();
// �õ�һ��fat�����һ��fat�����û���򴴽�
int getNextFat(int id);
// ���һ�����ļ����±��Ƿ�Ϸ�
int check_fd(int fd);
// ��һ��·����'/'�ָ�
int spiltDir(char dirs[DIRLEN][DIRLEN], char *filename);
// ��һ��·�������һ��Ŀ¼���ַ�����ɾȥ
void popLastDir(char *dir);
// ��һ��·�������һ��Ŀ¼���ַ����зָ��
void splitLastDir(char *dir, char new_dir[2][DIRLEN]);
// �õ�ĳ��������ĳ��fat�еĶ�Ӧ���̿�ź�ƫ������������¼һ�����ļ������丸Ŀ¼��Ӧfat��λ��
void getPos(int *id, int *offset, unsigned short first, int length);
// ��·���淶�������
int rewrite_dir(char *dir);

// basics
// �����̿�ź�ƫ������ֱ�Ӵ�FAT�϶�ȡָ�����ȵ���Ϣ
int fat_read(unsigned short id, unsigned char *text, int offset, int len);
// ��ȡĳ���Ѵ��ļ���ָ��������Ϣ
int do_read(int fd, unsigned char *text, int len);
// �����̿�ź�ƫ������ֱ�Ӵ�FAT��д��ָ�����ȵ���Ϣ
int fat_write(unsigned short id, unsigned char *text, int blockoffset, int len);
// ��ĳ���Ѵ��ļ�д��ָ��������Ϣ
int do_write(int fd, unsigned char *text, int len);
// ��һ���Ѵ�Ŀ¼�ļ��ҵ���Ӧ���Ƶ��ļ���fcb������һЩ���ϵݹ���ļ��еĺ�����
int getFcb(fcb* fcbp, int *dirno, int *diroff, int fd, const char *dir);
// ��һ���Ѵ�Ŀ¼�ļ��´�ĳ���ļ�
int getOpenlist(int fd, const char *org_dir);
// ���ļ�
int my_open(char *filename);

// read
// ��ȡһ���ļ����µ�fcb��Ϣ
int read_ls(int fd, unsigned char *text, int len);
// ��һ���ļ����µ�fcb��Ϣ��ӡ����
void my_ls();
// ��һ�����ļ������ݸ����ļ�ָ���ӡ����
int my_read(int fd);
// ���´Ӵ����ж�ȡһ�����ļ���fcb����
void my_reload(int fd);

//write
// �Ѽ����������Ϣд��һ�����ļ�
int my_write(int fd);
// ��һ��ָ��Ŀ¼��fcb��free��Ϊ1
void my_rmdir(char *dirname);
// ��һ��ָ���ļ���fcb��free��Ϊ1
void my_rm(char *filename);

//create
// ��ʽ��
void my_format();
// ��ָ��Ŀ¼�´���һ���ļ����ļ���
int my_touch(char *filename, int attribute, int *rpafd);
// ����touch������һ���ļ�
int my_create(char *filename);
// ����touch������һ���ļ���
void my_mkdir(char *dirname);

//others
// ����ϵͳ��������صĳ�ʼ��
void startsys();
// �˳�ϵͳ��������Ӧ���ݹ���
void my_exitsys();
// ��һ�����ļ���fat��Ϣ��������
void my_save(int fd);
// �ر�һ�����ļ�
void my_close(int fd);
// ����my_open�ѵ�ǰĿ¼�л���ָ��Ŀ¼��
void my_cd(char *dirname);
// ��ʾ�����˵�
void my_help();

unsigned char *myvhard;
useropen openfilelist[MAXOPENFILE];
int curdirid; // ָ���û����ļ����еĵ�ǰĿ¼���ڴ��ļ������λ��

unsigned char *blockaddr[BLOCKNUM];
block0 initblock;
fat fat1[BLOCKNUM], fat2[BLOCKNUM];

char str[SIZE];

int main() {


	
	int fd;
	char command[DIRLEN << 1];
	printf("File System ver 1.0 by ssd.\n");
	printf("***************************************************************\n");
	printf("������\t\t�������\t\t����˵��\n\n");
	printf("cd\t\tĿ¼��(·����)\t\t�л���ǰĿ¼��ָ��Ŀ¼\n");
	printf("mkdir\t\tĿ¼��\t\t\t�ڵ�ǰĿ¼������Ŀ¼\n");
	printf("rmdir\t\tĿ¼��\t\t\t�ڵ�ǰĿ¼ɾ��ָ��Ŀ¼\n");
	printf("ls\t\t��\t\t\t��ʾ��ǰĿ¼�µ�Ŀ¼���ļ�\n");
	printf("create\t\t�ļ���\t\t\t�ڵ�ǰĿ¼�´���ָ���ļ�\n");
	printf("rm\t\t�ļ���\t\t\t�ڵ�ǰĿ¼��ɾ��ָ���ļ�\n");
	printf("open\t\t�ļ���\t\t\t�ڵ�ǰĿ¼�´�ָ���ļ�\n");
	printf("write\t\tfd\t\t\t�ڴ��ļ�״̬�£�д���ļ�\n");
	printf("read\t\tfd\t\t\t�ڴ��ļ�״̬�£���ȡ���ļ�\n");
	printf("close\t\tfd\t\t\t�ڴ��ļ�״̬�£��رո��ļ�\n");
	printf("help\t\t��\t\t\t�鿴����\n");
	printf("sf\t\t��\t\t\t�鿴���ļ�\n");
	printf("format\t\t��\t\t\t��ʽ���ļ�ϵͳ\n");
	printf("exit\t\t��\t\t\t�˳�ϵͳ\n\n");
	printf("***************************************************************\n");
	startsys();
	printf("%s: %s$ ", USERNAME, openfilelist[curdirid].dir);

	while (~scanf("%s", command) && strcmp(command, "exit")) {
		if (!strcmp(command, "ls")) {
			my_ls();
		}
		else if (!strcmp(command, "help")) {
			my_help();
		}
		else if (!strcmp(command, "mkdir")) {
			scanf("%s", command);
			if (rewrite_dir(command))
				my_mkdir(command);
		}
		else if (!strcmp(command, "close")) {
			scanf("%d", &fd);
			my_close(fd);
		}
		else if (!strcmp(command, "open")) {
			scanf("%s", command);
			if (!rewrite_dir(command)) continue;
			fd = my_open(command);
			if (0 <= fd && fd < MAXOPENFILE) {
				if (!openfilelist[fd].open_fcb.attribute) {
					my_close(fd);
					printf("%s is dirictory, please use cd command\n", command);
				}
				else {
					printf("%s is open, it\'s id is %d\n", openfilelist[fd].dir, fd);
				}
			}
		}
		else if (!strcmp(command, "cd")) {
			scanf("%s", command);
			if (rewrite_dir(command)) my_cd(command);
		}
		else if (!strcmp(command, "create")) {
			scanf("%s", command);
			if (!rewrite_dir(command)) continue;
			fd = my_create(command);
			if (0 <= fd && fd < MAXOPENFILE) {
				printf("%s is created, it\'s id is %d\n", openfilelist[fd].dir, fd);
			}
		}
		else if (!strcmp(command, "rm")) {
			scanf("%s", command);
			if (rewrite_dir(command)) 
				my_rm(command);
		}
		else if (!strcmp(command, "rmdir")) {
			scanf("%s", command);
			if (rewrite_dir(command)) 
				my_rmdir(command);
		}
		else if (!strcmp(command, "read")) {
			scanf("%d", &fd);
			my_read(fd);
		}
		else if (!strcmp(command, "write")) {
			scanf("%d", &fd);
			my_write(fd);
		}
		else if (!strcmp(command, "sf")) {
			for (int i = 0; i < MAXOPENFILE; ++i) {
				if (openfilelist[i].topenfile) printf("  %d : %s\n", i, openfilelist[i].dir);
			}
		}
		else if (!strcmp(command, "format")) {
			scanf("%s", command);
			my_format();
		}
		else {
			printf("command %s : no such command\n", command);
		}
		my_reload(curdirid);
		printf("%s: %s$ ", USERNAME, openfilelist[curdirid].dir);

		memcpy(blockaddr[0], &initblock, sizeof(initblock));
		memcpy(blockaddr[1], fat1, sizeof(fat1));
		memcpy(blockaddr[3], fat1, sizeof(fat1));
		FILE *fp = fopen("myfsys", "wb");
		fwrite(myvhard, BLOCKSIZE, BLOCKNUM, fp);
		fclose(fp);
	}

	my_exitsys();
	return 0;
}

// utils

void fcb_init(fcb *new_fcb, const char* filename, unsigned short first, unsigned char attribute) {
	time_t now;
	struct tm *nowtime;
	strcpy(new_fcb->filename, filename);
	new_fcb->first = first;
	new_fcb->attribute = attribute;
	new_fcb->free = 0;

	if (attribute) new_fcb->length = 0;
	else new_fcb->length =  sizeof(fcb);
}

void useropen_init(useropen *openfile, int dirno, int diroff, const char* dir) {
	openfile->dirno = dirno;
	openfile->diroff = diroff;
	strcpy(openfile->dir, dir);
	openfile->fcbstate = 0;
	openfile->topenfile = 1;
	openfile->count = openfile->open_fcb.length;
}

void fatFree(int id) {
	if (id == END) return;
	if (fat1[id].id != END) fatFree(fat1[id].id);
	fat1[id].id = FREE;
}

int getFreeFatid() {
	for (int i = 5; i < BLOCKNUM; ++i) if (fat1[i].id == FREE) return i;
	return END;
}

int getFreeOpenlist() {
	for (int i = 0; i < MAXOPENFILE; ++i) if (!openfilelist[i].topenfile) return i;
	return -1;
}

int getNextFat(int id) {
	if (fat1[id].id == END) fat1[id].id = getFreeFatid();
	return fat1[id].id;
}

int check_fd(int fd) {
	if (!(0 <= fd && fd < MAXOPENFILE)) {
		SAYERROR;
		printf("check_fd: %d is invaild index\n", fd);
		return 0;
	}
	return 1;
}

int spiltDir(char dirs[DIRLEN][DIRLEN], char *filename) {
	int bg = 0; int ed = strlen(filename);
	if (filename[0] == '/') ++bg;
	if (filename[ed - 1] == '/') --ed;

	int ret = 0, tlen = 0;
	for (int i = bg; i < ed; ++i) {
		if (filename[i] == '/') {
			dirs[ret][tlen] = '\0';
			tlen = 0;
			++ret;
		}
		else {
			dirs[ret][tlen++] = filename[i];
		}
	}
	dirs[ret][tlen] = '\0';

	return ret + 1;
}

void popLastDir(char *dir) {
	int len = strlen(dir) - 1;
	while (dir[len - 1] != '/') --len;
	dir[len] = '\0';
}

void splitLastDir(char *dir, char new_dir[2][DIRLEN]) {
	int len = strlen(dir);
	int flag = -1;
	for (int i = 0; i < len; ++i) if (dir[i] == '/') flag = i;

	if (flag == -1) {
		SAYERROR;
		printf("splitLastDir: can\'t split %s\n", dir);
		return;
	}

	int tlen = 0;
	for (int i = 0; i < flag; ++i) {
		new_dir[0][tlen++] = dir[i];
	}
	new_dir[0][tlen] = '\0';
	tlen = 0;
	for (int i = flag + 1; i < len; ++i) {
		new_dir[1][tlen++] = dir[i];
	}
	new_dir[1][tlen] = '\0';
}

void getPos(int *id, int *offset, unsigned short first, int length) {
	int blockorder = length >> 10;
	*offset = length % 1024;
	*id = first;
	while (blockorder) {
		--blockorder;
		*id = fat1[*id].id;
	}
}

int rewrite_dir(char *dir) {
	int len = strlen(dir);
	if (dir[len - 1] == '/') --len;
	int pre = -1;
	for (int i = 0; i < len; ++i) if (dir[len] == '/') {
		if (pre != -1) {
			if (pre + 1 == i) {
				printf("rewrite_dir: %s is invaild, please check!\n", dir);
				return 0;
			}
		}
		pre = i;
	}
	char newdir[DIRLEN];
	if (dir[0] == '/') {
		strcpy(newdir, "~");
	}
	else {
		strcpy(newdir, openfilelist[curdirid].dir);
	}
	strcat(newdir, dir);
	strcpy(dir, newdir);
	return 1;
}

// basics
int fat_read(unsigned short id, unsigned char *text, int offset, int len) {
	int ret = 0;
	unsigned char *buf = (unsigned char*)malloc(BLOCKSIZE);

	int count = 0;
	while (len) {
		memcpy(buf, blockaddr[id], BLOCKSIZE);
		count = min(len, 1024 - offset);
		memcpy(text + ret, buf + offset, count);
		len -= count;
		ret += count;
		offset = 0;
		id = fat1[id].id;
	}

	free(buf);
	return ret;
}

int do_read(int fd, unsigned char *text, int len) {
	int blockorder = openfilelist[fd].count >> 10;
	int blockoffset = openfilelist[fd].count % 1024;
	unsigned short id = openfilelist[fd].open_fcb.first;
	while (blockorder) {
		--blockorder;
		id = fat1[id].id;
	}

	int ret = fat_read(id, text, blockoffset, len);

	return ret;
}

int fat_write(unsigned short id, unsigned char *text, int blockoffset, int len) {
	int ret = 0;
	char *buf = (char*)malloc(1024);
	if (buf == NULL) {
		SAYERROR;
		printf("fat_write: malloc error\n");
		return -1;
	}

	// д֮ǰ�ȰѴ��̳������䵽�����С
	int tlen = len;
	int toffset = blockoffset;
	unsigned short tid = id;
	while (tlen) {
		if (tlen <= 1024 - toffset) break;
		tlen -= (1024 - toffset);
		toffset = 0;
		id = getNextFat(id);
		if (id == END) {
			SAYERROR;
			printf("fat_write: no next fat\n");
			return -1;
		}
	}

	int count = 0;
	while (len) {
		memcpy(buf, blockaddr[id], BLOCKSIZE);
		count = min(len, 1024 - blockoffset);
		memcpy(buf + blockoffset, text + ret, count);
		memcpy(blockaddr[id], buf, BLOCKSIZE);
		len -= count;
		ret += count;
		blockoffset = 0;
		id = fat1[id].id;
	}

	free(buf);
	return ret;
}

int do_write(int fd, unsigned char *text, int len) {
	fcb *fcbp = &openfilelist[fd].open_fcb;

	int blockorder = openfilelist[fd].count >> 10;
	int blockoffset = openfilelist[fd].count % 1024;
	unsigned short id = openfilelist[fd].open_fcb.first;
	while (blockorder) {
		--blockorder;
		id = fat1[id].id;
	}

	int ret = fat_write(id, text, blockoffset, len);

	fcbp->length += ret;
	openfilelist[fd].fcbstate = 1;
	// ����ļ��б�д�ˣ���ô��'.'ҲҪ��д��ȥ
	// �����ļ��е�'..'ҲҪ������
	if (!fcbp->attribute) {
		fcb tmp;
		memcpy(&tmp, fcbp, sizeof(fcb));
		strcpy(tmp.filename, ".");
		memcpy(blockaddr[fcbp->first], &tmp, sizeof(fcb));

		// ����Ǹ�Ŀ¼�Ļ���".."ҲҪ���޸�
		strcpy(tmp.filename, "..");
		if (fcbp->first == 5) {
			memcpy(blockaddr[fcbp->first] + sizeof(fcb), &tmp, sizeof(fcb));
		}

		// �Ӵ����ж�����ǰĿ¼����Ϣ
		unsigned char buf[SIZE];
		int read_size = read_ls(fd, buf, fcbp->length);
		if (read_size == -1) {
			SAYERROR;
			printf("do_write: read_ls error\n");
			return 0;
		}
		fcb dirfcb;
		for (int i = 2 * sizeof(fcb); i < read_size; i += sizeof(fcb)) {
			memcpy(&dirfcb, buf + i, sizeof(fcb));
			if (dirfcb.free || dirfcb.attribute) continue;
			memcpy(blockaddr[dirfcb.first] + sizeof(fcb), &tmp, sizeof(fcb));
		}
	}

	return ret;
}

int getFcb(fcb* fcbp, int *dirno, int *diroff, int fd, const char *dir) {
	if (fd == -1) {
		memcpy(fcbp, blockaddr[5], sizeof(fcb));
		*dirno = 5;
		*diroff = 0;
		return 1;
	}

	useropen *file = &openfilelist[fd];

	// �Ӵ����ж�����ǰĿ¼����Ϣ
	unsigned char *buf = (unsigned char *)malloc(SIZE);
	int read_size = read_ls(fd, buf, file->open_fcb.length);
	if (read_size == -1) {
		SAYERROR;
		printf("getFcb: read_ls error\n");
		return -1;
	}
	fcb dirfcb;
	int flag = -1;
	for (int i = 0; i < read_size; i += sizeof(fcb)) {
		memcpy(&dirfcb, buf + i, sizeof(fcb));
		if (dirfcb.free) continue;
		if (!strcmp(dirfcb.filename, dir)) {
			flag = i;
			break;
		}
	}

	free(buf);

	// û���ҵ���Ҫ���ļ�
	if (flag == -1) return -1;

	// �ҵ��Ļ��Ϳ�ʼ���������Ϣ���ı��Ӧ���ļ����ֵ
	getPos(dirno, diroff, file->open_fcb.first, flag);
	memcpy(fcbp, &dirfcb, sizeof(fcb));

	return 1;
}

int getOpenlist(int fd, const char *org_dir) {
	// ��·��������ɾ���·��
	char dir[DIRLEN];
	if (fd == -1) {
		strcpy(dir, "~/");
	}
	else {
		strcpy(dir, openfilelist[fd].dir);
		strcat(dir, org_dir);
	}

	// ����д򿪵�Ŀ¼����򿪵�Ŀ¼�����������ԭĿ¼������д�ش���
	for (int i = 0; i < MAXOPENFILE; ++i) if (i != fd) {
		if (openfilelist[i].topenfile && !strcmp(openfilelist[i].dir, dir)) {
			my_save(i);
		}
	}

	int fileid = getFreeOpenlist();
	if (fileid == -1) {
		SAYERROR;
		printf("getOpenlist: openlist is full\n");
		return -1;
	}

	fcb dirfcb;
	useropen *file = &openfilelist[fileid];
	int ret;
	if (fd == -1) {
		ret = getFcb(&file->open_fcb, &file->dirno, &file->diroff, -1, ".");
	}
	else {
		ret = getFcb(&file->open_fcb, &file->dirno, &file->diroff, fd, org_dir);
	}
	strcpy(file->dir, dir);
	file->fcbstate = 0;
	file->topenfile = 1;

	//����򿪵���һ���ļ��У�����·���������'/'
	if (!file->open_fcb.attribute) {
		int len = strlen(file->dir);
		if (file->dir[len - 1] != '/') strcat(file->dir, "/");
	}

	if (ret == -1) {
		file->topenfile = 0;
		return -1;
	}
	return fileid;
}

int my_open(char *filename) {
	char dirs[DIRLEN][DIRLEN];
	int count = spiltDir(dirs, filename);

	char realdirs[DIRLEN][DIRLEN];
	int tot = 0;
	for (int i = 1; i < count; ++i) {
		if (!strcmp(dirs[i], ".")) continue;
		if (!strcmp(dirs[i], "..")) {
			if (tot) --tot;
			continue;
		}
		strcpy(realdirs[tot++], dirs[i]);
	}

	// ���ɸ�Ŀ¼�ĸ���
	int fd = getOpenlist(-1, "");

	// ���õ�ǰĿ¼�ĸ��������ҵ���һ��Ŀ¼
	int flag = 0;
	for (int i = 0; i < tot; ++i) {
		int newfd = getOpenlist(fd, realdirs[i]);
		if (newfd == -1) {
			flag = 1;
			break;
		}
		my_close(fd);
		fd = newfd;
	}
	if (flag) {
		printf("my_open: %s no such file or directory\n", filename);
		openfilelist[fd].topenfile = 0;
		return -1;
	}

	if (openfilelist[fd].open_fcb.attribute) openfilelist[fd].count = 0;
	else openfilelist[fd].count = openfilelist[fd].open_fcb.length;
	return fd;
}

// read
int read_ls(int fd, unsigned char *text, int len) {
	int tcount = openfilelist[fd].count;
	openfilelist[fd].count = 0;
	int ret = do_read(fd, text, len);
	openfilelist[fd].count = tcount;
	return ret;
}

void my_ls() {
	// �Ӵ����ж�����ǰĿ¼����Ϣ
	unsigned char *buf = (unsigned char*)malloc(SIZE);
	int read_size = read_ls(curdirid, buf, openfilelist[curdirid].open_fcb.length);
	if (read_size == -1) {
		free(buf);
		SAYERROR;
		printf("my_ls: read_ls error\n");
		return;
	}
	fcb dirfcb;
	for (int i = 0; i < read_size; i += sizeof(fcb)) {
		memcpy(&dirfcb, buf + i, sizeof(fcb));
		if (dirfcb.free) continue;
		if (dirfcb.attribute) printf("<FILE>\t%s\n", dirfcb.filename);
		else printf("<DIR>\t%s\n", dirfcb.filename);
	}
	free(buf);
}

int my_read(int fd) {
	if (!(0 <= fd && fd < MAXOPENFILE) || !openfilelist[fd].topenfile ||
		!openfilelist[fd].open_fcb.attribute) {
		printf("my_read: fd invaild\n");
		return -1;
	}

	unsigned char *buf = (unsigned char *)malloc(SIZE);
	int len = openfilelist[fd].open_fcb.length - openfilelist[fd].count;
	int ret = do_read(fd, buf, len);
	if (ret == -1) {
		free(buf);
		printf("my_read: do_read error\n");
		return -1;
	}
	buf[ret] = '\0';
	printf("%s\n", buf);
	return ret;
}

void my_reload(int fd) {
	if (!check_fd(fd)) return;
	fat_read(openfilelist[fd].dirno, (unsigned char*)&openfilelist[fd].open_fcb, openfilelist[fd].diroff, sizeof(fcb));
	return;
}

// write
int my_write(int fd) {
	if (!(0 <= fd && fd < MAXOPENFILE) || !openfilelist[fd].topenfile ||
		!openfilelist[fd].open_fcb.attribute) {
		printf("my_write: fd invaild\n");
		return -1;
	}

	useropen *file = &openfilelist[fd];
	printf("������д�ļ���ʽ\n");
	printf("  a : ׷��д\n");
	printf("  w : �ض�д\n");
	printf("  o : ����д\n");
	char op[5];
	scanf("%s", op);
	if (op[0] == 'a') {
		file->count = file->open_fcb.length;
	}
	else if (op[0] == 'w') {
		file->count = 0;
		file->open_fcb.length = 0;
		fatFree(fat1[file->open_fcb.first].id);
	}
	else if (op[0] != 'o') {
		printf("my_write: invaild write style!\n");
		return -1;
	}

	int ret = 0;
	int tmp;
	while (gets_s(str, 2048)) {
		int len = strlen(str);
		str[len] = '\n';
		tmp = do_write(fd, (unsigned char*)str, len + 1);
		if (tmp == -1) {
			SAYERROR;
			printf("my_write: do_write error\n");
			return -1;
		}
		file->count += tmp;
		ret += tmp;
	}
	return ret;
}

// delete
void my_rmdir(char *dirname) {
	int fd = my_open(dirname);
	if (0 <= fd && fd < MAXOPENFILE) {
		if (openfilelist[fd].open_fcb.attribute) {
			printf("my_rmdir: %s is a file, please use rm command\n", dirname);
			my_close(fd);
			return;
		}
		if (!strcmp(openfilelist[fd].dir, openfilelist[curdirid].dir)) {
			printf("my_rmdir: can not remove the current directory!\n");
			my_close(fd);
			return;
		}

		// �Ӵ����ж�����ǰĿ¼����Ϣ
		int cnt = 0;
		unsigned char *buf = (unsigned char*)malloc(SIZE);
		int read_size = read_ls(fd, buf, openfilelist[fd].open_fcb.length);
		if (read_size == -1) {
			my_close(fd);
			free(buf);
			SAYERROR;
			printf("my_rmdir: read_ls error\n");
			return;
		}
		fcb dirfcb;
		int flag = -1;
		for (int i = 0; i < read_size; i += sizeof(fcb)) {
			memcpy(&dirfcb, buf + i, sizeof(fcb));
			if (dirfcb.free) continue;
			++cnt;
		}

		if (cnt > 2) {
			my_close(fd);
			printf("my_rmdir: %s is not empty\n", dirname);
			return;
		}

		openfilelist[fd].open_fcb.free = 1;
		fatFree(openfilelist[fd].open_fcb.first);
		openfilelist[fd].fcbstate = 1;
		my_close(fd);
	}
}

void my_rm(char *filename) {
	int fd = my_open(filename);
	if (0 <= fd && fd < MAXOPENFILE) {
		if (openfilelist[fd].open_fcb.attribute == 0) {
			printf("my_rm: %s is a directory, please use rmdir command\n", filename);
			my_close(fd);
			return;
		}

		openfilelist[fd].open_fcb.free = 1;
		fatFree(openfilelist[fd].open_fcb.first);
		openfilelist[fd].fcbstate = 1;
		my_close(fd);
	}
}

// creat
void my_format() {
	strcpy(initblock.information, "10101010");
	initblock.root = 5;
	initblock.startblock = blockaddr[5];

	for (int i = 0; i < 5; ++i) fat1[i].id = END;
	for (int i = 5; i < BLOCKNUM; ++i) fat1[i].id = FREE;
	for (int i = 0; i < BLOCKNUM; ++i) fat2[i].id = fat1[i].id;

	fat1[5].id = END;
	fcb root;
	fcb_init(&root, ".", 5, 0);
	memcpy(blockaddr[5], &root, sizeof(fcb));

#ifdef DEBUG_INFO
	printf("my_format %s\n", root.filename);
#endif // DEBUG_INFO

	strcpy(root.filename, "..");
	memcpy(blockaddr[5] + sizeof(fcb), &root, sizeof(fcb));

#ifdef DEBUG_INFO
	printf("my_format %s\n", root.filename);
#endif // DEBUG_INFO

	printf("��ʼ�����\n");
}

int my_touch(char *filename, int attribute, int *rpafd) {
	// �ȴ�file���ϼ�Ŀ¼������ϼ�Ŀ¼�����ھͱ��������Լ������ϵ�Ubuntu������߼���
	char split_dir[2][DIRLEN];
	splitLastDir(filename, split_dir);

	int pafd = my_open(split_dir[0]);
	if (!(0 <= pafd && pafd < MAXOPENFILE)) {
		SAYERROR;
		printf("my_creat: my_open error\n");
		return -1;
	}

	// �Ӵ����ж�����ǰĿ¼����Ϣ�����м��
	unsigned char *buf = (unsigned char*)malloc(SIZE);
	int read_size = read_ls(pafd, buf, openfilelist[pafd].open_fcb.length);
	if (read_size == -1) {
		SAYERROR;
		printf("my_touch: read_ls error\n");
		return -1;
	}
	fcb dirfcb;
	for (int i = 0; i < read_size; i += sizeof(fcb)) {
		memcpy(&dirfcb, buf + i, sizeof(fcb));
		if (dirfcb.free) continue;
		if (!strcmp(dirfcb.filename, split_dir[1])) {
			printf("%s is already exit\n", split_dir[1]);
			return -1;
		}
	}

	// ���ÿ��д��̿鴴���ļ�
	int fatid = getFreeFatid();
	if (fatid == -1) {
		SAYERROR;
		printf("my_touch: no free fat\n");
		return -1;
	}
	fat1[fatid].id = END;
	fcb_init(&dirfcb, split_dir[1], fatid, attribute);

	// д�븸��Ŀ¼�ڴ�
	memcpy(buf, &dirfcb, sizeof(fcb));
	int write_size = do_write(pafd, buf, sizeof(fcb));
	if (write_size == -1) {
		SAYERROR;
		printf("my_touch: do_write error\n");
		return -1;
	}
	openfilelist[pafd].count += write_size;

	// �����Լ��Ĵ��ļ���
	int fd = getFreeOpenlist();
	if (!(0 <= fd && fd < MAXOPENFILE)) {
		SAYERROR;
		printf("my_touch: no free fat\n");
		return -1;
	}
	getPos(&openfilelist[fd].dirno, &openfilelist[fd].diroff, openfilelist[pafd].open_fcb.first, openfilelist[pafd].count - write_size);
	memcpy(&openfilelist[fd].open_fcb, &dirfcb, sizeof(fcb));
	if (attribute) openfilelist[fd].count = 0;
	else openfilelist[fd].count = openfilelist[fd].open_fcb.length;
	openfilelist[fd].fcbstate = 1;
	openfilelist[fd].topenfile = 1;
	strcpy(openfilelist[fd].dir, openfilelist[pafd].dir);
	strcat(openfilelist[fd].dir, split_dir[1]);

	free(buf);
	*rpafd = pafd;
	return fd;
}

int my_create(char *filename) {
	int pafd;
	int fd = my_touch(filename, 1, &pafd);
	if (!check_fd(fd)) return -1;
	my_close(pafd);
	return fd;
}

void my_mkdir(char *dirname) {
	int pafd;
	int fd = my_touch(dirname, 0, &pafd);
	if (!check_fd(fd)) return;
	unsigned char *buf = (unsigned char*)malloc(SIZE);

	// ��"."��".."װ���Լ��Ĵ���
	fcb dirfcb;
	memcpy(&dirfcb, &openfilelist[fd].open_fcb, sizeof(fcb));
	int fatid = dirfcb.first;
	strcpy(dirfcb.filename, ".");
	memcpy(blockaddr[fatid], &dirfcb, sizeof(fcb));
	memcpy(&dirfcb, &openfilelist[pafd].open_fcb, sizeof(fcb));
	strcpy(dirfcb.filename, "..");
	memcpy(blockaddr[fatid] + sizeof(fcb), &dirfcb, sizeof(fcb));

	my_close(pafd);
	my_close(fd);
	free(buf);
}

// others
void startsys() {
	// ���ֱ�����ʼ��
	myvhard = (unsigned char*)malloc(SIZE);
	for (int i = 0; i < BLOCKNUM; ++i) blockaddr[i] = i * BLOCKSIZE + myvhard;
	for (int i = 0; i < MAXOPENFILE; ++i) openfilelist[i].topenfile = 0;

	// ׼������ myfsys �ļ���Ϣ
	FILE *fp = fopen("myfsys", "rb");
	char need_format = 0;

	// �ж��Ƿ���Ҫ��ʽ��
	if (fp != NULL) {
		unsigned char *buf = (unsigned char*)malloc(SIZE);
		fread(buf, 1, SIZE, fp);
		memcpy(myvhard, buf, SIZE);
		memcpy(&initblock, blockaddr[0], sizeof(block0));
		if (strcmp(initblock.information, "10101010") != 0) need_format = 1;
		free(buf);
		fclose(fp);
	}
	else {
		need_format = 1;
	}

	// ����Ҫ��ʽ���Ļ����Ŷ���fat��Ϣ
	if (!need_format) {
		memcpy(fat1, blockaddr[1], sizeof(fat1));
		memcpy(fat2, blockaddr[3], sizeof(fat2));
	}
	else {
		printf("myfsys �ļ�ϵͳ�����ڣ����ڿ�ʼ�����ļ�ϵͳ\n");
		my_format();
	}

	// �Ѹ�Ŀ¼fcb������ļ����У��趨��ǰĿ¼Ϊ��Ŀ¼
	curdirid = 0;
	memcpy(&openfilelist[curdirid].open_fcb, blockaddr[5], sizeof(fcb));
#ifdef DEBUG_INFO
	printf("starsys: %s\n", openfilelist[curdirid].open_fcb.filename);
#endif // DEBUG_INFO
	useropen_init(&openfilelist[curdirid], 5, 0, "~/");
}

void my_exitsys() {
	// �ȹر����д��ļ���
	for (int i = 0; i < MAXOPENFILE; ++i) my_close(i);

	memcpy(blockaddr[0], &initblock, sizeof(initblock));
	memcpy(blockaddr[1], fat1, sizeof(fat1));
	memcpy(blockaddr[3], fat1, sizeof(fat1));
	FILE *fp = fopen("myfsys", "wb");
	fwrite(myvhard, BLOCKSIZE, BLOCKNUM, fp);

	free(myvhard);
	fclose(fp);
}

void my_save(int fd) {
	if (!check_fd(fd)) return;

	useropen *file = &openfilelist[fd];
	if (file->fcbstate) fat_write(file->dirno, (unsigned char *)&file->open_fcb, file->diroff, sizeof(fcb));
	file->fcbstate = 0;
	return;
}

void my_close(int fd) {
	if (!check_fd(fd)) return;
	if (openfilelist[fd].topenfile == 0) return;

	// �������иı䣬��fcb����д�ظ��׵Ĵ��̿���
	if (openfilelist[fd].fcbstate) my_save(fd);

	openfilelist[fd].topenfile = 0;
	return;
}

void my_cd(char *dirname) {
	int fd = my_open(dirname);
	if (!check_fd(fd)) return;
	if (openfilelist[fd].open_fcb.attribute) {
		my_close(fd);
		printf("%s is a file, please use open command\n", openfilelist[fd].dir);
		return;
	}

	// �õ���fd���ļ��еĻ����Ͱ�ԭ����Ŀ¼����,�����ڵ�Ŀ¼��Ϊ��ǰĿ¼
	my_close(curdirid);
	curdirid = fd;
}

void my_help() {
	printf("***************************************************************\n");
	printf("������\t\t�������\t\t����˵��\n\n");
	printf("cd\t\tĿ¼��(·����)\t\t�л���ǰĿ¼��ָ��Ŀ¼\n");
	printf("mkdir\t\tĿ¼��\t\t\t�ڵ�ǰĿ¼������Ŀ¼\n");
	printf("rmdir\t\tĿ¼��\t\t\t�ڵ�ǰĿ¼ɾ��ָ��Ŀ¼\n");
	printf("ls\t\t��\t\t\t��ʾ��ǰĿ¼�µ�Ŀ¼���ļ�\n");
	printf("create\t\t�ļ���\t\t\t�ڵ�ǰĿ¼�´���ָ���ļ�\n");
	printf("rm\t\t�ļ���\t\t\t�ڵ�ǰĿ¼��ɾ��ָ���ļ�\n");
	printf("open\t\t�ļ���\t\t\t�ڵ�ǰĿ¼�´�ָ���ļ�\n");
	printf("write\t\tfd\t\t\t�ڴ��ļ�״̬�£�д���ļ�\n");
	printf("read\t\tfd\t\t\t�ڴ��ļ�״̬�£���ȡ���ļ�\n");
	printf("close\t\tfd\t\t\t�ڴ��ļ�״̬�£��رո��ļ�\n");
	printf("help\t\t��\t\t\t�鿴����\n");
	printf("sf\t\t��\t\t\t�鿴���ļ�\n");
	printf("format\t\t��\t\t\t��ʽ���ļ�ϵͳ\n");
	printf("exit\t\t��\t\t\t�˳�ϵͳ\n\n");
	printf("***************************************************************\n");
}



