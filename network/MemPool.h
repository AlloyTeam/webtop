#ifndef _MEMPOOL_H
#define _MEMPOOL_H

//头文件
#include <stdio.h>
#include <windows.h>
#include <malloc.h>
#include <process.h>

#define DEBUG 1

//宏定义
#define BLOCK 1024
#define MAXNUM 8

//数据结构
struct ListNode
{
	void * ptr;
	struct ListNode * next;
};
ListNode lstMemRoot[MAXNUM];//未分配节点链表
ListNode lstNodeBuff;		//已分配节点链表
CRITICAL_SECTION cs;		//临界段

//初始化内存池
void InitializeMemPool()
{
	InitializeCriticalSection(&cs);
	if(DEBUG) {
		//Log("初始化临界段");
	}
	
	for(int i=0;i<MAXNUM;i++)
	{
		lstMemRoot[i].ptr = NULL;
		lstMemRoot[i].next = NULL;
	}

	lstNodeBuff.ptr = NULL;
	lstNodeBuff.next = NULL;
}

//分配内存
void * MallocMem(int piMemLen)
{
	ListNode * tmp = NULL;
	void * ptr = NULL;

	int iChoose = 0; //选择链表

	if(piMemLen <= 0){
		return 0;
	}
	else if (piMemLen > BLOCK * MAXNUM) {
		ptr = malloc(piMemLen+2); //直接分配，多分配2个字节
		*(char*)ptr = MAXNUM; //

		return (void*)((char*)ptr+2);
	}

	iChoose = (piMemLen-1)/BLOCK;

	if(lstMemRoot[iChoose].next == NULL)
	{
		//直接分配内存
		ptr = malloc(piMemLen+2);	//多分配2个字节
		*((char*)ptr) = iChoose;

		return (void*)((char*)ptr+2);
	}
	else
	{
		//进入临界区域
		EnterCriticalSection(&cs);
		if(DEBUG) {
			//Log("进入临界段");
		}

		tmp = lstMemRoot[iChoose].next;

		//删除此节点
		lstMemRoot[iChoose].next = tmp->next;

		//将此节点插入到节点链表
		tmp->next = lstNodeBuff.next;
		lstNodeBuff.next = tmp;

		if(DEBUG) {
			//Log("退出临界段");
		}
		LeaveCriticalSection(&cs);

		return tmp->ptr;
	}

	return 0;
}

void FreeMem(void *pMemPtr)
{
	ListNode * tmp = NULL;

	int iChoose = 0; //选择链表
	iChoose = *((char*)pMemPtr-2); //

	//
	if(iChoose > MAXNUM-1) {
		free((char*)pMemPtr-2);
		return;
	}

	//进入临界区域
	EnterCriticalSection(&cs);
	if(DEBUG) {
		//Log("进入临界段");
	}

	//如果已分配节点链表为空
	if(lstNodeBuff.next == NULL)
	{
		tmp = (ListNode*)malloc(sizeof(ListNode));
		tmp -> ptr = NULL;
		tmp -> next = NULL;
	}
	else
	{
		tmp = lstNodeBuff.next;
		tmp -> ptr = NULL;
		tmp -> next = lstNodeBuff.next -> next;

		lstNodeBuff.next = tmp -> next;
	}

	//保存回收的内存地址
	tmp -> next = lstMemRoot[iChoose].next;
	tmp -> ptr  = pMemPtr;
	lstMemRoot[iChoose].next = tmp;

	if(DEBUG) {
		//Log("退出临界段");
	}
	LeaveCriticalSection(&cs);
}

void CloseMemPool()
{
	DeleteCriticalSection(&cs);

	if(DEBUG) {
		//Log("销毁临界段");
	}

	ListNode * pos = NULL;

	//释放链表
	for(int i=0;i<MAXNUM;i++) {
		pos = lstMemRoot[i].next;
		while(pos != NULL) {
			if(pos->ptr != NULL) {
				free(pos->ptr);	//释放内存
			}
			pos = pos->next;
		}
	}

	//释放链表
	pos = lstNodeBuff.next;
	while(pos != NULL) {
		if(pos->ptr != NULL) {
			free(pos->ptr);	//释放指针
		}
		pos = pos->next;
	}

	//恢复全局变量初始值
	for(int i=0;i<MAXNUM;i++)
	{
		lstMemRoot[i].ptr = NULL;
		lstMemRoot[i].next = NULL;
	}
	lstNodeBuff.ptr = NULL;
	lstNodeBuff.next = NULL;
}
#endif

