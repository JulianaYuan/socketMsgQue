/*server.c*/
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

#define MAX_TEXT 100

/*这个结构体是自己定义的，但是有一点通信双方都要一样*/
struct mybuf {
    long int type;
    char msg_text[MAX_TEXT];
};

int main()
{
	printf("main\n");
    int mid, ret;
	long int type;
    key_t key;
    char buf[MAX_TEXT];
    struct mybuf data;
	printf("main start\n");
   key = ftok("../", TEST);
	printf("ftok key=%x\n",key);
	key = ftok("../../", TEST1);
	printf("ftok key=%x\n",key);
    if(key == -1) {
        perror("ftok error");
        exit(1);
    }
    mid = msgget(key, IPC_CREAT|0777);
	printf("msgget\n");
    if(mid == -1) {
        perror("msgget error");
        exit(2);
    }
    
    while(1) {
		fputs("输入你要传送的数据类型:", stdout);
        setbuf(stdin, NULL);
        scanf("%ld", &type);
        data.type = type;
        fputs("输入要传送的字符数据:", stdout);
        //清空键盘缓冲区，在linux下getchar(),fflush(stdin),rewind(stdin)都不起作用
        setbuf(stdin, NULL);
        fgets(buf, MAX_TEXT, stdin);
        memcpy(data.msg_text, buf, MAX_TEXT);
        ret = msgsnd(mid, (void *)&data, MAX_TEXT, 0);
        if(ret == -1) {
            exit(3);
            perror("msgsnd error");
        }
        if(strncmp(data.msg_text, "exit", 4) == 0) {
            exit(0);
        }
		
		printf("loop,please input the type\n");
		scanf("%ld", &type);
        ret = msgrcv(mid, (void *)&data, MAX_TEXT, type, 0);
		printf("msgrcv:data.msg_text = %s , ret =%d\n",data.msg_text,ret);
        if(strncmp(data.msg_text, "exit", 4) == 0) {
            printf("error\n");
		break;
        }
        if(ret == -1) {
            exit(3);
            perror("msgrcv error");
        }
        printf("%ld    %s\n", data.type, data.msg_text);
    }
    msgctl(mid, IPC_RMID, (struct msqid_ds *)0);
    return 0;
}
