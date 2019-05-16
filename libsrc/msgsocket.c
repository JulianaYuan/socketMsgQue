#include "msgsocket.h"
#include <sys/time.h>

void nowtime_ns(bool sts)
{
    static struct timespec ts_old={0,0};
    struct timespec ts={0,0};

    ts.tv_sec = ts_old.tv_sec;
    ts.tv_nsec = ts_old.tv_nsec;
    clock_gettime(CLOCK_MONOTONIC, &ts_old);
    if(true == sts)
    {
        ts.tv_sec =  ts_old.tv_sec-ts.tv_sec;
        ts.tv_nsec =  ts_old.tv_nsec-ts.tv_nsec;
        printf("clock_gettime : tv_sec=%ld, tv_nsec=%ld\n", ts.tv_sec, ts.tv_nsec);
    }
}

/*
 * Create a UNIX-domain socket address in the Linux "abstract namespace".
 *
 * The socket code doesn't require null termination on the filename, but
 * we do it anyway so string functions work.
 */
static int makeAddr(const char* name, struct sockaddr_un* pAddr, socklen_t* pSockLen)
{
    int nameLen = strlen(name);
    if (nameLen >= (int) sizeof(pAddr->sun_path) -1)  /* too long? */
        return -1;
    pAddr->sun_path[0] = '\0';  /* abstract namespace */
    strcpy(pAddr->sun_path+1, name);
    pAddr->sun_family = AF_UNIX;
    *pSockLen = 1 + nameLen + offsetof(struct sockaddr_un, sun_path);
    return 0;
}
 
bool set_tcp_nodelay(int fd)
{
    bool ret = true;
    int yes = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes)) == -1) {
            ret = false;
        }/**/
    return ret;
}

int server_create(const char* svnm)
{
	int server_sockfd;
    socklen_t server_len = 0;;
	struct sockaddr_un server_addr;

	//delete the old server socket
	unlink(svnm);
	//create socket
	server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    set_tcp_nodelay(server_sockfd);
	makeAddr(svnm, &server_addr, &server_len);
	bind(server_sockfd, (struct sockaddr *)&server_addr, server_len); 
	//listen the server
	listen(server_sockfd, 5);
	return server_sockfd;
}

int client_create(const char* clnm)
{
	int sockfd;
    socklen_t len = 0;
	struct sockaddr_un address;
	int conret = 0;

	//create socket 
	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    set_tcp_nodelay(sockfd);
	//name the server socket
	makeAddr(clnm, &address, &len);
	//connect to server
	conret = connect(sockfd, (struct sockaddr*)&address, len);
	if(-1 == conret)
	{
		perror("opps:client1");
	}	
	return sockfd;
}

int writeToSocket(int fd ,socketData *skDat)
{
	int ret = -1;
	int n = 0;
	n = sizeof(socketData);
	ret = write(fd, skDat, n);
    nowtime_ns(false);
    //printf("\r\n"__DATE__"-"__TIME__"\r\n");
    //printf("writeToSocket size: %d\n",ret);
	if(ret == n)
	{
#if 0
		printf("writeToSocket client_fd = %d ,connection is exist \n",fd);
		printf("writeToSocket skDat->cmd:%d\n",skDat->cmd);
        printf("writeToSocket skDat->key:%x\n",skDat->key);
		printf("writeToSocket skDat->msqId:%d\n",skDat->msqId);
		printf("writeToSocket skDat->sts:%d\n",skDat->sts);
		printf("writeToSocket skDat->msg.msgType:%ld\n",skDat->msg.msgType);
        printf("readFromSocket skDat->msg.size:%d\n",skDat->msg.size);
		printf("writeToSocket skDat->msg.msg:%s\n",skDat->msg.msg);
		printf("writeToSocket ok\n");
#endif
	}
    else if(-1 == ret)
    {
        int errsv = errno;
        //printf("error code is %d",errsv);
    }
	return ret;
}


