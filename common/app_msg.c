/**************************************************************************
 *         Copyright(c) 2012 by iCatch Technology Co., Ltd.              *
 *                                                                        *
 *  This software is copyrighted by and is the property of iCatch Tech-  *
 *  nology Co., Ltd. All rights are reserved by iCatch Technology Co.,   *
 *  Ltd. This software may only be used in accordance with the            *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice "M U S T" not be removed or modified without    *
 *  prior written consent of iCatch Technology Co., Ltd.                 *
 *                                                                        *
 *  iCatch Technology Co., Ltd. reserves the right to modify this        *
 *  software without notice.                                              *
 *                                                                        *
 *  iCatch Technology Co., Ltd.                                          *
 *  19-1, Innovation First Road, Science-Based Industrial Park,           *
 *  Hsin-Chu, Taiwan, R.O.C.                                              *
 *                                                                        *
 *************************************************************************/

/**
 * @file		app_msg.c
 * @brief		App for host msg queue process
 * @author	GQ
 * @since	2012-12-11
 * @date		2012-12-31
 */

#define HOST_DBG 1

#include "app_com_def.h"
#include "sp5k_os_api.h"
#include "sp5k_global_api.h"
#include "app_key_def.h"
#include "app_msg.h"
#include "app_dbg_api.h"

#if MSG_NEW_PROC

/**************************************************************************
 *                          			 GLOBAL DATA                            *
 **************************************************************************/
static UINT32 queOverFlag = 0x00;  // bit0:keyque  bit1:UIque  bit2:kernelque  bit0~bit2:  1:overflow  0:not overflow 

/**************************************************************************
 *								M A C R O S 							  *
 **************************************************************************/ 
#define DEBUG_PRINT    0
	/*queue msg size:1 mean is 32bit,  support 1,2,4*/
#define APP_MSG_QUE_MSG_SIZE		1
	
	/*queSize:n*4*msgsize */
#define APP_MSG_KEY_QUE_SIZE		(16 * 4 * APP_MSG_QUE_MSG_SIZE)
#define APP_MSG_UI_QUE_SIZE			(16 * 4 * APP_MSG_QUE_MSG_SIZE)
#define APP_MSG_KERNEL_QUE_SIZE		(96 * 4 * APP_MSG_QUE_MSG_SIZE)
	
#define APP_MSG_PRI					15
#define APP_MSG_TIME_SLICE			0
#define APP_MSG_TX_AUTO_START		1
	
#define APP_MSG_QUE_CTRL_MAX		128
#define IS_APP_UI_MSG(msg)			(((msg)&0xff000000)==SP5K_MSGX_UI)

#define IS_QUE_OVERFLOW(queID)				(queOverFlag & (0x01 << queID))  
#define APP_MSG_QUE_OVERFLAG_SET(queID)		(queOverFlag |= (0x01<<queID))
#define APP_MSG_QUE_OVERFLAG_CLR(queID)		(queOverFlag &= ~(0x01<<queID))

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static TX_THREAD *ptrdAppMsg;

static TX_QUEUE queptrKey;
static TX_QUEUE queptrUI;
static TX_QUEUE queptrKernel;

static appMsgQue_t appMsgQue[APP_MSG_QUE_CTRL_MAX];
static appMsgQue_t *pMsgQueHeader=NULL;
static appMsgQue_t *pMsgQueFree=NULL;

static TX_SEMAPHORE msgCnt;

static TX_MUTEX appMsgHdlToken;  		//lock the operate with MSG add & free
static TX_MUTEX appMsgRcvLock;

/**************************************************************************
 *                      E X T E R N A L   R E F E R E N C E
 **************************************************************************/
//extern void ros_thread_sleep(UINT32 ms);
//extern UINT32 ros_queue_cnt_get(UINT32 queue_id, UINT32 *cnt, UINT32 *size);

/**************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S                   *
 **************************************************************************/
static UINT32 appMsgQueInit(void);
static appMsgQue_t* appMsgAddNew(UINT32 msg,UINT32 param);
static void appMsgFree(appMsgQue_t *pNeedFree);
static void appMsgProc(ULONG para);

void appMsgDebugPrint(void)
{
	UINT32 i,j=0;
	UINT32 cnt;

	sp5kOsSemaphoreCntGet(&msgCnt,&cnt);
	printf("MSG semaphore:%d\n",cnt);
	for(i=0;i<APP_MSG_QUE_CTRL_MAX;i++){
		if(appMsgQue[i].msg){
			j++;
			printf("appMsgQue[%3d]:%p,  0x%8x,  0x%2x,  0x%x\n",i,&appMsgQue[i],appMsgQue[i].msg,appMsgQue[i].param,appMsgQue[i].queID);
		}
	}
	if(!j){
		printf("appMsgQue is empty!\n");
	}
}

