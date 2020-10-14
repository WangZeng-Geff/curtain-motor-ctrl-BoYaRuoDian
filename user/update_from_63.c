#include "headfiles.h"

#define UPDATE_FINISHED     0xFFFF
#define APP_ADDR            0x9000
#define UPDATE_FLAG_OFFSET  0x28
#define UPDATE_FLAG_ADDR    (APP_ADDR + UPDATE_FLAG_OFFSET)
#define UPDATE_FLAG         0x00


static uint8 delay_reset=0;
void delayreset(void)
{
    if(delay_reset>0)
    {
        delay_reset--;
        if(0x00 == delay_reset)
        {
            _asm("JPF [0x8001]");
        }
    }
}

extern const uint8 _dev_soft_ver[];
uint8 update_hframe_opt(struct UPDATE *pupdate, uint8 len)
{
    uint8 ret = 0, _device_type[8];
    uint16 seq_no;
    struct UPDATE_FILE *pup_file = (struct UPDATE_FILE *)pupdate->data;

    _get_dev_type(_device_type);
    if ((0x00 != memcmp(_device_type, pup_file->type, sizeof(pup_file->type)))
            ||(len <= offset_of(struct UPDATE_FILE, data)))//判断设备类型不相等或长度小于data
    {
        mymemcpy(pup_file->type, _device_type, sizeof(pup_file->type));
        ret = offset_of(struct UPDATE_FILE, data);
        //mymemcpy(pup_file->data, (uint8 *)dev_infor, strlen(dev_infor));
        //ret += strlen(dev_infor);
        //ret += _get_dev_infor(pup_file->data);
        seq_no = 0;//错误包
        goto end_lbl;
    }
    if(0x00 == compare_soft_ver(pup_file->data, len))
    {//update end
        seq_no = UPDATE_FINISHED;
        goto end_lbl;
    }
    seq_no = 1;//可以升级
end_lbl:
    word_to_little_bytes(seq_no, pupdate->seq);
    return (ret);
}

uint8 update_frame_opt(uint8 data[], uint8 len)
{
    uint8 ret = 0;
    struct UPDATE *pupdate = (struct UPDATE *)&data[0];

    //判断数据长度是否正确
    if (len != pupdate->len + offset_of(struct UPDATE, data))
        return (ret);

    ret = update_hframe_opt(pupdate, pupdate->len);
    pupdate->len = ret;
    ret = offset_of(struct UPDATE, data) + pupdate->len;
    pupdate->ack = 0;
    return (ret);
}

void bootloader_start_update(uint8 data[], uint8 len)
{
    struct UPDATE *pupdate = (struct UPDATE *)data;
    uint8 buffer[14] = {0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x23,0x01,0x00};
    uint32 flag = UPDATE_FLAG;
		
    if (pupdate->data[1])
    {
        buffer[13] = 0xA4;
        uart_write(buffer, sizeof(buffer));
        FLASH_Unlock(FLASH_MEMTYPE_PROG);
		*((uint8 *)UPDATE_FLAG_ADDR) = UPDATE_FLAG;
		while ((u8)(FLASH->IAPSR & ((u8)(0x04) | (u8)(0x01))) == 0x00);
		FLASH_Lock(FLASH_MEMTYPE_PROG);
        delay_reset = 10;//delay per 100ms reset system
    }
}



