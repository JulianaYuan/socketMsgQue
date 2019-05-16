#include "../libsrc/msgsocket.h"
#include "../libsrc/remsgque.h"
 

pthread_t pthrdsvListenCleint;
pthread_t pthrdsvRecieveMsg;


typedef struct keyQue_s
{
	int key;
	MsgNodeQue que;
}keyQue,*pkeyQue;




keyQue gKeyQue[QUE_MAX];
int client_sockfd[FD_SETSIZE];




/*
	return -1:key is not exist in map
	others: key index in map;
*/
int findKeyInMap(int key)
{
	int ret = -1;
	int j;
    if(-1 != key)
    {
        for( j=0;j<QUE_MAX;j++)
        {
            if(gKeyQue[j].key == key)
            {
                ret = gKeyQue[j].que.msqid;
                printf("findKeyInMap ret=%d\n",ret);
                break;
            }
        }
    }
	return ret;
}

int clearFdInMap(int fd)
{
    int ret = -1;
    int j;
    for(j = 0;j<FD_SETSIZE;j++)
    {
        if ( client_sockfd[j] == fd)
        {
            client_sockfd[j] = -1;
            break;
        }
    }
    return ret;
}

static key_t getKeyByQueid(int queid)
{
    key_t key = -1;
    printf("getKeyByQueid queid =%d\n",queid);
    if(-1 != queid)
    {
        for(int i =0;i<QUE_MAX;i++){
            if(gKeyQue[i].que.msqid == queid)
            {
                key = gKeyQue[i].key;
                break;
            }
        }
    }
    return key;

}


int createQueue(int key)
{
	int queId = -1;
	printf("createQueue key =%d\n",key);

	for(int i =0;i<QUE_MAX;i++){
        if(gKeyQue[i].key == -1)//the que is unused
		{
            gKeyQue[i].key = key;
			pMsgNodeQue pque = &gKeyQue[i].que;
			queId = InitQueue(pque);
            if(-1 != queId)
			{
				printf("createQueue key =%d queId=%d success\n",key,queId);
                //setMsgqueAttr(gKeyQue[i].que.msqid,(struct msqid_ds *)0);
			}
			break;
		}
	}
	return queId;

}

static MsgNodeQue *findQueByKey(key_t key)
{
    MsgNodeQue *pque = NULL;
	printf("findQueByKey key =%d \n",key);
    if(-1 != key)
    {
        for(int i =0;i<QUE_MAX;i++){
            printf("findQueByKey gKeyQue[%d].key =%d \n",i,gKeyQue[i].key);
            if(gKeyQue[i].key == key)
            {
                pque = &gKeyQue[i].que;
                printf("findQueByKey pque :%lx\n",(long int)pque);
                break;
            }
        }
    }
    return pque;
}

static MsgNodeQue *findQueByQueid(int qid)
{
    MsgNodeQue *pque = NULL;
    if(-1 != qid)
    {
        for(int i =0;i<QUE_MAX;i++){
            if(gKeyQue[i].que.msqid == qid)
            {
                pque = &gKeyQue[i].que;
                printf("findQueByQueid :%d\n",qid);
                break;
            }
        }
    }
    return pque;
}
static void DestroyQueByQueid(int qid)
{
    for(int i =0;i<QUE_MAX;i++){
        if(gKeyQue[i].que.msqid == qid)
        {
            pMsgNodeQue pque = &gKeyQue[i].que;
            DestroyQueue(pque);
            gKeyQue[i].key = -1;
            printf("findQueByQueid :%d\n",qid);
            break;
        }
    }
}
static bool isQueFull(int __msqid)
{
    bool bret = false;
    MsgNodeQue *pque = findQueByQueid(__msqid);
	if(NULL == pque)
	{
		return bret;
	}
    if(LengthQueue(pque)>=1024)
    {
       bret = true;
    }

    return bret;
}
static bool isQueEmpty(int __msqid)
{
    bool bret = false;
    MsgNodeQue *pque = findQueByQueid(__msqid);
	if(NULL == pque)
	{
		return bret;
	}
    if(IsEmptyQueue(pque))
    {
		bret = true;
	}
    return bret;
}


