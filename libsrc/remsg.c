#include "remsg.h"
#include "remsgque.h"
#include <stdio.h>
#include "msgsocket.h"
#include <string.h>
#if 0
int msgctl (int __msqid, int __cmd, struct msqid_ds *__buf)
{
	int ret = -1;

	if( IPC_STAT == __cmd){
        ret = clientGetMsgqueAttr(__msqid,__buf);
        if(-1 != ret ){
			printf("get message queue header success\n");
		}
		else{
			printf("get message queue header fail\n");
		}
	}
	else if( IPC_SET == __cmd){
        ret = clientSetMsgqueAttr(__msqid,__buf);
        if(-1 != ret ){
			printf("set message queue attr success\n");
		}
		else{
			printf("set message queue attr fail\n");
		}
	}
	else if( IPC_RMID == __cmd){
		printf("msgctl remove que\n");
        clientMsgCtrlRemv(__msqid);
	}
	else{
		printf("msgctl else\n");
	}

	return ret;
}
#endif
int msgget (key_t __key, int __msgflg)
{
	int ret = -1;
    bool bExist = false;


    //printf("so,msgget __msgflg =%x\n",__msgflg);
	if( IPC_PRIVATE == __key ){
        //printf("creat new message queue\n");
        ret = clientMsgGetCreate(__key,bExist);
        //printf("return queue id\n");
	}
	else{
		if( 0 == __msgflg){
            //printf("get queue id\n");
            ret = clientMsgGetNoCrte(__key,bExist);
            //printf("return queue id\n");
		}
        else if(__msgflg&IPC_CREAT){
            //printf("__msgflg&IPC_CREAT = %x\n",__msgflg&IPC_CREAT);
            if(__msgflg&IPC_EXCL){
                //printf("__msgflg&IPC_EXCL = %x\n",__msgflg&IPC_EXCL);
                ret = clientMsgGetCreate(__key,bExist);
                if(true == bExist)
                {
                    ret = -1;
                    //printf("que exist ,return error\n");
                }
            }
            else{
                ret = clientMsgGetCreate(__key,bExist);
            }
        }
		else{
            //printf("unknow error\n");
		}
	}

	return ret;
}
ssize_t msgrcv (int __msqid, void *__msgp, size_t __msgsz,
		       long int __msgtyp, int __msgflg)
{
	int ret = -1;
    printf("msgrcv __msgtyp=%ld __msgflg = %d\n",__msgtyp,__msgflg);
	if(0 == __msgflg){
		printf("recive msg, if there isn't a message which type is msgtyp,hold and wait the msg coming\n");
        ret = clientMsgRcvWaitBytype(__msqid,__msgtyp,__msgp,__msgsz,false);
	}
	else
	{
		if(IPC_NOWAIT&__msgflg){
			printf("recive msg, if que is empty, return immediately\n");
            if(MSG_EXCEPT& __msgflg){
				printf("recive msg, return the first msg which type is not msgtyp\n");
                if(MSG_NOERROR& __msgflg){
                    ret = clientMsgRcvNowaitExtype(__msqid,__msgtyp,__msgp,__msgsz,true);
                }
                else
                {
                    ret = clientMsgRcvNowaitExtype(__msqid,__msgtyp,__msgp,__msgsz,false);
                }

			}
			else{
                if(MSG_NOERROR& __msgflg){
                    ret = clientMsgRcvNowaitBytype(__msqid,__msgtyp,__msgp,__msgsz,true);
                }
                else
                {
                    ret = clientMsgRcvNowaitBytype(__msqid,__msgtyp,__msgp,__msgsz,false);
                }
			}
						
		}
		else
		{
            if(MSG_EXCEPT& __msgflg){
                if(MSG_NOERROR& __msgflg){
                    ret = clientMsgRcvWaitExtype(__msqid,__msgtyp,__msgp,__msgsz,true);
                }
                else
                {
                    ret = clientMsgRcvWaitExtype(__msqid,__msgtyp,__msgp,__msgsz,false);
                }
			}
			else{
                if(MSG_NOERROR& __msgflg){
                    ret = clientMsgRcvWaitBytype(__msqid,__msgtyp,__msgp,__msgsz,true);
                }
                else
                {
                    ret = clientMsgRcvWaitBytype(__msqid,__msgtyp,__msgp,__msgsz,false);
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
    printf("msgsnd __msgflg = %d\n",__msgflg);
	if(0 == __msgflg){
		printf("send msg, if que is full, hold and wait the msg can be send to que\n");
        ret = clientMsgSendWaitExt(__msqid,__msgp,__msgsz);
	}
	else{
		if(IPC_NOWAIT & __msgflg){
			printf("send msg, if que is full, return immediately\n");
            ret = clientMsgSendNoWaitExt(__msqid,__msgp,__msgsz);
		}
		else{
            ret = clientMsgSendWaitExt(__msqid,__msgp,__msgsz);
		}
	}
	return ret;
}

