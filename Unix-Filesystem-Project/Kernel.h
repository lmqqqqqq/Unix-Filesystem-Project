#pragma once
//#ifndef KERNEL_H
//#define KERNEL_H
#include "BufferManager.h"
#include "FileManager.h"
#include "FileSystem.h"
#include "OpenfileManager.h"  /* */
#include <iostream>
using namespace std;

/*
 * �ļ�I/O�Ĳ�����
 * ���ļ�����дʱ���õ��Ķ���дƫ������
 * �ֽ����Լ�Ŀ�������׵�ַ������
 */
struct IOParameter
{
	char* m_Base;	/* ��ǰ����д�û�Ŀ��������׵�ַ */    //unsigned char*
	int m_Offset;	/* ��ǰ����д�ļ����ֽ�ƫ���� */
	int m_Count;	/* ��ǰ��ʣ��Ķ���д�ֽ����� */
};

/* �����ļ�ϵͳ�ĺ�����(�ں�)��ֻ��ʼ��һ��ʵ��*/
class Kernel
{
public:
	/* �ο�User.h�е�u_error's Error Code */
	enum ERROR {
		NO_ERROR = 0,            /* û�г��� */
		ISDIR = 1,               /* ���ݷ������ļ� */
		NOTDIR = 2,              /* cd������������ļ� */
		NOENT = 3,               /* �ļ������� */
		BADF = 4,                /* �ļ���ʶfd���� */
		NOOUTENT = 5,            /* �ⲿ�ļ������� */
		NOSPACE = 6              /* ���̿ռ䲻�� */
	};
	Kernel();
	~Kernel();
	static Kernel* getInstance();  /* ��ȡΨһ���ں���ʵ�� */

	BufferManager* getBufMgr();        /* ��ȡ�ں˵ĸ��ٻ������ʵ�� */
	FileSystem* getFileSys();          /* ��ȡ�ں˵��ļ�ϵͳʵ�� */
	FileManager* getFileMgr();         /* ��ȡ�ں˵��ļ�����ʵ�� */
	InodeTable* getInodeTable();       /* ��ȡ�ں˵��ڴ�Inode�� */
	OpenFiles* getOpenFiles();         /* ��ȡ�ں˵Ĵ��ļ��������� */
	OpenFileTable* getOpenFileTable(); /* ��ȡϵͳȫ�ֵĴ��ļ��������� */
	SuperBlock* getSuperBlock();       /* ��ȡȫ�ֵ�SuperBlock�ڴ渱��*/
public:
	/* ϵͳ������س�Ա */
	char* dirp;			   	      /* ָ��·������ָ��,����nameI���� */

	/* �ļ�ϵͳ��س�Ա */
	Inode* cdir;		          /* ָ��ǰĿ¼��Inodeָ�� */
	Inode* pdir;                  /* ָ��ǰĿ¼��Ŀ¼��Inodeָ�� */
	DirectoryEntry dent;		  /* ��ǰĿ¼��Ŀ¼�� */
	char dbuf[DirectoryEntry::DIRSIZ];	/* ��ǰ·������ */
	char curdir[128];            /* ��ǰ��������Ŀ¼ */
	ERROR error;                  /* ��Ŵ����� */

	/* �ļ�I/O���� */
	IOParameter k_IOParam;        /* ��¼��ǰ����д�ļ���ƫ�������û�Ŀ�������ʣ���ֽ������� */

	/* ��ǰϵͳ���ò��� */
	char* buf;                    /* ָ���д�Ļ����� */   //u_arg[0]
	int fd;                       /* ��¼�ļ���ʶ */     //u_arg[0]
	char* pathname;               /* Ŀ��·���� */     //u_arg[0]
	int nbytes;                   /* ��¼��д���ֽ��� */   //u_arg[1]
	int offset;                   /* ��¼Seek�Ķ�дָ��λ�� */   //u_arg[1]
	int mode;                     /* ��¼�����ļ��ķ�ʽ��seek��ģʽ */   //u_arg[2]
	int callReturn;               /* ��¼���ú����ķ���ֵ */   //u_ar0[User::EAX]

	static const char* DISK_IMG;
	bool isDir;                   /* ��ǰ�����Ƿ����Ŀ¼�ļ� */
private:
	static Kernel instance;      /* Ψһ���ں���ʵ�� */

	BufferManager* BufMgr;       /* �ں˵ĸ��ٻ������ʵ�� */
	FileSystem* fileSys;         /* �ں˵��ļ�ϵͳʵ�� */
	FileManager* fileMgr;        /* �ں˵��ļ�����ʵ�� */
	InodeTable* k_InodeTable;    /* �ں˵��ڴ�Inode�� */
	OpenFileTable* s_openFiles;  /* ϵͳȫ�ִ��ļ��������� */
	OpenFiles* k_openFiles;      /* �ں˵Ĵ��ļ��������� */
	SuperBlock* spb;              /* ȫ�ֵ�SuperBlock�ڴ渱�� */
public:
	void initialize();                                  /* �ں˳�ʼ�� */
	void callInit();                                    /* ÿ���������õĳ�ʼ������ */
	void format();                                      /* ��ʽ������ */
	int open(char* pathname, int mode);                 /* ���ļ� */
	int create(char* pathname, int mode);               /* �½��ļ� */
	void mkdir(char* pathname);                         /* �½�Ŀ¼ */
	void cd(char* pathname);                            /* �ı䵱ǰ����Ŀ¼ */
	void ls();                                          /* ��ʾ��ǰĿ¼�µ������ļ� */
	int fread(int readFd, char* buf, int nbytes);       /* ��һ���ļ���Ŀ���� */
	int fwrite(int writeFd, char* buf, int nbytes);     /* ����Ŀ�������ַ�дһ���ļ� */
	void fseek(int seekFd, int offset, int ptrname);    /* �ı��дָ���λ�� */
	void fdelete(char* pathname);                       /* ɾ���ļ� */
	void fmount(char* from, char* to);                  /* ���ļ�����������ĳĿ¼�� */
	void frename(char* ori, char* cur);                 /* ���ļ������� */
	void dfs_tree(string path, int depth);
	void ftree(string path);                             /* ��ʾĿ¼�� */
	int close(int fd);                                  /* �ر��ļ� */
	void clear();                                       /* ϵͳ�ر�ʱ��β���� */
};

//#endif
