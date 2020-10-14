#ifndef	_ENCODE_H_
#define	_ENCODE_H_
#include "headfiles.h"

#define TIM3_CLK_FREQUENCY    TIM_CLK_FREQUENCY/16
#if 0                                      //debug pin
    #define SWIM_PIN        GPIO_PIN_0
    #define SWIM_PORT       GPIOB
#else
    #define SWIM_PIN        GPIO_PIN_1
    #define SWIM_PORT       GPIOD
#endif

#define	RX_SWIM_VALUE()	 GPIO_ReadInputPin(SWIM_PORT,SWIM_PIN)
#define	TX_SWIM_SET()	 GPIO_WriteHigh(SWIM_PORT,SWIM_PIN)
#define	TX_SWIM_CLR()	 GPIO_WriteLow(SWIM_PORT,SWIM_PIN)

#define test_pin()       GPIO_WriteReverse(SWIM_PORT,SWIM_PIN)

#define TIM3_DISABLE() do{TIM3->CR1 &= (u8)(~TIM3_CR1_CEN);}while(0)

#define MAX_ATTRIBUTE_LEN   60
#define SN_LEN          12
#define DKEY_LEN        8
#define ENCODE_LEN      (SN_LEN+DKEY_LEN+ID_LEN+PWD_LEN)

#define ENCODE_MAGIC    0x552B2BAA
                          
struct CODE_FRAME
{
    uint8 start;
    uint8 len;
    uint8 data[1];
};


struct DEV_INFOR
{
    uint8   sn[SN_LEN];
    uint8   dkey[DKEY_LEN];
    uint8   id[ID_LEN];
    uint8   password[PWD_LEN];
    uint8   len;
    uint8   infor[1];
};

struct ENCODE_PARAM
{
    uint32  sole_magic;
    struct DEV_INFOR dev;
};



void device_encode_opt(void);
#endif