int insertMsgToQue(int __msqid,socketMsg *msg)
{
    printf("inser msg to que __msqid =%d\n",__msqid);
	int ret = 0;
    MsgData msgDat;
    MsgNodeQue *pque = findQueByQueid(__msqid);
	if(NULL == pque)
	{
		printf("insertMsgToQue NULL == pque\n");
		ret = -1;
		return ret;
    }
    msgDat.msgTyp = msg->msgType;
    debugprintf(msg);
    char *p = msg->msg;
    printf("msg->size = %d\n",msg->size);
    msgDat.size = msg->size;
    msgDat.msg = malloc(msgDat.size);
    if(NULL!=msgDat.msg)
    {
        memset(msgDat.msg,0,msgDat.size);
        memcpy(msgDat.msg,msg->msg,msgDat.size);
        printf("msgDat.msg addr %lx\n",(long int)msgDat.msg);
        printf("insertMsgToQue NULL!=msgDat.msg msgDat.msg =%s\n",(char*)msgDat.msg);
    }
    else
    {
        ret = -1;
		printf("insertMsgToQue NULL==msgDat.msg\n");
		return ret;
    }
    ret = InsertQueue(pque,&msgDat);
	return ret;
}

static int getMsgByType(int msqid,void *__msgp,long int __msgtyp)//return -1:no Msg, return msg real size
{
	int ret = -1;
    socketMsg *pmsg = (socketMsg *)__msgp;
    MsgData msgDat;
    memset(&msgDat,0,sizeof(MsgData));
    msgDat.msg = &(pmsg->msg);
	MsgNodeQue *pque = findQueByQueid(msqid);
	if(NULL == pque)
	{
		return ret;
	}
	pMsgNode p = pque->Front->next;
	if(0 == __msgtyp)
	{
        ret	= DeleteQueue(pque,&msgDat);
	}
	else
	{
        printf("getMsgByType p %lx\n",(long int)p);
		while(p != pque->Rear->next)
		{
			if(p->data.msgTyp == __msgtyp)
			{
                printf("getMsgByType msgDat.msg %lx\n",(long int)(msgDat.msg));
                ret	= DeleteQueByNode(pque,p,&msgDat);
				break;
			}
			p=p->next;
		}
    }
    pmsg->msgType = msgDat.msgTyp;
    pmsg->size = msgDat.size;
    printf("getMsgByType pmsg->size = %d\n",pmsg->size);
	return ret;
}

static int getMsgExcType(int msqid,void *__msgp,long int __msgtyp)//return -1:no Msg, return msg real size
{
	int ret = -1;
    socketMsg *pmsg = (socketMsg *)__msgp;
    MsgData msgDat;
    memset(&msgDat,0,sizeof(MsgData));
    msgDat.msg = &(pmsg->msg);
	MsgNodeQue *pque = findQueByQueid(msqid);
	printf("getMsgExcType\n");
	if(NULL == pque)
	{
		return ret;
	}
	pMsgNode p = pque->Front->next;
	if(0 == __msgtyp)
	{
        ret	= DeleteQueue(pque,&msgDat);
	}
	else
	{
		while(p != pque->Rear->next)
		{
			if(p->data.msgTyp != __msgtyp)
			{
                ret	= DeleteQueByNode(pque,p,&msgDat);
				break;
			}
			p=p->next;
		}
	}
    pmsg->msgType = msgDat.msgTyp;
    pmsg->size = msgDat.size;
    printf("getMsgExcType pmsg->size = %d\n",pmsg->size);
	return ret;
}

int getMsgFromQueByType(int fd,socketData *skdt)
{
	printf("getMsgFromQueByType\n");
    socketData tmpskd;
	int realSz = 0;	
    memset(&tmpskd,0,sizeof(socketData));
    printf("&(tmpskd.msg) address %lx\n",(long int)&(tmpskd.msg));
    printf("&(tmpskd.msg.msg) address %lx\n",(long int)&(tmpskd.msg.msg[0]));
	realSz = getMsgByType(skdt->msqId,(void*)&(tmpskd.msg),skdt->msg.msgType);
    debugprintf(&tmpskd.msg);
	if(-1 == realSz)
	{
		tmpskd.sts = EMPTY;
	}
    else
    {
        tmpskd.sts = NORMAL;
    }
	tmpskd.key  = getKeyByQueid(skdt->msqId);
	tmpskd.msqId= skdt->msqId;
	tmpskd.cmd        = skdt->cmd;
    writeToSocket(fd,&tmpskd);
    debugprintf(&tmpskd.msg);
}

