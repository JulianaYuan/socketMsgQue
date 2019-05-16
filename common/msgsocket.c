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
 
 
int server_create(const char* svnm)
{
	int server_sockfd;
	socklen_t server_len;
	struct sockaddr_un server_addr;

	//delete the old server socket
	unlink(svnm);
	//create socket
	server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	makeAddr(svnm, &server_addr, &server_len);
	bind(server_sockfd, (struct sockaddr *)&server_addr, server_len); 
	//listen the server
	listen(server_sockfd, 5);
	return server_sockfd;
}

int client_create(const char* clnm)
{
	int sockfd;
	socklen_t len;
	struct sockaddr_un address;
	int conret = 0;

	//create socket 
	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	//name the server socket
	makeAddr(clnm, &address, &len);
	//connect to server
	conret = connect(sockfd, (struct sockaddr*)&address, len);
	printf("conret = %d\n",conret);
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
	if(ret == n)
	{
		printf("writeToSocket client_fd = %d ,connection is exist \n",fd);
		printf("writeToSocket skDat->cmd:%d\n",skDat->cmd);
		printf("writeToSocket skDat->key:%d\n",skDat->key);
		printf("writeToSocket skDat->msqId:%d\n",skDat->msqId);
		printf("writeToSocket skDat->sts:%d\n",skDat->sts);
		printf("writeToSocket skDat->msg.msgType:%ld\n",skDat->msg.msgType);
		printf("writeToSocket skDat->msg.msg:%s\n",skDat->msg.msg);
		printf("writeToSocket ok\n");
	}
	printf("write end\n");
	return ret;
}

/*this only used by client*/
int readFromSocket(int fd ,socketData *skDat)
{
	int ret = -1;
	int n;
	if ( (n = read(fd, skDat, sizeof(socketData))) == 0)
	{
		
        printf("readFromSocket client_fd = %d ,connection closed by server \n",fd);
	}
	else
	{
		printf("readFromSocket client_fd = %d ,connection is exist \n",fd);
		printf("readFromSocket skDat->cmd:%d\n",skDat->cmd);
		printf("readFromSocket skDat->key:%d\n",skDat->key);
		printf("readFromSocket skDat->msqId:%d\n",skDat->msqId);
		printf("readFromSocket skDat->sts:%d\n",skDat->sts);
		printf("readFromSocket skDat->msg.msgType:%ld\n",skDat->msg.msgType);
		printf("readFromSocket skDat->msg.msg:%s\n",skDat->msg.msg);
		printf("read ok\n");
		ret = 0;
	}
	close(fd);
	printf("read end\n");
	return ret;
}
int clientMsgGet(int key)
{
	int msqid = -1;
	int n,sockfd;
	sockfd = client_create(SERVER_NAME);
	socketData skDat;

	skDat.key = key;
	skDat.cmd = CMD_MSG_GET;
	skDat.sts =NORMAL;
	skDat.msqId=0;
	memset(&skDat.msg,0,sizeof(socketMsg));
	n = writeToSocket(sockfd,&skDat);
	if(-1!=n)
	{
		printf("clientMsgGet write ok\n");
	}
	printf("clientMsgGet read\n");
	if(0 == readFromSocket(sockfd,&skDat))
	{
		if(skDat.cmd == CMD_MSG_GET)
		{
			msqid = skDat.msqId;
		}
		if(skDat.sts == EXISTED)
		{
			printf("clientMsgGet que is exist\n");
		}
	}
	printf("clientMsgGet msqid = %d\n",msqid);
	return msqid;
}

