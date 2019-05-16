#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
	int					listenfd, connfd;
	struct sockaddr_in	servaddr;
	char				buff[MAXLINE];
	time_t				ticks;

    if (argc != 3)
		printf("input the msg you want send\n");
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(8496);	/* daytime server */

	bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

	listen(listenfd, 1024);

		
	for ( ; ; ) {
		connfd = accept(listenfd, (struct sockaddr*) NULL, NULL);

        //ticks = time(NULL);
        //snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        int msgid = 0;
        char *p = argv[1];
        printf("%s,%s\n",p,argv[2]);
        while(*p!='\0')
        {
            msgid = msgid*10+(*p-'0');
            p++;
        }
        /*for(int i;p[i]!='\0';i++)
        {
            msgid = msgid*10+(p[i]-'0');
        }*/
        printf("msgid=%d,msg:%s\n",msgid,argv[2]);
        //snprintf(buff, sizeof(buff),"%010d\n%s", msgid,argv[2]);
        printf("send buf:%s\n",buff);
        data msgdat;
        msgdat.msgType = 134;
        msgdat.msqId = 2;
        snprintf(msgdat.msg, (strlen(argv[2])+1),"%s", argv[2]);
        printf("send buf:%d ,%d,%s\n",msgdat.msqId,msgdat.msgType,msgdat.msg);
        printf("%ld,%ld\n",(strlen(argv[2])+1),(strlen(msgdat.msg)+sizeof(int)*2+1));
        send(connfd, &msgdat, (strlen(msgdat.msg)+sizeof(int)*2+1),0);
	}
	close(connfd);
}
