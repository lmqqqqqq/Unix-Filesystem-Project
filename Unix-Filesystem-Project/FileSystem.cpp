#include "FileSystem.h"
#include "Utility.h"
#include "Kernel.h"

FileSystem::FileSystem()
{

}

FileSystem::~FileSystem()
{

}

void FileSystem::LoadSuperBlock()
{
	Kernel* k = Kernel::getInstance();
	BufferManager* bufMgr = k->getBufMgr(); 
	SuperBlock* spb = k->getSuperBlock();  /* ����Դ���е�ϵͳȫ�ֳ�����SuperBlock���� */
	Buf* pBuf;

	for (int i = 0; i < 2; i++)
	{
		int* p = (int*)spb + i * 128;
		pBuf = bufMgr->Bread(FileSystem::SUPER_BLOCK_SECTOR_NUMBER + i);
		Utility::copy<int>((int*)pBuf->b_addr, p, 128);    /* ����Դ����Utility::DWordCopy*/
		bufMgr->Brelse(pBuf);
	}
}

void FileSystem::Update()
{
	SuperBlock* spb = Kernel::getInstance()->getSuperBlock();
	BufferManager* bufMgr = Kernel::getInstance()->getBufMgr();
	Buf* pBuf;

	/* ͬ��SuperBlock������ */
	if (spb->s_fmod == 0)
	{
		return;
	}

	/* ��SuperBlock�޸ı�־ */
	spb->s_fmod = 0;

	/*
	* Ϊ��Ҫд�ص�������ȥ��SuperBlock����һ�黺�棬���ڻ�����СΪ512�ֽڣ�
	* SuperBlock��СΪ1024�ֽڣ�ռ��2��������������������Ҫ2��д�������
	*/
	for (int i = 0; i < 2; i++)
	{
		/* ��һ��pָ��SuperBlock�ĵ�0�ֽڣ��ڶ���pָ���512�ֽ� */
		int* p = (int*)spb + i * 128;

		/* ��Ҫд�뵽SUPER_BLOCK_SECTOR_NUMBER + i������ȥ */
		pBuf = bufMgr->GetBlk(FileSystem::SUPER_BLOCK_SECTOR_NUMBER + i);

		/* ��SuperBlock�е�0 - 511�ֽ�д�뻺���� */
		Utility::copy<int>(p, (int*)pBuf->b_addr, 128);

		/* ���������е�����д�������� */
		bufMgr->Bwrite(pBuf);
	}

	/* ͬ���޸Ĺ����ڴ�Inode����Ӧ���Inode */
	InodeTable* k_InodeTable = Kernel::getInstance()->getInodeTable();
	k_InodeTable->UpdateInodeTable();

	/* ���ӳ�д�Ļ����д�������� */
	bufMgr->Bflush();
}

 /* ����ջʽ������ջ��Ϊ�գ���ֱ�ӷ��䣻��ջ��ָ��Ϊ�գ������ DiskINode�������ҵ��Ŀ��нڵ�ѹ��ջ�� */
