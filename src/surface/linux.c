/*
 https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/
 http://who-t.blogspot.de/2013/09/libevdev-handling-input-events.html
 https://cgit.freedesktop.org/libevdev/tree/libevdev
 https://www.tutorialspoint.com/cprogramming/c_arrays.htm
*/
/*
 * Copyright 2012 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <time.h>

#include "libnsfb.h"
#include "libnsfb_event.h"
#include "libnsfb_plot.h"
#include "libnsfb_plot_util.h"
#include "nsfb.h"
#include "plot.h"
#include "surface.h"
#include "cursor.h"
#define UNUSED(x) ((x) = (x))
#define FB_NAME     "/dev/fb0"
#define INPUT_NAME  "/dev/input/event0"

enum nsfb_key_code_e linux_nsfb_map[] = {
    NSFB_KEY_UNKNOWN,
    NSFB_KEY_ESCAPE,
    NSFB_KEY_1,
    NSFB_KEY_2,
    NSFB_KEY_3,
    NSFB_KEY_4,
    NSFB_KEY_5,
    NSFB_KEY_6,
    NSFB_KEY_7,
    NSFB_KEY_8,
    NSFB_KEY_9,
    NSFB_KEY_0,
    NSFB_KEY_MINUS,
    NSFB_KEY_EQUALS,
    NSFB_KEY_BACKSPACE,
    NSFB_KEY_TAB,
    NSFB_KEY_q,
    NSFB_KEY_w,
    NSFB_KEY_e,
    NSFB_KEY_r,
    NSFB_KEY_t,
    NSFB_KEY_y,
    NSFB_KEY_u,
    NSFB_KEY_i,
    NSFB_KEY_o,
    NSFB_KEY_p,
    NSFB_KEY_UNKNOWN, //    KEY_LEFTBRACE        26
    NSFB_KEY_UNKNOWN, //    KEY_RIGHTBRACE        27
    NSFB_KEY_RETURN,
    NSFB_KEY_LCTRL,
    NSFB_KEY_a,
    NSFB_KEY_s,
    NSFB_KEY_d,
    NSFB_KEY_f,
    NSFB_KEY_g,
    NSFB_KEY_h,
    NSFB_KEY_j,
    NSFB_KEY_k,
    NSFB_KEY_l,
    NSFB_KEY_SEMICOLON,
    NSFB_KEY_UNKNOWN, //    KEY_APOSTROPHE        40 //unknown
    NSFB_KEY_UNKNOWN, //    KEY_GRAVE        41
    NSFB_KEY_LSHIFT,
    NSFB_KEY_BACKSLASH,
    NSFB_KEY_z,
    NSFB_KEY_x,
    NSFB_KEY_c,
    NSFB_KEY_v,
    NSFB_KEY_b,
    NSFB_KEY_n,
    NSFB_KEY_m,
    NSFB_KEY_COMMA,
    NSFB_KEY_UNKNOWN, //    KEY_DOT            52
    NSFB_KEY_SLASH,
    NSFB_KEY_RSHIFT,
    NSFB_KEY_UNKNOWN, //    KEY_KPASTERISK        55
    NSFB_KEY_LALT,
    NSFB_KEY_SPACE,
    NSFB_KEY_CAPSLOCK,
    NSFB_KEY_F1,
    NSFB_KEY_F2,
    NSFB_KEY_F3,
    NSFB_KEY_F4,
    NSFB_KEY_F5,
    NSFB_KEY_F6,
    NSFB_KEY_F7,
    NSFB_KEY_F8,
    NSFB_KEY_F9,
    NSFB_KEY_F10,
    NSFB_KEY_NUMLOCK,
    NSFB_KEY_SCROLLOCK,
    NSFB_KEY_KP7,
    NSFB_KEY_KP8,
    NSFB_KEY_KP9,
    NSFB_KEY_KP_MINUS,
    NSFB_KEY_KP4,
    NSFB_KEY_KP5,
    NSFB_KEY_KP6,
    NSFB_KEY_KP_PLUS,
    NSFB_KEY_KP1,
    NSFB_KEY_KP2,
    NSFB_KEY_KP3,
    NSFB_KEY_KP0,
    NSFB_KEY_UNKNOWN, //    KEY_KPDOT        83
    NSFB_KEY_UNKNOWN, //    KEY_ZENKAKUHANKAKU    85
    NSFB_KEY_UNKNOWN, //    KEY_102ND        86
    NSFB_KEY_F11,
    NSFB_KEY_F12,
    NSFB_KEY_UNKNOWN, //    KEY_RO            89
    NSFB_KEY_UNKNOWN, //    KEY_KATAKANA        90
    NSFB_KEY_UNKNOWN, //    KEY_HIRAGANA        91
    NSFB_KEY_UNKNOWN, //    KEY_HENKAN        92
    NSFB_KEY_UNKNOWN, //    KEY_KATAKANAHIRAGANA    93
    NSFB_KEY_UNKNOWN, //    KEY_MUHENKAN        94
    NSFB_KEY_UNKNOWN, //    KEY_KPJPCOMMA        95
    NSFB_KEY_KP_ENTER,
    NSFB_KEY_RCTRL,
    NSFB_KEY_SLASH,
    NSFB_KEY_UNKNOWN, //    KEY_SYSRQ        99
    NSFB_KEY_RALT,
    NSFB_KEY_UNKNOWN, //    KEY_LINEFEED        101
    NSFB_KEY_HOME,
    NSFB_KEY_UP,
    NSFB_KEY_PAGEUP,
    NSFB_KEY_LEFT,
    NSFB_KEY_RIGHT,
    NSFB_KEY_END,
    NSFB_KEY_DOWN,
    NSFB_KEY_PAGEDOWN,
    NSFB_KEY_INSERT,
    NSFB_KEY_DELETE,
    NSFB_KEY_UNKNOWN, //    KEY_MACRO        112
    NSFB_KEY_UNKNOWN, //    KEY_MUTE        113
    NSFB_KEY_UNKNOWN, //    KEY_VOLUMEDOWN        114
    NSFB_KEY_UNKNOWN, //    KEY_VOLUMEUP        115
    NSFB_KEY_POWER,
    NSFB_KEY_KP_EQUALS,
    KEY_KPPLUSMINUS,
    NSFB_KEY_PAUSE,
    NSFB_KEY_UNKNOWN, //    KEY_SCALE        120    /* AL Compiz Scale (Expose) */
    NSFB_KEY_UNKNOWN, //    KEY_KPCOMMA        121
    NSFB_KEY_UNKNOWN, //    KEY_HANGEUL        122
    NSFB_KEY_UNKNOWN, //    KEY_HANGUEL        KEY_HANGEUL
    NSFB_KEY_UNKNOWN, //    KEY_HANJA        123
    NSFB_KEY_UNKNOWN, //    KEY_YEN            124
    NSFB_KEY_UNKNOWN, //    KEY_LEFTMETA        125
    NSFB_KEY_UNKNOWN, //    KEY_RIGHTMETA        126
    NSFB_KEY_UNKNOWN, //    KEY_COMPOSE        127
    NSFB_KEY_UNKNOWN, //    KEY_STOP        128    /* AC Stop */
    NSFB_KEY_UNKNOWN, //    KEY_AGAIN        129
    NSFB_KEY_UNKNOWN, //    KEY_PROPS        130    /* AC Properties */
    NSFB_KEY_UNKNOWN, //    KEY_UNDO        131    /* AC Undo */
    NSFB_KEY_UNKNOWN, //    KEY_FRONT        132
    NSFB_KEY_UNKNOWN, //    KEY_COPY        133    /* AC Copy */
    NSFB_KEY_UNKNOWN, //    KEY_OPEN        134    /* AC Open */
    NSFB_KEY_UNKNOWN, //    KEY_PASTE        135    /* AC Paste */
    NSFB_KEY_UNKNOWN, //    KEY_FIND        136    /* AC Search */
    NSFB_KEY_UNKNOWN, //    KEY_CUT            137    /* AC Cut */
    NSFB_KEY_UNKNOWN, //    KEY_HELP        138    /* AL Integrated Help Center */
    NSFB_KEY_UNKNOWN, //    KEY_MENU        139    /* Menu (show menu) */
    NSFB_KEY_UNKNOWN, //    KEY_CALC        140    /* AL Calculator */
    NSFB_KEY_UNKNOWN, //    KEY_SETUP        141
    NSFB_KEY_UNKNOWN, //    KEY_SLEEP        142    /* SC System Sleep */
    NSFB_KEY_UNKNOWN, //    KEY_WAKEUP        143    /* System Wake Up */
    NSFB_KEY_UNKNOWN, //    KEY_FILE        144    /* AL Local Machine Browser */
    NSFB_KEY_UNKNOWN, //    KEY_SENDFILE        145
    NSFB_KEY_UNKNOWN, //    KEY_DELETEFILE        146
    NSFB_KEY_UNKNOWN, //    KEY_XFER        147
    NSFB_KEY_UNKNOWN, //    KEY_PROG1        148
    NSFB_KEY_UNKNOWN, //    KEY_PROG2        149
    NSFB_KEY_UNKNOWN, //    KEY_WWW            150    /* AL Internet Browser */
    NSFB_KEY_UNKNOWN, //    KEY_MSDOS        151
    NSFB_KEY_UNKNOWN, //    KEY_COFFEE        152    /* AL Terminal Lock/Screensaver */
    NSFB_KEY_UNKNOWN, //    KEY_SCREENLOCK        KEY_COFFEE
    NSFB_KEY_UNKNOWN, //    KEY_ROTATE_DISPLAY    153    /* Display orientation for e.g. tablets */
    NSFB_KEY_UNKNOWN, //    KEY_DIRECTION        KEY_ROTATE_DISPLAY
    NSFB_KEY_UNKNOWN, //    KEY_CYCLEWINDOWS    154
    NSFB_KEY_UNKNOWN, //    KEY_MAIL        155
    NSFB_KEY_UNKNOWN, //    KEY_BOOKMARKS        156    /* AC Bookmarks */
    NSFB_KEY_UNKNOWN, //    KEY_COMPUTER        157
    NSFB_KEY_UNKNOWN, //    KEY_BACK        158    /* AC Back */
    NSFB_KEY_UNKNOWN, //    KEY_FORWARD        159    /* AC Forward */
    NSFB_KEY_UNKNOWN, //    KEY_CLOSECD        160
    NSFB_KEY_UNKNOWN, //    KEY_EJECTCD        161
    NSFB_KEY_UNKNOWN, //    KEY_EJECTCLOSECD    162
    NSFB_KEY_UNKNOWN, //    KEY_NEXTSONG        163
    NSFB_KEY_UNKNOWN, //    KEY_PLAYPAUSE        164
    NSFB_KEY_UNKNOWN, //    KEY_PREVIOUSSONG    165
    NSFB_KEY_UNKNOWN, //    KEY_STOPCD        166
    NSFB_KEY_UNKNOWN, //    KEY_RECORD        167
    NSFB_KEY_UNKNOWN, //    KEY_REWIND        168
    NSFB_KEY_UNKNOWN, //    KEY_PHONE        169    /* Media Select Telephone */
    NSFB_KEY_UNKNOWN, //    KEY_ISO            170
    NSFB_KEY_UNKNOWN, //    KEY_CONFIG        171    /* AL Consumer Control Configuration */
    NSFB_KEY_UNKNOWN, //    KEY_HOMEPAGE        172    /* AC Home */
    NSFB_KEY_UNKNOWN, //    KEY_REFRESH        173    /* AC Refresh */
    NSFB_KEY_UNKNOWN, //    KEY_EXIT        174    /* AC Exit */
    NSFB_KEY_UNKNOWN, //    KEY_MOVE        175
    NSFB_KEY_UNKNOWN, //    KEY_EDIT        176
    NSFB_KEY_UNKNOWN, //    KEY_SCROLLUP        177
    NSFB_KEY_UNKNOWN, //    KEY_SCROLLDOWN        178
    NSFB_KEY_UNKNOWN, //    KEY_KPLEFTPAREN        179
    NSFB_KEY_UNKNOWN, //    KEY_KPRIGHTPAREN    180
    NSFB_KEY_UNKNOWN, //    KEY_NEW            181    /* AC New */
    NSFB_KEY_UNKNOWN, //    KEY_REDO        182    /* AC Redo/Repeat */
    NSFB_KEY_F13,
    NSFB_KEY_F14,
    NSFB_KEY_F15,
    NSFB_KEY_UNKNOWN, //    KEY_F16            186
    NSFB_KEY_UNKNOWN, //    KEY_F17            187
    NSFB_KEY_UNKNOWN, //    KEY_F18            188
    NSFB_KEY_UNKNOWN, //    KEY_F19            189
    NSFB_KEY_UNKNOWN, //    KEY_F20            190
    NSFB_KEY_UNKNOWN, //    KEY_F21            191
    NSFB_KEY_UNKNOWN, //    KEY_F22            192
    NSFB_KEY_UNKNOWN, //    KEY_F23            193
    NSFB_KEY_UNKNOWN, //    KEY_F24            194
    NSFB_KEY_UNKNOWN, //    KEY_PLAYCD        200
    NSFB_KEY_UNKNOWN, //    KEY_PAUSECD        201
    NSFB_KEY_UNKNOWN, //    KEY_PROG3        202
    NSFB_KEY_UNKNOWN, //    KEY_PROG4        203
    NSFB_KEY_UNKNOWN, //    KEY_DASHBOARD        204    /* AL Dashboard */
    NSFB_KEY_UNKNOWN, //    KEY_SUSPEND        205
    NSFB_KEY_UNKNOWN, //    KEY_CLOSE        206    /* AC Close */
    NSFB_KEY_UNKNOWN, //    KEY_PLAY        207
    NSFB_KEY_UNKNOWN, //    KEY_FASTFORWARD        208
    NSFB_KEY_UNKNOWN, //    KEY_BASSBOOST        209
    NSFB_KEY_UNKNOWN, //    KEY_PRINT        210    /* AC Print */
    NSFB_KEY_UNKNOWN, //    KEY_HP            211
    NSFB_KEY_UNKNOWN, //    KEY_CAMERA        212
    NSFB_KEY_UNKNOWN, //    KEY_SOUND        213
    NSFB_KEY_UNKNOWN, //    KEY_QUESTION        214
    NSFB_KEY_UNKNOWN, //    KEY_EMAIL        215
    NSFB_KEY_UNKNOWN, //    KEY_CHAT        216
    NSFB_KEY_UNKNOWN, //    KEY_SEARCH        217
    NSFB_KEY_UNKNOWN, //    KEY_CONNECT        218
    NSFB_KEY_UNKNOWN, //    KEY_FINANCE        219    /* AL Checkbook/Finance */
    NSFB_KEY_UNKNOWN, //    KEY_SPORT        220
    NSFB_KEY_UNKNOWN, //    KEY_SHOP        221
    NSFB_KEY_UNKNOWN, //    KEY_ALTERASE        222
    NSFB_KEY_UNKNOWN, //    KEY_CANCEL        223    /* AC Cancel */
    NSFB_KEY_UNKNOWN, //    KEY_BRIGHTNESSDOWN    224
    NSFB_KEY_UNKNOWN, //    KEY_BRIGHTNESSUP    225
    NSFB_KEY_UNKNOWN, //    KEY_MEDIA        226
    NSFB_KEY_UNKNOWN, //    KEY_SWITCHVIDEOMODE    227    /* Cycle between available video
    NSFB_KEY_UNKNOWN, //    KEY_KBDILLUMTOGGLE    228
    NSFB_KEY_UNKNOWN, //    KEY_KBDILLUMDOWN    229
    NSFB_KEY_UNKNOWN, //    KEY_KBDILLUMUP        230
    NSFB_KEY_UNKNOWN, //    KEY_SEND        231    /* AC Send */
    NSFB_KEY_UNKNOWN, //    KEY_REPLY        232    /* AC Reply */
    NSFB_KEY_UNKNOWN, //    KEY_FORWARDMAIL        233    /* AC Forward Msg */
    NSFB_KEY_UNKNOWN, //    KEY_SAVE        234    /* AC Save */
    NSFB_KEY_UNKNOWN, //    KEY_DOCUMENTS        235
    NSFB_KEY_UNKNOWN, //    KEY_BATTERY        236
    NSFB_KEY_UNKNOWN, //    KEY_BLUETOOTH        237
    NSFB_KEY_UNKNOWN, //    KEY_WLAN        238
    NSFB_KEY_UNKNOWN, //    KEY_UWB            239
    NSFB_KEY_UNKNOWN, //    KEY_UNKNOWN        240
    NSFB_KEY_UNKNOWN, //    KEY_VIDEO_NEXT        241    /* drive next video source */
    NSFB_KEY_UNKNOWN, //    KEY_VIDEO_PREV        242    /* drive previous video source */
    NSFB_KEY_UNKNOWN, //    KEY_BRIGHTNESS_CYCLE    243    /* brightness up, after max is min */
    NSFB_KEY_UNKNOWN, //    KEY_BRIGHTNESS_AUTO    244    /* Set Auto Brightness: manual
    NSFB_KEY_UNKNOWN, //    KEY_BRIGHTNESS_ZERO    KEY_BRIGHTNESS_AUTO
    NSFB_KEY_UNKNOWN, //    KEY_DISPLAY_OFF        245    /* display device to off state */
    NSFB_KEY_UNKNOWN, //    KEY_WWAN        246    /* Wireless WAN (LTE, UMTS, GSM, etc.) */
    NSFB_KEY_UNKNOWN, //    KEY_WIMAX        KEY_WWAN
    NSFB_KEY_UNKNOWN, //    KEY_RFKILL        247    /* Key that controls all radios */
    NSFB_KEY_UNKNOWN //    KEY_MICMUTE        248    /* Mute / unmute the microphone */
};
struct lnx_priv {
    struct fb_fix_screeninfo FixInfo;
    struct fb_var_screeninfo VarInfo;
    int fd;
    int fd_input;
};
static int linux_set_geometry(nsfb_t *nsfb, int width, int height, enum nsfb_format_e format)
{
    if (nsfb->surface_priv != NULL) {
        return -1; /* if we are already initialised fail */
    }
    nsfb->width = width;
    nsfb->height = height;
    nsfb->format = format;
    /* select default sw plotters for bpp */
    if (select_plotters(nsfb) != true) {
   return -1;
    }
    return 0;
}
static enum nsfb_format_e
format_from_lstate(struct lnx_priv *lstate)
{
    enum nsfb_format_e fmt = NSFB_FMT_ANY;
    switch(lstate->VarInfo.bits_per_pixel) {
    case 32:
   if (lstate->VarInfo.transp.length == 0)
       fmt = NSFB_FMT_XBGR8888;
   else
       fmt = NSFB_FMT_ABGR8888;
   break;
    case 24:
   fmt = NSFB_FMT_RGB888;
   break;
    case 16:
   fmt = NSFB_FMT_RGB565;
   break;
    case 8:
   fmt = NSFB_FMT_I8;
   break;
    case 1:
   fmt = NSFB_FMT_RGB565;
   break;
    }

    return fmt;
}
static int linux_initialise(nsfb_t *nsfb)
{
    int iFrameBufferSize;
    struct lnx_priv *lstate;
    enum nsfb_format_e lformat;
    if (nsfb->surface_priv != NULL)
   return -1;
    lstate = calloc(1, sizeof(struct lnx_priv));
    if (lstate == NULL) {
   return -1;
    }
    /* Open the framebuffer device in read write */
    lstate->fd = open(FB_NAME, O_RDWR);
    if (lstate->fd < 0) {
   printf("Unable to open %s.\n", FB_NAME);
   free(lstate);
   return -1;
    }
    /* Do Ioctl. Retrieve fixed screen info. */
    if (ioctl(lstate->fd, FBIOGET_FSCREENINFO, &lstate->FixInfo) < 0) {
   printf("get fixed screen info failed: %s\n",
          strerror(errno));
   close(lstate->fd);
   free(lstate);
   return -1;
    }
    /* Do Ioctl. Get the variable screen info. */
    if (ioctl(lstate->fd, FBIOGET_VSCREENINFO, &lstate->VarInfo) < 0) {
   printf("Unable to retrieve variable screen info: %s\n",
          strerror(errno));
   close(lstate->fd);
   free(lstate);
   return -1;
    }
    /* Calculate the size to mmap */
    iFrameBufferSize = lstate->FixInfo.line_length * lstate->VarInfo.yres;
    /* Now mmap the framebuffer. */
    nsfb->ptr = mmap(NULL, iFrameBufferSize, PROT_READ | PROT_WRITE,
          MAP_SHARED, lstate->fd, 0);
    if (nsfb->ptr == NULL) {
   printf("mmap failed:\n");
   close(lstate->fd);
   free(lstate);
   return -1;
    }
    nsfb->linelen = lstate->FixInfo.line_length;
    nsfb->width = lstate->VarInfo.xres;
    nsfb->height = lstate->VarInfo.yres;

    lformat = format_from_lstate(lstate);
    if (nsfb->format != lformat) {
   nsfb->format = lformat;
   /* select default sw plotters for format */
   if (select_plotters(nsfb) != true) {
       munmap(nsfb->ptr, 0);
       close(lstate->fd);
       free(lstate);
       return -1;
   }
    }
    /* Open the input devices */
    lstate->fd_input = open(INPUT_NAME, O_RDONLY | O_NONBLOCK);
    if (lstate->fd_input < 0) {
        printf("Unable to open %s.\n", INPUT_NAME);
    }else{
        printf("Sucsess open %s. \n", INPUT_NAME);
        printf("fd_input %d \n",lstate->fd_input);
    }

    nsfb->surface_priv = lstate;
    return 0;
}
static int linux_finalise(nsfb_t *nsfb)
{
    struct lnx_priv *lstate = nsfb->surface_priv;
    if (lstate != NULL) {
        /* close framebuffer */
        munmap(nsfb->ptr, 0);
        close(lstate->fd);
        /* close input devices*/
        close(lstate->fd_input);
        free(lstate);
    }
    return 0;
}
static bool linux_input(nsfb_t *nsfb, nsfb_event_t *event, int timeout)
{
    //////////////////// SELECT READ FD /////////////////////////////////

    struct lnx_priv *lstate = nsfb->surface_priv;
    int got_event = -1;
    int rb;
    struct input_event ev[64];

    if(lstate == NULL)
        return false;

    if(timeout >= 0){

        fd_set rfds; // rfds read_file_descriptors
        FD_ZERO(&rfds);
        FD_SET(lstate->fd_input, &rfds);

        if(timeout > 0){
            // wait for event with select on fd_input
            // printf("TS: %d searching......\n", (int)time(NULL));
            int secs = (timeout/1000);
            int usecs = ((timeout % 1000)*1000);
            //printf("timeout( msecs; %d - secs: %d - usecs: %d )\n", timeout, secs, usecs);
            // setup select
            struct timeval tv;
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = (timeout % 1000)*1000;
            got_event = select(lstate->fd_input +1, &rfds, NULL, NULL, &tv);
        }else{
            //printf("timeout = 0 \n");
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 0;
            got_event = select(lstate->fd_input +1, &rfds, NULL, NULL, &tv);

        }
        if(got_event == 0){
            // printf("TS: %d no wild event appeared event\n", (int)time(NULL));
            event->type = NSFB_EVENT_CONTROL;
            event->value.controlcode = NSFB_CONTROL_TIMEOUT;
            return true;
        }else if(got_event < 0){

           // ERROR !!!!!!
           // printf("TS: %d Oh dear, something went wrong with select()! %s\n", (int)time(NULL), strerror(errno));
           if(got_event == -EAGAIN){
                   printf("-EAGAIN %d \n",got_event);
           }
           return false;
        }
    }
    //
    // leave if no event appeared
    //
    if(got_event <= 0)
        return false;
    //
    // get actual events
    // read fd_input
    //
    event->type = NSFB_EVENT_NONE;
    rb = read(lstate->fd_input, ev, sizeof(struct input_event)*64);

    //check if read succeeded
    if (rb < (int) sizeof(struct input_event)) {
//        printf("TS: %d evtest: short read \n", (int)time(NULL));
        return false;
    }

    //printf("TS: %d got --> %d <-- events \n", (int)time(NULL), rb / sizeof(struct input_event));

    for (int i = 0;
        i <= (int) (rb / sizeof(struct input_event));
        i++)
    {
        //printf("event --> %d \n", i);
        if (EV_KEY == ev[i].type){

//            printf("EV_KEY   %ld.%06ld ",
//                    ev[i].time.tv_sec,
//                    ev[i].time.tv_usec);
//            printf("type %d code %d value %d\n",
//                    ev[i].type,
//                    ev[i].code, ev[i].value);

            if(ev[i].value == 0){
                event->type = NSFB_EVENT_KEY_UP;
                event->value.keycode = linux_nsfb_map[ev[i].code];
                //printf("NSFB_EVENT_KEY_UP linux_code: %d nsfb_code: %d\n", ev[i].code, linux_nsfb_map[ev[i].code]);
                return true;
            }else if(ev[i].value == 1){
                event->type = NSFB_EVENT_KEY_DOWN;
                event->value.keycode = linux_nsfb_map[ev[i].code];
                //printf("NSFB_EVENT_KEY_DOWN linux_code: %d nsfb_code: %d\n", ev[i].code, linux_nsfb_map[ev[i].code]);
                return true;
            }else{
                return false;
                // key pressed ev[i].value == 2
                // not implemented for netsurf
            }

        }
//        else if (EV_SYN == ev[i].type){
////            printf("EV_SYN %ld.%06ld ",
////                ev[i].time.tv_sec,
////                ev[i].time.tv_usec);
////            printf("type %d code %d value %d\n",
////                ev[i].type,
////                ev[i].code, ev[i].value);
//        }
//        else if (EV_MSC == ev[i].type){
////            printf("EV_MSC %ld.%06ld ",
////                ev[i].time.tv_sec,
////                ev[i].time.tv_usec);
////            printf("type %d code %d value %d\n",
////                ev[i].type,
////                ev[i].code, ev[i].value);
//        }else{
//            printf("USELESS %ld.%06ld ",
//                ev[i].time.tv_sec,
//                ev[i].time.tv_usec);
//            printf("type %d code %d value %d\n",
//                ev[i].type,
//                ev[i].code, ev[i].value);
//        }
    }
    return false;
    //return false;
}
 /*
    if(got_event == LIBEVDEV_READ_STATUS_SUCCESS){
        printf("LIBEVDEV_READ_STATUS_SUCCESS %d \n",got_event);
        printf("Event: %s %s %d\n",
            libevdev_event_type_get_name(ev.type),
            libevdev_event_code_get_name(ev.type, ev.code),
            ev.value);
        if(ev.type == EV_KEY){
            event->type = NSFB_EVENT_KEY_DOWN;
            event->value.keycode = NSFB_KEY_DOWN;
            printf("sending event to netsurf\n");
            return true;
        }
    }
    if(got_event == LIBEVDEV_READ_STATUS_SYNC){
        printf("LIBEVDEV_READ_STATUS_SYNC %d \n",got_event);
        struct input_event ev;
        int rc = LIBEVDEV_READ_STATUS_SYNC;
        while (rc == LIBEVDEV_READ_STATUS_SYNC) {
            rc = libevdev_next_event(lstate->input_dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
            if (rc < 0) {
                if (rc != -EAGAIN)
                    fprintf(stderr, "error %d (%s)\n", -rc, strerror(-rc));
                return;
            }
            printf("State change since SYN_DROPPED for %s %s value %d\n",
                    libevdev_event_type_get_name(ev.type),
                    libevdev_event_code_get_name(ev.type, ev.code),
                    ev.value);
        }
    }
    */
    //////////////////// LIBEVDEV FUNCTIONAL /////////////////////////////
  /*
    struct lnx_priv *lstate = nsfb->surface_priv;
    //timeout = timeout;
    // struct input_event ( )
    if(lstate == NULL)
        return false;
//    check if there are pending events
        int event_pending;
        event_pending = libevdev_has_event_pending(lstate->input_dev);
    //printf("got_event %d",event_pending);
    if(timeout < 0){
        printf("Timeout < 0");
    }
    if(timeout == 0){
        //printf("Timeout == 0"); happens quite often
    }
    int got_event;
    struct input_event ev;
    // read next event if event pending
    if (timeout == 0){
        printf("TS: %d timeout == 0 \n", (int)time(NULL));
        got_event = libevdev_next_event(lstate->input_dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    } else {
        if(timeout > 0){
            // wait for event with select on evdev
            // return timeout if no event
            printf("TS: %d let me wait\n", (int)time(NULL));
            fd_set rfds;
            struct timeval tv;
            int retval;
            FD_ZERO(&rfds);
            FD_SET(lstate->fd_input, &rfds);
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = timeout % 1000;
            printf("Timeout %d %d %d \n", timeout, timeout / 1000, timeout % 1000);
            retval = select(lstate->fd_input +1, &rfds, NULL, NULL, &tv);
            if(retval == 0){
                printf("TS: %d no wild event appeared event\n", (int)time(NULL));
                event->type = NSFB_EVENT_CONTROL;
                event->value.controlcode = NSFB_CONTROL_TIMEOUT;
                return true;
            }else{
               printf("TS: %d catch em all \n", (int)time(NULL));
               got_event = libevdev_next_event(lstate->input_dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
            }
        }else{
           printf("TS: %d think i will never get there \n", (int)time(NULL));
           got_event = libevdev_next_event(lstate->input_dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        }
    }
    if(got_event == -EAGAIN){
        printf("-EAGAIN %d \n",got_event);
    }
    if(got_event == LIBEVDEV_READ_STATUS_SUCCESS){
        printf("LIBEVDEV_READ_STATUS_SUCCESS %d \n",got_event);
        printf("Event: %s %s %d\n",
            libevdev_event_type_get_name(ev.type),
            libevdev_event_code_get_name(ev.type, ev.code),
            ev.value);
        if(ev.type == EV_KEY){
            event->type = NSFB_EVENT_KEY_DOWN;
            event->value.keycode = NSFB_KEY_DOWN;
            printf("sending event to netsurf\n");
            return true;
        }
    }
    if(got_event == LIBEVDEV_READ_STATUS_SYNC){
        printf("LIBEVDEV_READ_STATUS_SYNC %d \n",got_event);
        struct input_event ev;
        int rc = LIBEVDEV_READ_STATUS_SYNC;
        while (rc == LIBEVDEV_READ_STATUS_SYNC) {
            rc = libevdev_next_event(lstate->input_dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
            if (rc < 0) {
                if (rc != -EAGAIN)
                    fprintf(stderr, "error %d (%s)\n", -rc, strerror(-rc));
                return;
            }
            printf("State change since SYN_DROPPED for %s %s value %d\n",
                    libevdev_event_type_get_name(ev.type),
                    libevdev_event_code_get_name(ev.type, ev.code),
                    ev.value);
        }
    }
*/
/*
    // read event in input_event struct
    // check status if EV_KEY -> return event
    if(event_pending == 1){
        struct input_event ev;
        int rc = libevdev_next_event(lstate->input_dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if(rc == -EAGAIN){
            printf("-EAGAIN %d \n",rc);
        }
        if(rc == LIBEVDEV_READ_STATUS_SUCCESS){
            printf("LIBEVDEV_READ_STATUS_SUCCESS %d \n",rc);
            printf("Event: %s %s %d\n",
                libevdev_event_type_get_name(ev.type),
                libevdev_event_code_get_name(ev.type, ev.code),
                ev.value);
            if(ev.type == EV_KEY){
                event->type = NSFB_EVENT_KEY_DOWN;
                event->value.keycode = NSFB_KEY_DOWN;
                printf("sending event to netsurf\n");
                return true;
            }
        }
        if(rc == LIBEVDEV_READ_STATUS_SYNC){
            printf("LIBEVDEV_READ_STATUS_SYNC %d \n",rc);
            struct input_event ev;
            int rc = LIBEVDEV_READ_STATUS_SYNC;
            while (rc == LIBEVDEV_READ_STATUS_SYNC) {
                rc = libevdev_next_event(lstate->input_dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
                if (rc < 0) {
                    if (rc != -EAGAIN)
                        fprintf(stderr, "error %d (%s)\n", -rc, strerror(-rc));
                    return;
                }
                printf("State change since SYN_DROPPED for %s %s value %d\n",
                        libevdev_event_type_get_name(ev.type),
                        libevdev_event_code_get_name(ev.type, ev.code),
                        ev.value);
            }
        }
    }else{
        // wati for event with select
    }
*/
//    struct libevdev *dev = NULL;
//    int fd;
//    int rc = 1;
    //fd = open("/dev/input/event6", O_RDONLY|O_NONBLOCK);
    //rc = libevdev_new_from_fd(fd, &dev);
    /*
    if (rc < 0) {
            fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-rc));
            exit(1);
    }
    printf("Input device name: \"%s\"\n", libevdev_get_name(dev));
    printf("Input device ID: bus %#x vendor %#x product %#x\n",
           libevdev_get_id_bustype(dev),
           libevdev_get_id_vendor(dev),
           libevdev_get_id_product(dev));
    if (!libevdev_has_event_type(dev, EV_REL) ||
        !libevdev_has_event_code(dev, EV_KEY, BTN_LEFT)) {
            printf("This device does not look like a mouse\n");
            exit(1);
    }
    do {
            struct input_event ev;
            rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
            if (rc == 0)
                    printf("Event: %s %s %d\n",
                           libevdev_event_type_get_name(ev.type),
                           libevdev_event_code_get_name(ev.type, ev.code),
                           ev.value);
    } while (rc == 1 || rc == 0 || rc == -EAGAIN);
    */
