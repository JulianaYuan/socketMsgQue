#ifndef _MSGSOCKET_H_
#define _MSGSOCKET_H_

#include<sys/types.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include<stdio.h>
#include<sys/un.h>
#include<unistd.h>
#include<stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define SERVER_NAME "@server_socket"
#define MAXLINE 8192//212994//8192//2147479552//9612
#define QUE_MAX 4096

enum SKCMD
{
    CMD_MSG_GET_CREATE,
    CMD_MSG_GET_NOCRTE,
	CMD_MSG_SEND_WAIT,
	CMD_MSG_SEND_NOWT,
	CMD_MSG_RECV_WAIT_BYTYPE,
	CMD_MSG_RECV_NOWT_BYTYPE,
	CMD_MSG_RECV_WAIT_EXTYPE,
	CMD_MSG_RECV_NOWT_EXTYPE,
    /*CMD_MSG_CTRL_REMOVE,
    CMD_MSG_CTRL_GETATTR,
    CMD_MSG_CTRL_SETATTR,*/
};
enum QUE_STS
{
	NORMAL, //que is not full, or que is not exist
    FULLED, //answer que is fulled,
    EMPTY,//que is empty
    EXISTED,//que is exist
    NOTEXIST,//que is exist
    REMOVED,
    ERROR,//que is exist
};


typedef struct socketMsg_s
{
	long int msgType;
    int size;
	char msg[MAXLINE];
}socketMsg;

typedef struct socketData_s
{
	int key;
    int msqId;
    enum SKCMD cmd;
	enum QUE_STS sts;
	socketMsg msg;
}socketData,*psocketData;

int server_create(const char* svnm);
int writeToSocket(int fd ,socketData *skDat);
int readFromSocket(int fd ,socketData *skDat);

int clientMsgGetCreate(int key,bool &exist);
int clientMsgGetNoCrte(int key,bool &exist);

#if 0
int clientSetMsgqueAttr(int __msqid, struct msqid_ds *__buf);
int clientGetMsgqueAttr(int __msqid, struct msqid_ds *__buf);
int clientMsgCtrlRemv(int msqid);
#endif

int clientMsgRcvNowaitBytype(int msqid,long int __msgtyp,void *msg,size_t __msgsz,bool noerror);
int clientMsgRcvNowaitExtype(int msqid,long int __msgtyp,void *msg,size_t __msgsz,bool noerror);
int clientMsgRcvWaitBytype(int msqid,long int __msgtyp,void *msg,size_t __msgsz,bool noerror);
int clientMsgRcvWaitExtype(int msqid,long int __msgtyp,void *msg,size_t __msgsz,bool noerror);

int clientMsgSendWaitExt(int msqid,const void *msg,size_t __msgsz);
int clientMsgSendNoWaitExt(int msqid,const void *msg,size_t __msgsz);

void debugprintf(socketMsg *msg);

#endif