static UINT32 appMsgQueInit(void)
{	
	UINT32 ret = SUCCESS;
	UINT32 i;

	/* init linked list table */
	memset(&appMsgQue[0],0,sizeof(appMsgQue));

	for(i=0;i<APP_MSG_QUE_CTRL_MAX-1;i++){
		appMsgQue[i].next = &appMsgQue[i+1];
		appMsgQue[i].msg  = 0;
		appMsgQue[i].param= 0;
		appMsgQue[i].queID= -1;
		#if DEBUG_PRINT
		printf("appMsgQue[%d]:%p\n",i,&appMsgQue[i]);
		#endif
	}

	appMsgQue[APP_MSG_QUE_CTRL_MAX-1].next = NULL;
	appMsgQue[APP_MSG_QUE_CTRL_MAX-1].msg  = 0;
	appMsgQue[APP_MSG_QUE_CTRL_MAX-1].param= 0;
	appMsgQue[APP_MSG_QUE_CTRL_MAX-1].queID= -1;

	pMsgQueHeader = NULL;
	pMsgQueFree = &appMsgQue[0];

	ret = sp5kOsQueueCreate(&queptrKernel,"appKernelMsg",APP_MSG_QUE_MSG_SIZE,NULL,APP_MSG_KERNEL_QUE_SIZE);
	if(ret == FAIL){
		printf("ERR:kernel que create fail!\n");
		return ret;
	}
	
	ret = sp5kOsQueueCreate(&queptrUI,"appUIMsg",APP_MSG_QUE_MSG_SIZE,NULL,APP_MSG_UI_QUE_SIZE);
	if(ret == FAIL){
		printf("ERR:UI que create fail!\n");
		return ret;
	}
	
	ret = sp5kOsQueueCreate(&queptrKey,"appKeyMsg",APP_MSG_QUE_MSG_SIZE,NULL,APP_MSG_KEY_QUE_SIZE);
	if(ret == FAIL){
		printf("ERR:key que create fail!\n");
		return ret;
	}
	
	ret = sp5kOsSemaphoreCreate(&msgCnt,"appMsgCount",0);
	if(ret == FAIL){
		printf("ERR:msg flag semaphore create fail!\n");
		return ret;
	}

	sp5kOsMutexCreate(&appMsgHdlToken,"appMsgMutex",0);
	if(!appMsgHdlToken){
		printf("ERR:app msg mutex create fail\n");
		ret = FAIL;
	}

	sp5kOsMutexCreate(&appMsgRcvLock,"appMsgFlushMutex",0);
	if(!appMsgRcvLock){
		printf("ERR:app msg flush mutex create fail\n");
		ret = FAIL;
	}
	
	return ret;
}

static appMsgQue_t* appMsgAddNew(UINT32 msg,UINT32 param)
{
	appMsgQue_t *pCur;
	UINT32 err;

	err = sp5kOsMutexGet(&appMsgHdlToken,TX_WAIT_FOREVER);
	DBG_ASSERT(err == SUCCESS);

	if(!pMsgQueFree){
		#if DEBUG_PRINT
		printf("no free space\n");
		#endif
		return NULL;
	}

	if(!pMsgQueHeader){
		pMsgQueHeader = pMsgQueFree;
		pMsgQueFree = pMsgQueFree->next;
		pMsgQueHeader->next = NULL;
		pMsgQueHeader->msg = msg;
		pMsgQueHeader->param=param;
		pCur = pMsgQueHeader;
	}else{
		pCur = pMsgQueHeader;
		while(pCur->next){
			/* pointer to the end */
			pCur = pCur->next;
		}

		pCur->next = pMsgQueFree;
		pCur = pCur->next;
		pMsgQueFree = pMsgQueFree->next;
		pCur->next = NULL;
		pCur->msg = msg;
		pCur->param=param;
	}

	sp5kOsMutexPut(&appMsgHdlToken);
	return pCur;
}