int getMsgFromQueExType(int fd,socketData *skdt)
{
	printf("getMsgFromQueExType\n");
	socketData tmpskd;
    int realSz = 0;
    memset(&tmpskd,0,sizeof(socketData));
	realSz = getMsgExcType(skdt->msqId,(void*)&tmpskd.msg,skdt->msg.msgType);
	if(-1 == realSz)
	{
		tmpskd.sts = EMPTY;
	}
    else
    {
        tmpskd.sts = NORMAL;
    }
	tmpskd.key  = getKeyByQueid(skdt->msqId);
	tmpskd.msqId= skdt->msqId;
	tmpskd.cmd        = skdt->cmd;
    writeToSocket(fd,&tmpskd);
    debugprintf(&tmpskd.msg);
}

void *acceptClient(void *para)
{
	int nready,i;
	int maxi,maxfd,connfd;
	fd_set				rset, allset;
    int socket_fd,client_fd;
    socklen_t client_len;
    struct sockaddr_un client_addr;

	
    socket_fd = *((int*)para);

	maxfd = socket_fd;			/* initialize */
	maxi = -1;					/* index into client[] array */


	FD_ZERO(&allset);
	FD_SET(socket_fd, &allset);
	printf("server is listening socket\n");
	while(1)
	{
		rset = allset;
        usleep(20*1000);
		printf("socket_fd = %d\n",socket_fd);
		nready = select(socket_fd+1, &rset, NULL, NULL, NULL);
		printf("nready = %d\n",nready);
		if (FD_ISSET(socket_fd, &rset)) {	/* new client connection */
			printf("listening socket readable\n");	
            //usleep(20*1000);
			client_len = sizeof(client_addr);
			connfd = accept(socket_fd, (struct sockaddr*)&client_addr, &client_len);
			//accept(server_sockfd,(struct sockaddr*)&client_addr, &client_len);
			if(connfd < 0)
			   perror("accept:");
            printf("connfd = %d\n",connfd);
			for ( i = 0; i < FD_SETSIZE; i++)
				if (client_sockfd[i] < 0) {
					client_sockfd[i] = connfd; /* save descriptor */
					printf("new client client_sockfd[%d] = %d\n",i,client_sockfd[i]);
					break;
				}
			if (i == FD_SETSIZE)
				printf("too many clients\n");
 
			//FD_SET(connfd, &allset);	/* add new descriptor to set */
			if (connfd > maxfd)
				maxfd = connfd; 		/* for select */
			if (i > maxi)
				maxi = i;				/* max index in client[] array */
 			printf("nready = %d\n",nready);
			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}
		printf("maxi = %d\n",maxi);
		for ( i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (client_fd = client_sockfd[i]) < 0)
				continue;
			printf("wait read\n");
		}
	}

}

int checkExistClient()
{
	int ret =1;
	int i;
	
	for(i = 0;i<FD_SETSIZE;i++)
	{
		if ( client_sockfd[i] != -1)
		{
			break;
		}
	}
	if(i == FD_SETSIZE)
	{
		ret = 0;
	}
	return ret;
}


