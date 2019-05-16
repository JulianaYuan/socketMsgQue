#ifndef _MSGSOCKET_H_
#define _MSGSOCKET_H_

#include<sys/types.h>
#include<sys/socket.h>
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
#define MAXLINE 4096
#define QUE_MAX 16

enum SKCMD
{
	CMD_MSG_GET,
	CMD_MSG_SEND_WAIT,
	CMD_MSG_SEND_NOWT,
	CMD_MSG_RECV_WAIT_BYTYPE,
	CMD_MSG_RECV_NOWT_BYTYPE,
	CMD_MSG_RECV_WAIT_EXTYPE,
	CMD_MSG_RECV_NOWT_EXTYPE,
	CMD_MSG_CTRL,
};
enum QUE_STS
{
	NORMAL, //que is not full, or que is not exist
    FULLED, //answer que is fulled,
    EMPTY,//que is empty
    EXISTED,//que is exist
    ERROR,//que is exist
};


typedef struct socketMsg_s
{
	long int msgType;
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





#endif
