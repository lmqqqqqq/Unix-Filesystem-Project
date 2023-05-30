#include "Kernel.h"
#include "Utility.h"
#include <fstream>

Kernel Kernel::instance;

Kernel::Kernel()
{
	Kernel::DISK_IMG = "myDisk.img";
	this->BufMgr = new BufferManager;
	this->fileSys = new FileSystem;
	this->fileMgr = new FileManager;
	this->k_InodeTable = new InodeTable;
	this->s_openFiles = new OpenFileTable;
	this->k_openFiles = new OpenFiles;
	this->spb = new SuperBlock;
}

Kernel::~Kernel()
{
}

void Kernel::clear()
{
	delete this->BufMgr;
	delete this->fileSys;
	delete this->fileMgr;
	delete this->k_InodeTable;
	delete this->s_openFiles;
	delete this->k_openFiles;
	delete this->spb;
}

void Kernel::initialize()
{
	this->fileSys->LoadSuperBlock();
	this->fileMgr->rootDirInode = this->k_InodeTable->IGet(FileSystem::ROOTINO); //��ø�Ŀ¼�ڴ�INode
	this->cdir = this->fileMgr->rootDirInode;      //��ָ��ǰĿ¼(��Ŀ¼)��ָ�븳ֵ
	Utility::StringCopy("/", this->curdir);
}

void Kernel::callInit()
{
	this->fileMgr->rootDirInode = this->k_InodeTable->IGet(FileSystem::ROOTINO); //��ø�Ŀ¼�ڴ�INode
	this->callReturn = -1;
	this->error = NO_ERROR;           //���־λ
}

Kernel* Kernel::getInstance()
{
	return &instance;
}

BufferManager* Kernel::getBufMgr()
{
	return this->BufMgr;
}

FileSystem* Kernel::getFileSys()
{
	return this->fileSys;
}

FileManager* Kernel::getFileMgr()
{
	return this->fileMgr;
}

InodeTable* Kernel::getInodeTable()
{
	return this->k_InodeTable;
}

OpenFiles* Kernel::getOpenFiles()
{
	return this->k_openFiles;
}

OpenFileTable* Kernel::getOpenFileTable()
{
	return this->s_openFiles;
}

SuperBlock* Kernel::getSuperBlock()
{
	return this->spb;
}

void Kernel::format()
{
	/* ��ʽ������ */
	fstream f(Kernel::DISK_IMG, ios::out | ios::binary);
	for (int i = 0; i <= this->getFileSys()->DATA_ZONE_END_SECTOR; i++)
	{
		for (int j = 0; j < this->getBufMgr()->BUFFER_SIZE; j++)
		{
			f.write((char*)" ", 1);
		}
			
	}
	f.close();

	/* ��ʽ��SuperBlock */
	Buf* pBuf;
	SuperBlock& spb = (*this->spb);
	spb.s_isize = FileSystem::INODE_ZONE_SIZE;
	spb.s_fsize = FileSystem::DATA_ZONE_SIZE;
	spb.s_ninode = 100;
	spb.s_nfree = 0;
	for (int i = 0; i < 100; i++)  //ջʽ�洢
	{
		spb.s_inode[99 - i] = i + 1;
	}

	for (int i = FileSystem::DATA_ZONE_END_SECTOR; i >= FileSystem::DATA_ZONE_START_SECTOR; i--) //�����������
	{
		this->fileSys->Free(i);
	}

	for (int i = 0; i < 2; i++)
	{
		int* p = (int*)&spb + i * 128;
		pBuf = this->BufMgr->GetBlk(FileSystem::SUPER_BLOCK_SECTOR_NUMBER + i);
		Utility::copy<int>(p, (int*)pBuf->b_addr, 128);
		this->BufMgr->Bwrite(pBuf);
	}

	/* ��ʽ��Inode�� */
	for (int i = 0; i < FileSystem::INODE_ZONE_SIZE; i++)
	{
		pBuf = this->BufMgr->GetBlk(FileSystem::ROOTINO + FileSystem::INODE_ZONE_START_SECTOR + i);
		DiskInode DiskInode[FileSystem::INODE_NUMBER_PER_SECTOR];
		for (int j = 0; j < FileSystem::INODE_NUMBER_PER_SECTOR; j++)
		{
			//����DiskINode��ʽ��
			DiskInode[j].d_mode = DiskInode[j].d_nlink = DiskInode[j].d_size = 0;
			for (int k = 0; k < 10; k++)
			{
				DiskInode[j].d_addr[k] = 0;
			}
		}
		/* Ϊ��Ŀ¼����Ŀ¼��־ */
		if (i == 0)
		{
			DiskInode[0].d_mode |= Inode::IFDIR;
		}
		Utility::copy<int>((int*)&DiskInode, (int*)pBuf->b_addr, 128);
		//д�ش���
		this->BufMgr->Bwrite(pBuf);
	}
	//ȫ�µĸ���ģ��
	this->clear();
	this->BufMgr = new BufferManager;
	this->fileSys = new FileSystem;
	this->fileMgr = new FileManager;
	this->k_InodeTable = new InodeTable;
	this->s_openFiles = new OpenFileTable;
	this->k_openFiles = new OpenFiles;
	this->spb = new SuperBlock;
}