static void appMsgFree(appMsgQue_t *pNeedFree)
{
	appMsgQue_t *pCur;
	appMsgQue_t *pPre;
	UINT32 err;
	
	err = sp5kOsMutexGet(&appMsgHdlToken,TX_WAIT_FOREVER);
	DBG_ASSERT(err == SUCCESS);
	
	pCur=pMsgQueHeader;
	pPre=NULL;
	
	if(pCur == pNeedFree){
		pMsgQueHeader = pCur->next;
		pCur->next = pMsgQueFree;
		pMsgQueFree = pCur;
	}else{
		while(pCur){
			if(pCur == pNeedFree){
				break;
			}
			pPre = pCur;
			pCur = pCur->next;
		}
		pPre->next = pCur->next;
		pCur->next = pMsgQueFree;
		pMsgQueFree = pCur; 
	}
	
	pMsgQueFree->msg = 0;
	pMsgQueFree->param=0;
	pMsgQueFree->queID=-1;	
	
	sp5kOsMutexPut(&appMsgHdlToken);
}

/* it is allowed use in sp5kJobDo only*/
UINT32 appMsgReceive(UINT32 *cmd,UINT32 *param)
{
	UINT32 ret = FAIL;
	UINT32 cnt,size;
	appMsgQue_t *pTmp;
	TX_QUEUE que;

	ret = sp5kOsSemaphoreGet(&msgCnt,TX_WAIT_FOREVER);
	DBG_ASSERT(ret == SUCCESS);

	if(!pMsgQueHeader){
		#if DEBUG_PRINT
		appMsgDebugPrint();
		printf("pHeader:%p\n",pMsgQueHeader);
		printf("pFree:%p\n",pMsgQueFree);
		#endif
		return FAIL;
	}

	if(ret == SUCCESS){			
		*cmd = pMsgQueHeader->msg;
		*param=pMsgQueHeader->param;

		switch(pMsgQueHeader->queID){
			case APP_MSG_KEY_QUE:
				que = queptrKey;
				break;
			case APP_MSG_UI_QUE:
				que = queptrUI;
				break;
			case APP_MSG_KERNEL_QUE:
				que = queptrKernel;
				break;
			default:
				return FAIL;
		}
		ret = sp5kOsQueueReceive(&que,&pTmp,TX_WAIT_FOREVER);
		DBG_ASSERT(ret == SUCCESS);

        sp5kOsQueueCntGet(&que,&cnt,&size);

		if(size && IS_QUE_OVERFLOW(pMsgQueHeader->queID)){
			APP_MSG_QUE_OVERFLAG_CLR(pMsgQueHeader->queID);
		}
		
		#if DEBUG_PRINT
		profLogPrintf(0,"rcv msg:0x%x param:0x%x",*cmd,*param);
		#endif
	
		appMsgFree(pMsgQueHeader);
		ret = SUCCESS;
	}
	return ret;
}

UINT32 appMsgFlush(UINT32 queID)
{
	UINT32 err;
	TX_QUEUE que;
	appMsgQue_t *toFree=NULL;
	
	switch(queID){
		case APP_MSG_KEY_QUE:
			que = queptrKey;
			break;
		case APP_MSG_UI_QUE:
			que = queptrUI;
			break;
		case APP_MSG_KERNEL_QUE:
			que = queptrKernel;
			break;
		default:
			return FAIL;
	}
	
	err = sp5kOsMutexGet(&appMsgRcvLock,TX_WAIT_FOREVER);
	DBG_ASSERT(err == SUCCESS);
	
	while(1){
		if(sp5kOsQueueReceive(&que,&toFree,TX_NO_WAIT) == SUCCESS){
			err = sp5kOsSemaphoreGet(&msgCnt,TX_WAIT_FOREVER);
			DBG_ASSERT(err == SUCCESS);
			if(toFree){
				appMsgFree(toFree);
			}
		}else{
			#if DEBUG_PRINT
			printf("queue is empty\n");
			#endif
			break;
		}
	}
	sp5kOsMutexPut(&appMsgRcvLock);
	return SUCCESS;
}

UINT32 appMsgSeek(UINT32 waitMsg,UINT32 *pParam)
{
	UINT32 ret;
	appMsgQue_t *pRet = NULL;

	ret = appMsgSeekExt(waitMsg,-1,NULL,&pRet);
	if(pParam && pRet){
		*pParam = pRet->param;
	}
	return ret;
}