int readFromSocket(int fd ,socketData *skDat)
{
	int ret = -1;
	int n;
	if ( (n = read(fd, skDat, sizeof(socketData))) == 0)
	{
        nowtime_ns(true);
        //printf("\r\n"__DATE__"-"__TIME__"\r\n");
        //printf("readFromSocket client_fd = %d ,connection closed by client \n",fd);
	}
    else if(-1 == n)
    {
        nowtime_ns(true);
        //printf("\r\n"__DATE__"-"__TIME__"\r\n");
        int errsv = errno;
        //printf("error code is %d",errsv);
    }
	else
	{
        nowtime_ns(true);
        //printf("\r\n"__DATE__"-"__TIME__"\r\n");
        debugprintf(&(skDat->msg));
#if 0
		printf("readFromSocket client_fd = %d ,connection is exist \n",fd);
		printf("readFromSocket skDat->cmd:%d\n",skDat->cmd);
        printf("readFromSocket skDat->key:%x\n",skDat->key);
		printf("readFromSocket skDat->msqId:%d\n",skDat->msqId);
		printf("readFromSocket skDat->sts:%d\n",skDat->sts);
		printf("readFromSocket skDat->msg.msgType:%ld\n",skDat->msg.msgType);
        printf("readFromSocket skDat->msg.size:%d\n",skDat->msg.size);
		printf("readFromSocket skDat->msg.msg:%s\n",skDat->msg.msg);
		printf("read ok\n");
#endif
		ret = 0;
	}
	close(fd);
	return ret;
}
static int clientMsgGet(int key,enum SKCMD skcmd,bool &exist)
{
	int msqid = -1;
	int n,sockfd;
	sockfd = client_create(SERVER_NAME);
	socketData skDat;

	skDat.key = key;
    skDat.cmd = skcmd;
	skDat.sts =NORMAL;
	skDat.msqId=0;
	memset(&skDat.msg,0,sizeof(socketMsg));
	n = writeToSocket(sockfd,&skDat);
	if(-1!=n)
	{
        //printf("clientMsgGet write ok\n");
        //printf("clientMsgGet read\n");
        if(0 == readFromSocket(sockfd,&skDat))
        {
            if(skDat.cmd == CMD_MSG_GET_CREATE || skDat.cmd == CMD_MSG_GET_NOCRTE)
            {
                msqid = skDat.msqId;
            }
            if(skDat.sts == EXISTED)
            {
                exist = true;
                //printf("clientMsgGet que is exist\n");
            }
            else if(skDat.sts == NOTEXIST)
            {
                exist = false;
                //printf("clientMsgGet que is not exist\n");
            }
        }
    }
    //printf("clientMsgGet msqid = %d\n",msqid);
	return msqid;
}

int clientMsgGetCreate(int key,bool &exist)
{
    int ret = -1;
    ret = clientMsgGet(key,CMD_MSG_GET_CREATE,exist);
    return ret;
}
int clientMsgGetNoCrte(int key,bool &exist)
{
    int ret = -1;
    ret = clientMsgGet(key,CMD_MSG_GET_NOCRTE,exist);
    return ret;
}
#if 0
int clientSetMsgqueAttr(int __msqid, struct msqid_ds *__buf)
{
    int ret = -1;
    int n,sockfd;
    sockfd = client_create(SERVER_NAME);
    socketData skDat;

    skDat.key = 0;
    skDat.cmd = CMD_MSG_CTRL_SETATTR;
    skDat.sts = NORMAL;
    skDat.msqId = __msqid;
    memset(&skDat.msg,0,sizeof(socketMsg));
    memcpy(skDat.msg.msg,__buf,sizeof(struct msqid_ds));
    n = writeToSocket(sockfd,&skDat);
    if(-1 != n)
    {
        printf("clientSetMsgqueAttr write ok\n");
        printf("clientSetMsgqueAttr read\n");
        if(0 == readFromSocket(sockfd,&skDat))
        {
            if(skDat.cmd == CMD_MSG_CTRL_SETATTR)
            {

            }
        }
    }
    printf("clientSetMsgqueAttr msqid = %d\n",msqid);

    return ret;
}

