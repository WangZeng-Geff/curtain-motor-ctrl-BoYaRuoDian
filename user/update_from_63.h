#ifndef __UPDATE_H__
#define __UPDATE_H__

#include "headfiles.h"

#define little_bytes_to_word(byte)  ((uint16)(byte[1]<<8)+byte[0])
#define big_bytes_to_word(byte)    ((uint16)(byte[0]<<8)+byte[1])
#define word_to_little_bytes(word,byte)  byte[0]=word&0xFF; byte[1]=(word>>8);

struct UPDATE
{
    uint8 seq[2];
    uint8 ack;
    uint8 crc[2];
    uint8 len;
    uint8 data[1];
};

struct UPDATE_FILE
{
    uint8 file_sz[4];
    uint8 ver[2];
    uint8 blk_sz;
    uint8 type[8];
    uint8 data[1];
};

void delayreset(void);
uint8 update_frame_opt(uint8 data[], uint8 len);
void bootloader_start_update(uint8 data[], uint8 len);

#endif





