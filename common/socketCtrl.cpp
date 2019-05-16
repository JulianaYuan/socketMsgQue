/**
 * @file socketCtrl.c
 * @brief
 */ 


#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "socketCtrl.h"
#include "socketUtils.h"


/**
 * struct wpa_ctrl - Internal structure for control interface library
 *
 * This structure is used by the wpa_supplicant/hostapd control interface
 * library to store internal data. Programs using the library should not touch
 * this data directly. They can only use the pointer to the data structure as
 * an identifier for the control interface connection and use this as one of
 * the arguments for most of the control interface library functions.
 */
struct wpa_ctrl 
{
    int s;
    struct sockaddr_un local;
    struct sockaddr_un dest;
};



struct wpa_ctrl * wpa_ctrl_open(const char *ctrl_path)
{
    struct wpa_ctrl *ctrl;
    static int counter = 0;
    int ret;
    int tries = 0;
    int flags;

    if (ctrl_path == NULL) {
        return NULL;
    }

    ctrl = (struct wpa_ctrl *)malloc(sizeof(*ctrl));
    if (ctrl == NULL) {
        return NULL;
    }
    memset(ctrl, 0, sizeof(*ctrl));

    ctrl->s = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (ctrl->s < 0) {
        free(ctrl);
        ctrl = NULL;
        return NULL;
    }

    ctrl->local.sun_family = AF_UNIX;
    counter++;
try_again:
    ret = snprintf(ctrl->local.sun_path, sizeof(ctrl->local.sun_path),
            CONFIG_CTRL_IFACE_CLIENT_DIR "/"
              CONFIG_CTRL_IFACE_CLIENT_PREFIX "%d-%d",
              (int) getpid(), counter);
    printf("[wxh] wpa_ctrl_open tries:%d ctrl->local.sun_path :%s\n",tries, ctrl->local.sun_path);          
    if (ret < 0 || (size_t) ret >= sizeof(ctrl->local.sun_path)) {
        close(ctrl->s);
        free(ctrl);
        ctrl = NULL;
        return NULL;
    }
    tries++;
    if (bind(ctrl->s, (struct sockaddr *) &ctrl->local,
                        sizeof(ctrl->local)) < 0) {
        if (errno == EADDRINUSE && tries < 2) {
            unlink(ctrl->local.sun_path);
            goto try_again;
        }
        close(ctrl->s);
        free(ctrl);
        ctrl = NULL;
        return NULL;
    }

    ctrl->dest.sun_family = AF_UNIX;
    if (strncmp(ctrl_path, "@abstract:", 10) == 0) {
        ctrl->dest.sun_path[0] = '\0';
        WifiStrLCpy(ctrl->dest.sun_path + 1, ctrl_path + 10,
               sizeof(ctrl->dest.sun_path) - 1);
		printf("[wxh] wpa_ctrl_open 1 ctrl->dest.sun_path :%s\n", ctrl->dest.sun_path); 
    }
    else {
        size_t res = WifiStrLCpy(ctrl->dest.sun_path, ctrl_path,
                             sizeof(ctrl->dest.sun_path));
		printf("[wxh] wpa_ctrl_open 2 ctrl->dest.sun_path :%s  res:%d   sizeof(ctrl->dest.sun_path):%d\n", ctrl->dest.sun_path, res, sizeof(ctrl->dest.sun_path)); 
        if (res >= sizeof(ctrl->dest.sun_path)) {
            close(ctrl->s);
            free(ctrl);
            ctrl = NULL;
		printf("[wxh] wpa_ctrl_open 2\n"); 
            return NULL;
        }
		printf("[wxh] wpa_ctrl_open 3\n");
    }
    
    if (connect(ctrl->s, (struct sockaddr *) &ctrl->dest,
                sizeof(ctrl->dest)) < 0) {
        close(ctrl->s);
        unlink(ctrl->local.sun_path);
        free(ctrl);
        ctrl = NULL;
		printf("[wxh] wpa_ctrl_open 4\n");
        return NULL;
    }
   printf("[wxh] wpa_ctrl_open 5\n");
    /*
     * Make socket non-blocking so that we don't hang forever if
     * target dies unexpectedly.
     */
    flags = fcntl(ctrl->s, F_GETFL);
    if (flags >= 0) {
        flags |= O_NONBLOCK;
        if (fcntl(ctrl->s, F_SETFL, flags) < 0) {
            perror("fcntl(ctrl->s, O_NONBLOCK)");
        }
    }
	LOGI("ctrl->s=%d,ctrl->dest.sun_path=%s,ctrl->local.sun_path=%s",\
	ctrl->s,ctrl->dest.sun_path,ctrl->local.sun_path);

    return ctrl;
}



void wpa_ctrl_close(struct wpa_ctrl *ctrl)
{
    if (ctrl == NULL) {
        return;
    }    
    
    unlink(ctrl->local.sun_path);
    if (ctrl->s >= 0) {
        close(ctrl->s);
    }
    free(ctrl);
}

