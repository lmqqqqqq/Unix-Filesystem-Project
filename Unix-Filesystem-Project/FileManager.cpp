#include "FileManager.h"
#include "Kernel.h"
#include "Utility.h"

/*
 * ���ܣ����ļ�
 * Ч�����������ļ��ṹ���ڴ�i�ڵ㿪�� ��i_count Ϊ������i_count ++��
 * */
void FileManager::Open()
{
	Inode* pInode;
	Kernel* k = Kernel::getInstance();

	pInode = this->NameI(NextChar, FileManager::OPEN);
	/* û���ҵ���Ӧ��Inode */
	if (pInode == NULL)
		return;
	this->Open1(pInode, k->mode, 0);
}

/*
 * ���ܣ�����һ���µ��ļ�
 * Ч�����������ļ��ṹ���ڴ�i�ڵ㿪�� ��i_count Ϊ������Ӧ���� 1��
 * */
void FileManager::Creat()
{
	Inode* pInode;
	Kernel* k = Kernel::getInstance();
	unsigned int newACCMode = k->mode & (Inode::IRWXU | Inode::IRWXG | Inode::IRWXO);

	pInode = this->NameI(NextChar, FileManager::CREATE);
	/* û���ҵ���Ӧ��Inode����NameI���� */
	if (pInode == NULL)
	{
		if (k->error != Kernel::NO_ERROR)
			return;
		/* ����Inode */
		pInode = this->MakNode(newACCMode & (~Inode::ISVTX));
		if (pInode == NULL)
		{
			return;
		}
		/*
		 * �����ϣ�������ֲ����ڣ�ʹ�ò���trf = 2������open1()��
		 * ����Ҫ����Ȩ�޼�飬��Ϊ�ոս������ļ���Ȩ�޺ʹ������mode
		 * ����ʾ��Ȩ��������һ���ġ�
		 */
		this->Open1(pInode, File::FWRITE, 2);
	}
	else
	{
		/* ���NameI()�������Ѿ�����Ҫ�������ļ�������ո��ļ������㷨ITrunc()����UIDû�иı�
		 * ԭ��UNIX��������������ļ�����ȥ�����½����ļ�һ����Ȼ�������ļ������ߺ����Ȩ��ʽû�䡣
		 * Ҳ����˵creatָ����RWX������Ч��
		 * ������Ϊ���ǲ�����ģ�Ӧ�øı䡣
		 * ���ڵ�ʵ�֣�creatָ����RWX������Ч */
		this->Open1(pInode, File::FWRITE, 1);
		pInode->i_mode |= newACCMode;
	}
}

/*
* trf == 0��open����
* trf == 1��creat���ã�creat�ļ���ʱ��������ͬ�ļ������ļ�
* trf == 2��creat���ã�creat�ļ���ʱ��δ������ͬ�ļ������ļ��������ļ�����ʱ��һ������
* mode���������ļ�ģʽ����ʾ�ļ������� ����д���Ƕ�д
*/
void FileManager::Open1(Inode* pInode, int mode, int trf)
{
	Kernel* k = Kernel::getInstance();

	/*
	 * ����ϣ�����ļ��Ѵ��ڵ�����£���trf == 0��trf == 1����Ȩ�޼��
	 * �����ϣ�������ֲ����ڣ���trf == 2������Ҫ����Ȩ�޼�飬��Ϊ�ս���
	 * ���ļ���Ȩ�޺ʹ���Ĳ���mode������ʾ��Ȩ��������һ���ġ�
	 */
	if (trf != 2 && (mode & File::FWRITE))
	{
		/* openȥдĿ¼�ļ��ǲ������ */
		if ((pInode->i_mode & Inode::IFMT) == Inode::IFDIR)
		{
			k->error = Kernel::ISDIR;
		}
	}
	if (k->error != Kernel::NO_ERROR) {
		k->getInodeTable()->IPut(pInode);
		return;
	}

	/* ��creat�ļ���ʱ��������ͬ�ļ������ļ����ͷŸ��ļ���ռ�ݵ������̿� */
	if (trf == 1)
	{
		pInode->ITrunc();
	}

	/* ������ļ����ƿ�File�ṹ */
	File* pFile = k->getOpenFileTable()->FAlloc();
	if (pFile == NULL)
	{
		k->getInodeTable()->IPut(pInode);
		return;
	}
	/* ���ô��ļ���ʽ������File�ṹ���ڴ�Inode�Ĺ�����ϵ */
	pFile->f_flag = mode & (File::FREAD | File::FWRITE);
	pFile->f_inode = pInode;

	/* ���ܴ���Ŀ¼�ļ������� */
	if (trf != 0 && k->isDir)
	{
		pInode->i_mode |= Inode::IFDIR;
	}
	return;
}