Inode* FileSystem::IAlloc()
{
	SuperBlock* spb = Kernel::getInstance()->getSuperBlock();
	Buf* pBuf;
	Inode* pNode;

	BufferManager* bufMgr = Kernel::getInstance()->getBufMgr();
	Kernel* k = Kernel::getInstance();
	InodeTable* k_InodeTable = Kernel::getInstance()->getInodeTable();

	int ino;      /* ���䵽�Ŀ������Inode��� */

	/* SuperBlockֱ�ӹ���Ŀ���Inode�������ѿգ����뵽��������������Inode��*/
	if (spb->s_ninode <= 0)
	{
		/* ���Inode��Ŵ�0��ʼ���ⲻͬ��Unix V6�����Inode��1��ʼ��� */
		ino = -1;

		/* ���ζ������Inode���еĴ��̿飬�������п������Inode���������Inode������ */
		for (int i = 0; i < spb->s_isize; i++)
		{
			pBuf = bufMgr->Bread(FileSystem::INODE_ZONE_START_SECTOR + i);

			/* ��ȡ��������ַ */
			int* p = (int*)pBuf->b_addr;

			/* ���û�������ÿ�����Inode����i_mode != 0����ʾ�Ѿ���ռ�� */
			for (int j = 0; j < FileSystem::INODE_NUMBER_PER_SECTOR; j++)
			{
				ino++;

				int mode = *(p + j * sizeof(DiskInode) / sizeof(int));

				/* �����Inode�ѱ�ռ�ã����ܼ������Inode������ */
				if (mode != 0)
				{
					continue;
				}

				/*
				* ������inode��i_mode==0����ʱ������ȷ��
				* ��inode�ǿ��еģ���Ϊ�п������ڴ�inodeû��д��
				* ������,����Ҫ���������ڴ�inode���Ƿ�����Ӧ����
				*/
				if (k_InodeTable->IsLoaded(ino) == -1)
				{
					/* �����Inodeû�ж�Ӧ���ڴ濽��������������Inode������ */
					spb->s_inode[spb->s_ninode++] = ino;

					/* ��������������Ѿ�װ�����򲻼������� */
					if (spb->s_ninode >= 100)
					{
						break;
					}
						
				}
			}

			/* �����Ѷ��굱ǰ���̿飬�ͷ���Ӧ�Ļ��� */
			bufMgr->Brelse(pBuf);

			/* ��������������Ѿ�װ�����򲻼������� */
			if (spb->s_ninode >= 100)
			{
				break;
			}
				
		}

		/* ����ڴ�����û���������κο������Inode������NULL */
		if (spb->s_ninode <= 0)
		{
			return NULL;
		}
			
	}

	/*
	 * ���沿���Ѿ���֤������ϵͳ��û�п������Inode��
	 * �������Inode�������бض����¼�������Inode�ı�š�
	 */
	while (true)
	{
		/* ��������ջ������ȡ�������Inode��� */
		ino = spb->s_inode[--spb->s_ninode];

		/* ������Inode�����ڴ� */
		pNode = k_InodeTable->IGet(ino);

		/* δ�ܷ��䵽�ڴ�inode */
		if (pNode == NULL)
		{
			return NULL;
		}

		/* �����Inode����,���Inode�е����� */
		if (pNode->i_mode == 0)
		{
			pNode->Clean();
			/* ����SuperBlock���޸ı�־ */
			spb->s_fmod = 1;
			return pNode;
		}
		else	/* �����Inode�ѱ�ռ�� */
		{
			k_InodeTable->IPut(pNode);
			continue;	/* whileѭ�� */
		}
	}
	return NULL;
}

void FileSystem::IFree(int number)
{
	SuperBlock* spb = Kernel::getInstance()->getSuperBlock();
	/*
	 * ���������ֱ�ӹ���Ŀ������Inode����100����
	 * ͬ�����ͷŵ����Inodeɢ���ڴ���Inode���С�
	 */
	if (spb->s_ninode >= 100)
	{
		return;
	}
		
	spb->s_inode[spb->s_ninode++] = number;

	/* ����SuperBlock���޸ı�־ */
	spb->s_fmod = 1;
}