int clientMsgRcvNoWait(int msqid,long int __msgtyp,enum SKCMD skCmd,socketMsg *msg)
{
	int ret = -1;
	int n,sockfd;
	sockfd = client_create(SERVER_NAME);
	socketData skDat;

	skDat.key = 0;
	skDat.cmd = skCmd;
	skDat.sts =NORMAL;
	skDat.msqId = msqid;
	skDat.msg.msgType = __msgtyp;
	memset(&(skDat.msg),0,sizeof(socketMsg));
	n = writeToSocket(sockfd,&skDat);
	if(-1!=n)
	{
		printf("clientMsgRcv write ok\n");
	}
	printf("clientMsgRcv read\n");
	ret = readFromSocket(sockfd,&skDat);
	printf("clientMsgRcv read skDat.msg.msgType = %ld,skDat.msg.msg =%s\n",skDat.msg.msgType,skDat.msg.msg);
	if(!ret)memcpy(msg,&skDat.msg,sizeof(socketMsg));
	printf("clientMsgRcv end \n");
	return ret;
}
int clientMsgRcvWait(int msqid,long int __msgtyp,enum SKCMD skCmd,socketMsg *msg)
{
	int ret = -1;
	int n,sockfd;
	socketData skDat;

	do{
		sockfd = client_create(SERVER_NAME);		
		skDat.key = 0;
		skDat.cmd = skCmd;
		skDat.sts =NORMAL;
		skDat.msqId = msqid;
		skDat.msg.msgType = __msgtyp;
		memset(&(skDat.msg),0,sizeof(socketMsg));
		n = writeToSocket(sockfd,&skDat);
		if(-1!=n)
		{
			printf("clientMsgRcv write ok\n");
		}
		printf("clientMsgRcv read\n");
		ret = readFromSocket(sockfd,&skDat);
		printf("clientMsgRcv read skDat.msg.msgType = %ld,skDat.msg.msg =%s\n",skDat.msg.msgType,skDat.msg.msg);
		if(!ret)memcpy(msg,&skDat.msg,sizeof(socketMsg));
		printf("clientMsgRcv end \n");

	}
	while(EMPTY== skDat.sts);
	if(ERROR== skDat.sts)printf("send msg error\n");
	return ret;
}

int clientMsgRcv(int msqid,long int __msgtyp,enum SKCMD skCmd,socketMsg *msg)
{
	int ret = -1;

	switch(skCmd)
	{
	case CMD_MSG_RECV_NOWT_BYTYPE:
	case CMD_MSG_RECV_NOWT_EXTYPE:
		ret = clientMsgRcvNoWait(msqid,__msgtyp,msg,skCmd,msg);
		break;
	case CMD_MSG_RECV_WAIT_BYTYPE:
	case CMD_MSG_RECV_WAIT_EXTYPE:
		ret = clientMsgRcvWait(msqid,__msgtyp,msg,skCmd,msg);
		break;	
	default:
		printf("error cmd\n");
		break;		
	}
	
	return ret;
}

int clientMsgSendWait(int msqid,socketMsg *msg)
{
	int ret = -1;
	int n,sockfd;
	socketData skDat;

	do{
		sockfd = client_create(SERVER_NAME);
		skDat.key = 0;
		skDat.cmd = CMD_MSG_SEND_WAIT;
		skDat.sts =NORMAL;
		skDat.msqId = msqid;
		memset(&skDat.msg,0,sizeof(socketMsg));
		memcpy(&skDat.msg,msg,sizeof(socketMsg));
		n = writeToSocket(sockfd,&skDat);
		if(-1!=n)
		{
			printf("clientMsgSendWait write ok\n");
		}
		printf("clientMsgSendWait read\n");
		printf("clientMsgSendWait read\n");
		ret = readFromSocket(sockfd,&skDat);
	}
	while(FULLED == skDat.sts);
	if(ERROR== skDat.sts)printf("send msg error\n");
	return ret;
}

int clientMsgSendNoWait(int msqid,socketMsg *msg)
{
	int ret = -1;
	int n,sockfd;
	socketData skDat;
	
	sockfd = client_create(SERVER_NAME);
	skDat.key = 0;
	skDat.cmd = CMD_MSG_SEND_NOWT;
	skDat.sts =NORMAL;
	skDat.msqId = msqid;
	memset(&skDat.msg,0,sizeof(socketMsg));
	memcpy(&skDat.msg,msg,sizeof(socketMsg));
	n = writeToSocket(sockfd,&skDat);
	if(-1!=n)
	{
		printf("clientMsgSendWait write ok\n");
	}
	printf("recvMsgHandler read\n");
	ret = readFromSocket(sockfd,&skDat);	
	if(FULLED == skDat.sts)ret = -1;
	
	return ret;
}


int clientMsgSend(int msqid,enum SKCMD skCmd,socketMsg *msg)
{
	int ret = -1;

	switch(skCmd)
		{
		case CMD_MSG_SEND_WAIT:
			ret = clientMsgSendWait(msqid,msg);
			break;
		case CMD_MSG_SEND_NOWT:
			ret = clientMsgSendNoWait(msqid,msg);
			break;
		default:
			printf("error cmd\n");
			break;		
		}
	
	return ret;
}


