/*client.c*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
//#include "../common/remsg.h"

#define TEST 'a'
#define TEST1 'b'

#define MAX_TEXT 101

/*这个结构体是自己定义的，但是有一点通信双方都要一样*/
struct MsgText
{
    char i;
//    int j;
//    int m;
//    int n;
//    char text[MAX_TEXT];
//    char text1[MAX_TEXT];
//    char text2[MAX_TEXT];
//    char text3[MAX_TEXT];
//    char text4[MAX_TEXT];
//    char text5[MAX_TEXT];
//    char text6[MAX_TEXT];
//    char text7[MAX_TEXT];
//    char text8[MAX_TEXT];
//    char text9[MAX_TEXT];
};

typedef struct MsgBlock_s
{
    long id;
    struct MsgText text;//char text[MAX_TEXT];
}MsgBlock;

int testGetQueId(int key_a)
{

    int mid;
    key_t key = key_a;


    //key = ftok("../", TEST);
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
            //exit(2);
            }
    }
    return mid;
}

int testSendMsg(int mid, int i,int j)
{
    MsgBlock data;
    //char buf[MAX_TEXT];
    int ret =-1;

    //printf("testSendMsg\n");
    data.id = (long int)j;
    data.text.i=i;
//    data.text.j=j;
 //   data.text.m = i;
//    data.text.n = j;
//    sprintf(buf,"aaaaaaaaaabbbbbbbbbbcccccccccccdddddddddddeeeeeeeeeeffffffffffgggggggggghhhhhhhhhhhiiiiiiiiiiijjjjjjjjjj");
//    memcpy(data.text.text, buf, MAX_TEXT);
//    memcpy(data.text.text1, buf, MAX_TEXT);
//    memcpy(data.text.text2, buf, MAX_TEXT);
//    memcpy(data.text.text3, buf, MAX_TEXT);
//    memcpy(data.text.text4, buf, MAX_TEXT);
//    memcpy(data.text.text5, buf, MAX_TEXT);
//    memcpy(data.text.text6, buf, MAX_TEXT);
//    memcpy(data.text.text7, buf, MAX_TEXT);
//    memcpy(data.text.text8, buf, MAX_TEXT);
//    memcpy(data.text.text9, buf, MAX_TEXT);
    printf("sizeof(struct tMsgText) = %ld\n",sizeof(struct MsgText));
    ret = msgsnd(mid, (void *)&data, sizeof(struct MsgText), 0);
    if(ret == -1) {
        printf("msgsnd error\n");
    }
    return 0;
}

int testRecvMsg(int mid,int typ)
{
    long int type;
    MsgBlock data;
    int ret =-1;
    printf("testRecvMsg\n");
    type = typ;
    ret = msgrcv(mid, (void *)&data, MAX_TEXT, type, IPC_NOWAIT);
    //printf("msgrcv:ret =%d data.text = (%d,%d))\n",ret,data.text.i,data.text.j);
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
    int i=0,j=0;
    //communicate with server socket

    while(1)
    {
        int cmd = 0;
        printf("please enter cmd:\n[1]:get que id\n[2]: send msg to que; \n[3]: receive msg;\n");
        setbuf(stdin, NULL);
        scanf("%d",&cmd);
        printf("while %d\n",cmd);
        switch(cmd)
        {
            case 1: //get msg que
                i=0;
                while(1)
                {
                    qid = testGetQueId(1235+i);
                    printf("i=%d,qid=%d\n",i,qid);
                    if(-1!=qid)
                    {
                        testSendMsg(qid,1,1);
                    }
                    else
                    {
                        break;
                    }
                    i++;
                }
                j=i;
                i=0;
                while(i<j)
                {
                    qid = testGetQueId(1235+i);
                    printf("i=%d,qid=%d\n",i,qid);
                    msgctl(qid, IPC_RMID, (struct msqid_ds *)0);
                    i++;
                }
                break;
            case 2://send msg to que
                {
                    printf("send msg to que\n");
                    if(-1 == qid){printf("que is not exist or create fail!\n");break;}

                    while(1)
                    {
                        j = i%100+1;
                        printf("testSendMsg i=%d\n",i);
                        testSendMsg(qid,i,j);
                        i++;
                        usleep(10);
                    }
                    #if 0
                    for(i=1;i<3;i++)
                    {
                        printf("testSendMsg,%d,%d\n",i,j);
                        for(j=1;j<3;j++)
                        {
                            printf("testSendMsg,%d,%d\n",i,j);
                            testSendMsg(qid,i,j);
                            usleep(500*1000);
                            printf("testSendMsg end\n");
                        }
                    }
                    #endif
                }
                break;
                case 3://recieve msg from que
                {
                    printf("recieve msg from que\n");
                    if(-1 == qid){printf("que is not exist or create fail!\n");break;}
                    int m,n;
                    for(m=1;m<3;m++)
                    {
                        printf("testRecvMsg,%d,%d\n",m,n);
                        for(n=1;n<3;n++)
                        {
                            printf("testRecvMsg,%d,%d\n",m,n);
                            testRecvMsg(qid,m);
                            usleep(500*1000);
                            printf("testRecvMsg end\n");
                        }
                    }

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