int Kernel::open(char* pathname, int mode)
{
	this->callInit();
	this->mode = mode;
	this->pathname = this->dirp = pathname;
	this->fileMgr->Open();
	return this->callReturn;
}

int Kernel::create(char* pathname, int mode)
{
	this->callInit();
	this->isDir = false;
	this->mode = mode;
	this->pathname = this->dirp = pathname;
	this->fileMgr->Creat();
	this->fileSys->Update();
	return this->callReturn;
}

void Kernel::mkdir(char* pathname)
{
	this->callInit();
	this->isDir = true;
	this->pathname = this->dirp = pathname;
	this->fileMgr->Creat();
	this->fileSys->Update();
	if (this->callReturn != -1)
		this->close(this->callReturn);
}

int Kernel::close(int fd)
{
	this->callInit();
	this->fd = fd;
	this->fileMgr->Close();
	return this->callReturn;
}

void Kernel::cd(char* pathname)
{
	this->callInit();
	this->pathname = this->dirp = pathname;
	this->fileMgr->ChDir();
}

int Kernel::fread(int readFd, char* buf, int nbytes)
{
	this->callInit();
	this->fd = readFd;
	this->buf = buf;
	this->nbytes = nbytes;
	this->fileMgr->Read();
	return this->callReturn;
}

int Kernel::fwrite(int writeFd, char* buf, int nbytes)
{
	this->callInit();
	this->fd = writeFd;
	this->buf = buf;
	this->nbytes = nbytes;
	this->getFileMgr()->Write();
	return this->callReturn;
}

void Kernel::ls()
{
	this->k_IOParam.m_Offset = 0;
	this->k_IOParam.m_Count = this->cdir->i_size / (DirectoryEntry::DIRSIZ + 4);
	Buf* pBuf = NULL;
	while (true)
	{
		/* ��Ŀ¼���Ѿ�������� */
		if (this->k_IOParam.m_Count == 0)
		{
			if (pBuf != NULL)
			{
				this->getBufMgr()->Brelse(pBuf);
			}
			break;
		}

		/* �Ѷ���Ŀ¼�ļ��ĵ�ǰ�̿飬��Ҫ������һĿ¼�������̿� */
		if (this->k_IOParam.m_Offset % Inode::BLOCK_SIZE == 0)
		{
			if (pBuf != NULL)
			{
				this->getBufMgr()->Brelse(pBuf);
			}
			int phyBlkno = this->cdir->Bmap(this->k_IOParam.m_Offset / Inode::BLOCK_SIZE);
			pBuf = this->getBufMgr()->Bread(phyBlkno);
		}

		/* û�ж��굱ǰĿ¼���̿飬���ȡ��һĿ¼����dent */
		int* src = (int*)(pBuf->b_addr + (this->k_IOParam.m_Offset % Inode::BLOCK_SIZE));
		Utility::copy<int>(src, (int*)&this->dent, sizeof(DirectoryEntry) / sizeof(int));
		this->k_IOParam.m_Offset += (DirectoryEntry::DIRSIZ + 4);
		this->k_IOParam.m_Count--;
		if (this->dent.inode == 0)
		{
			continue;
		}
		cout << this->dent.name << ' ';
	}
	cout << endl;
}

void Kernel::fseek(int seekFd, int offset, int ptrname)
{
	this->callInit();
	this->fd = seekFd;
	this->offset = offset;
	this->mode = ptrname;
	this->fileMgr->Seek();
}

void Kernel::fdelete(char* pathname)
{
	this->callInit();
	this->dirp = pathname;
	this->fileMgr->Delete();
}

