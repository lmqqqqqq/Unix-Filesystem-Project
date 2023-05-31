#pragma once
#include "INode.h"
#include "FileSystem.h"

/*
 * �ڴ�Inode��(class InodeTable)
 * �����ڴ�Inode�ķ�����ͷš�
 */
class InodeTable
{
public:
	static const int NINODE = 100;	/* �ڴ�Inode������ */
public:
	InodeTable();
	~InodeTable();
	Inode* IGet(int inumber); /* �������Inode��Ż�ȡ��ӦInode */
	void IPut(Inode* pNode);  /* ���ٸ��ڴ�Inode�����ü����������Inode�Ѿ�û��Ŀ¼��ָ���������޽������ø�Inode�����ͷŴ��ļ�ռ�õĴ��̿� */
	void UpdateInodeTable();  /* �����б��޸Ĺ����ڴ�Inode���µ���Ӧ���Inode�� */
	int IsLoaded(int inumber); /* �����Ϊinumber�����inode�Ƿ����ڴ濽����������򷵻ظ��ڴ�Inode���ڴ�Inode���е����� */
	Inode* GetFreeInode();     /* ���ڴ�Inode����Ѱ��һ�����е��ڴ�Inode */
public:
	Inode m_Inode[NINODE];		/* �ڴ�Inode���飬ÿ�����ļ�����ռ��һ���ڴ�Inode */
};

/*
 * ���ļ����ƿ�File�ࡣ
 * �ýṹ��¼�˽��̴��ļ�
 * �Ķ���д�������ͣ��ļ���дλ�õȵȡ�
 */
/* ��¼�ļ���ͬһ���߲�ͬ��·�����򿪣��ò�ͬ�Ĳ���Ҫ���*/
/* ��Դ��File.h�� */
class File
{
public:
	enum FileFlags
	{
		FREAD = 0x1,			/* ���������� */
		FWRITE = 0x2,			/* д�������� */
		FPIPE = 0x4				/* �ܵ����� */
	};
public:
	File();
	~File();
	unsigned int f_flag;		/* �Դ��ļ��Ķ���д����Ҫ�� */
	int		f_count;			/* ��ǰ���ø��ļ����ƿ�Ľ������� */
	Inode* f_inode;			    /* ָ����ļ����ڴ�Inodeָ�� */
	int		f_offset;			/* �ļ���дλ��ָ�� */
};

/* һ������Ϊ File �����飬����ÿһ���һ�� File �ļ����ƿ� */
class OpenFileTable
{
public:
	static const int NFILE = 100;	/* ���ļ����ƿ�File�ṹ������ */
public:
	OpenFileTable() {};
	~OpenFileTable() {};
	File* FAlloc();            /* ��ϵͳ���ļ����з���һ�����е�File�ṹ */
	void CloseF(File* pFile); /* �Դ��ļ����ƿ�File�ṹ�����ü���f_count��1. �����ü���f_countΪ0�����ͷ�File�ṹ*/
public:
	File m_File[NFILE];			/* ϵͳ���ļ��� */
};

/* ����ÿ�����̵Ĵ��ļ����䱾����Ҳ��һ�� File ������� */
class OpenFiles
{
public:
	static const int NOFILES = 15;	/* ����򿪵�����ļ��� */
public:
	OpenFiles();
	~OpenFiles() {};
	int AllocFreeSlot();            /* ���ں˴��ļ����������з���һ�����б��� */
	File* GetF(int fd);             /* �����ļ�����������fd�ҵ���Ӧ�Ĵ��ļ����ƿ�File�ṹ */
	void SetF(int fd, File* pFile); /* Ϊ�ѷ��䵽�Ŀ���������fd���ѷ���Ĵ��ļ����п���File������������ϵ*/
public:
	File* k_OpenFileTable[NOFILES];		/* File�����ָ�����飬ָ��ϵͳ���ļ����е�File���� */
};