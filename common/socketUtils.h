/**
 * @file WifiPandaUtils.h
 * @brief Declaration file of wifi utils.
 */

#ifndef WIFIPANDAUTILS_H
#define WIFIPANDAUTILS_H
//#include "TaskManagerDefine.h"
//#include "WifiDefineType.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <errno.h>



#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(exp) ({         \
    typeof (exp) _rc;                      \
    do {                                    \
        _rc = (exp);                       \
    } while (_rc == ((typeof (exp)) -1) && errno == EINTR); \
    _rc; })
#endif

#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif




size_t WifiStrLCpy(char *dest, const char *src, size_t siz);

/*********** Time relative *************/
typedef long wifi_time_t;

typedef struct 
{
    wifi_time_t sec;
    wifi_time_t usec;
}WifiRelTime;

int WifiGetRelTime(WifiRelTime *t);


static inline void WifiRelTimeSub(WifiRelTime *a,
                                       WifiRelTime *b,
                                       WifiRelTime *res)
{
    res->sec = a->sec - b->sec;
    res->usec = a->usec - b->usec;
    if (res->usec < 0) {
        res->sec--;
        res->usec += 1000000;
    }
}

static inline int WifiRelTimeExpired(WifiRelTime *now,
                     WifiRelTime *ts,
                     wifi_time_t timeout_secs)
{
    WifiRelTime age;
    WifiRelTimeSub(now, ts, &age);
    return (age.sec > timeout_secs) 
                || (age.sec == timeout_secs && age.usec > 0);
}

void WifiSleep(wifi_time_t sec, wifi_time_t usec);


#define MACADDR_LEN                 (6)


bool mac2String(uint8 *pout_mac, macaddr in_mac);
bool macstring2struct(macaddr *pout_mac, uint8* in_mac);
bool macint2struct(macaddr *pout_mac, uint64 in_mac);
uint64 macstring2uint64(uint8* pstrMac);
uint64 mac2uint64(macaddr in_mac);
uint8 int2asc(uint8 ch);
uint8 asc2int(uint8 ch);



#ifdef __cplusplus
}
#endif

#endif /* WIFIPANDAUTILS_H */
/* EOF */