void FileManager::Close()
{
	Kernel* k = Kernel::getInstance();

	/* ��ȡ���ļ����ƿ�File�ṹ */
	File* pFile = k->getOpenFiles()->GetF(k->fd);
	if (pFile == NULL)
		return;
	/* �ͷŴ��ļ�������fd���ݼ�File�ṹ���ü��� */
	k->getOpenFiles()->SetF(k->fd, NULL);
	/* ������ϵͳ���ļ�����File�����ü��� */
	k->getOpenFileTable()->CloseF(pFile);
}

void FileManager::Seek()
{
	File* pFile;
	Kernel* k = Kernel::getInstance();
	int fd = k->fd;

	pFile = k->getOpenFiles()->GetF(fd);
	if (NULL == pFile)
	{
		return;     /* ��FILE�����ڣ�GetF��������� */
	}

	int offset = k->offset;

	/* ���seekģʽ��3 ~ 5֮�䣬�򳤶ȵ�λ���ֽڱ�Ϊ512�ֽ� */
	if (k->mode > 2)
	{
		offset = offset << 9;
		k->mode -= 3;
	}

	switch (k->mode)
	{
		/* ��дλ������Ϊoffset */
	case 0:
		pFile->f_offset = offset;
		break;
		/* ��дλ�ü�offset(�����ɸ�) */
	case 1:
		pFile->f_offset += offset;
		break;
		/* ��дλ�õ���Ϊ�ļ����ȼ�offset */
	case 2:
		pFile->f_offset = pFile->f_inode->i_size + offset;
		break;
	}
}

void FileManager::Read()
{
	/* ֱ�ӵ���Rdwr()�������� */
	this->Rdwr(File::FREAD);
}

void FileManager::Write()
{
	/* ֱ�ӵ���Rdwr()�������� */
	this->Rdwr(File::FWRITE);
}

void FileManager::Rdwr(enum File::FileFlags mode)
{
	File* pFile;
	Kernel* k = Kernel::getInstance();

	/* ����Read()/Write()��ϵͳ���ò���fd��ȡ���ļ����ƿ�ṹ */
	pFile = k->getOpenFiles()->GetF(k->fd);
	if (pFile == NULL)
	{
		/* �����ڸô��ļ���GetF�Ѿ����ù������룬�������ﲻ��Ҫ�������� */
		return;
	}

	k->k_IOParam.m_Base = (char*)k->buf;         /* Ŀ�껺������ַ */
	k->k_IOParam.m_Count = k->nbytes;            /* Ҫ���/д���ֽ��� */
	k->k_IOParam.m_Offset = pFile->f_offset;     /* �����ļ���ʼ��λ�� */
	if (File::FREAD == mode)
		pFile->f_inode->ReadI();
	else
		pFile->f_inode->WriteI();
	/* ���ݶ�д�������ƶ��ļ���дƫ��ָ�� */
	pFile->f_offset += (k->nbytes - k->k_IOParam.m_Count);
	/* ����ʵ�ʶ�д���ֽ������޸Ĵ��ϵͳ���÷���ֵ�ĺ���ջ��Ԫ */
	k->callReturn = k->nbytes - k->k_IOParam.m_Count;
}

