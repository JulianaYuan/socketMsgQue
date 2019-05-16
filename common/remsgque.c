#include "remsgque.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int queId = 1;

int InitQueue(pMsgNodeQue queue)    //
{
	int ret = 0;
	queue->Front = queue->Rear = (pMsgNode)malloc(sizeof(MsgNode));    //
    if (queue->Front == NULL) {        //
		ret = -1;
		printf("create queue,memory alloc fail\n");
		return ret;
    }
    queue->Rear->next = NULL;    //
	queue->msqid = queId++;
	printf("creat queue success\n");
	ret = queue->msqid;
	return ret;
}
bool IsEmptyQueue(pMsgNodeQue queue)    //
{
	bool bret = false;
	if(queue->Front == queue->Rear){
		bret = true;
		printf("queue is empty\n");
	}
	else{
		bret = false;
		printf("queue is not empty\n");
	}
	return bret;
}
int InsertQueue(pMsgNodeQue queue, MsgData data)    //
{
	int ret = 0;

	pMsgNode p = (pMsgNode)malloc(sizeof(MsgNode));    // 
	if(p == NULL){  
		ret = -1;
		printf("memory alloc fail,data = %lx\n",(long unsigned int)(&data));
		return ret;
	}

	p->data.msg = data.msg;
    printf("p->data.msg=%lx,data.msg=%lx\n",(long int)p->data.msg,(long int)data.msg);
	printf("p->data.msg=%s,data.msg=%s\n",(char *)(p->data.msg),(char *)(data.msg));
	p->data.msgTyp = data.msgTyp;
	p->next = NULL;
	queue->Rear->next = p;
	queue->Rear = p;
	printf("inser data success\n");
	
	return ret;

}

int DeleteQueue(pMsgNodeQue queue,MsgData* data)    //
{
	int ret = 0;

	if(IsEmptyQueue(queue)){
		ret = -1;
		printf("DeleteQueue que is empty");
		return ret;
	}
	pMsgNode p = queue->Front->next;
	void *pMsg = &(data->msg);
	if(pMsg!=NULL)
	{
		memcpy(pMsg,p->data.msg,(strlen((char*)(p->data.msg))+1));
		ret = strlen((char*)(p->data.msg))+1;
        printf("DeleteQueue data->msg = %s ret =%d\n",(char*)pMsg,ret);
	}
	data->msgTyp = p->data.msgTyp;
	queue->Front->next = p->next;
	if(queue->Rear == p){
		queue->Rear = queue->Front;
	}
	free(p->data.msg);
	p->data.msg = NULL;
	free(p);
	p=NULL;
	printf("DeleteQueue delete data success");
	return ret;
}

int DeleteQueByNode(pMsgNodeQue queue,pMsgNode pNode,MsgData* data)    //
{
	int ret = 0;

	if(IsEmptyQueue(queue)){
		ret = -1;
		printf("DeleteQueByNode que is empty");
		return ret;
	}
	pMsgNode p = queue->Front->next;
	pMsgNode ppre = queue->Front->next;
	void *pMsg = &(data->msg);
	while(p!=queue->Rear->next)
	{	
		if(p==pNode)
		{
			if(pMsg!=NULL)
			{
				memcpy(pMsg,p->data.msg,(strlen((char*)(p->data.msg))+1));
				ret = strlen((char*)(p->data.msg))+1;
                printf("DeleteQueByNode data->msg = %s ret =%d\n",(char*)pMsg,ret);
			}
			data->msgTyp = p->data.msgTyp;
			ppre->next = p->next;
			if(ppre == p){
				queue->Rear = queue->Front;
			}
			free(p->data.msg);
			p->data.msg = NULL;
			free(p);
			p=NULL;
			printf("DeleteQueByNode delete data success\n");
			break;
		}
		ppre = p;
		p=p->next;
	}
	return ret;
}


void DestroyQueue(pMsgNodeQue queue)    //
{
	//
	while (queue->Front != NULL) {
		queue->Rear = queue->Front->next;
		free(queue->Front->data.msg);
		queue->Front->data.msg = NULL;
		free(queue->Front);
		queue->Front = queue->Rear;
	}
	queue->msqid = -1;
	printf("destroy queue ok\n");

}
int TraverseQueue(pMsgNodeQue queue)    //
{
	int ret = 0;
    if (IsEmptyQueue(queue)) {
        ret = -1;
		printf("queue is empty\n");
		return ret;
    }        
    pMsgNode p = queue->Front->next;    //
    printf("traveral result\n");
    while (p != NULL) {
        printf("msgTyp = %ld, msg =%s\n", p->data.msgTyp,(char*)(p->data.msg));
        p = p->next;
    }
	return ret;
}
void ClearQueue(pMsgNodeQue queue)    //  
{
    pMsgNode P = queue->Front->next;    //
    pMsgNode Q = NULL;        //
    queue->Rear = queue->Front;        //
    queue->Front->next = NULL;
    //
    while (P != NULL) {
        Q = P;
        P = P->next;
		free(Q->data.msg);
		Q->data.msg = NULL;
        free(Q);
    }
    printf("clear queue ok\n");

}
int LengthQueue(pMsgNodeQue queue)    //
{
	int len = 0;
	pMsgNode P = queue->Front->next;

	while (P != NULL) {
		len ++;
        P = P->next;
    }
	return len;
}