/* �����̿����ջʽ�������Ҳ��÷�����ʽ���������� */
Buf* FileSystem::Alloc()
{
	int blkno;     /* ���䵽�Ŀ��д��̿��� */
	SuperBlock* spb = Kernel::getInstance()->getSuperBlock();
	Buf* pBuf;
	BufferManager* bufMgr = Kernel::getInstance()->getBufMgr();
	Kernel* k = Kernel::getInstance();

	/* ��������ջ������ȡ���д��̿��� */
	blkno = spb->s_free[--spb->s_nfree];

	/*
	 * ����ȡ���̿���Ϊ�㣬���ʾ�ѷ��価���еĿ��д��̿顣
	 * ���߷��䵽�Ŀ��д��̿��Ų����������̿�������(��BadBlock()���)��
	 * ����ζ�ŷ�����д��̿����ʧ�ܡ�
	 */
	if (blkno == 0)
	{
		spb->s_nfree = 0;
		k->error = Kernel::NOSPACE;
		return NULL;
	}

	/*
	 * ջ�ѿգ��·��䵽���д��̿��м�¼����һ����д��̿�ı��,
	 * ����һ����д��̿�ı�Ŷ���SuperBlock�Ŀ��д��̿�������s_free[100]�С�
	 */
	if (spb->s_nfree <= 0)
	{
		/* ����ÿ��д��̿� */
		pBuf = bufMgr->Bread(blkno);

		/* �Ӹô��̿��0�ֽڿ�ʼ��¼����ռ��4(s_nfree)+400(s_free[100])���ֽ� */
		int* p = (int*)pBuf->b_addr;

		/* ���ȶ��������̿���s_nfree */
		spb->s_nfree = *p++;

		/* ��ȡ�����к���λ�õ����ݣ�д�뵽SuperBlock�����̿�������s_free[100]�� */
		Utility::copy<int>(p, spb->s_free, 100);

		/* ����ʹ����ϣ��ͷ��Ա㱻��������ʹ�� */
		bufMgr->Brelse(pBuf);
	}

	/* ��ͨ����³ɹ����䵽һ���д��̿�(�����Ǽ�¼����һ����д��̿�ı�ŵĿ��д���) */
	pBuf = bufMgr->GetBlk(blkno);	        /* Ϊ�ô��̿����뻺�� */
	bufMgr->ClrBuf(pBuf);	                /* ��ջ����е����� */
	spb->s_fmod = 1;	                    /* ����SuperBlock���޸ı�־ */

	return pBuf;
}

void FileSystem::Free(int blkno)
{
	SuperBlock* spb = Kernel::getInstance()->getSuperBlock();
	Buf* pBuf;
	BufferManager* bufMgr = Kernel::getInstance()->getBufMgr();

	/*
	 * ��������SuperBlock���޸ı�־���Է�ֹ���ͷ�
	 * ���̿�Free()ִ�й����У���SuperBlock�ڴ渱��
	 * ���޸Ľ�������һ�룬�͸��µ�����SuperBlockȥ
	 */
	spb->s_fmod = 1;

	/*
	 * �����ǰϵͳ���Ѿ�û�п����̿飬
	 * �����ͷŵ���ϵͳ�е�1������̿�
	 */
	if (spb->s_nfree <= 0)
	{
		spb->s_nfree = 1;
		spb->s_free[0] = 0;   /* ʹ��0��ǿ����̿���������־ */
	}

	/* SuperBlock��ֱ�ӹ�����д��̿�ŵ�ջ���� */
	if (spb->s_nfree >= 100)
	{
		/*
		 * ʹ�õ�ǰFree()������Ҫ�ͷŵĴ��̿飬���ǰһ��100������
		 * ���̿��������
		 */
		pBuf = bufMgr->GetBlk(blkno);

		/* �Ӹô��̿��0�ֽڿ�ʼ��¼����ռ��4(s_nfree)+400(s_free[100])���ֽ� */
		int* p = (int*)pBuf->b_addr;

		/* ����д������̿��������˵�һ��Ϊ99�飬����ÿ�鶼��100�� */
		*p++ = spb->s_nfree;
		/* ��SuperBlock�Ŀ����̿�������s_free[100]д�뻺���к���λ�� */
		Utility::copy<int>(spb->s_free, p, 100);

		spb->s_nfree = 0;
		/* ����ſ����̿�������ġ���ǰ�ͷ��̿顱д����̣���ʵ���˿����̿��¼�����̿�ŵ�Ŀ�� */
		bufMgr->Bwrite(pBuf);
	}
	spb->s_free[spb->s_nfree++] = blkno;	/* SuperBlock�м�¼�µ�ǰ�ͷ��̿�� */
	spb->s_fmod = 1;
}