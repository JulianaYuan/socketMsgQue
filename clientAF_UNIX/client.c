#include "../libsrc/remsg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TEST 'a'
#define TEST1 'b'

#define MAX_TEXT 8192//212994//8192//2147479552  //9612

/*这个结构体是自己定义的，但是有一点通信双方都要一样*/
struct tMsgText
{
    int bStart;
    unsigned char lvlLF;
    unsigned char lvlRF;
    unsigned char lvlMLR;
    unsigned char lvlLR;
    unsigned char lvlRR;
    unsigned char flash;
    unsigned char isShow;
    unsigned char lvlDistance;
    unsigned int steering_angle;
    unsigned char isAUXLineShow;
};

typedef struct stMsgBlock
{
    long id;
    struct tMsgText text;
}tMsgBlock;

typedef struct MsgBlock_s
{
    long id;
    char text[MAX_TEXT];
}MsgBlock;

int testGetQueId(int key_a)
{

    int mid;
    key_t key = key_a;


    //key = ftok("../", TEST);
    //printf("key =%x,IPC_PRIVATE = %x,IPC_CREAT=%x,IPC_EXCL=%x,IPC_CREAT|IPC_EXCL=%x\n",key,IPC_PRIVATE,IPC_CREAT,IPC_EXCL,IPC_CREAT|IPC_EXCL);
    //printf("MSGMAX=%d,MSGMNB=%d,MSGMNI=%d\n",MSGMAX,MSGMNB,MSGMNI);
    //key = ftok("../../", TEST1);
    //printf("key =%x\n",key);
    if(key == -1) {
        perror("ftok error");
        exit(1);
    }
    mid = msgget(key,IPC_EXCL);
    if(mid < 0){
        //printf("mid = %d\n",mid);
        mid = msgget(key, IPC_CREAT|0777);
        if(mid == -1) {
            //perror("msgget error");
            exit(2);
            }
    }
    return mid;
}

int testSendMsg(int mid)
{
    long int type;
    char buf[MAX_TEXT];
    MsgBlock data;
    int ret =-1;

    fputs("输入你要传送的数据类型:", stdout);
    setbuf(stdin, NULL);
    ret = scanf("%ld", &type);
    printf("ret = %d\n",ret);
    if(0 == ret)
    {
        ret == -1;
        setbuf(stdin, NULL);
        printf("wrong input\n");
        return ret;
    }
    data.id = type;
    fputs("输入要传送的字符数据:", stdout);
    //清空键盘缓冲区，在linux下getchar(),fflush(stdin),rewind(stdin)都不起作用
    fputs("data.text:", stdout);
    setbuf(stdin, NULL);
    int cnt = 0;
    while(cnt<(MAX_TEXT-1))
    {
        buf[cnt]= 'a';
        cnt++;
    }
    memcpy(data.text, buf, (MAX_TEXT-1));
    //printf("sizeof(struct tMsgText) = %ld\n",MAX_TEXT);
    ret = msgsnd(mid, (void *)&data, (MAX_TEXT-1), 0);
    if(ret == -1) {
        printf("msgsnd error\n");
    }
    return 0;
}

int testRecvMsg(int mid)
{
    long int type;
    MsgBlock data;
    int ret =-1;

    fputs("输入你要接收的数据类型:", stdout);
    scanf("%ld*", &type);
    ret = msgrcv(mid, (void *)&data, (MAX_TEXT-1), type, 0);
    printf("msgrcv:ret =%d data.text = %s\n",ret,data.text);
    /*printf("msgrcv:%d, %c, %c, %c, %c, %c, %c, %c, %c, %c, %d\n", data.text.bStart,\
                  data.text.flash,\
                  data.text.isAUXLineShow,\
                  data.text.isShow,\
                  data.text.lvlDistance,\
                  data.text.lvlLF,\
                  data.text.lvlLR,\
                  data.text.lvlMLR,\
                  data.text.lvlRF,\
                  data.text.lvlRR,\
                  data.text.steering_angle);*/
    return 0;
}
#if 0
int testRemoveQue(int mid)
{
    msgctl(mid,IPC_RMID,0); //删除消息队列
    return 0;
}
#endif

int main()
{
	int qid = -1;

	//communicate with server socket
	
	while(1)
	{
		int cmd = 0;
        printf("please enter cmd:\n[1]:get que id\n[2]: send msg to que; \n[3]: receive msg;\n");
        setbuf(stdin, NULL);
		scanf("%d",&cmd);
		switch(cmd)
		{
            case 1: //get msg que
                for(int i = 0;i<20;i++)
                {
                    qid = testGetQueId(1234+i);
                    sleep(1);
                }
				break;
			case 2://send msg to que
				{
					if(-1 == qid){printf("que is not exist or create fail!\n");break;}
                    testSendMsg(qid);
				}
				break;
			case 3://recieve msg from que
				{
					if(-1 == qid){printf("que is not exist or create fail!\n");break;}
                    testRecvMsg(qid);
				}
				break;
#if 0
            case 4://delete que
                testRemoveQue(qid);
                qid = -1;
				break;
#endif
			default:
				printf("wrong cmd\n");
				break;

		}
	}
	exit(0);
}