int clientGetMsgqueAttr(int __msqid, struct msqid_ds *__buf)
{
    int ret = -1;
    int n,sockfd;
    sockfd = client_create(SERVER_NAME);
    socketData skDat;

    skDat.key = 0;
    skDat.cmd = CMD_MSG_CTRL_GETATTR;
    skDat.sts = NORMAL;
    skDat.msqId = __msqid;
    memset(&skDat.msg,0,sizeof(socketMsg));
    n = writeToSocket(sockfd,&skDat);
    if(-1 != n)
    {
        printf("clientSetMsgqueAttr write ok\n");
        printf("clientSetMsgqueAttr read\n");
        if(0 == readFromSocket(sockfd,&skDat))
        {
            if(skDat.cmd == CMD_MSG_CTRL_GETATTR)
            {
                printf("CMD_MSG_CTRL_GETATTR\n");
                memcpy(__buf,skDat.msg.msg,sizeof(struct msqid_ds));
            }
        }
    }
    printf("clientSetMsgqueAttr msqid = %d\n",msqid);
    return ret;
}


int clientMsgCtrlRemv(int msqid)
{
    int n,sockfd;
    sockfd = client_create(SERVER_NAME);
    socketData skDat;

    skDat.key = 0;
    skDat.cmd = CMD_MSG_CTRL_REMOVE;
    skDat.sts =NORMAL;
    skDat.msqId=msqid;
    memset(&skDat.msg,0,sizeof(socketMsg));
    n = writeToSocket(sockfd,&skDat);
    if(-1!=n)
    {
        printf("clientMsgCtrlRemv write ok\n");
    }
    printf("clientMsgCtrlRemv read\n");
    if(0 == readFromSocket(sockfd,&skDat))
    {
        if(skDat.sts == REMOVED)
        {
            printf("clientMsgCtrlRemv que is removed ok\n");
        }
    }
    return 0;
}
#endif
static int clientMsgRcvNoWait(int msqid,long int __msgtyp,enum SKCMD skCmd,socketMsg *msg)
{
	int ret = -1;
	int n,sockfd;
	sockfd = client_create(SERVER_NAME);
    socketData skDatW;
    socketData skDatR;
    memset(&skDatW,0,sizeof(socketData));
    memset(&skDatR,0,sizeof(socketData));

    skDatW.key = 0;
    skDatW.cmd = skCmd;
    skDatW.sts =NORMAL;
    skDatW.msqId = msqid;
    skDatW.msg.msgType = __msgtyp;
    printf("clientMsgRcvNoWait skDatW.msg.msgType =  %ld\n",skDatW.msg.msgType);
    n = writeToSocket(sockfd,&skDatW);
	if(-1!=n)
	{
        printf("clientMsgRcvNoWait write ok\n");
	}
    printf("clientMsgRcvNoWait read\n");
    ret = readFromSocket(sockfd,&skDatR);
    printf("clientMsgRcvNoWait read skDatR.msg.msgType = %ld,skDatR.msg.msg =%s\n",skDatR.msg.msgType,skDatR.msg.msg);
    if(!ret)memcpy(msg,&skDatR.msg,sizeof(socketMsg));
    if(EMPTY == skDatR.sts)
    {
        ret = -1;
        printf("clientMsgRcvNoWait empty \n");
    }
    printf("clientMsgRcvNoWait end \n");
	return ret;
}
static int clientMsgRcvWait(int msqid,long int __msgtyp,enum SKCMD skCmd,socketMsg *msg)
{
	int ret = -1;
	int n,sockfd;
    socketData skDatW;
    socketData skDatR;

	do{
        sockfd = client_create(SERVER_NAME);
        memset(&skDatW,0,sizeof(socketData));
        skDatW.key = 0;
        skDatW.cmd = skCmd;
        skDatW.sts =NORMAL;
        skDatW.msqId = msqid;
        skDatW.msg.msgType = __msgtyp;
        n = writeToSocket(sockfd,&skDatW);
		if(-1!=n)
		{
            memset(&skDatR,0,sizeof(socketData));
            ret = readFromSocket(sockfd,&skDatR);
            if(!ret)memcpy(msg,&skDatR.msg,sizeof(socketMsg));
        }
	}
    while(EMPTY== skDatR.sts);
    if(ERROR== skDatR.sts)printf("clientMsgRcvWait send msg error\n");
    printf("clientMsgRcvWait read skDatR.sts=%d, skDatR.msg.msgType = %ld,skDatR.msg.msg =%s\n",skDatR.sts,skDatR.msg.msgType,skDatR.msg.msg);
	return ret;
}

