#pragma once
#include "FileSystem.h"
#include "OpenfileManager.h"

/*
 * �ļ�������(FileManager)
 * ��װ���ļ�ϵͳ�ĸ���ϵͳ�����ں���̬�´�����̣�
 * ����ļ���Open()��Close()��Read()��Write()�ȵ�
 * ��װ�˶��ļ�ϵͳ���ʵľ���ϸ�ڡ�
 */
class FileManager
{
public:
	/* Ŀ¼����ģʽ������NameI()���� */
	enum DirectorySearchMode
	{
		OPEN = 0,		/* �Դ��ļ���ʽ����Ŀ¼ */
		CREATE = 1,		/* ���½��ļ���ʽ����Ŀ¼ */
		DELETE = 2		/* ��ɾ���ļ���ʽ����Ŀ¼ */
	};
public:
	FileManager() {};
	~FileManager() {};
	void Open();			 /* ��һ���ļ� */
	void Creat();             /* �½�һ���ļ� */
	void Open1(Inode* pInode, int mode, int trf); /* Open��Create�Ĺ������� */
	void Close();             /* �ر�һ���ļ� */
	void Seek();          /* �ı䵱ǰ��дָ���λ�� */
	void Read();          /* ���ļ� */
	void Write();         /* д�ļ� */
	void Rdwr(enum File::FileFlags mode);  /* Read��Write�����Ĺ������� */
	Inode* NameI(char(*func)(), enum DirectorySearchMode mode); /* ·������ ��·��ת��Ϊ��Ӧ��Inode*/
	static char NextChar();  /* ��ȡ·���е���һ���ַ� */
	Inode* MakNode(unsigned int mode); /* ��Creat()���ã��½�һ���ļ�ʱ��������Դ */
	void ChDir();             /* �ı䵱ǰ����Ŀ¼ */
	void Delete();        /* ɾ���ļ� */    
	void Rename(string ori, string cur);        /* �������ļ� */  
	
public:
	Inode* rootDirInode; /* ��Ŀ¼�ڴ�Inode */
};

class DirectoryEntry
{
public:
	static const int DIRSIZ = 28;	/* Ŀ¼����·�����ֵ�����ַ������� */
public:
	DirectoryEntry() {};
	~DirectoryEntry() {};
public:
	int inode;		        /* Ŀ¼����Inode��Ų��� */
	char name[DIRSIZ];	    /* Ŀ¼����·�������� */
};