/* ����NULL��ʾĿ¼����ʧ�ܣ������Ǹ�ָ�룬ָ���ļ����ڴ��i�ڵ� ���������ڴ�i�ڵ�  */
Inode* FileManager::NameI(char(*func)(), enum DirectorySearchMode mode)
{
	Inode* pInode;
	Buf* pBuf;
	char curchar;
	char* pChar;
	int freeEntryOffset;	         /* �Դ����ļ�ģʽ����Ŀ¼ʱ����¼����Ŀ¼���ƫ���� */
	Kernel* k = Kernel::getInstance();
	BufferManager* bufMgr = k->getBufMgr();

	/*
	 * �����·����'/'��ͷ�ģ��Ӹ�Ŀ¼��ʼ������
	 * ����ӽ��̵�ǰ����Ŀ¼��ʼ������
	 */
	pInode = k->cdir;
	if ('/' == (curchar = (*func)()))
	{
		pInode = this->rootDirInode;
	}
		
	/* ����Inode�Ƿ����ڱ�ʹ�ã��Լ���֤������Ŀ¼���������и�Inode�����ͷ� */
	k->getInodeTable()->IGet(pInode->i_number);

	/* �������////a//b ����·�� ����·���ȼ���/a/b */
	while ('/' == curchar)
	{
		curchar = (*func)();
	}

	/* �����ͼ���ĺ�ɾ����ǰĿ¼�ļ������ */
	if ('\0' == curchar && mode != FileManager::OPEN)
		goto out;

	/* ���ѭ��ÿ�δ���pathname��һ��·������ */
	while (true)
	{
		/* ����������ͷŵ�ǰ��������Ŀ¼�ļ�Inode�����˳� */
		if (k->error != Kernel::NO_ERROR)
		{
			break;     /* goto out; */
		}
			
		/* ����·��������ϣ�������ӦInodeָ�롣Ŀ¼�����ɹ����ء� */
		if ('\0' == curchar)
		{
			return pInode;
		}
			
		/* ��Pathname�е�ǰ׼������ƥ���·������������Kernel��dbuf[]�� ���ں�Ŀ¼����бȽ� */
		pChar = &(k->dbuf[0]);
		while ('/' != curchar && '\0' != curchar && k->error == Kernel::NO_ERROR)
		{
			if (pChar < &(k->dbuf[DirectoryEntry::DIRSIZ]))
			{
				*pChar = curchar;
				pChar++;
			}
			curchar = (*func)();
		}
		/* ��dbufʣ��Ĳ������Ϊ'\0' */
		while (pChar < &(k->dbuf[DirectoryEntry::DIRSIZ]))
		{
			*pChar = '\0';
			pChar++;
		}

		/* �������////a//b ����·�� ����·���ȼ���/a/b */
		while ('/' == curchar)
		{
			curchar = (*func)();
		}
			
		if (k->error != Kernel::NO_ERROR)
		{
			return NULL;           /* */
		}

		/* �ڲ�ѭ�����ֶ���dbuf[]�е�·���������������Ѱƥ���Ŀ¼�� */
		k->k_IOParam.m_Offset = 0;
		/* ����ΪĿ¼����� �����հ׵�Ŀ¼��*/
		k->k_IOParam.m_Count = pInode->i_size / (DirectoryEntry::DIRSIZ + 4);   /* 4�����INode��С */
		freeEntryOffset = 0;
		pBuf = NULL;

		while (true)
		{
			/* ��Ŀ¼���Ѿ�������� */
			if (k->k_IOParam.m_Count == 0)
			{
				if (pBuf != NULL)
				{
					bufMgr->Brelse(pBuf);
				}
				/* ����Ǵ������ļ� */
				if (FileManager::CREATE == mode && curchar == '\0')
				{
					/* ����Ŀ¼Inodeָ�뱣���������Ժ�дĿ¼�WriteDir()���������õ� */
					k->pdir = pInode;

					if (freeEntryOffset)	/* �˱�������˿���Ŀ¼��λ��Ŀ¼�ļ��е�ƫ���� */
					{
						/* ������Ŀ¼��ƫ��������u���У�дĿ¼��WriteDir()���õ� */
						k->k_IOParam.m_Offset = freeEntryOffset - (DirectoryEntry::DIRSIZ + 4);
					}
					else   /*���⣺Ϊ��if��֧û����IUPD��־��  ������Ϊ�ļ��ĳ���û�б�ѽ*/
					{
						pInode->i_flag |= Inode::IUPD;
					}
					/* �ҵ�����д��Ŀ���Ŀ¼��λ�ã�NameI()�������� */
					return NULL;
				}

				/* Ŀ¼��������϶�û���ҵ�ƥ����ͷ����Inode��Դ,���˳� */
				k->error = Kernel::NOENT;
				goto out;
			}

			/* �Ѷ���Ŀ¼�ļ��ĵ�ǰ�̿飬��Ҫ������һĿ¼�������̿� */
			if (k->k_IOParam.m_Offset % Inode::BLOCK_SIZE == 0)
			{
				if (pBuf != NULL)
				{
					bufMgr->Brelse(pBuf);
				}
				/* ����Ҫ���������̿�� */
				int phyBlkno = pInode->Bmap(k->k_IOParam.m_Offset / Inode::BLOCK_SIZE);
				pBuf = bufMgr->Bread(phyBlkno);
			}

			/* û�ж��굱ǰĿ¼���̿飬���ȡ��һĿ¼����dent */
			int* src = (int*)(pBuf->b_addr + (k->k_IOParam.m_Offset % Inode::BLOCK_SIZE));
			Utility::copy<int>(src, (int*)&k->dent, sizeof(DirectoryEntry) / sizeof(int));

			k->k_IOParam.m_Offset += (DirectoryEntry::DIRSIZ + 4);
			k->k_IOParam.m_Count--;

			/* ����ǿ���Ŀ¼���¼����λ��Ŀ¼�ļ���ƫ���� */
			if (k->dent.inode == 0)
			{
				if (freeEntryOffset == 0)
				{
					freeEntryOffset = k->k_IOParam.m_Offset;
				}
				/* ��������Ŀ¼������Ƚ���һĿ¼�� */
				continue;
			}

			int i;
			for (i = 0; i < DirectoryEntry::DIRSIZ; i++)
			{
				if (k->dbuf[i] != k->dent.name[i])
				{
					break;	/* ƥ����ĳһ�ַ�����������forѭ�� */
				}
					
			}

			if (i < DirectoryEntry::DIRSIZ)
			{
				/* ����Ҫ������Ŀ¼�����ƥ����һĿ¼�� */
				continue;
			}
			else
			{
				/* Ŀ¼��ƥ��ɹ����ص����While(true)ѭ�� */
				break;
			}
		}

		/*
		 * ���ڲ�Ŀ¼��ƥ��ѭ�������˴���˵��pathname��
		 * ��ǰ·������ƥ��ɹ��ˣ�����ƥ��pathname����һ·��
		 * ������ֱ������'\0'������
		 */
		if (pBuf != NULL)
			bufMgr->Brelse(pBuf);

		/* �����ɾ���������򷵻ظ�Ŀ¼Inode����Ҫɾ���ļ���Inode����dent.inode�� */
		if (FileManager::DELETE == mode && '\0' == curchar)
			return pInode;

		/*
		 * ƥ��Ŀ¼��ɹ������ͷŵ�ǰĿ¼Inode������ƥ��ɹ���
		 * Ŀ¼��m_ino�ֶλ�ȡ��Ӧ��һ��Ŀ¼���ļ���Inode��
		 */
		k->getInodeTable()->IPut(pInode);
		pInode = k->getInodeTable()->IGet(k->dent.inode);
		/* �ص����While(true)ѭ��������ƥ��Pathname����һ·������ */

		if (pInode == NULL)   /* ��ȡʧ�� */
			return NULL;
	}
out:
	k->getInodeTable()->IPut(pInode);
	return NULL;
}