int clientMsgRcvNowaitBytype(int msqid,long int __msgtyp,void *msg,size_t __msgsz,bool noerror)
{
    int ret = -1;
    int msgsize = -1;
    socketMsg skmsg;
    memset(&skmsg,0,sizeof(socketMsg));
    ret = clientMsgRcvNoWait(msqid,__msgtyp,CMD_MSG_RECV_NOWT_BYTYPE,&skmsg);
    if(-1 != ret)
    {
        debugprintf(&skmsg);
        msgsize = skmsg.size;
        printf("clientMsgRcvNowaitBytype msgsize = %d,__msgsz =%d,noerror:%s\n",msgsize,(int)__msgsz,(noerror==true)?"true":"false");
        if(msgsize <= (int)__msgsz||((msgsize >(int)__msgsz)&&(noerror==true)))
        {
            *((long int *)msg) = skmsg.msgType;
            memcpy(((char *)msg+sizeof(long int)),&skmsg.msg,__msgsz);
            msgsize = __msgsz;
        }
        else
        {
            msgsize = -1;
        }
    }
    else
    {
        printf("clientMsgRcvNowaitBytype reciev msg error or no this type msg\n");
    }
    return msgsize;
}
int clientMsgRcvNowaitExtype(int msqid,long int __msgtyp,void *msg,size_t __msgsz,bool noerror)
{
    int ret = -1;
    int msgsize = -1;
    socketMsg skmsg;
    memset(&skmsg,0,sizeof(socketMsg));
    ret = clientMsgRcvNoWait(msqid,__msgtyp,CMD_MSG_RECV_NOWT_EXTYPE,&skmsg);
    if(-1 != ret)
    {
        debugprintf(&skmsg);
        msgsize = skmsg.size;
        printf("clientMsgRcvNowaitExtype msgsize = %d,__msgsz =%d,noerror:%s\n",msgsize,(int)__msgsz,(noerror==true)?"true":"false");
        if(msgsize <= (int)__msgsz||((msgsize > (int)__msgsz)&&(noerror==true)))
        {
            *((long int *)msg) = skmsg.msgType;
            memcpy(((char *)msg+sizeof(long int)),&skmsg.msg,__msgsz);
            msgsize = __msgsz;
        }
        else
        {
            msgsize = -1;
        }
    }
    else
    {
        printf("clientMsgRcvNowaitExtype reciev msg error or no this type msg\n");
    }
    return msgsize;
}
int clientMsgRcvWaitBytype(int msqid,long int __msgtyp,void *msg,size_t __msgsz,bool noerror)
{
    int ret = -1;
    int msgsize = -1;
    socketMsg skmsg;
    memset(&skmsg,0,sizeof(socketMsg));
    ret = clientMsgRcvWait(msqid,__msgtyp,CMD_MSG_RECV_WAIT_BYTYPE,&skmsg);
    if(-1 != ret)
    {
        debugprintf(&skmsg);
        msgsize = skmsg.size;
        printf("clientMsgRcvWaitBytype msgsize = %d,__msgsz =%d,noerror:%s\n",msgsize,(int)__msgsz,(noerror==true)?"true":"false");
        if(msgsize <= (int)__msgsz||((msgsize > (int)__msgsz)&&(noerror==true)))
        {
            *((long int *)msg) = skmsg.msgType;
            memcpy(((char *)msg+sizeof(long int)),&skmsg.msg,__msgsz);
            msgsize = __msgsz;
            printf("msgsize = %d\n",msgsize);
        }
        else
        {
            msgsize = -1;
        }
    }
    else
    {
        printf("clientMsgRcvWaitBytype reciev msg error or no this type msg\n");
    }
    return msgsize;
}