void *recvMsg(void * para)
{
	fd_set				rset, allset;
	int nready,i,n,client_fd;
	struct timeval tmot = {0,1000};

    para = para;
	
	printf("recvMsg\n");
	FD_ZERO(&allset);
	while(1)
	{
        usleep(20*1000);
		if(!checkExistClient())continue;
		for ( i = 0; i < FD_SETSIZE; i++)
		{
			if (-1 != client_sockfd[i]) {
				client_fd = client_sockfd[i];
				if (!FD_ISSET(client_fd, &rset))
				{
					FD_SET(client_fd, &allset);
				}
				rset = allset;
				nready = select(client_fd+1, &rset, NULL, NULL, &tmot);
				
				if((0 != nready)&&((-1 != nready)))
				{
                    printf("recvMsg client_fd =%d, nready = %d\n",client_fd,nready);
                    if (FD_ISSET(client_fd, &rset)) {
                    //usleep(20*1000);
					socketData skdat;
					if ( (n = read(client_fd, &skdat, sizeof(socketData))) == 0) {
						/*4connection closed by client */
						printf("recvMsg client_fd = %d ,connection closed by client \n",client_fd);
						close(client_fd);
						FD_CLR(client_fd, &allset);
                        client_sockfd[i] = -1;
                        clearFdInMap(client_fd);
					}
					else
					{
                        printf("recvMsg size: %d\n",n);
                        //printf("recvMsg client_fd = %d ,connection is exist \n",client_fd);
						printf("recvMsg skdat.cmd:%d\n",skdat.cmd);
						printf("recvMsg skdat.key:%d\n",skdat.key);
                        //printf("recvMsg skdat.sts:%d\n",skdat.sts);
						printf("recvMsg skdat.msqid:%d\n",skdat.msqId);
						printf("recvMsg skdat.msg.msgType:%ld\n",skdat.msg.msgType);
                        //printf("recvMsg skdat.msg.msg:%s\n",skdat.msg.msg);
                        debugprintf(&skdat.msg);
                        if((CMD_MSG_GET_CREATE == skdat.cmd)||(CMD_MSG_GET_NOCRTE == skdat.cmd))
						{
							socketData tmpskd;
                            int id = -1;
                            memset(&tmpskd,0,sizeof(socketData));
                            if(0 == skdat.key)
                            {
                                id = createQueue(skdat.key);
                            }
                            else
                            {
                                id = findKeyInMap(skdat.key);
                                printf("findKeyInMap id = %d\n",id);
                                if(-1 != id)
                                {
                                    printf("this que and key is exist,send que id to client\n");
                                    tmpskd.sts        = EXISTED;
                                }
                                else
                                {
                                    if(CMD_MSG_GET_NOCRTE == skdat.cmd)
                                    {
                                        tmpskd.sts        = NOTEXIST;
                                    }
                                    else
                                    {
                                        id = createQueue(skdat.key);
                                        tmpskd.sts        = NOTEXIST;
                                    }
                                }
                            }
                            tmpskd.msqId = id;
							tmpskd.key  = skdat.key;
							tmpskd.cmd        = skdat.cmd;
							memcpy(&tmpskd.msg,&skdat.msg,sizeof(socketMsg));
                            writeToSocket(client_fd,&tmpskd);
                        }
						else if((CMD_MSG_SEND_NOWT== skdat.cmd)||(CMD_MSG_SEND_WAIT== skdat.cmd))
						{	socketData tmpskd;
                            memset(&tmpskd,0,sizeof(socketData));
							if(isQueFull(skdat.msqId))
							{
								tmpskd.sts        = FULLED;
							}
							else
							{
                                if(-1 == insertMsgToQue(skdat.msqId,&skdat.msg))
								{
									tmpskd.sts        = ERROR;
								}
								else
								{
									tmpskd.sts        = NORMAL;
								}
							}
							tmpskd.key  = getKeyByQueid(skdat.msqId);
							tmpskd.msqId= skdat.msqId;
							tmpskd.cmd        = skdat.cmd;
							memcpy(&tmpskd.msg,&skdat.msg,sizeof(socketMsg));
                            writeToSocket(client_fd,&tmpskd);
						}
                        else if((CMD_MSG_RECV_NOWT_BYTYPE== skdat.cmd)||(CMD_MSG_RECV_WAIT_BYTYPE== skdat.cmd))
						{	
							getMsgFromQueByType(client_fd,&skdat);
						}
                        else if((CMD_MSG_RECV_NOWT_EXTYPE== skdat.cmd)||(CMD_MSG_RECV_WAIT_EXTYPE== skdat.cmd))
						{	
							getMsgFromQueExType(client_fd,&skdat);
						}
#if 0
                        else if(CMD_MSG_CTRL_REMOVE == skdat.cmd)
                        {
                            socketData tmpskd;
                            memset(&tmpskd,0,sizeof(socketData));
                            DestroyQueByQueid(skdat.msqId);
                            tmpskd.sts        = REMOVED;
                            tmpskd.key  = getKeyByQueid(skdat.msqId);
                            tmpskd.msqId= skdat.msqId;
                            tmpskd.cmd        = skdat.cmd;
                            memcpy(&tmpskd.msg,&skdat.msg,sizeof(socketMsg));
                            writeToSocket(client_fd,&tmpskd);
                        }
                        else if(CMD_MSG_CTRL_SETATTR == skdat.cmd)
                        {
                            SetMsgqueAttr(client_fd,&skdat);
                        }
                        else if(CMD_MSG_CTRL_GETATTR == skdat.cmd)
                        {
                            GetMsgqueAttr(client_fd,&skdat);
                        }
#endif
					}
				}
				}
			}			
		}
    }
}

int main()
 
{
    int server_sockfd ,i;
	
	for( i = 0;i<FD_SETSIZE;i++)
	{
		client_sockfd[i] = -1;
	}

	for( i=0;i<QUE_MAX;i++)
	{
		memset(&gKeyQue[i],-1,sizeof(keyQue));
	}

	server_sockfd = server_create(SERVER_NAME);
	
	int trdret = pthread_create(&pthrdsvListenCleint,NULL,acceptClient,(void*)(&server_sockfd));
	trdret = pthread_create(&pthrdsvRecieveMsg,NULL,recvMsg,NULL);
	
	
	while(1){
        sleep(10);
	};
    return 0;
 
}

