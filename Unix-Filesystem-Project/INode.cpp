#include "INode.h"
#include "FileSystem.h"
#include "Utility.h"
#include "Kernel.h"

/* �ڴ�� i�ڵ�*/
Inode::Inode()
{
	/* ���Inode�����е����� */
	// this->Clean(); 
	/* ȥ��this->Clean();�����ɣ�
	 * Inode::Clean()�ض�����IAlloc()������·���DiskInode��ԭ�����ݣ�
	 * �����ļ���Ϣ��Clean()�����в�Ӧ�����i_dev, i_number, i_flag, i_count,
	 * ���������ڴ�Inode����DiskInode�����ľ��ļ���Ϣ����Inode�๹�캯����Ҫ
	 * �����ʼ��Ϊ��Чֵ��
	 */

	 /* ��Inode����ĳ�Ա������ʼ��Ϊ��Чֵ */
	this->i_flag = 0;
	this->i_mode = 0;
	this->i_count = 0;
	this->i_nlink = 0;
	this->i_number = -1;
	this->i_size = 0;
	for (int i = 0; i < 10; i++)
	{
		this->i_addr[i] = 0;
	}
		
}

void Inode::ReadI()
{
	int lbn;	/* �ļ��߼���� */
	int bn;		/* lbn��Ӧ�������̿�� */
	int offset;	/* ��ǰ�ַ�������ʼ����λ�� */
	int nbytes;	/* �������û�Ŀ�����ֽ����� */

	Buf* pBuf;
	Kernel* k = Kernel::getInstance();
	BufferManager* bufMgr = Kernel::getInstance()->getBufMgr();

	if (k->k_IOParam.m_Count == 0)
	{
		/* ��Ҫ���ֽ���Ϊ�㣬�򷵻� */
		return;
	}

	this->i_flag |= Inode::IACC;

	/* һ��һ���ַ���ض�������ȫ�����ݣ�ֱ�������ļ�β */
	while (k->k_IOParam.m_Count != 0)
	{
		lbn = bn = k->k_IOParam.m_Offset / Inode::BLOCK_SIZE;
		offset = k->k_IOParam.m_Offset % Inode::BLOCK_SIZE;
		/* ���͵��û������ֽ�������ȡ�������ʣ���ֽ����뵱ǰ�ַ�������Ч�ֽ�����Сֵ */
		nbytes = min(Inode::BLOCK_SIZE - offset, k->k_IOParam.m_Count);

		int remain = this->i_size - k->k_IOParam.m_Offset;
		/* ����Ѷ��������ļ���β */
		if (remain <= 0)
		{
			return;
		}
		/* ���͵��ֽ�������ȡ����ʣ���ļ��ĳ��� */
		nbytes = min(nbytes, remain);

		/* ���߼����lbnת���������̿��bn */
		if ((bn = this->Bmap(lbn)) == 0)
		{
			return;
		}

		pBuf = bufMgr->Bread(bn);

		/* ������������ʼ��λ�� */
		char* start = pBuf->b_addr + offset;          
		Utility::copy<char>(start, k->k_IOParam.m_Base, nbytes);

		/* �ô����ֽ���nbytes���¶�дλ�� */
		k->k_IOParam.m_Base += nbytes;
		k->k_IOParam.m_Offset += nbytes;
		k->k_IOParam.m_Count -= nbytes;

		bufMgr->Brelse(pBuf);	/* ʹ���껺�棬�ͷŸ���Դ */
	}
}

