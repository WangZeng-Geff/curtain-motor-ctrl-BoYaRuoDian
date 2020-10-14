#include "headfiles.h"

#define MAX_BIT_CNT     (1+8+1)
#define MAX_TIME_VOER   (500)             //100us unit   debug modify

void TIM3_SetCompare(uint16 compare)
{
    TIM3->ARRH = (u8)((compare - 1) >> 8);
    TIM3->ARRL = (u8)(compare - 1);
    TIM3->CNTRH = 0;
    TIM3->CNTRL = 0;
    TIM3->SR1 = (u8)(~((u8)(TIM3_FLAG_UPDATE)));
}

void tim3_init(void)//初始化定时器
{
    TIM3->PSCR = (u8)(TIM3_PRESCALER_16);  //16分频
    TIM3->CR1 |= (u8)(TIM3_CR1_URS);
    TIM3_SetCompare(TIM3_CLK_FREQUENCY / 9600);
    TIM3->EGR = (u8)TIM3_PSCRELOADMODE_IMMEDIATE;
    TIM3->SR1 = (u8)(~((u8)(TIM3_FLAG_UPDATE)));//TIM3_ClearFlag(TIM3_FLAG_UPDATE);
    TIM3->CR1 |= (u8)(TIM3_CR1_CEN);
}

void wait_t(void)
{
    while(0x00 == (TIM3->SR1 & TIM3_FLAG_UPDATE));
    TIM3->SR1 = (u8)(~TIM3_FLAG_UPDATE);//清中断标志
}

uint16 _send_fmt(uint8 x)
{
	uint16 databit = x;
	databit |= 1 << 8; //stop bit
	return(databit);
}
uint8 recv_chk(unsigned int x)
{
	return(x == _send_fmt(x & 0xFF));
}

void put_char(unsigned char ch)
{
    uint16 data;
    uint8 i;

    data = _send_fmt(ch);
    data  <<= 0x01;
	data &= (1 << 10)-1;

    wait_t();
    for(i = 0; i < MAX_BIT_CNT; i++) 
    {
        if(data & 0x01) 
        {
            TX_SWIM_SET();
        }
        else
        {
            TX_SWIM_CLR();
        }
        data >>= 0x01;
        wait_t();
    }
}

void send_data_to_swim(uint8 buf[], uint8 len)
{
    uint8 i;

    GPIO_Init(SWIM_PORT,SWIM_PIN,GPIO_MODE_OUT_PP_HIGH_FAST);

    wait_t();
    for(i=0; i<len; i++)
    {
        put_char(buf[i]);
    }
}

uint8 recv_from_swim(uint8 *c)
{
    uint16 data = 0;
    uint16 time_over = 0;
    volatile uint16 bit = 1;
    uint8 i;

    while(RX_SWIM_VALUE()) 
    {
        if(TIM3->SR1 & TIM3_FLAG_UPDATE) 
        {
            TIM3->SR1 = (u8)(~TIM3_FLAG_UPDATE);//清中断标志
            time_over++;
            if(time_over >= MAX_TIME_VOER) return(0);
        }
    }
    TIM3_SetCompare((TIM3_CLK_FREQUENCY / 9600)>>1);
    wait_t();
    TIM3_SetCompare(TIM3_CLK_FREQUENCY / 9600);
    for(i = 0; i < MAX_BIT_CNT-1; i++) 
    {
        wait_t();
        if(RX_SWIM_VALUE()) data |= bit;
        bit <<= 1;
    }
    if(recv_chk(data)) 
    {
        *c = (uint8)(data&0xFF);
        return(1);
    }
    return(0);
}

void swim_init(void)//
{
    CFG_GCR |= CFG_GCR_SWD;//禁用SWIM 配置为普通IO
    GPIO_Init(SWIM_PORT,SWIM_PIN,GPIO_MODE_IN_PU_NO_IT);
}

void swim_io_recover(void)
{
    TIM3_DISABLE();
    CFG_GCR &= (u8)(~CFG_GCR_SWD);//启用SWIM
    //GPIO_Init(SWIM_PORT,SWIM_PIN,GPIO_MODE_OUT_PP_HIGH_FAST);
}

struct CODE_FRAME *get_code_frame(uint8 buf[], uint8 len)
{
    uint8 i;
    uint8 length;
    struct CODE_FRAME *pframe = NULL;

    for(i=0; i < len; i++)
    {
start_lbl:
        if(STC == buf[i]) break;
    }
    if(i >= len) return(NULL);

    pframe = (struct CODE_FRAME*)&buf[i];
    length = pframe->len;
    if((length > len) || (pframe->data[length] != checksum(&buf[i], length+2)))
    {
        i++;
        goto start_lbl;
    }
    return(pframe);
}

uint8 get_device_infor(uint8 buf[], uint8 len)//SN编码写入
{
    uint8 ret;
    uint32 magic;
    struct CODE_FRAME *pframe;

    if(len < 2) return(0);

    pframe = get_code_frame(buf, len);
    if(NULL == pframe) return(0);

    if(ENCODE_LEN != pframe->len)
    {   
        if(ENCODE_LEN+pframe->data[ENCODE_LEN]+1!=pframe->len) return(0);
        if(pframe->data[ENCODE_LEN] > MAX_ATTRIBUTE_LEN) return(0);
    }
    else
    {
        ret = 0;
        EEP_Write((ENCODE_PARAM_ADDR - OF_EEPROM_ADDRESS) + ENCODE_LEN, &ret, 1);
    }
    EEP_Write((ENCODE_PARAM_ADDR - OF_EEPROM_ADDRESS), pframe->data, pframe->len);
    magic = ENCODE_MAGIC;
    EEP_Write((ENCODE_MAGIC_ADDR - OF_EEPROM_ADDRESS), (uint8 *)&magic, 4);
    EEP_Read((ENCODE_PARAM_ADDR - OF_EEPROM_ADDRESS), pframe->data, pframe->len);
    ret = pframe->len;
    pframe->data[ret] = checksum ((uint8 *)pframe, ret+2);
    send_data_to_swim((uint8 *)pframe, ret+3);
    return(1);
}

void device_encode_opt(void)
{
    static unsigned char tmp_buf[4]={0x7E, 0x01, 0x00, 0x7F};
    unsigned char try_cnt=3, ret, len;

    swim_init();
    tim3_init();

try_lbl:
    if(try_cnt < 1) 
    {
        swim_io_recover();
        return;
    }
    try_cnt--;
    send_data_to_swim((uint8 *)tmp_buf, sizeof((tmp_buf)));

    GPIO_Init(SWIM_PORT,SWIM_PIN,GPIO_MODE_IN_PU_NO_IT);
    len =0;
    do
    {
        IWDG->KR = IWDG_KEY_REFRESH; 
        ret = recv_from_swim(&g_frame_buffer[len]);
        len++;
    }while(ret);

    ret = get_device_infor(g_frame_buffer, len);
    IWDG->KR = IWDG_KEY_REFRESH; 
    if(0x00 == ret) goto try_lbl;
}