int wpa_ctrl_request(struct wpa_ctrl *ctrl, const char *cmd, 
                            size_t cmd_len, char *reply, size_t *reply_len,
                            void(*msg_cb)(char *msg, size_t len))
{
    struct timeval tv;  
    WifiRelTime started_at;
    int res;
	int fd_ctrl=-1;
    fd_set rfds;
    const char *_cmd;
    char *cmd_buf = NULL;
    size_t _cmd_len;

    _cmd = cmd;
    _cmd_len = cmd_len;

    errno = 0;
    started_at.sec = 0;
    started_at.usec = 0;

	if(ctrl == NULL)
	{
		return -1;
	}
	else
	{
		fd_ctrl = ctrl->s;
		LOGI("ctrl->s=%d,ctrl->dest.sun_path=%s,ctrl->local.sun_path=%s,reply(addr)=%x,reply_len(addr) = %x",\
		ctrl->s,ctrl->dest.sun_path,ctrl->local.sun_path,reply,reply_len);
	}
		
retry_send:
    if (send(fd_ctrl, _cmd, _cmd_len, 0) < 0) {
        if (errno == EAGAIN || errno == EBUSY || errno == EWOULDBLOCK) {
            /*
             * Must be a non-blocking socket... Try for a bit
             * longer before giving up.
             */
            if (started_at.sec == 0) {
                WifiGetRelTime(&started_at);
            }
            else {
                WifiRelTime n;
                WifiGetRelTime(&n);
                /* Try for a few seconds. */
                if (WifiRelTimeExpired(&n, &started_at, 5)) {
                    goto send_err;
                }
            }
            WifiSleep(1, 0);
            goto retry_send;
    }
    send_err:
        free(cmd_buf);
        cmd_buf = NULL;
		LOGE("send_err");
        return -1;
    }
    free(cmd_buf);
    cmd_buf = NULL;

    for (;;) {
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(fd_ctrl, &rfds);
        res = select(fd_ctrl + 1, &rfds, NULL, NULL, &tv);
        if (res < 0) {
			LOGE("select_err");
            return res;
        }    
        if (FD_ISSET(fd_ctrl, &rfds)) {
            res = recv(fd_ctrl, reply, *reply_len, 0);
            if (res < 0) {
				LOGE("recv_err errno=%d",errno);
                return res;
            }
			LOGI("res = %d,reply = %s",res,reply);
            if (res > 0 && reply[0] == '<') {
                /* This is an unsolicited message from
                 * wpa_supplicant, not the reply to the
                 * request. Use msg_cb to report this to the
                 * caller. */
                if (msg_cb) {
                    /* Make sure the message is nul
                     * terminated. */
                    if ((size_t) res == *reply_len) {
                        res = (*reply_len) - 1;
                    }
                    reply[res] = '\0';
                    msg_cb(reply, res);
                }
                continue;
            }
            *reply_len = res;
            break;
        }
        else {
            return -2;
        }
    }
    return 0;
}


static int wpa_ctrl_attach_helper(struct wpa_ctrl *ctrl, int attach)
{
    char buf[10];
    int ret;
    size_t len = 10;

    ret = wpa_ctrl_request(ctrl, attach ? "ATTACH" : "DETACH", 6,
                   buf, &len, NULL);
    if (ret < 0) {
        return ret;
    }    
    if (len == 3 && strncmp(buf, "OK\n", 3) == 0) {
        return 0;
    }    
    return -1;
}


int wpa_ctrl_attach(struct wpa_ctrl *ctrl)
{
    return wpa_ctrl_attach_helper(ctrl, 1);
}


int wpa_ctrl_detach(struct wpa_ctrl *ctrl)
{
    return wpa_ctrl_attach_helper(ctrl, 0);
}


int wpa_ctrl_recv(struct wpa_ctrl *ctrl, char *reply, size_t *reply_len)
{
    int res;
	int fd = -1;
	if(ctrl != NULL)
	{
		fd = ctrl->s;
	}
	else
	{
		return -2;
	}
    res = recv(fd, reply, *reply_len, 0);
    if (res < 0) {
        return res;
    }
    
    *reply_len = res;
    return 0;
}


int wpa_ctrl_pending(struct wpa_ctrl *ctrl)
{
    struct timeval tv;
    fd_set rfds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(ctrl->s, &rfds);
    select(ctrl->s + 1, &rfds, NULL, NULL, &tv);
    return FD_ISSET(ctrl->s, &rfds);
}


int wpa_ctrl_get_fd(struct wpa_ctrl *ctrl)
{
	int fd = -1;
	if(ctrl != NULL)
	{
		fd = ctrl->s;
	}
    return fd;
}


#ifdef __cplusplus
}
#endif

