#include "Kernel.h"

InodeTable::InodeTable()
{

}

InodeTable::~InodeTable()
{

}

/* ��ֱ�Ӳ��Ҹ� DiskINode �Ƿ��ж�Ӧ���ڴ� INode�����У��򷵻ظ� INode����û�У����������һ���ڴ� INode��*/
Inode* InodeTable::IGet(int inumber)
{
	Inode* pInode;
	Kernel* k = Kernel::getInstance();
	BufferManager* bufMgr = Kernel::getInstance()->getBufMgr();

	while (true)
	{
		/* �����Ϊinumber�����Inode�Ƿ����ڴ濽�� */
		int index = this->IsLoaded(inumber);
		if (index >= 0)	/* �ҵ��ڴ濽�� */
		{
			pInode = &(this->m_Inode[index]);
			pInode->i_count++;
			return pInode;
		}
		else	/* û��Inode���ڴ濽���������һ�������ڴ�Inode */
		{
			pInode = this->GetFreeInode();
			/* ���ڴ�Inode���������������Inodeʧ�� */
			if (pInode == NULL)
				return NULL;
			else    /* �������Inode�ɹ��������Inode�����·�����ڴ�Inode */
			{
				/* �����µ����Inode��ţ��������ü��� */
				pInode->i_number = inumber;
				pInode->i_count++;
				/* �������Inode���뻺���� */
				Buf* pBuf = bufMgr->Bread(FileSystem::INODE_ZONE_START_SECTOR + inumber / FileSystem::INODE_NUMBER_PER_SECTOR);
				/* ���������е����Inode��Ϣ�������·�����ڴ�Inode�� */
				pInode->ICopy(pBuf, inumber);
				/* �ͷŻ��� */
				bufMgr->Brelse(pBuf);
				return pInode;
			}
		}
	}
	return NULL;
}

/* ��ֱ���ͷŶ�Ӧ�� DiskINode��Ȼ�� INode ���¿����� DiskINode�������ٸ�INode �����ü�����*/
void InodeTable::IPut(Inode* pNode)
{
	FileSystem* fileSys = Kernel::getInstance()->getFileSys();
	/* ��ǰ����Ϊ���ø��ڴ�Inode��Ψһ���̣���׼���ͷŸ��ڴ�Inode */
	if (pNode->i_count == 1)
	{
		/* ���ļ��Ѿ�û��Ŀ¼·��ָ���� */
		if (pNode->i_nlink <= 0)
		{
			/* �ͷŸ��ļ�ռ�ݵ������̿� */
			pNode->ITrunc();
			pNode->i_mode = 0;
			/* �ͷŶ�Ӧ�����Inode */
			fileSys->IFree(pNode->i_number);
		}

		/* �������Inode��Ϣ */
		pNode->IUpdate();
		/* ����ڴ�Inode�����б�־λ */
		pNode->i_flag = 0;
		/* �����ڴ�inode���еı�־֮һ����һ����i_count == 0 */
		pNode->i_number = -1;
	}
	/* �����ڴ�Inode�����ü��� */
	pNode->i_count--;
}

void InodeTable::UpdateInodeTable()
{
	for (int i = 0; i < InodeTable::NINODE; i++)
	{
		/* count������0��count == 0��ζ�Ÿ��ڴ�Inodeδ���κδ��ļ����ã�����ͬ�� */
		if (this->m_Inode[i].i_count != 0)
			this->m_Inode[i].IUpdate();
	}
}

int InodeTable::IsLoaded(int inumber)
{
	/* Ѱ��ָ�����Inode���ڴ濽�� */
	for (int i = 0; i < InodeTable::NINODE; i++)
	{
		if (this->m_Inode[i].i_number == inumber && this->m_Inode[i].i_count != 0)
			return i;
	}
	return -1;
}

Inode* InodeTable::GetFreeInode()
{
	/* ������ڴ�Inode���ü���Ϊ�㣬���Inode��ʾ���� */
	for (int i = 0; i < InodeTable::NINODE; i++)
	{
		if (this->m_Inode[i].i_count == 0)
			return &(this->m_Inode[i]);
	}
	return NULL;      /* Ѱ��ʧ�� */
}

File::File()
{
	this->f_count = 0;
	this->f_flag = 0;
	this->f_offset = 0;
	this->f_inode = NULL;
}

File::~File()
{

}

File* OpenFileTable::FAlloc()
{
	int fd;
	Kernel* k = Kernel::getInstance();

	/* ���ں˴��ļ����������л�ȡһ�������� */
	fd = k->getOpenFiles()->AllocFreeSlot();

	if (fd < 0)  /* ���Ѱ�ҿ�����ʧ�� */
	{
		return NULL;
	}

	for (int i = 0; i < OpenFileTable::NFILE; i++)
	{
		/* f_count==0��ʾ������� */
		if (this->m_File[i].f_count == 0)
		{
			/* ������������File�ṹ�Ĺ�����ϵ */
			k->getOpenFiles()->SetF(fd, &this->m_File[i]);
			/* ���Ӷ�file�ṹ�����ü��� */
			this->m_File[i].f_count++;
			/* ����ļ�����дλ�� */
			this->m_File[i].f_offset = 0;
			return (&this->m_File[i]);
		}
	}
	return NULL;
}

void OpenFileTable::CloseF(File* pFile)
{
	/* ���ü���f_count����Ϊ0�����ͷ�File�ṹ */
	if (pFile->f_count <= 1)
	{
		Kernel::getInstance()->getInodeTable()->IPut(pFile->f_inode);
	}

	/* ���õ�ǰFile�Ľ�������1 */
	pFile->f_count--;
}

OpenFiles::OpenFiles()
{
	for (int i = 0; i < OpenFiles::NOFILES; i++)
		SetF(i, NULL);
}

int OpenFiles::AllocFreeSlot()
{
	int i;
	for (i = 0; i < OpenFiles::NOFILES; i++)
	{
		/* ���̴��ļ������������ҵ�������򷵻�֮ */
		if (this->k_OpenFileTable[i] == NULL)
		{
			/* ϵͳ���÷���ֵ */
			Kernel::getInstance()->callReturn = i;
			return i;
		}
	}
	Kernel::getInstance()->callReturn = -1;   /* Open1����Ҫһ����־�������ļ��ṹ����ʧ��ʱ�����Ի���ϵͳ��Դ*/
	return -1;
}

File* OpenFiles::GetF(int fd)
{
	File* pFile;
	Kernel* k = Kernel::getInstance();

	/* ������ļ���������ֵ�����˷�Χ */
	if (fd < 0 || fd >= OpenFiles::NOFILES)
	{
		k->error = Kernel::BADF;
		return NULL;
	}

	if ((pFile = this->k_OpenFileTable[fd]) == NULL)
	{
		k->error = Kernel::BADF;
		return NULL;
	}
	return pFile;   /* ��ʹpFile==NULLҲ���������ɵ���GetF�ĺ������жϷ���ֵ */
}

void OpenFiles::SetF(int fd, File* pFile)
{
	if (fd < 0 || fd >= OpenFiles::NOFILES)
		return;
	/* ���̴��ļ�������ָ��ϵͳ���ļ�������Ӧ��File�ṹ */
	this->k_OpenFileTable[fd] = pFile;
}
