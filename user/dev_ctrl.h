
#ifndef _DEV_CTRL_H_
#define _DEV_CTRL_H_

#include "headfiles.h"
#include "alloter.h" 

typedef volatile unsigned char    OS_CPU_SR;

#define FRAME_OK        0x01
#define FRAME_ERROR     0xFF

#define uart_peek_data(buffer, len) peek_chn_byte((struct _CHN_SLOT *)&(uart_file_infor[0].rx_slot),buffer,len)

#define uart_read(buffer, len)   get_chn_bytes((struct _CHN_SLOT *)&(uart_file_infor[0].rx_slot),buffer,len)


#define  OS_CRITICAL_METHOD   3
#define  OS_ENTER_CRITICAL()  do{cpu_sr = ITC_GetSoftIntStatus() & 0x08;disableInterrupts();}while(0)
#define  OS_EXIT_CRITICAL()   do{if(cpu_sr){ disableInterrupts();}else{ enableInterrupts(); }}while(0)

#define GPIO_ReadInputPin(port,pin)     (port->IDR & pin)
#define GPIO_ReadOutputData(port, pin)  (port->ODR & pin)

#if COLOR_DIMMER
    #define PLC_RESET_PIN        GPIO_PIN_2
    #define PLC_RESET_PORT       GPIOD
#elif PWRCTRL_86
    #define PLC_RESET_PIN        GPIO_PIN_4
    #define PLC_RESET_PORT       GPIOD
#elif CURTAIN_MOTOR		//窗帘控制器使用PC3
#ifdef CURTAIN_STM8S005
	#define PLC_RESET_PIN        GPIO_PIN_7
    #define PLC_RESET_PORT       GPIOD
#else
    #define PLC_RESET_PIN        GPIO_PIN_3
    #define PLC_RESET_PORT       GPIOC
#endif
#endif
#if COLOR_DIMMER
    #define RELAY_CTRL1_PIN       GPIO_PIN_3
    #define RELAY_CTRL1_PORT      GPIOD
#elif PWRCTRL_86
    #define RELAY_CTRL1_PIN       GPIO_PIN_5
    #define RELAY_CTRL1_PORT      GPIOC
#endif

#if PWRCTRL_3
    #define RELAY_CTRL2_PIN       GPIO_PIN_6
    #define RELAY_CTRL2_PORT      GPIOC
    #define RELAY_CTRL3_PIN       GPIO_PIN_7
    #define RELAY_CTRL3_PORT      GPIOC
#endif

#if CURTAIN_MOTOR
#define	MOTOR_R_PIN          GPIO_PIN_4
#define MOTOR_R_PORT         GPIOD
#define	MOTOR_F_PIN          GPIO_PIN_3
#define MOTOR_F_PORT         GPIOD
#define MOTOR_OFF_PIN       GPIO_PIN_6
#define MOTOR_OFF_PORT      GPIOC

#define	CHECK_F_PIN         GPIO_PIN_5
#define CHECK_F_PORT        GPIOC
#define CHECK_R_PIN	    GPIO_PIN_7
#define CHECK_R_PORT        GPIOC

#endif

#define ZCP_PIN       GPIO_PIN_3
#define ZCP_PORT      GPIOB

#define ZEP_TIME_CNT    100  //Prescaler 16M/8  0.1ms记200次


#define RELAY_ON(pin)		do{(GPIOC->ODR |= pin);}while(0)
#define RELAY_OFF(pin)	    do{(GPIOC->ODR &= ~pin);}while(0)

#define SET_PIN_L(port,pin)            do{port->ODR &= ~pin;}while(0)
#define SET_PIN_H(port,pin)            do{port->ODR |= pin;}while(0)

struct soft_uart
{
	// int databitcnt; //data bit count
	// int bit_interval; 
	vuint16 bit;
	vuint16 data;
	vint8 bitcnt;	 //tx or rx bit count
	// volatile unsigned int *ptr;
}; 

#define UART_CH_CNT                 0x01

struct _UART_FILE_INFOR
{
	struct _CHN_SLOT    tx_slot;
	struct _CHN_SLOT    rx_slot;
//	uint8   chn_no;    
	uint8   busy_rxing; 
//	uint8   over_time_tick;
};

#if DEBUG
struct soft_uart_chn
{
    struct soft_uart tx;
    struct soft_uart rx;   
    uint8 databitcnt;   //data bit count
    int bit_interval; 
    struct _UART_FILE_INFOR *puart;
};
#endif

/*************************************************************/
//eeprom address 地址分配

#define ENCODE_MAGIC_ADDR   (0x4040)
#define ENCODE_PARAM_ADDR   (ENCODE_MAGIC_ADDR+4)

#define MAGIC_NUM          0x55AA2B2B
#define ADDRESS_OFFSET      (0x04)

#define OF_MAGIC_NUM        (0)
#define OF_MAGIC_NUM_LEN    (4)
#define OF_EEPROM_ADDRESS   (0x4000)
#define OF_PLC_PARAM        (OF_MAGIC_NUM+OF_MAGIC_NUM_LEN)
#define OF_RELAY_STAUS      (OF_PLC_PARAM+sizeof(struct EEP_PARAM))
#define OF_UPDATE_FLAG      (0x270)
#define OF_FILE_CRC         (0x271)
#define OF_RELAY_FLAG		(0x100)
/*************************************************************/

#define SET_UART_BPS(baud)          UART_DeInit(baud)


DATA_SEG extern volatile struct _UART_FILE_INFOR uart_file_infor[UART_CH_CNT];
DATA_SEG extern volatile time_t __sys_tick;
#if DEBUG
DATA_SEG extern volatile struct soft_uart_chn _uart_chns[1];
#endif

void uart_rx_hook(void);
void modules_init(void);
void uart_write(unsigned char buffer[],uint8 len);
uint8 uart_chn_rx_byte(uint8 c);
uint8 uart_chn_tx_byte(uint8 buffer[]);
void EEP_Read(uint8 addr, uint8 buf[], uint8 len);
void EEP_Write(uint8 addr, uint8 buf[], uint8 len);
void UART_DeInit(uint16 baud);
void chk_key_pressed(void);
void uart_tick_hook(void);
void clear_uart(uint8 len);
void sys_tick(void *args);
void relay_delay_time(void);
void pwm_color_adjust(uint8 chn, uint8 prescaler);
#if DEBUG
void get_soft_uart_rx_data(uint8 c);
uint8 get_soft_uart_tx_data(struct soft_uart_chn *pchn);
#endif
#endif