//    if(rc < 0){
//        fprintf(stderr, "error: %d %s\n", -rc, strerror(-rc));
//    }else{
//        printf("Input device name: \"%s\"\n", libevdev_get_name(dev));
//        printf("Input device ID: bus %#x vendor %#x product %#x\n",
//               libevdev_get_id_bustype(dev),
//               libevdev_get_id_vendor(dev),
//               libevdev_get_id_product(dev));
//    }
   /*
    if (rc < 0) {
        if (rc != -EAGAIN)
            fprintf(stderr, "error: %d %s\n", -rc, strerror(-rc));
    else if (rc == LIBEVDEV_READ_STATUS_SYNC)
        printf
        libhandle_syn_dropped(dev);
    else if (rc == LIBEVDEV_READ_STATUS_SUCCESS)
        printf("We have an event!\n%d (%s) %s (%d) value %d\n",
              ev.type, libevdev_event_type_get_name(ev.type),
              ev.code, libevdev_event_code_get_name(ev.type, ev.code),
              ev.value);
    }
*/
////////////////////// inotify test //////////////////////////////
/*
    event = event;
    timeout = timeout;
    struct lnx_priv *lstate = nsfb->surface_priv;
    int length, i = 0;
    char buffer[BUF_LEN];
    int result;
    fd_set set;
    struct timeval tv;
    FD_ZERO(&set);
    FD_SET(lstate->fd_inotify, &set);
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = timeout % 1000;
    result = select(lstate->fd_inotify + 1, &set, NULL, NULL, &tv);
    if(result != 0){
        printf("can read");
        length = read( lstate->fd_inotify, buffer, BUF_LEN );
        if ( length < 0 ) {
            perror( "read" );
            }
        printf( " read complete " );
        while ( i < length ) {
            struct inotify_event *nev = ( struct inotify_event * ) &buffer[ i ];
            if ( nev->len ) {
              if ( nev->mask & IN_CREATE ) {
                if ( nev->mask & IN_ISDIR ) {
                  printf( "The directory %s was created.\n", nev->name );
                }
                else {
                  printf( "The file %s was created.\n", nev->name );
                }
              }
              else if ( nev->mask & IN_DELETE ) {
                if ( nev->mask & IN_ISDIR ) {
                  printf( "The directory %s was deleted.\n", nev->name );
                }
                else {
                  printf( "The file %s was deleted.\n", nev->name );
                }
              }
              else if ( nev->mask & IN_MODIFY ) {
                if ( nev->mask & IN_ISDIR ) {
                  printf( "The directory %s was modified.\n", nev->name );
                }
                else {
                  printf( "The file %s was modified.\n", nev->name );
                }
              }
              else if ( nev->mask & IN_ACCESS ) {
                  if ( nev->mask & IN_ISDIR ) {
                    printf( "The directory %s was accessed.\n", nev->name );
                  }
                  else {
                    printf( "The file %s was accessed.\n", nev->name );
                  }
                }
            else if ( nev->mask & IN_OPEN ) {
                if ( nev->mask & IN_ISDIR ) {
                    printf( "The directory %s was opened.\n", nev->name );
                }
                else {
                    printf( "The file %s was opened.\n", nev->name );
                }
            }
             else if ( nev->mask & IN_CLOSE_WRITE ) {
                            if ( nev->mask & IN_ISDIR ) {
                                printf( "The directory %s was close write.\n", nev->name );
                            }
                            else {
                                printf( "The file %s was close write.\n", nev->name );
                            }
                        }
            }
            i += EVENT_SIZE + nev->len;
        }
    }else{
        printf("nothing changed");
    }
 */