void Kernel::fmount(char* from, char* to)
{
	fstream f(from, ios::in | ios::binary);
	if (f)
	{
		f.seekg(0, f.end);  /* ��һ��������ƫ�������ڶ��������ǻ���ַ */
		int length = f.tellg();  /* ���ص�ǰ��λָ���λ�ã�Ҳ�������������Ĵ�С */
		f.seekg(0, f.beg);
		char* tmpBuffer = new char[length];
		f.read(tmpBuffer, length);  //�����ݶ����м������
		int tmpFd = this->open(to, 511);
		if (this->error != NO_ERROR)
			goto end;
		this->fwrite(tmpFd, tmpBuffer, length);
		if (this->error != NO_ERROR)
			goto end;
		this->close(tmpFd);
	end:
		f.close();
		delete tmpBuffer;
		return;
	}
	else
	{
		this->error = NOOUTENT;
		return;
	}
}

void Kernel::frename(char* Ori, char* Cur)
{
	this->callInit();
	char* curDir = curdir;
	string ori = Ori;
	string cur = Cur;
	if (ori.find('/') != string::npos) {
		string nextDir = ori.substr(0, ori.find_last_of('/'));
		if(nextDir=="")
		{
			nextDir = "/";
		}
		char nd[128];
		strcpy_s(nd, nextDir.c_str());
		cd(nd);
		ori = ori.substr(ori.find_last_of('/') + 1);
		if (cur.find('/') != string::npos)
			cur = cur.substr(cur.find_last_of('/') + 1);
	}
	this->fileMgr->Rename(ori,cur);
	cd(curDir);
}

void Kernel::dfs_tree(string path, int depth)
{
	vector<string> dirs; /* Ŀ¼�� */
	string nextDir;

	this->k_IOParam.m_Offset = 0;
	this->k_IOParam.m_Count = this->cdir->i_size / (DirectoryEntry::DIRSIZ + 4);
	Buf* pBuf = NULL;
	while (true)
	{
		/* ��Ŀ¼���Ѿ�������� */
		if (this->k_IOParam.m_Count == 0)
		{
			if (pBuf != NULL)
			{
				this->getBufMgr()->Brelse(pBuf);
			}
			break;
		}

		/* �Ѷ���Ŀ¼�ļ��ĵ�ǰ�̿飬��Ҫ������һĿ¼�������̿� */
		if (this->k_IOParam.m_Offset % Inode::BLOCK_SIZE == 0)
		{
			if (pBuf != NULL)
			{
				this->getBufMgr()->Brelse(pBuf);
			}
			int phyBlkno = this->cdir->Bmap(this->k_IOParam.m_Offset / Inode::BLOCK_SIZE);
			pBuf = this->getBufMgr()->Bread(phyBlkno);
		}

		/* û�ж��굱ǰĿ¼���̿飬���ȡ��һĿ¼����dent */
		int* src = (int*)(pBuf->b_addr + (this->k_IOParam.m_Offset % Inode::BLOCK_SIZE));
		Utility::copy<int>(src, (int*)&this->dent, sizeof(DirectoryEntry) / sizeof(int));
		this->k_IOParam.m_Offset += (DirectoryEntry::DIRSIZ + 4);
		this->k_IOParam.m_Count--;
		if (this->dent.inode == 0)
		{
			continue;
		}
		dirs.emplace_back(this->dent.name);
	}
	for (int i = 0; i < dirs.size(); i++) {
		nextDir = (path == "/" ? "" : path) + '/' + dirs[i];
		for (int j = 0; j < depth + 1; j++)
			cout << "|   ";
		cout << "|---" << dirs[i] << endl;
		char nd[128];
		strcpy_s(nd, nextDir.c_str());
		cd(nd);
		if(error == Kernel::NOTDIR)  /* ���ʵ��������ļ�������Ŀ¼�ļ� */
		{
			error = NO_ERROR;
			continue;
		}
		dfs_tree(nextDir, depth + 1);
	}
	char nd[128];
	strcpy_s(nd, path.c_str());
	cd(nd);
	return;
}

void Kernel::ftree(string path)
{
	string curDirPath = curdir;
	/*if (curDirPath.length() > 1 && curDirPath.back() == '/')
		curDirPath.pop_back();
	string curDir = curDirPath;*/
	//path = dirp;
	char nd[128];
	strcpy_s(nd, path.c_str());
	cd(nd);
	
	cout << "|---" << (path == "/" ? "/" : path.substr(path.find_last_of('/') + 1)) << endl;
	dfs_tree(path, 0);
	strcpy_s(nd, curDirPath.c_str());
	cd(nd);
}
