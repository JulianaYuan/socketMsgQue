#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include<errno.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXLINE 4096

typedef struct data_s
{
  int msqId;
  int msgType;
  char msg[MAXLINE];
}data,pdata;

int
main(int argc, char **argv)
{
	int					sockfd, n;
	char				recvline[MAXLINE + 1];
	struct sockaddr_in	servaddr;
    data dat;

	//if (argc != 2)
	//	err_quit("usage: a.out <IPaddress>");

	if ( (sockfd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0)) < 0)
		printf("socket error\n");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(8496);	/* daytime server */
	if (inet_pton(AF_INET, "127.0.0.1"/*argv[1]*/, &servaddr.sin_addr) <= 0)
		printf("inet_pton error for %s\n", argv[1]);

	if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
		printf("connect error\n");

#if 0
    while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
		recvline[n] = 0;	/* null terminate */
		if (fputs(recvline, stdout) == EOF)
			err_sys("fputs error");
	}
#endif
    printf("strlen(dat.msg)=%ld,sizeof(dat.msg)=%ld\n",strlen(dat.msg),sizeof(dat.msg));

	while((n = recv(sockfd, &dat, (sizeof(dat.msg)+sizeof(int)*2+1),0)) == -1) {  
	   } 


    /*while ( (n = read(sockfd, &dat, (sizeof(dat.msg)+sizeof(int)*2+1))) > 0) {

    }*/
    int msqid=0;
    //int msgLen = 0;
    int msgtyp = 0;
    char *msg=NULL;
    //char *p=recvline;
    msqid = dat.msqId;
    msgtyp = dat.msgType;
    msg = dat.msg;
    printf("strlen(msg) = %ld\n",strlen(msg));
    printf("recive buf:%d ,%d,%s\n",msqid,msgtyp,msg);
#if 0
    printf("recive buf:%s\n",p);
    while(*p!='\n')
    {
        char ch = *p;
        p++;
        msqid = (msqid)*10+ch-'0';
    }
    p++;
    /*for(int i;p[i]!='\n';i++)
    {
        printf("%c\n",p[i]);
        if(p[i]=='\n')break;
        msqid = msqid*10+(p[i]-'0');
    }*/
    msgLen = strlen(p);
    printf("msgLen=%d,p:%s\n",msgLen,p);
    msg = (char *)malloc(msgLen+1);
    snprintf(msg,msgLen+1,"%s",p);
    printf("msqid =%d,msg:%s",msqid,msg);
    free(msg);
    msg=NULL;
#endif
	if (n < 0)
		printf("read error\n");

	exit(0);
}