char FileManager::NextChar()
{
	Kernel* k = Kernel::getInstance();
	/* k->dirpָ��pathname�е��ַ� */
	return *k->dirp++;
}

Inode* FileManager::MakNode(unsigned int mode)
{
	Inode* pInode;
	Kernel* k = Kernel::getInstance();

	/* ����һ������DiskInode������������ȫ����� */
	pInode = k->getFileSys()->IAlloc();

	if (pInode == NULL)
	{
		return NULL;
	}

	pInode->i_flag |= (Inode::IACC | Inode::IUPD);
	pInode->i_mode = mode | Inode::IALLOC;
	pInode->i_nlink = 1;

	/* ��Ŀ¼��д��u.u_dent�����д��Ŀ¼�ļ� (this->WriteDir(pInode)) */
	/* ����Ŀ¼����Inode��Ų��� */
	k->dent.inode = pInode->i_number;

	/* ����Ŀ¼����pathname�������� */
	for (int i = 0; i < DirectoryEntry::DIRSIZ; i++)
	{
		k->dent.name[i] = k->dbuf[i];
	}
		
	k->k_IOParam.m_Count = DirectoryEntry::DIRSIZ + 4;
	k->k_IOParam.m_Base = (char*)&k->dent;      //unsigned char*

	/* ��Ŀ¼��д�븸Ŀ¼�ļ� */
	k->pdir->WriteI();
	k->getInodeTable()->IPut(k->pdir);
	return pInode;
}

