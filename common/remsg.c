#include "remsg.h"
#include "remsgque.h"
#include <stdio.h>
#include "msgsocket.h"
#include <string.h>
#include "pthread.h"
#include "../include/unp.h"


static int setMsgqueAttr(int __msqid, struct msqid_ds *__buf)
{
	int ret = -1;
	__msqid = __msqid;
	__buf = __buf;
	printf("setMsgqueAttr\n");
	return ret;
}

static int getMsgqueHeader(int __msqid, struct msqid_ds *__buf)
{
	int ret = -1;
	__msqid = __msqid;
	__buf = __buf;
	printf("getMsgqueHeader\n");
	return ret;
}

int msgctl (int __msqid, int __cmd, struct msqid_ds *__buf)
{
	int ret = -1;

	if( IPC_STAT == __cmd){
        ret = getMsgqueHeader(__msqid,__buf);
        if(-1 != ret ){
			printf("get message queue header success\n");
		}
		else{
			printf("get message queue header fail\n");
		}
	}
	else if( IPC_SET == __cmd){
        ret = setMsgqueAttr(__msqid,__buf);
        if(-1 != ret ){
			printf("set message queue attr success\n");
		}
		else{
			printf("set message queue attr fail\n");
		}
	}
	else if( IPC_RMID == __cmd){
		printf("msgctl remove que\n");
		destroyQueue(__msqid);
	}
	else{
		printf("msgctl else\n");
	}

	return ret;
}
int msgget (key_t __key, int __msgflg)
{
	int ret = -1;
	static int key = __key;

	printf("so,msgget __msgflg =%x\n",__msgflg);
	if( IPC_PRIVATE == __key ){
		printf("creat new message queue\n");
		ret = createQueue(__key);
		printf("return queue id\n");
	}
	else{
		if( 0 == __msgflg){
			printf("get queue id\n");
			ret = getQueidByKey(__key);
			printf("return queue id\n");
		}
		else if(__msgflg&IPC_CREAT){
			if(false == keyExistInQueList(__key)){
				printf("creat new message queue\n");
				ret = createQueue(__key);
				printf("return queue id\n");
			}
			else{
				ret = getQueidByKey(__key);
				printf("return queue id\n");
			}		
		}
		else if(__msgflg&(IPC_CREAT|IPC_EXCL)){
			if(false == keyExistInQueList(__key)){
				printf("creat new message queue\n");
				ret = createQueue(__key);
				printf("return queue id\n");
			}
			else{
				printf("return error\n");
				return ret;
			}			
		}
		else{
			printf("unknow error\n");
		}
	}

	return ret;
}
ssize_t msgrcv (int __msqid, void *__msgp, size_t __msgsz,
		       long int __msgtyp, int __msgflg)
{
	int ret = -1;
	initPort(0,__msqid);
	printf("msgrcv __msgflg = %d",__msgflg);
	if(0 == __msgflg){
		printf("recive msg, if there isn't a message which type is msgtyp,hold and wait the msg coming\n");
		while(-1 == getMsgByType(__msqid,__msgp,__msgtyp))
		{
			usleep(50);
		}
	}
	else
	{
		if(IPC_NOWAIT&__msgflg){
			printf("recive msg, if que is empty, return immediately\n");
			if(isQueEmpty(__msqid))
			{
				printf("que is empty\n");
				return ret;
			}
			if(IPC_EXCEPT& __msgflg){
				printf("recive msg, return the first msg which type is not msgtyp\n");
				ret = getMsgExcType(__msqid,__msgp,__msgtyp);
				if(-1 == ret)
				{
					printf("all msg in que which type is equal with msgtype\n");
				}else{
					
					if((IPC_NOERROR& __msgflg)&&((size_t)ret > __msgsz)){
						ret = __msgsz;
			        	printf("recive msg, if msg size is over than __msgsz,then cut off the msg,and do not return error to app\n");
					}
					else
					{
						ret = -1;
						printf("recive msg, if msg size is over than __msgsz,return error \n");
					}
				}
			}
			else{
				ret = getMsgByType(__msqid,__msgp,__msgtyp);
				if(-1 == ret)
				{
					printf("all msg in que which type is not msgtype\n");
				}else{
					if((IPC_NOERROR& __msgflg)&&((size_t)ret > __msgsz)){
						ret = __msgsz;
			        	printf("recive msg, if msg size is over than __msgsz,then cut off the msg,and do not return error to app\n");
					}
					else
					{
						ret = -1;
						printf("recive msg, if msg size is over than __msgsz,return error \n");
					}
				}
			}
						
		}
		else
		{
			while(isQueEmpty(__msqid))
			{
				usleep(50);
			}
			if(IPC_EXCEPT& __msgflg){
				printf("recive msg, return the first msg which type is not msgtyp\n");
				while(-1 == getMsgExcType(__msqid,__msgp,__msgtyp))
				{
					usleep(50);
				}
				if((IPC_NOERROR& __msgflg)&&((size_t)ret > __msgsz)){
					ret = __msgsz;
		        	printf("recive msg, if msg size is over than __msgsz,then cut off the msg,and do not return error to app\n");
				}
				else
				{
					ret = -1;
					printf("recive msg, if msg size is over than __msgsz,return error \n");
				}
			}
			else{
				while(-1 == getMsgByType(__msqid,__msgp,__msgtyp))
				{
					usleep(50);
				}
				if((IPC_NOERROR& __msgflg)&&((size_t)ret > __msgsz)){
					ret = __msgsz;
		        	printf("recive msg, if msg size is over than __msgsz,then cut off the msg,and do not return error to app\n");
				}
				else
				{
					ret = 0;
					printf("recive msg, if msg size is over than __msgsz,return error \n");
				}
			}
		}
	}
	return ret;
}
int msgsnd (int __msqid, const void *__msgp, size_t __msgsz,
		   int __msgflg)
{
	int ret = -1;
	initPort(1,__msqid);
	printf("msgsnd __msgflg = %d",__msgflg);
	if(0 == __msgflg){
		printf("send msg, if que is full, hold and wait the msg can be send to que\n");
		while(isSendQueFull(__msqid))
		{
			if(0 != recvPort)sendToSocket(sendPort,__msqid,IS_FULL);
		}
		if(0 != recvPort)sendToSocket(sendPort,__msqid,NORMAL,__msgp);

	}
	else{
		if(IPC_NOWAIT & __msgflg){
			printf("send msg, if que is full, return immediately\n");
			if(isSendQueFull(__msqid))
			{
				return ret;
			}
			else
			{
				if(IPC_NOERROR & __msgflg){
					printf("send msg, if msg size is over than __msgsz,then cut off the msg,and do not return error to app\n");
					int msgLen = strlen((char*)__msgp)-sizeof(long int);
					printf("msgLen = %d,sizeof(long)=%ld,__msgsz= %ld\n",msgLen,sizeof(long int),__msgsz);
					if((size_t)msgLen > __msgsz)
					{
						char *dest=(char*)malloc(__msgsz+sizeof(long int));
						strncpy(dest,(char*)__msgp,(__msgsz+sizeof(long int)));
						if(0 != recvPort)sendToSocket(sendPort,__msqid,NORMAL,dest);
						free(dest);
						dest = NULL;
					}
					else
					{
						if(0 != recvPort)sendToSocket(sendPort,__msqid,NORMAL,__msgp);
					}
				}
			}
		}
		else{
			if(IPC_NOERROR & __msgflg){
				printf("send msg, if msg size is over than __msgsz,then cut off the msg,and do not return error to app\n");
				int msgLen = strlen((char*)__msgp)-sizeof(long int);
				printf("msgLen = %d,sizeof(long)=%ld,__msgsz= %ld\n",msgLen,sizeof(long int),__msgsz);
				if((size_t)msgLen > __msgsz)
				{
					char *dest=(char*)malloc(__msgsz+sizeof(long int));
					strncpy(dest,(char*)__msgp,(__msgsz+sizeof(long int)));
					if(0 != recvPort)sendToSocket(sendPort,__msqid,NORMAL,dest);
					free(dest);
					dest = NULL;
				}
				else
				{
					if(0 != recvPort)sendToSocket(sendPort,__msqid,NORMAL,__msgp);
				}
			}
		}
	}
	return ret;
}

