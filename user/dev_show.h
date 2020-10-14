#ifndef _DEV_SHOW_H_
#define _DEV_SHOW_H_

#include"headfiles.h"

#define DEV_SHOW                 0x01

#if DEV_SHOW

#define TIME_LEN                 0x02
#define TYPE_LEN                 0x04
#define DID_LEN                  0x02
#define CMD_SHOW                 0x03

struct DEV_SEARCH_PARAM
{
    uint8   dev_show_flag;
    uint8   ack_seq;
    uint16  ack_count;
    uint8   search_id[ID_LEN];
    uint16  max_delay_time;
    uint16  silent_time;
    uint32  cur_time;
};
extern struct DEV_SEARCH_PARAM dev_search_param;


uint8 search_frame_opt(struct SHS_frame *pframe);
void check_dev_show(void);
void check_search_delay(void);

#endif
#endif



