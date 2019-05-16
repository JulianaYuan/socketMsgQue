#include "remsgserver.h"
#include "msgsocket.h"

keyQue gKeyQue[QUE_MAX];


/**********que process start**********/
static int initKeyQue()
{
	int ret = 9;
	for(int i =0;i<QUE_MAX;i++){
        memset(&gKeyQue[i],-1,sizeof(keyQue));
	}
	return ret;
}

static bool keyExistInQueList(key_t key)
{
	bool ret = false;
	printf("keyExistInQueList key = %d\n",key);
	for(int i =0;i<QUE_MAX;i++){
        if(gKeyQue[i].keyFd.key== key)
		{
			ret = true;
			break;
		}
	}
	return ret;
}

static int getQueidByKey(key_t key)
{
	int queId = -1;
	printf("getQueidByKey key =%d\n",key);
	for(int i =0;i<QUE_MAX;i++){
        if(gKeyQue[i].keyFd.key == key)
		{
            queId = gKeyQue[i].que.msqid;
			break;
		}
	}
	return queId;

}

static key_t getKeyByQueid(int queid)
{
    key_t key = -1;
    printf("getKeyByQueid queid =%d\n",queid);
    for(int i =0;i<QUE_MAX;i++){
        if(gKeyQue[i].que.msqid == queid)
        {
            key = gKeyQue[i].keyFd.key;
            break;
        }
    }
    return key;

}





static int createQueue(key_t key)
{
	int queId = -1;
	printf("createQueue key =%d\n",key);
	if(keyExistInQueList(key))
	{
		queId = getQueidByKey(key);
	}
	else
	{
		for(int i =0;i<QUE_MAX;i++){
            if(gKeyQue[i].keyFd.key == -1)//the que is unused
			{
                gKeyQue[i].keyFd.key = key;
				pMsgNodeQue pque = &gKeyQue[i].que;
				queId = InitQueue(pque);
                if(-1 != queId)
				{
					printf("createQueue key =%d queId=%d success\n",key,queId);
                    setMsgqueAttr(gKeyQue[i].que.msqid,(struct msqid_ds *)0);
				}
				break;
			}
		}
	}
	return queId;
}

static int destroyQueue(int __msqid)
{
	int ret = 0;
	MsgNodeQue *pque = findQueByQueid(__msqid);
	if(NULL != pque)
	{
		DestroyQueue(pque);
	}
	return ret;
}





/**********que process end**********/



/********************socket process start ********************/


/********************socket process end ********************/

int main()
{
	int server_fd = -1;

	initKeyQue();

	
	server_fd = server_socket_create(SERVER_NAME);
	int trdret = pthread_create(&pthrdsv,NULL,acceptClientAndRecvMsg,(void*)(&server_fd));
	if(trdret)
	{
		printf("thread creat success\n");
	}

}