void Inode::WriteI()
{
	int lbn;	/* �ļ��߼���� */
	int bn;		/* lbn��Ӧ�������̿�� */
	int offset;	/* ��ǰ�ַ�������ʼ����λ�� */
	int nbytes;	/* �����ֽ����� */
	Buf* pBuf;
	Kernel* k = Kernel::getInstance();
	BufferManager* bufMgr = Kernel::getInstance()->getBufMgr();

	/* ����Inode�����ʱ�־λ */
	this->i_flag |= (Inode::IACC | Inode::IUPD);

	if (k->k_IOParam.m_Count == 0)
		return;

	while (k->k_IOParam.m_Count != 0)
	{
		lbn = k->k_IOParam.m_Offset / Inode::BLOCK_SIZE;
		offset = k->k_IOParam.m_Offset % Inode::BLOCK_SIZE;
		nbytes = min(Inode::BLOCK_SIZE - offset, k->k_IOParam.m_Count);

		/* ���߼����lbnת���������̿��bn */
		if ((bn = this->Bmap(lbn)) == 0)
		{
			return;
		}

		if (Inode::BLOCK_SIZE == nbytes)
		{
			/* ���д������������һ���ַ��飬��Ϊ����仺�� */
			pBuf = bufMgr->GetBlk(bn);
		}
			
		else
		{
			/* д�����ݲ���һ���ַ��飬�ȶ���д���������ַ����Ա�������Ҫ��д�����ݣ� */
			pBuf = bufMgr->Bread(bn);
		}

		/* ���������ݵ���ʼдλ�� */
		char* start = pBuf->b_addr + offset;     

		/* д����: ���û�Ŀ�����������ݵ������� */
		Utility::copy<char>(k->k_IOParam.m_Base, start, nbytes);

		/* �ô����ֽ���nbytes���¶�дλ�� */
		k->k_IOParam.m_Base += nbytes;
		k->k_IOParam.m_Offset += nbytes;
		k->k_IOParam.m_Count -= nbytes;

		if ((k->k_IOParam.m_Offset % Inode::BLOCK_SIZE) == 0)	/* ���д��һ���ַ��� */
		{
			/* ���첽��ʽ���ַ���д����� */
			bufMgr->Bawrite(pBuf);
		}
		else /* ���������δд�� */
		{
			/* ��������Ϊ�ӳ�д */
			bufMgr->Bdwrite(pBuf);
		}

		/* �ļ��������� */
		if ((this->i_size < k->k_IOParam.m_Offset))
		{
			this->i_size = k->k_IOParam.m_Offset;
		}
	}
}