//////////////////////////

////////////////////////// reading file events 2 /////////////////////////
/*
    struct lnx_priv *lstate = nsfb->surface_priv;
    if(lstate == NULL)
        return false;
    size_t rb;
    struct input_event ev[64];
    int fd_input = lstate->fd_input;
    fd_set rfds;
    struct timeval tv;
    int retval;

    FD_ZERO(&rfds);
    FD_SET(fd_input, &rfds);
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = timeout % 1000;
    retval = select(fd_input +1, &rfds, NULL, NULL, &tv);
    if(retval = 0){
        // no event occured
        printf("nothing to read \n");
        event->type = NSFB_EVENT_CONTROL;
        event->value.controlcode = NSFB_CONTROL_TIMEOUT;
        return true;
    }else{
        rb=read(fd_input, ev, sizeof(struct input_event)*64);
        if (rb < (int) sizeof(struct input_event)) {
            printf("\nevtest: short read \n");
            return false;
        }
        for (int i = 0;
                 i < (int) (rb / sizeof(struct input_event));
                 i++)
        {
            //printf("inside");
            if (EV_KEY == ev[i].type){
                printf("\nEV_KEY   %ld.%06ld ",
                       ev[i].time.tv_sec,
                       ev[i].time.tv_usec);
                printf("type %d code %d value %d\n",
                       ev[i].type,
                       ev[i].code, ev[i].value);
            }
            else if (EV_SYN == ev[i].type){
                printf("\nEV_SYN %ld.%06ld ",
                       ev[i].time.tv_sec,
                       ev[i].time.tv_usec);
                printf("type %d code %d value %d\n",
                       ev[i].type,
                       ev[i].code, ev[i].value);
            }
            else if (EV_MSC == ev[i].type){
                printf("\nEV_MSC %ld.%06ld ",
                       ev[i].time.tv_sec,
                       ev[i].time.tv_usec);
                printf("type %d code %d value %d\n",
                       ev[i].type,
                       ev[i].code, ev[i].value);
            }else{
                 printf("\nUSELESS %ld.%06ld ",
                       ev[i].time.tv_sec,
                       ev[i].time.tv_usec);
                 printf("type %d code %d value %d\n",
                       ev[i].type,
                       ev[i].code, ev[i].value);
            }

        }
       return true;
    }
*/
/////////////////////////////////////////////////////////////////////
////////////////////////// reading file events /////////////////////////
/*
    struct lnx_priv *lstate = nsfb->surface_priv;
    if(lstate == NULL)
        return false;
    size_t rb;
    struct input_event ev[64];
    int fd_input;
    if(timeout != 0){
        if(timeout > 0){
            fd_set rfds;
            struct timeval tv;
            int retval;
            fd_input = lstate->fd_input;
            FD_ZERO(&rfds);
            FD_SET(fd_input, &rfds);
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = timeout % 1000;
            retval = select(fd_input +1, &rfds, NULL, NULL, &tv);
            if(retval = 0){
                event->type = NSFB_EVENT_CONTROL;
                event->value.controlcode = NSFB_CONTROL_TIMEOUT;
                return true;
            }
        }
        rb=read(fd_input, ev, sizeof(struct input_event)*64);
    }
    if (rb < (int) sizeof(struct input_event)) {
        perror("evtest: short read");
        return false;
    }
    for (int i = 0;
             i < (int) (rb / sizeof(struct input_event));
             i++)
    {
        //printf("inside");
        if (EV_KEY == ev[i].type){
            printf("%ld.%06ld ",
                   ev[i].time.tv_sec,
                   ev[i].time.tv_usec);
            printf("type %d code %d value %d\n",
                   ev[i].type,
                   ev[i].code, ev[i].value);
            return true;
        }
    }
*/
/////////////////////////////////////////////////////////////////////
    //nsfb = nsfb; /* unused */
    //printf("LINUX Timestamp: %d %d \n",(int)time(NULL),event->value.keycode);
    //printf("timeout: %d \n",timeout);
    /*
    if(timeout == 0){
        printf("LINUX Timestamp: %d \n",(int)time(NULL));
    }
    const char *dev = "/dev/input/by-id/usb-DELL_Dell_QuietKey_Keyboard-event-kbd";
    struct input_event ev[64];
    int fd, rd, value, size = sizeof (struct input_event);
    char name[256] = "Unknown";
    if ((getuid ()) != 0)
        printf ("You are not root! This may not work...\n");
    //Open Device
    if ((fd = open (dev, O_RDONLY | O_NONBLOCK)) == -1)
        printf ("%s is not a vaild device \n", dev);
    //printf("succes %s \n", dev);
    //Print Device Name
    ioctl (fd, EVIOCGNAME (sizeof (name)), name);
    printf ("Reading From : %s (%s) \n", dev, name);
    if ((rd = read (fd, ev, size * 64)) < size){
        //printf("error read \n");
        close(fd);
        return false;
    }*/
    /*
    struct input_event {
        struct timeval time;
        unsigned short type;
        unsigned short code;
        unsigned int value;
    };
    */
    //const char *dev = "/dev/input/by-id/usb-DELL_Dell_QuietKey_Keyboard-event-kbd";
    //char name[256] = "Unknown";
    //if ((getuid ()) != 0)
      //  printf ("You are not root! This may not work...\n");
    //if ((fd = open (dev, O_RDONLY | O_NONBLOCK)) == -1)
        //printf ("%s is not a vaild device \n", dev);

    /*

    if(ev[0].type == EV_MSC){
        printf("EVENT EV_MSC EV[0]: time - %d  type - %hu  code - %hu  value - %u \n ------------- \n", ev[0].time, ev[0].type, ev[0].code, ev[0].value);
    };
    if(ev[1].type == EV_KEY){
        printf("EVENT EV_KEY EV[1]: time - %d  type - %hu  code - %hu  value - %u \n ------------- \n", ev[1].time, ev[1].type, ev[1].code, ev[1].value);
        return true;
    };
    if(ev[1].type == EV_SYN){
        printf("EVENT EV_SYN EV[1]:  time - %d  type - %hu  code - %hu  value - %u \n ------------- \n", ev[1].time, ev[1].type, ev[1].code, ev[1].value);
    };
     if(ev[2].type == EV_KEY){
        printf("EVENT EV_KEY EV[2]:  time - %d  type - %hu  code - %hu  value - %u \n ------------- \n", ev[2].time, ev[2].type, ev[2].code, ev[2].value);
    };
    if(ev[2].type == EV_SYN){
        printf("EVENT EV_SYN EV[2]:  time - %d  type - %hu  code - %hu  value - %u \n ------------- \n", ev[2].time, ev[2].type, ev[2].code, ev[2].value);
    };
 */
 /*
    value = ev[0].value;
    if (value != ' ' && ev[1].value == 1 && ev[1].type == 1){ // Only read the key press event
     printf ("Code[%d]\n", (ev[1].code));
    }
 */
    /*
    event->type = NSFB_EVENT_NONE;
    switch (sdlevent.type) {
    case SDL_KEYDOWN:
    event->type = NSFB_EVENT_KEY_DOWN;
    event->value.keycode = sdl_nsfb_map[sdlevent.key.keysym.sym];
    break;
    case SDL_KEYUP:
    event->type = NSFB_EVENT_KEY_UP;
    event->value.keycode = sdl_nsfb_map[sdlevent.key.keysym.sym];
    break;
    */
    //close(fd);
    //printf("------------> END <--------------------------- \n");
    //event = event;
    //return false;