int clientMsgRcvWaitExtype(int msqid,long int __msgtyp,void *msg,size_t __msgsz,bool noerror)
{
    int ret = -1;
    int msgsize = -1;
    socketMsg skmsg;
    memset(&skmsg,0,sizeof(socketMsg));
    ret = clientMsgRcvWait(msqid,__msgtyp,CMD_MSG_RECV_WAIT_EXTYPE,&skmsg);
    if(-1 != ret)
    {
        debugprintf(&skmsg);
        msgsize = skmsg.size;
        printf("clientMsgRcvWaitExtype msgsize = %d,__msgsz =%d,noerror:%s\n",msgsize,(int)__msgsz,(noerror==true)?"true":"false");
        if(msgsize <= (int)__msgsz||((msgsize > (int)__msgsz)&&(noerror==true)))
        {
            *((long int *)msg) = skmsg.msgType;
            memcpy(((char *)msg+sizeof(long int)),&skmsg,__msgsz);
            msgsize = __msgsz;
        }
        else
        {
            msgsize = -1;
        }
    }
    else
    {
        printf("clientMsgRcvWaitExtype reciev msg error or no this type msg\n");
    }
    return msgsize;
}


static int clientMsgSendWait(int msqid,socketMsg *msg)
{
	int ret = -1;
	int n,sockfd;
    socketData skDatW;
    socketData skDatR;

	do{
		sockfd = client_create(SERVER_NAME);
        memset(&skDatW,0,sizeof(socketData));
        skDatW.key = 0;
        skDatW.cmd = CMD_MSG_SEND_WAIT;
        skDatW.sts =NORMAL;
        skDatW.msqId = msqid;
        memcpy(&skDatW.msg,msg,sizeof(socketMsg));
        n = writeToSocket(sockfd,&skDatW);
		if(-1!=n)
		{
			printf("clientMsgSendWait write ok\n");
		}
		printf("clientMsgSendWait read\n");
		printf("clientMsgSendWait read\n");
        memset(&skDatR,0,sizeof(socketData));
        ret = readFromSocket(sockfd,&skDatR);
	}
    while(FULLED == skDatR.sts);
    if(ERROR== skDatR.sts)printf("send msg error\n");
	return ret;
}

static int clientMsgSendNoWait(int msqid,socketMsg *msg)
{
	int ret = -1;
	int n,sockfd;
    socketData skDatW;
    socketData skDatR;
    memset(&skDatW,0,sizeof(socketData));
    memset(&skDatR,0,sizeof(socketData));

	sockfd = client_create(SERVER_NAME);
    skDatW.key = 0;
    skDatW.cmd = CMD_MSG_SEND_NOWT;
    skDatW.sts =NORMAL;
    skDatW.msqId = msqid;

    memcpy(&skDatW.msg,msg,sizeof(socketMsg));
    n = writeToSocket(sockfd,&skDatW);
	if(-1!=n)
	{
		printf("clientMsgSendWait write ok\n");
	}
	printf("recvMsgHandler read\n");
    ret = readFromSocket(sockfd,&skDatR);
    if(FULLED == skDatR.sts)ret = -1;
	
	return ret;
}

int clientMsgSendWaitExt(int msqid,const void *msg,size_t __msgsz)
{
    int ret = -1;
    socketMsg skmsg;
    if(__msgsz>(MAXLINE-1))
    {
        printf("the send msg should below (MAXLINE-1)(%d),cann't send to server\n",(MAXLINE-1));
    }
    else
    {
        skmsg.size = __msgsz;
        skmsg.msgType = *((long int*)msg);
        memcpy(skmsg.msg,((char*)msg+sizeof(long int)),__msgsz);
        ret = clientMsgSendWait(msqid,&skmsg);
        if(-1 == ret)
        {
            printf("reciev msg error or no this type msg\n");
        }
    }


    return ret;
}
int clientMsgSendNoWaitExt(int msqid,const void *msg,size_t __msgsz)
{
    int ret = -1;
    socketMsg skmsg;
    if(__msgsz>(MAXLINE-1))
    {
        printf("the send msg should below (MAXLINE-1)(%d),cann't send to server\n",(MAXLINE-1));
    }
    else
    {
        skmsg.size = __msgsz;
        skmsg.msgType = *((long int*)msg);
        memcpy(skmsg.msg,((char*)msg+sizeof(long int)),__msgsz);
        ret = clientMsgSendNoWait(msqid,&skmsg);
        if(-1 == ret)
        {
            printf("reciev msg error or no this type msg\n");
        }
    }
    return ret;
}

void debugprintf(socketMsg *msg)
{
    msg=msg;
}