int Inode::Bmap(int lbn)
{
	Buf* pFirstBuf;
	Buf* pSecondBuf;
	int phyBlkno;	/* ת����������̿�� */
	int* iTable;	/* ���ڷ��������̿���һ�μ�ӡ����μ�������� */
	int index;
	Kernel* k = Kernel::getInstance();
	BufferManager* bufMgr = k->getBufMgr();
	FileSystem* fileSys = Kernel::getInstance()->getFileSys();

	/*
	 * Unix V6++���ļ������ṹ��(С�͡����ͺ;����ļ�)
	 * (1) i_addr[0] - i_addr[5]Ϊֱ���������ļ����ȷ�Χ��0 - 6���̿飻
	 *
	 * (2) i_addr[6] - i_addr[7]���һ�μ�����������ڴ��̿�ţ�ÿ���̿�
	 * �ϴ��128���ļ������̿�ţ������ļ����ȷ�Χ��7 - (128 * 2 + 6)���̿飻
	 *
	 * (3) i_addr[8] - i_addr[9]��Ŷ��μ�����������ڴ��̿�ţ�ÿ�����μ��
	 * �������¼128��һ�μ�����������ڴ��̿�ţ������ļ����ȷ�Χ��
	 * (128 * 2 + 6 ) < size <= (128 * 128 * 2 + 128 * 2 + 6)
	 */

	if (lbn >= Inode::HUGE_FILE_BLOCK)
	{
		return 0;
	}

	if (lbn < 6)		/* С���ļ����ӻ���������i_addr[0-5]�л�������̿�ż��� */
	{
		phyBlkno = this->i_addr[lbn];

		/*
		 * ������߼���Ż�û����Ӧ�������̿����֮��Ӧ�������һ������顣
		 * ��ͨ�������ڶ��ļ���д�룬��д��λ�ó����ļ���С�����Ե�ǰ
		 * �ļ���������д�룬����Ҫ�������Ĵ��̿飬��Ϊ֮�����߼����
		 * �������̿��֮���ӳ�䡣
		 */
		if (phyBlkno == 0 && (pFirstBuf = fileSys->Alloc()) != NULL)
		{
			bufMgr->Bdwrite(pFirstBuf);
			phyBlkno = pFirstBuf->b_blkno;
			/* ���߼����lbnӳ�䵽�����̿��phyBlkno */
			this->i_addr[lbn] = phyBlkno;
			this->i_flag |= Inode::IUPD;
		}

		return phyBlkno;
	}
	else	/* lbn >= 6 ���͡������ļ� */
	{
		if (lbn < Inode::LARGE_FILE_BLOCK)	/* �����ļ����ӻ���������i_addr[6-7]�л�������� */
		{
			index = (lbn - Inode::SMALL_FILE_BLOCK) / Inode::ADDRESS_PER_INDEX_BLOCK + 6;
		}
			
		else    /* �����ļ����ӻ���������i_addr[8-9]�л�ö��μ�������� */
		{
			index = (lbn - Inode::LARGE_FILE_BLOCK) / (Inode::ADDRESS_PER_INDEX_BLOCK * Inode::ADDRESS_PER_INDEX_BLOCK) + 8;
		}

		phyBlkno = this->i_addr[index];
		/* ���μ������������ */
		if (phyBlkno == 0)
		{
			this->i_flag |= Inode::IUPD;
			/* ����һ�����̿��ż�������� */
			if ((pFirstBuf = fileSys->Alloc()) == NULL)
				return 0;	/* ����ʧ�� */
			/* i_addr[index]�м�¼���������������̿�� */
			this->i_addr[index] = pFirstBuf->b_blkno;
		}
		else
		{
			/* �����洢�����������ַ��� */
			pFirstBuf = bufMgr->Bread(phyBlkno);
		}
		/* ��ȡ��������ַ */
		iTable = (int*)pFirstBuf->b_addr;

		if (index >= 8)	/* �����ļ� */
		{
			/*
			 * ���ھ����ļ��������pFirstBuf���Ƕ��μ��������
			 * ��������߼���ţ����ɶ��μ���������ҵ�һ�μ��������
			 */
			index = ((lbn - Inode::LARGE_FILE_BLOCK) / Inode::ADDRESS_PER_INDEX_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;

			/* iTableָ�򻺴��еĶ��μ������������Ϊ�㣬������һ�μ�������� */
			phyBlkno = iTable[index];

			/* һ�μ������������ */
			if (phyBlkno == 0)
			{
				if ((pSecondBuf = fileSys->Alloc()) == NULL)
				{
					/* ����һ�μ����������̿�ʧ�ܣ��ͷŻ����еĶ��μ��������Ȼ�󷵻� */
					bufMgr->Brelse(pFirstBuf);
					return 0;
				}
				/* ���·����һ�μ����������̿�ţ�������μ����������Ӧ�� */
				iTable[index] = pSecondBuf->b_blkno;
				/* �����ĺ�Ķ��μ���������ӳ�д��ʽ��������� */
				bufMgr->Bdwrite(pFirstBuf);
			}
			else
			{
				/* �ͷŶ��μ��������ռ�õĻ��棬������һ�μ�������� */
				bufMgr->Brelse(pFirstBuf);
				pSecondBuf = bufMgr->Bread(phyBlkno);
			}

			pFirstBuf = pSecondBuf;
			/* ��iTableָ��һ�μ�������� */
			iTable = (int*)pSecondBuf->b_addr;
		}

		/* �����߼����lbn����λ��һ�μ���������еı������index */

		if (lbn < Inode::LARGE_FILE_BLOCK)
		{
			index = (lbn - Inode::SMALL_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;
		}
		else
		{
			index = (lbn - Inode::LARGE_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;
		}

		if ((phyBlkno = iTable[index]) == 0 && (pSecondBuf = fileSys->Alloc()) != NULL)
		{
			/* �����䵽���ļ������̿�ŵǼ���һ�μ���������� */
			phyBlkno = pSecondBuf->b_blkno;
			iTable[index] = phyBlkno;
			/* �������̿顢���ĺ��һ�μ�����������ӳ�д��ʽ��������� */
			bufMgr->Bdwrite(pSecondBuf);
			bufMgr->Bdwrite(pFirstBuf);
		}
		else
		{
			/* �ͷ�һ�μ��������ռ�û��� */
			bufMgr->Brelse(pFirstBuf);
		}
		return phyBlkno;
	}
}

void Inode::IUpdate()
{
	Buf* pBuf;
	DiskInode dInode;
	FileSystem* filesys = Kernel::getInstance()->getFileSys();
	BufferManager* bufMgr = Kernel::getInstance()->getBufMgr();

	/* ��IUPD��IACC��־֮һ�����ã�����Ҫ������ӦDiskInode
	* Ŀ¼����������������;����Ŀ¼�ļ���IACC��IUPD��־ */
	if ((this->i_flag & (Inode::IUPD | Inode::IACC)) != 0)
	{
		pBuf = bufMgr->Bread(FileSystem::INODE_ZONE_START_SECTOR + this->i_number / FileSystem::INODE_NUMBER_PER_SECTOR);

		/* ���ڴ�Inode�����е���Ϣ���Ƶ�dInode�У�Ȼ��dInode���ǻ����оɵ����Inode */
		dInode.d_mode = this->i_mode;
		dInode.d_nlink = this->i_nlink;
		dInode.d_size = this->i_size;
		for (int i = 0; i < 10; i++)
		{
			dInode.d_addr[i] = this->i_addr[i];
		}
		/* ��pָ�򻺴����о����Inode��ƫ��λ�� */
		char* p = pBuf->b_addr + (this->i_number % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);    //unsigned char*
		DiskInode* pNode = &dInode;

		/* ��dInode�е������ݸ��ǻ����еľ����Inode */
		Utility::copy<int>((int*)pNode, (int*)p, sizeof(DiskInode) / sizeof(int));

		/* ������д�������̣��ﵽ���¾����Inode��Ŀ�� */
		bufMgr->Bwrite(pBuf);
	}
}

void Inode::ITrunc()
{
	/* ���ɴ��̸��ٻ����ȡ���һ�μ�ӡ����μ��������Ĵ��̿� */
	BufferManager* bufMgr = Kernel::getInstance()->getBufMgr();
	/* ��ȡg_FileSystem��������ã�ִ���ͷŴ��̿�Ĳ��� */
	FileSystem* fileSys = Kernel::getInstance()->getFileSys();

	/* ����FILO��ʽ�ͷţ��Ծ���ʹ��SuperBlock�м�¼�Ŀ����̿������ */
	for (int i = 9; i >= 0; i--)		/* ��i_addr[9]��i_addr[0] */
	{
		/* ���i_addr[]�е�i��������� */
		if (this->i_addr[i] != 0)
		{
			/* �����i_addr[]�е�һ�μ�ӡ����μ�������� */
			if (i >= 6 && i <= 9)
			{
				/* �������������뻺�� */
				Buf* pFirstBuf = bufMgr->Bread(this->i_addr[i]);
				/* ��ȡ��������ַ */
				int* pFirst = (int*)pFirstBuf->b_addr;

				/* ÿ�ż���������¼ 512/sizeof(int) = 128�����̿�ţ�������ȫ��128�����̿� */
				for (int j = 128 - 1; j >= 0; j--)
				{
					if (pFirst[j] != 0)	/* �������������� */
					{
						/*
						 * ��������μ��������i_addr[8]��i_addr[9]�
						 * ��ô���ַ����¼����128��һ�μ���������ŵĴ��̿��
						 */
						if (i >= 8 && i <= 9)
						{
							Buf* pSecondBuf = bufMgr->Bread(pFirst[j]);
							int* pSecond = (int*)pSecondBuf->b_addr;
							for (int k = 128 - 1; k >= 0; k--)
							{
								if (pSecond[k] != 0)
								{
									/* �ͷ�ָ���Ĵ��̿� */
									fileSys->Free(pSecond[k]);
								}
									
							}
							/* ����ʹ����ϣ��ͷ��Ա㱻��������ʹ�� */
							bufMgr->Brelse(pSecondBuf);
						}
						fileSys->Free(pFirst[j]);
					}
				}
				bufMgr->Brelse(pFirstBuf);
			}
			/* �ͷ���������ռ�õĴ��̿� */
			fileSys->Free(this->i_addr[i]);
			/* 0��ʾ����������� */
			this->i_addr[i] = 0;
		}
	}
	/* �̿��ͷ���ϣ��ļ���С���� */
	this->i_size = 0;
	/* ����IUPD��־λ����ʾ���ڴ�Inode��Ҫͬ������Ӧ���Inode */
	this->i_flag |= Inode::IUPD;
	/* ����ļ���־ ��ԭ����RWXRWXRWX����*/
	this->i_mode &= ~(Inode::ILARG & Inode::IRWXU & Inode::IRWXG & Inode::IRWXO);
	this->i_nlink = 1;
}

void Inode::Clean()
{
	/*
	 * Inode::Clean()�ض�����IAlloc()������·���DiskInode��ԭ�����ݣ�
	 * �����ļ���Ϣ��Clean()�����в�Ӧ�����i_dev, i_number, i_flag, i_count,
	 * ���������ڴ�Inode����DiskInode�����ľ��ļ���Ϣ����Inode�๹�캯����Ҫ
	 * �����ʼ��Ϊ��Чֵ��
	 */
	this->i_mode = 0;
	this->i_nlink = 0;
	this->i_size = 0;
	for (int i = 0; i < 10; i++)
	{
		this->i_addr[i] = 0;
	}
		
}

void Inode::ICopy(Buf* bp, int inumber)
{
	DiskInode dInode;
	DiskInode* pNode = &dInode;

	/* ��pָ�򻺴����б��Ϊinumber���Inode��ƫ��λ�� */
	char* p = bp->b_addr + (inumber % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);  //unsigned char*
	/* �����������Inode���ݿ�������ʱ����dInode�У���4�ֽڿ��� */
	Utility::copy<int>((int*)p, (int*)pNode, sizeof(DiskInode) / sizeof(int));

	/* �����Inode����dInode����Ϣ���Ƶ��ڴ�Inode�� */
	this->i_mode = dInode.d_mode;
	this->i_nlink = dInode.d_nlink;
	this->i_size = dInode.d_size;
	for (int i = 0; i < 10; i++)
	{
		this->i_addr[i] = dInode.d_addr[i];
	}
		
}