//}
static int linux_claim(nsfb_t *nsfb, nsfb_bbox_t *box)
{
    struct nsfb_cursor_s *cursor = nsfb->cursor;
    if ((cursor != NULL) &&
        (cursor->plotted == true) &&
        (nsfb_plot_bbox_intersect(box, &cursor->loc))) {
        nsfb->plotter_fns->bitmap(nsfb,
                                  &cursor->savloc,
                                  cursor->sav,
                                  cursor->sav_width,
                                  cursor->sav_height,
                                  cursor->sav_width,
                                  false);
        cursor->plotted = false;
    }
    return 0;
}
static int linux_cursor(nsfb_t *nsfb, struct nsfb_cursor_s *cursor)
{
    nsfb_bbox_t sclip;
    if ((cursor != NULL) && (cursor->plotted == true)) {
        sclip = nsfb->clip;
        nsfb->plotter_fns->set_clip(nsfb, NULL);
        nsfb->plotter_fns->bitmap(nsfb,
                                  &cursor->savloc,
                                  cursor->sav,
                                  cursor->sav_width,
                                  cursor->sav_height,
                                  cursor->sav_width,
                                  false);
        nsfb_cursor_plot(nsfb, cursor);
        nsfb->clip = sclip;
    }
    return true;
}

static int linux_update(nsfb_t *nsfb, nsfb_bbox_t *box)
{
    struct nsfb_cursor_s *cursor = nsfb->cursor;
    UNUSED(box);
    if ((cursor != NULL) && (cursor->plotted == false)) {
        nsfb_cursor_plot(nsfb, cursor);
    }
    return 0;
}
const nsfb_surface_rtns_t linux_rtns = {
    .initialise = linux_initialise,
    .finalise = linux_finalise,
    .input = linux_input,
    .claim = linux_claim,
    .update = linux_update,
    .cursor = linux_cursor,
    .geometry = linux_set_geometry,
};
NSFB_SURFACE_DEF(linux, NSFB_SURFACE_LINUX, &linux_rtns)
/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
