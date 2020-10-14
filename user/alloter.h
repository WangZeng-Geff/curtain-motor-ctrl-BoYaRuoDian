
#ifndef _ALLOTER_H_
#define _ALLOTER_H_

#include "config.h"

//#define BUFFER_LEN 0x08

struct _CHN_SLOT
{
	uint16 rx;
	uint16 tx;
	uint8 data_cnt;
//	uint8 data_max;
	//uint8  buffer[BUFFER_LEN];
	//uint8  t_index;
	//uint8  r_index;
	//uint16 idle_tick;
};
#define MAX_BUFFER_SZ           (MAX_POOL_SZ-BUFFER_NO_SZ)
#define BUFFER_NO_SZ            (MAX_POOL_SZ>>BLK_NO_SHIFT)
#define MAX_POOL_SZ             0x100
//
struct _CHN_POOL_MGR
{
	uint8 buffer[MAX_POOL_SZ];
	uint8 free_bitmap;
};
#define BLK_NO_SHIFT  5
#define BLK_SZ     (0x01 << BLK_NO_SHIFT)//32
/*BUFFER_SIZE is the power of the BUFFER_SHIFT*/
#define INVALID_BLK_NO  0xFF

#define INVALID_PTR     (INVALID_BLK_NO << BLK_NO_SHIFT)

#if DEBUG
extern DATA_SEG time_t uart_tick;
#endif

void init_chn_pool_mgr(void);
uint8 put_chn_bytes(struct _CHN_SLOT *pCHN_SLOT,unsigned char buffer[] ,uint8 len);
uint8 get_chn_bytes(struct _CHN_SLOT *pCHN_SLOT,unsigned char buffer[] ,uint8 len);
//int get_chn_byte(struct _CHN_SLOT *pCHN_SLOT ,unsigned char buffer[]);
uint8 peek_chn_byte(struct _CHN_SLOT *pCHN_SLOT,unsigned char data[],uint8 len);
#endif
