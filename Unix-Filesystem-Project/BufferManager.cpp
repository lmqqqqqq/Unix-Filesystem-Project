#include "Buf.h"
#include "BufferManager.h"
#include "Kernel.h"

const char* Kernel::DISK_IMG;  //����Ϊ��static����Ҫ������һ��

/* ����BufferManager::Initialize() */
BufferManager::BufferManager()
{
	int i;
	Buf* bp;

	this->bFreeList.av_forw = this->bFreeList.av_back = &(this->bFreeList); /* ��ʼ�����ɶ��ж�ͷ */
	this->bDevtab.b_forw = this->bDevtab.b_back = &(this->bDevtab);       /* ��ʼ���豸���ж�ͷ */

	for (i = 0; i < NBUF; i++)
	{
		bp = &(this->m_Buf[i]);
		bp->b_addr = this->Buffer[i];

		
		/* ��ʼ���豸���� */  //NODEV���и�Ϊ�豸����
		bp->b_back = &(this->bDevtab);
		bp->b_forw = this->bDevtab.b_forw;
		this->bDevtab.b_forw->b_back = bp;
		this->bDevtab.b_forw = bp;

		/* ��ʼ�����ɶ��� */
		bp->b_flags = Buf::B_BUSY;
		Brelse(bp);
	}
	return;
}

BufferManager::~BufferManager()
{
	Bflush();
}

Buf* BufferManager::GetBlk(int blkno)
{
	Buf* bp;
	Buf* dp = &this->bDevtab;
loop:
	/* �������豸�����������Ƿ�����Ӧ�Ļ��� */
	for (bp = dp->b_forw; bp != dp; bp = bp->b_forw)
	{
		/* ����Ҫ�ҵĻ��棬����� */
		if (bp->b_blkno != blkno)
			continue;

		if (bp->b_flags & Buf::B_BUSY)
		{
			bp->b_flags |= Buf::B_WANTED;
			goto loop;
		}
		/* �����ɶ����г�ȡ���� */
		this->NotAvail(bp);
		return bp;
	}

	/* �豸����û�ҵ��������ɶ���ȡ */
	/* ������ɶ���Ϊ�� */
	if (this->bFreeList.av_forw == &this->bFreeList)
	{
		this->bFreeList.b_flags |= Buf::B_WANTED;
		goto loop;
	}

	/* ȡ���ɶ��е�һ�����п� */

	bp = this->bFreeList.av_forw;
	this->NotAvail(bp);

	/* ������ַ������ӳ�д�������첽д�������� */
	if (bp->b_flags & Buf::B_DELWRI)
	{
		bp->b_flags |= Buf::B_ASYNC;
		this->Bwrite(bp);
		goto loop;
	}

	/* ע��: �����������������λ��ֻ����B_BUSY */
	bp->b_flags = Buf::B_BUSY;
	bp->b_blkno = blkno;
	return bp;
}

void BufferManager::Brelse(Buf* bp)
{
	/* ע�����²�����û�����B_DELWRI��B_WRITE��B_READ��B_DONE��־
	 * B_DELWRI��ʾ��Ȼ���ÿ��ƿ��ͷŵ����ɶ������棬�����п��ܻ�û��Щ�������ϡ�
	 * B_DONE����ָ�û����������ȷ�ط�ӳ�˴洢�ڻ�Ӧ�洢�ڴ����ϵ���Ϣ
	 */
	bp->b_flags &= ~(Buf::B_WANTED | Buf::B_BUSY | Buf::B_ASYNC);
	/* �嵽���ɶ��ж�β */
	(this->bFreeList.av_back)->av_forw = bp;
	bp->av_back = this->bFreeList.av_back;
	bp->av_forw = &(this->bFreeList);
	this->bFreeList.av_back = bp;
	return;
}

Buf* BufferManager::Bread(int blkno)
{
	Buf* bp;
	bp = this->GetBlk(blkno);
	/* ������豸�������ҵ����軺�棬��B_DONE�����ã��Ͳ������I/O���� */
	if (bp->b_flags & Buf::B_DONE)
	{
		return bp;
	}
	/* û���ҵ���Ӧ���棬����I/O������� */
	bp->b_flags |= Buf::B_READ;
	bp->b_wcount = BufferManager::BUFFER_SIZE;
	fstream f(Kernel::DISK_IMG, ios::in | ios::binary);
	f.seekg(blkno * BufferManager::BUFFER_SIZE);
	f.read(bp->b_addr, bp->b_wcount);
	f.close();
	return bp;
}

void writing(writeArg* arg)
{
	std::fstream f(Kernel::DISK_IMG, ios::in | ios::out | ios::binary);
	f.seekp(arg->bp->b_blkno * BufferManager::BUFFER_SIZE);
	f.write((char*)arg->bp->b_addr, arg->bp->b_wcount);
	f.close();
	arg->b->Brelse(arg->bp);
}

void BufferManager::Bwrite(Buf* bp)
{
	bp->b_flags &= ~(Buf::B_READ | Buf::B_DONE | Buf::B_ERROR | Buf::B_DELWRI);
	bp->b_wcount = BufferManager::BUFFER_SIZE;
	writeArg* arg = new writeArg(this, bp);
	writing(arg);
	delete arg;
	return;
}

void BufferManager::Bdwrite(Buf* bp)
{
	/* ����B_DONE������������ʹ�øô��̿����� */
	bp->b_flags |= (Buf::B_DELWRI | Buf::B_DONE);
	this->Brelse(bp);
	return;
}

void BufferManager::Bawrite(Buf* bp)
{
	/* ���Ϊ�첽д */
	bp->b_flags |= Buf::B_ASYNC;
	this->Bwrite(bp);
	return;
}

void BufferManager::ClrBuf(Buf* bp)
{
	int* pInt = (int*)bp->b_addr;

	/* ������������������ */
	for (int i = 0; i < BufferManager::BUFFER_SIZE / sizeof(int); i++)
	{
		pInt[i] = 0;
	}
	return;
}

void BufferManager::Bflush()
{
	Buf* bp;
	/* ע�⣺����֮����Ҫ��������һ����֮�����¿�ʼ������
	 * ��Ϊ��bwite()���뵽����������ʱ�п��жϵĲ���������
	 * �ȵ�bwriteִ����ɺ�CPU�Ѵ��ڿ��ж�״̬�����Ժ�
	 * �п��������ڼ���������жϣ�ʹ��bfreelist���г��ֱ仯��
	 * �����������������������������¿�ʼ������ô�ܿ�����
	 * ����bfreelist���е�ʱ����ִ���
	 */
loop:
	for (bp = this->bFreeList.av_forw; bp != &(this->bFreeList); bp = bp->av_forw)
	{
		/* �ҳ����ɶ����������ӳ�д�Ŀ� */
		if (bp->b_flags & Buf::B_DELWRI)
		{
			this->NotAvail(bp);
			this->Bwrite(bp);
			goto loop;
		}
	}
	return;
}

void BufferManager::NotAvail(Buf* bp)
{

	/* �����ɶ�����ȡ�� */
	bp->av_back->av_forw = bp->av_forw;
	bp->av_forw->av_back = bp->av_back;
	/* ����B_BUSY��־ */
	bp->b_flags |= Buf::B_BUSY;
	return;
}