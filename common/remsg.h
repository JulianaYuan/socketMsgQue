#ifndef _MSG_QUE_H_
#define _MSG_QUE_H_
#include <sys/types.h>
#include <sys/ipc.h>
//#include "msq.h"
#include "remsgsocket.h"
#ifdef   __cplusplus
//extern   "C"   {
#endif


//#define IPC_PRIVATE  0

//#define IPC_STAT 1
//#define IPC_SET 2
//#define IPC_RMID 3

//#define IPC_NOWAIT  (0x01<0)
#define IPC_EXCEPT  (0x01<3)
#define IPC_NOERROR (0x01<4)


#if 1
int msgctl (int __msqid, int __cmd, struct msqid_ds *__buf);
int msgget (key_t __key, int __msgflg);
ssize_t msgrcv (int __msqid, void *__msgp, size_t __msgsz,
		       long int __msgtyp, int __msgflg);
int msgsnd (int __msqid, const void *__msgp, size_t __msgsz,
		   int __msgflg);
#endif

#ifdef   __cplusplus
//}
#endif

#endif //_MSG_QUE_H_