#define REFER_OLD_PROC	0    /* determines whether to need to clean up the waitMsg  */
UINT32 appMsgSeekExt(UINT32 waitMsg,UINT32 waitParam,appMsgQue_t *pPre,appMsgQue_t** pRet)
{
	appMsgQue_t *pCur;
	UINT32 ret;
	#if REFER_OLD_PROC
	UINT32 signal;
	#endif
	
	ret = sp5kOsMutexGet(&appMsgHdlToken,TX_WAIT_FOREVER);
	DBG_ASSERT(ret == SUCCESS);

	pCur=pMsgQueHeader;
	ret = FAIL;
	
	#if DEBUG_PRINT
	printf("MsgWait:%x start\n", waitMsg);
	#endif
	
	if(pPre){
		pCur = pPre;
	}
	
	while(pCur){
		#if DEBUG_PRINT
		printf("pSeek:%p  msg:0x%x\n",pCur,pCur->msg);
		#endif

		*pRet= pCur;
		
		#if REFER_OLD_PROC
		TX_QUEUE que;
		appMsgQue_t *pTmp=NULL;
		
		switch(pCur->queID){
			case APP_MSG_KEY_QUE:
				que = queptrKey;
				break;
			case APP_MSG_UI_QUE:
				que = queptrUI;
				break;
			case APP_MSG_KERNEL_QUE:
				que = queptrKernel;
				break;
			default:
				return FAIL;
		}
		#endif
			
		if( waitMsg == pCur->msg ){
			if(waitParam != (UINT32)-1){
				if( waitParam == pCur->param ){
					ret = SUCCESS;
				}
			}else{
				ret = SUCCESS;

			}
			#if REFER_OLD_PROC
			sp5kOsSemaphoreGet(&msgCnt,TX_WAIT_FOREVER);
			sp5kOsQueueReceive(&que,&pTmp,TX_WAIT_FOREVER);
			appMsgFree(pCur);
			#endif
			
			if(ret == SUCCESS){
				break;
			}
		}

		if (waitMsg==SP5K_MSGX_KEY && IS_SP5K_KEY_PRESS_MSG(pCur->msg, pCur->param)) {
			ret = SUCCESS;
			
			#if REFER_OLD_PROC
			sp5kOsSemaphoreGet(&msgCnt,TX_WAIT_FOREVER);
			sp5kOsQueueReceive(&que,&pTmp,TX_WAIT_FOREVER);
			appMsgFree(pCur);
			#endif
			
			break;
		}
		pCur = pCur->next;	
	}
	
	#if DEBUG_PRINT
	printf("MsgWait:%x %s\n", waitMsg, ret?"FAIL":"SUCCESS");
	#endif
	sp5kOsMutexPut(&appMsgHdlToken);
	return ret;
}

static void appMsgProc(ULONG para)
{
	UINT32 msgId=0, param=0;
	UINT32 err;
	UINT32 cnt,size,queID;
	appMsgQue_t *pCur;
	TX_QUEUE que;
	
	para=para;
	
	while(1){
		if(sp5kHostMsgReceive(&msgId, &param) == SUCCESS){
			err = sp5kOsMutexGet(&appMsgRcvLock,TX_WAIT_FOREVER);
			DBG_ASSERT(err == SUCCESS);
			
			if(IS_SP5K_BTN_MSG(msgId)){
				que=queptrKey;
				queID= APP_MSG_KEY_QUE;
			}else if(IS_APP_UI_MSG(msgId)){
				que=queptrUI;
				queID= APP_MSG_UI_QUE;
			}else{
				que=queptrKernel;
				queID= APP_MSG_KERNEL_QUE;
			}

			if(IS_QUE_OVERFLOW(queID)){
				printf("flag:0x%x\n",queOverFlag);
				printf("%s is full!!!\n",(queOverFlag & 0x01)?"APP_MSG_KEY_QUE":
						(queOverFlag & 0x02)?"APP_MSG_UI_QUE":"APP_MSG_KERNEL_QUE");
				pCur = NULL;
			}else{
				pCur = appMsgAddNew(msgId,param);
			}
			
			if(pCur){
				pCur->queID= queID;
			
				#if DEBUG_PRINT
				profLogPrintf(0,"add msg:0x%x  param:0x%x",msgId,param);
				#endif

				err = sp5kOsQueueSend(&que,&pCur,TX_NO_WAIT);
				sp5kOsQueueCntGet(&que,&cnt,&size);
				if(!size){
					APP_MSG_QUE_OVERFLAG_SET(queID);
				}
				
				if(err == SUCCESS){
					if(sp5kOsSemaphorePut(&msgCnt) == FAIL){
						profLogAdd(0,"semaphore put ERR");
					}
				}
			}	
			sp5kOsMutexPut(&appMsgRcvLock);
		}
	}
}

void appMsgInit(void)
{	
	if(appMsgQueInit() == SUCCESS){
		profLogAdd(0,"appMsg thread create!");
		ptrdAppMsg = sp5kOsThreadCreate(
						"HostMsgPro",
						appMsgProc,
						0,
						APP_MSG_PRI,
						APP_MSG_PRI,
						APP_MSG_TIME_SLICE,
						APP_MSG_TX_AUTO_START);	
	}
}

#endif

