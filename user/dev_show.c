#include "headfiles.h"

#if DEV_SHOW

struct DEV_SEARCH_PARAM dev_search_param;

static void send_search_ack_frame(void)
{
   struct SHS_frame *pshs_frame = (struct SHS_frame *)g_frame_buffer;
   struct FBD_Frame *pfbd_frame;
   uint8 len = 0, dev_len, dev_infor[50];
   struct ENCODE_PARAM encode_param;
   EEP_Read((ENCODE_MAGIC_ADDR - OF_EEPROM_ADDRESS), (uint8 *)&encode_param, sizeof(struct ENCODE_PARAM));

   if (ENCODE_MAGIC != encode_param.sole_magic)
   {
      return;
   }
   dev_len = _get_dev_infor(dev_infor);
   pshs_frame->stc = STC;
   pshs_frame->seq = dev_search_param.ack_seq;

   mymemcpy(pshs_frame->said, eep_param.id, ID_LEN);
   mymemcpy(pshs_frame->taid, dev_search_param.search_id, ID_LEN);
   if((is_all_xx(pshs_frame->said, 0x00, ID_LEN))||(is_all_xx(pshs_frame->taid, 0x00, ID_LEN)))
      return;
   pshs_frame->infor[0] = CMD_SHOW;
   pfbd_frame = (struct FBD_Frame *)&pshs_frame->infor[1];

   //if you want more infor, please add org frame....
   len += org_fbd_frame((uint8 *)pfbd_frame, did_item(0x0007), encode_param.dev.sn, SN_LEN);
   len += org_fbd_frame((uint8 *)pfbd_frame + len, did_item(0x0005), encode_param.dev.dkey, DKEY_LEN);
   len += org_fbd_frame((uint8 *)pfbd_frame + len, did_item(0x000A), eep_param.password, PWD_LEN);
   len += org_fbd_frame((uint8 *)pfbd_frame + len, did_item(0x0003), dev_infor, dev_len);

   len += 1; //cmd
   pshs_frame->length = len;
   pshs_frame->infor[len] = checksum((uint8 *)pshs_frame, len + SHS_FRAME_HEAD);
   len += SHS_FRAME_HEAD + 1; //cs
   uart_write(g_frame_buffer, len);
}

void check_search_delay(void)
{
    if (dev_search_param.silent_time > 0) dev_search_param.silent_time--;
    if (dev_search_param.ack_count > 0)
    {
       dev_search_param.ack_count--;
       if (dev_search_param.ack_count > 0) return;
       send_search_ack_frame();
    }
}

//---- please modify this function according u device -------
void check_dev_show(void)
{
    if(dev_search_param.dev_show_flag)
    {
        if(dev_search_param.dev_show_flag == 5)
        {
            SET_PIN_H(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
            SET_PIN_L(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
            SET_PIN_L(Motor1.GPIOx_off, Motor1.GPIO_Pin_off);
            relay_timer = 8;
        }

        if (--dev_search_param.dev_show_flag == 0)
        {
            SET_PIN_H(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
            SET_PIN_L(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
            SET_PIN_L(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
            relay_timer = 8;
        }
    }
}


//------------------------------------------------------------------------
static uint8 sn_type_check(uint8 data[],uint8 sn_data[])
{
    if (is_all_xx(data, 0xFF, TYPE_LEN))
    {
        return 1;
    }
    else if (((0x0f & data[1]) == (0x0f & sn_data[1]))
            && ((0xf0 & data[3]) == (0xf0 & sn_data[3]))
            && (data[2] == sn_data[2]))
    {
        return 1;
    }
    return 0;
}

//7E XX XX XX XX FF FF FF FF 00 0D 03  41 00  08 00   07   00 06 00 XX XX XX XX CS
uint8 search_frame_opt(struct SHS_frame *pframe)
{
    struct ENCODE_PARAM encode_param;
    struct FBD_Frame * fbd = (struct FBD_Frame *)&pframe->infor[3];
    
    uint8_t search_type,search_did[DID_LEN] = {0x08,0x00};
    
    EEP_Read((ENCODE_MAGIC_ADDR - OF_EEPROM_ADDRESS), (uint8 *)&encode_param, sizeof(struct ENCODE_PARAM));
    if (ENCODE_MAGIC != encode_param.sole_magic)
        goto end;

    if(!(is_all_xx(pframe->taid, 0xff, ID_LEN)) \
        || (memcmp(fbd->did,search_did,DID_LEN)) \
        || (pframe->length < (fbd->ctrl&0x7F) + FBD_FRAME_HEAD))   
        goto end;
        
        
    if((dev_search_param.silent_time > 0) || (!is_all_xx(eep_param.panid, 0x00, PANID_LEN)))   
        goto end;   
        
    search_type = fbd->data[0];//00,dev search, 01 key search
    mymemcpy(dev_search_param.search_id, pframe->said, ID_LEN); //save
    if (is_gid_equal(&pframe->infor[1])) //41 00
    {
        if((!search_type) && (sn_type_check(&fbd->data[1+TIME_LEN],encode_param.dev.sn))) 
        {
            dev_search_param.max_delay_time = get_le_val(&fbd->data[1], TIME_LEN);;
            if(!dev_search_param.max_delay_time) dev_search_param.max_delay_time = 10;//default time
            dev_search_param.ack_seq = pframe->seq | 0x80;
            srand(dev_search_param.cur_time + get_le_val(eep_param.id,ID_LEN));
            dev_search_param.ack_count = (rand() % (dev_search_param.max_delay_time)) + 1;
        }
    }

end:
    return (0); //no ack     
}

#endif




