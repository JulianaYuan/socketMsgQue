#ifndef _REMSGQUE_H_
#define _REMSGQUE_H_


/* Define options for message queue functions.  */
#define MSG_NOERROR	010000	/* no error if message is too big */
#ifdef __USE_GNU
# define MSG_EXCEPT	020000	/* recv any msg except of specified type */
# define MSG_COPY	040000	/* copy (not remove) all queue messages */
#endif

/* Types used in the structure definition.  */
typedef __syscall_ulong_t msgqnum_t;
typedef __syscall_ulong_t msglen_t;


struct msqid_ds
{
  struct ipc_perm msg_perm;	/* structure describing operation permission */
  __time_t msg_stime;		/* time of last msgsnd command */
#ifndef __x86_64__
  unsigned long int __glibc_reserved1;
#endif
  __time_t msg_rtime;		/* time of last msgrcv command */
#ifndef __x86_64__
  unsigned long int __glibc_reserved2;
#endif
  __time_t msg_ctime;		/* time of last change */
#ifndef __x86_64__
  unsigned long int __glibc_reserved3;
#endif
  __syscall_ulong_t __msg_cbytes; /* current number of bytes on queue */
  msgqnum_t msg_qnum;		/* number of messages currently on queue */
  msglen_t msg_qbytes;		/* max number of bytes allowed on queue */
  __pid_t msg_lspid;		/* pid of last msgsnd() */
  __pid_t msg_lrpid;		/* pid of last msgrcv() */
  __syscall_ulong_t __glibc_reserved4;
  __syscall_ulong_t __glibc_reserved5;
};

#ifdef __USE_MISC

# define msg_cbytes	__msg_cbytes

/* ipcs ctl commands */
# define MSG_STAT 11
# define MSG_INFO 12

/* buffer for msgctl calls IPC_INFO, MSG_INFO */
struct msginfo
  {
    int msgpool;
    int msgmap;
    int msgmax;
    int msgmnb;
    int msgmni;
    int msgssz;
    int msgtql;
    unsigned short int msgseg;
  };

#endif /* __USE_MISC */




typedef struct MsgData_s{
	long int msgTyp;
	void *msg;
} MsgData;

typedef struct MsgNode_s{
	MsgData data;
	struct MsgNode_s *next;
} MsgNode,*pMsgNode;


typedef struct MsgNodeQue_s{
	pMsgNode Front;
	pMsgNode Rear;
	int msqid;
} MsgNodeQue,*pMsgNodeQue;


//    声明函数体
int InitQueue(pMsgNodeQue);    //    创建队列函数
bool IsEmptyQueue(pMsgNodeQue);    //    判断队列是否为空函数
int InsertQueue(pMsgNodeQue, MsgData data);    //    入队函数
int DeleteQueue(pMsgNodeQue,MsgData* data);    //    出队函数
int DeleteQueByNode(pMsgNodeQue queue,pMsgNode pNode,MsgData* data);
void DestroyQueue(pMsgNodeQue);    //    摧毁队列函数
int TraverseQueue(pMsgNodeQue);    //    遍历队列函数
void ClearQueue(pMsgNodeQue);    //    清空队列函数
int LengthQueue(pMsgNodeQue);    //    求队列长度函数

#endif //_REMSGQUE_H_