void FileManager::ChDir()
{
	Inode* pInode;
	Kernel* k = Kernel::getInstance();

	pInode = this->NameI(FileManager::NextChar, FileManager::OPEN);
	if (pInode == NULL)
		return;
	/* ���������ļ�����Ŀ¼�ļ� */
	if ((pInode->i_mode & Inode::IFMT) != Inode::IFDIR)
	{
		k->error = Kernel::NOTDIR;
		k->getInodeTable()->IPut(pInode);
		return;
	}
	k->getInodeTable()->IPut(k->cdir);
	k->cdir = pInode;

	/* ���õ�ǰ����Ŀ¼�ַ���curdir this->SetCurDir */
	/* ·�����ǴӸ�Ŀ¼'/'��ʼ����������u.u_curdir������ϵ�ǰ·������ */
	if (k->pathname[0] != '/')
	{
		int length = Utility::strlen(k->curdir);
		if (k->curdir[length - 1] != '/')
		{
			k->curdir[length] = '/';
			length++;
		}
		Utility::StringCopy(k->pathname, k->curdir + length);
	}
	else    /* ����ǴӸ�Ŀ¼'/'��ʼ����ȡ��ԭ�й���Ŀ¼ */
	{
		Utility::StringCopy(k->pathname, k->curdir);
	}
}

void FileManager::Delete()
{
	Inode* pInode;  //��ǰĿ¼��INodeָ��
	Inode* pDeleteInode;  //��ǰ�ļ���INodeָ��
	Kernel* k = Kernel::getInstance();

	pDeleteInode = this->NameI(FileManager::NextChar, FileManager::DELETE);
	//û�ҵ���Ҫɾ�����ļ�
	if (pDeleteInode == NULL)
		return;
	pInode = k->getInodeTable()->IGet(k->dent.inode);
	k->k_IOParam.m_Offset -= (DirectoryEntry::DIRSIZ + 4);
	k->k_IOParam.m_Base = (char*)&k->dent;
	k->k_IOParam.m_Count = DirectoryEntry::DIRSIZ + 4;
	k->dent.inode = 0;
	pDeleteInode->WriteI();        //ɾ�����ļ�д�ش���
	pInode->i_nlink--;
	pInode->i_flag |= Inode::IUPD; //nlink--
	k->getInodeTable()->IPut(pDeleteInode);
	k->getInodeTable()->IPut(pInode);
}

void FileManager::Rename(string ori, string cur)
{
	Inode* pInode;  //��ǰĿ¼��INodeָ��
	Kernel* k = Kernel::getInstance();
	Buf* pBuf = NULL;
	BufferManager* bufMgr = k->getBufMgr();
	pInode = k->getInodeTable()->IGet(k->dent.inode);
	k->k_IOParam.m_Offset = 0;
	k->k_IOParam.m_Count = pInode->i_size / sizeof(DirectoryEntry);
	while (k->k_IOParam.m_Count) {
		if (0 == k->k_IOParam.m_Offset % Inode::BLOCK_SIZE) {  //Ҫ���̿���
			if (pBuf) //��������Ϣ
				bufMgr->Brelse(pBuf); //�ͷ�
			int phyBlkno = pInode->Bmap(k->k_IOParam.m_Offset / Inode::BLOCK_SIZE); //�µ��̿��
			pBuf = bufMgr->Bread(phyBlkno); //������µ��̿�
		}

		DirectoryEntry tmp; //
		memcpy(&tmp, pBuf->b_addr + (k->k_IOParam.m_Offset % Inode::BLOCK_SIZE), sizeof(k->dent));

		if (strcmp(tmp.name, ori.c_str()) == 0) {
			strcpy_s(tmp.name, cur.c_str());
			memcpy(pBuf->b_addr + (k->k_IOParam.m_Offset % Inode::BLOCK_SIZE), &tmp, sizeof(k->dent));
		}
		k->k_IOParam.m_Offset += sizeof(DirectoryEntry);
		k->k_IOParam.m_Count--;
	}
	//char* ans = pBuf->b_addr + ((k->k_IOParam.m_Offset -32 ) % Inode::BLOCK_SIZE)+4 ;
	if (pBuf)
	{
		bufMgr->Bwrite(pBuf);
		//bufMgr->Brelse(pBuf);
	}
		
}