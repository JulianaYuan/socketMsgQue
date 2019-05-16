#ifndef _REMSGSERVER_H_
#define _REMSGSERVER_H_
#include "remsg.h"

#define QUE_MAX 16

typedef struct keyFdMap_s
{
	key_t key;
	int fd1;
	int fd2;
}keyFdMap,*pkeyFdMap;


typedef struct keyQue_s{
	msqid_ds msqds;
	MsgNodeQue que;
	keyFdMap keyFd;
}keyQue,*pkeyQue;


#endif //_REMSGSERVER_H_