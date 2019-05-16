#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <stddef.h>
#include <unistd.h>

#include "socketUtils.h"

/*********** String relative **************************************************/
size_t WifiStrLCpy(char *dest, const char *src, size_t siz)
{
    const char *s = src;
    size_t left = siz;

    if (left) 
    {
        /* Copy string up to the maximum size of the dest buffer */
        while (--left != 0) 
        {
            if ((*dest++ = *s++) == '\0')
                break;
        }
    }

    if (left == 0) 
    {
        /* Not enough room for the string; force NUL-termination */
        if (siz != 0)
            *dest = '\0';
        while (*s++)
            ; /* determine total src string length */
    }

    return s - src - 1;
}



/*********** Time relative ****************************************************/
int WifiGetRelTime(WifiRelTime *t)
{
    static clockid_t clock_id = CLOCK_MONOTONIC;
    struct timespec ts;
    int res;

    while (1) 
    {
        res = clock_gettime(clock_id, &ts);
        if (res == 0) 
        {
            t->sec = ts.tv_sec;
            t->usec = ts.tv_nsec / 1000;
            return 0;
        }
        switch (clock_id) 
        {
            case CLOCK_MONOTONIC:
            {
                clock_id = CLOCK_REALTIME;
                break;
            }    
            case CLOCK_REALTIME:
            {
                return -1;
            }
        }
    }
}

void WifiSleep(wifi_time_t sec, wifi_time_t usec)
{
    if (sec)
        sleep(sec);
    if (usec)
        usleep(usec);
}

uint8 asc2int(uint8 ch)
{
    uint8 out = 0;

    if(ch >= '0' && ch <= '9')
        out = ch - '0';
    else if(ch >= 'a' && ch <= 'z')
        out = ch - 'a' + 0xa;
    else if(ch >= 'A' && ch <= 'Z')
        out = ch - 'A' + 0xa;
    else{
        // error
    }
    return out;
}

uint8 int2asc(uint8 ch)
{
    uint8 ret_ch = 0xFF;

    if(ch <= 9)
    {
        ret_ch = ch + '0';
    }
    else if(ch >= 0xa && ch <= 0xf)
    {
        ret_ch = (ch - 0xa) + 'A';
    }
    else
    {
        // LOGE("%s, line=%d\n", __FUNCTION__, __LINE__);
        return 0xFF;
    }
    return ret_ch;
}


// transfer from {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}
// to "00:11:22:33:44:55"
bool mac2String(uint8 *pout_mac, macaddr in_mac)
{
    int8 i,j;
    uint8 *pin_mac = (uint8*)in_mac;

    if((pin_mac[0] + pin_mac[1] + pin_mac[2] +
        pin_mac[3] + pin_mac[4] + pin_mac[5]) == 0)
    {
        // LOGE("%s, line=%d\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if(NULL == pout_mac)
    {
        // LOGE("%s, line=%d\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    j = 0;
    for(i=0; i < MACADDR_LEN; i++)
    {
        pout_mac[j++] = int2asc(pin_mac[i] >> 4);
        pout_mac[j++] = int2asc(pin_mac[i]&0xF);
        if(i == (MACADDR_LEN - 1))
        {
            break;
        }
        pout_mac[j++] = ':';
    }
    pout_mac[j] = 0;
    if(i != (MACADDR_LEN - 1))
    {
        return FALSE;
    }
    return TRUE;
}

// 12:34:56:78:90:ab => {0x12, 0x34, 0x56, 0x78, 0x90, 0xab}
bool macstring2struct(macaddr *pout_mac, uint8* in_mac)
{
    uint8 i,j = 0, ch;
    uint8 *pout = (uint8*)&pout_mac[0];
    uint8 len = strlen((char*)in_mac);
    int8 temp;

    if(len > 17)
        len = 17;
    if(pout_mac == NULL || in_mac == NULL)
        return FALSE;
    for(i=0; i < len; i+=3){
        ch = asc2int(in_mac[i]);
        ch <<= 4;
        ch |= asc2int(in_mac[i+1]);
        pout[j++] = ch;
    }

    return TRUE;
}

// 12:34:56:78:90:ab => 0x1234567890ab
uint64 macstring2uint64(uint8* pstrMac)
{
    macaddr mac;

    macstring2struct(&mac, pstrMac);
    return mac2uint64(mac);
}

// transfer uint64 mac  from 0x1234567890ab to  {0x12, 0x34, 0x56,0x78,0x90,0xab}
bool macint2struct(macaddr *pout_mac, uint64 in_mac)
{
    uint8 i;
    uint8 *pout = (uint8*)&pout_mac[0];
    uint64 temp = in_mac;

    if (in_mac == 0) {
        return FALSE;
    }
    for (i = 0; i < MACADDR_LEN; i++) {
        pout[MACADDR_LEN - 1 - i] = (temp & 0xFF);
        temp >>= 8;
    }
    return TRUE;
}

// macaddr => uint64
// {0x12, 0x34, 0x56,0x78,0x90,0xab} => 0x
uint64 mac2uint64(macaddr in_mac)
{
    uint64 out_mac = 0;
    uint8 i;

    for (i = 0; i < MACADDR_LEN; i++) {
        out_mac <<= 8;
        out_mac |= (in_mac[i] & 0xFF);
    }
    return out_mac;
}



#ifdef __cplusplus
}

#endif

