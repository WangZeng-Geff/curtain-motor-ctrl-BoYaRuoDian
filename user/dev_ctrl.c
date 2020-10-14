#include "headfiles.h"

#ifdef CALIBRATION
DATA_SEG vuint8 measure_state = MEASURE_START;
#endif

u8 ITC_GetCPUCC(void)
{
#ifdef _COSMIC_
    _asm("push cc");
    _asm("pop a");
    return;	/* Ignore compiler warning, the returned value is in A register */
#endif
#ifdef _IAR_SYSTEMS_
    __asm("push cc");
    __asm("pop a");
#endif
}
u8 ITC_GetSoftIntStatus(void)
{
	return(u8)(ITC_GetCPUCC() & CPU_CC_I1I0);
}

DATA_SEG volatile struct _UART_FILE_INFOR uart_file_infor[UART_CH_CNT];
//DATA_SEG volatile struct soft_uart_chn _uart_chns[2];

void UART_CHN_Soft_Init(void)
{
	struct _UART_FILE_INFOR *puart_infor = (struct _UART_FILE_INFOR *)&uart_file_infor[0];    

//	puart_infor = &uart_file_infor[chn];

//	puart_infor->chn_no = chn;
	puart_infor->rx_slot.data_cnt = 0x00;
//	puart_infor->rx_slot.data_max = 0xF0 ;
	puart_infor->rx_slot.tx = INVALID_PTR;
	puart_infor->rx_slot.rx = INVALID_PTR;

	mymemcpy(&puart_infor->tx_slot,&puart_infor->rx_slot,sizeof(puart_infor->rx_slot));

//	puart_infor->over_time_tick = UART_CHAR_MAX_DELAY ;
}
void empty_a_chn_slot(struct _CHN_SLOT *pCHN_SLOT, uint8 len)
{
	unsigned char c;

	while(len > 0)
    {
        len--;
        get_chn_bytes(pCHN_SLOT, &c, 1);
    }
}

#ifdef CURTAIN_STM8S005
void UART2_ITConfig(UART2_IT_TypeDef UART2_IT, FunctionalState NewState)
{
    u8 itpos = 0x00;
	itpos = (u8)((u8)1 << (u8)((u8)UART2_IT & (u8)0x0F));

	if(NewState != DISABLE)
	{
		/**< Enable the Interrupt bits according to UART1_IT mask */
		UART2->CR2 |= itpos;
	}
	else
	{
		UART2->CR2 &= (u8)(~itpos);
	}
}

void UART_DeInit(uint16 baud)
{

	UART2->CR1 |= (UART2_WORDLENGTH_8D | UART2_PARITY_NO); /**< Set the word length bit according to UART1_WordLength value */

	UART2->CR3 |= UART2_STOPBITS_1;	 /**< Set the STOP bits number according to UART1_StopBits value  */

    UART2->BRR2 = (u8)(((SYS_CLK/baud >> 8) & 0xF0) | (SYS_CLK/baud & 0x0F));
    UART2->BRR1 = (u8)((SYS_CLK/baud >> 4) & 0xFF);

	UART2->CR2 = (u8)UART2_CR2_TEN | (u8)UART2_CR2_REN;  //enable the TXRX
	/**< Set the Clock Enable bit, lock Polarity, lock Phase and Last Bit Clock pulse bits according to UART1_Mode value */

    UART2_ITConfig(UART2_IT_RXNE, ENABLE);
}
#else
void UART1_ITConfig(UART1_IT_TypeDef UART1_IT, FunctionalState NewState)
{
//	u8 uartreg;
    u8 itpos = 0x00;
	/* Get the UART1 register index */
//	uartreg = (u8)(UART1_IT >> (u8)0x08);
	/* Get the UART1 IT index */
	itpos = (u8)((u8)1 << (u8)((u8)UART1_IT & (u8)0x0F));

	if(NewState != DISABLE)
	{
		/**< Enable the Interrupt bits according to UART1_IT mask */
//		if(uartreg == 0x01)
//		{
//			UART1->CR1 |= itpos;
//		}
//		else if(uartreg == 0x02)
		{
			UART1->CR2 |= itpos;
		}
 //   	else
 //   	{
 //   		UART1->CR4 |= itpos;
 //   	}
	}
	else
	{
		/**< Disable the interrupt bits according to UART1_IT mask */
 //   	if(uartreg == 0x01)
 //   	{
 //   		UART1->CR1 &= (u8)(~itpos);
 //   	}
 //   	else if(uartreg == 0x02)
		{
			UART1->CR2 &= (u8)(~itpos);
		}
 //   	else
 //   	{
 //   		UART1->CR4 &= (u8)(~itpos);
 //   	}
	}

}

void UART_DeInit(uint16 baud)
{
//	u8 dummy;

	/*< Clear the Idle Line Detected bit in the status rerister by a read
	   to the UART1_SR register followed by a Read to the UART1_DR register */
//	dummy = UART1->SR;
//	dummy = UART1->DR;
//
//	UART1->BRR2 = UART1_BRR2_RESET_VALUE;  /*< Set UART1_BRR2 to reset value 0x00 */
//	UART1->BRR1 = UART1_BRR1_RESET_VALUE;  /*< Set UART1_BRR1 to reset value 0x00 */
//
//	UART1->CR1 = UART1_CR1_RESET_VALUE;	/*< Set UART1_CR1 to reset value 0x00  */
//	UART1->CR2 = UART1_CR2_RESET_VALUE;	/*< Set UART1_CR2 to reset value 0x00  */
//	UART1->CR3 = UART1_CR3_RESET_VALUE;	/*< Set UART1_CR3 to reset value 0x00  */
//	UART1->CR4 = UART1_CR4_RESET_VALUE;	/*< Set UART1_CR4 to reset value 0x00  */
//	UART1->CR5 = UART1_CR5_RESET_VALUE;	/*< Set UART1_CR5 to reset value 0x00  */
//
//	UART1->GTR = UART1_GTR_RESET_VALUE;
//	UART1->PSCR = UART1_PSCR_RESET_VALUE;

//    UART1->CR1 &= (u8)(~UART1_CR1_M);  /**< Clear the word length bit */
	UART1->CR1 |= (UART1_WORDLENGTH_8D | UART1_PARITY_NO); /**< Set the word length bit according to UART1_WordLength value */

//	UART1->CR3 &= (u8)(~UART1_CR3_STOP);  /**< Clear the STOP bits */
	UART1->CR3 |= UART1_STOPBITS_1;	 /**< Set the STOP bits number according to UART1_StopBits value  */

//	UART1->CR1 &= (u8)(~(UART1_CR1_PCEN | UART1_CR1_PS  ));	 /**< Clear the Parity Control bit */
//	UART1->CR1 |= UART1_PARITY_EVEN;  /**< Set the Parity Control bit to UART1_Parity value */
  
    /* 16M/1200 = 0x3415, so BRR2(bit12:15 0:3) = 0x35 BRR1(bit 4:11) = 0x41 */
    UART1->BRR2 = (u8)(((SYS_CLK/baud >> 8) & 0xF0) | (SYS_CLK/baud & 0x0F));
    UART1->BRR1 = (u8)((SYS_CLK/baud >> 4) & 0xFF);

//	UART1->CR2 &= (u8)~(UART1_CR2_TEN | UART1_CR2_REN);	/**< Disable the Transmitter and Receiver before seting the LBCL, CPOL and CPHA bits */
//	UART1->CR3 &= (u8)~(UART1_CR3_CPOL | UART1_CR3_CPHA | UART1_CR3_LBCL); /**< Clear the Clock Polarity, lock Phase, Last Bit Clock pulse */
//	UART1->CR3 |= (u8)((u8)UART1_SYNCMODE_CLOCK_DISABLE & (u8)(UART1_CR3_CPOL | UART1_CR3_CPHA | UART1_CR3_LBCL));	/**< Set the Clock Polarity, lock Phase, Last Bit Clock pulse */

	UART1->CR2 = (u8)UART1_CR2_TEN | (u8)UART1_CR2_REN;  //enable the TXRX
	/**< Set the Clock Enable bit, lock Polarity, lock Phase and Last Bit Clock pulse bits according to UART1_Mode value */
//	UART1->CR3 &= (u8)(~UART1_CR3_CKEN); /**< Clear the Clock Enable bit */

   // UART1->CR1 &= (u8)(~UART1_CR1_UARTD); /**< UART1 Enable */

    UART1_ITConfig(UART1_IT_RXNE, ENABLE);
}
#endif

uint8 uart_chn_tx_byte(uint8 buffer[])
{
	return(get_chn_bytes((struct _CHN_SLOT *)&(uart_file_infor[0].tx_slot),buffer,1));
}

uint8 uart_chn_rx_byte(uint8 c)
{
	return(put_chn_bytes((struct _CHN_SLOT *)&(uart_file_infor[0].rx_slot),&c,1));
}

void uart_rx_hook(void)
{
	uart_file_infor[0].busy_rxing = UART_CHAR_MAX_DELAY;   
    notify(EV_RXCHAR);
}

void clear_uart(uint8 len)
{
	empty_a_chn_slot((struct _CHN_SLOT *)&(uart_file_infor[0].rx_slot),len);
}
void uart_tick_hook(void)
{    
    uint8 datacnt,idx;

    if(uart_file_infor[0].busy_rxing  > 0x00)
    {
        datacnt = uart_file_infor[0].rx_slot.data_cnt;//防止中断竞争
        uart_file_infor[0].busy_rxing--;
        if(0x00 == uart_file_infor[0].busy_rxing)
        {
#if DEBUG
            idx = uart_peek_data(g_frame_buffer, sizeof(g_frame_buffer));
            if (NULL != get_smart_frame(g_frame_buffer,idx))
            {
                GPIO_WriteReverse(RELAY_CTRL1_PORT,RELAY_CTRL1_PIN);
            }
#endif			            
			clear_uart(datacnt);
        }
    }
}

void uart_write(unsigned char buffer[],uint8 len)
{
	put_chn_bytes((struct _CHN_SLOT *)&(uart_file_infor[0].tx_slot),buffer,len);
#ifdef CURTAIN_STM8S005
	UART2_ITConfig(UART2_IT_TC, ENABLE);
#else
    UART1_ITConfig(UART1_IT_TC, ENABLE);
#endif
}  

#define UART_RX_BIT     (1 << 0)
#define UART_TX_BIT     (1 << 1)
/**
   *gpio初始化后，使用io输入输出互相转换，CR2控制寄存器
   *中断使能位与输出10MHz位一致，可能导致在输出10MHz配置，
   *转换成输入时导致中断使能位置位，产生中断！！
*/
void GPIO_Init(GPIO_TypeDef* GPIOx,
			   GPIO_Pin_TypeDef GPIO_Pin,
			   GPIO_Mode_TypeDef GPIO_Mode)
{
    GPIOx->CR2 &= (u8)(~(GPIO_Pin));

	if((((u8)(GPIO_Mode)) & (u8)0x80) != (u8)0x00) /* Output mode */
	{
		if((((u8)(GPIO_Mode)) & (u8)0x10) != (u8)0x00) /* High level */
		{
			GPIOx->ODR |= (u8)GPIO_Pin;
		}
		else /* Low level */
		{
			GPIOx->ODR &= (u8)(~(GPIO_Pin));
		}
		/* Set Output mode */
		GPIOx->DDR |= (u8)GPIO_Pin;
	}
	else /* Input mode */
	{
		/* Set Input mode */
		GPIOx->DDR &= (u8)(~(GPIO_Pin));
	}

	if((((u8)(GPIO_Mode)) & (u8)0x40) != (u8)0x00) /* Pull-Up or Push-Pull */
	{
        GPIOx->CR1 |= (u8)GPIO_Pin;
	}
	else /* Float or Open-Drain */
	{
		GPIOx->CR1 &= (u8)(~(GPIO_Pin));
	}

	if((((u8)(GPIO_Mode)) & (u8)0x20) != (u8)0x00) /* Interrupt or Slow slope */
	{
		GPIOx->CR2 |= (u8)GPIO_Pin;
	}
//	else /* No external interrupt or No slope control */
//	{
//		GPIOx->CR2 &= (u8)(~(GPIO_Pin));
//	}

}

DATA_SEG static uint8 __chk_key_seq=0;

#if 0
void chk_key_pressed(void)
{
    __chk_key_seq <<= 0x01;
    if (0x00 == GPIO_ReadInputPin(GPIOA,GPIO_PIN_3))
    {
        __chk_key_seq |= 0x01;
    }

    if (0xFF == __chk_key_seq)
    {//启动按键注册任务
        notify(EV_KEY);
    }
}
#endif

#define MAX_KEYREG_TIME         5
#ifdef CALIBRATION
vuint8 calibtate_tick = 0;
#endif
/*
函数描述：
*/
DATA_SEG volatile time_t __sys_tick = 0;
void sys_tick(void *args)
{
#if OS_CRITICAL_METHOD == 3     /* Allocate storage for CPU status register */
	OS_CPU_SR     cpu_sr = 0;
#endif
	static volatile time_t last_tick; 
	time_t deltms;
	static volatile uint8 _msec = 0xff,tick_cnt = 0, tick_20ms = 0;
    static volatile uint8 _sec = 0;

    if (0xff == _msec) 
    {
	   _msec = 0;
	   last_tick = __sys_tick;
	   return;
    }
	deltms = __sys_tick - last_tick;
        dev_search_param.cur_time++;

	if(deltms >= 10)
	{
        last_tick += 10;
        notify(EV_10MS);

        if (++tick_cnt < 10) return; 
        tick_cnt = 0;
        notify(EV_100MS);
		delayreset();
#ifdef CALIBRATION
        if (calibtate_tick > 0)
        {
            calibtate_tick--;
        }
#endif		        
        if(++_msec < 10) return;
        _msec = 0; 
        notify(EV_SEC);
        if(++_sec < 60) return;
        _sec = 0;
        notify(EV_MIN);
	}
}
#ifdef CALIBRATION

#define IDEAL_BIT_INTERVAL_CNT_VALUE 1667//Actually 16000000/9600=1666.666667
#define MAX_TRIM_VAL 3
#define MIN_TRIM_VAL -4
#define MAX_ERR 9*(1+8+1)
vuint16 diff;
void hsi_calibration(void)
{
	vs8 trim_val;
	uint8 i = 0;

	if(MEASURE_COMPLETED != measure_state)
	{
		return;
	}
	while(diff >= IDEAL_BIT_INTERVAL_CNT_VALUE)
	{
		diff -= IDEAL_BIT_INTERVAL_CNT_VALUE;
		i++;
	}
	if(diff >= (IDEAL_BIT_INTERVAL_CNT_VALUE>>1))
	{
		i++;
	}
	if(i > 11)
	{
		measure_state = MEASURE_START;
		return;
	}

	trim_val = CLK->HSITRIMR & CLK_HSITRIMR_HSITRIM;
	if(trim_val & (0x01 << 2))
	{
		trim_val = ((~trim_val) & 0x07) + 1;
		trim_val = -trim_val;
	}
	if((diff >= 9*i) && (diff < (IDEAL_BIT_INTERVAL_CNT_VALUE>>1)))//faster,should be slower
	{
		trim_val++;
		if(trim_val > 3)
		{
			trim_val = 3;
		}
		CLK->HSITRIMR = (trim_val & CLK_HSITRIMR_HSITRIM);
	}
	else
		if((diff >= (IDEAL_BIT_INTERVAL_CNT_VALUE>>1)) && (diff < IDEAL_BIT_INTERVAL_CNT_VALUE - 9*i))//slower,should be faster
	{

		trim_val--;
		if(trim_val < -4)
		{
			trim_val = -4;
		}
		CLK->HSITRIMR = (trim_val & CLK_HSITRIMR_HSITRIM);
	}
	measure_state = MEASURE_START;
}
#endif


void zcp_gpio_init(void)
{
    GPIO_Init(ZCP_PORT, ZCP_PIN, GPIO_MODE_IN_FL_NO_IT );
    EXTI->CR1 &= (u8)(~EXTI_CR1_PBIS);
    EXTI->CR1 |= 0x04;//01xx xxxx   仅仅上升沿中断
}


void TIM2_DeInit(void)
{
	TIM2->CR1 = (u8)TIM2_CR1_RESET_VALUE;
	TIM2->IER = (u8)TIM2_IER_RESET_VALUE;
  TIM2->SR1 = (u8)TIM2_SR1_RESET_VALUE;
	TIM2->SR2 = (u8)TIM2_SR2_RESET_VALUE;

	/* Disable channels */
	TIM2->CCER1 = (u8)TIM2_CCER1_RESET_VALUE;
	TIM2->CCER2 = (u8)TIM2_CCER2_RESET_VALUE;

	/* Then reset channel registers: it also works if lock level is equal to 2 or 3 */
	TIM2->CCMR1 = (u8)TIM2_CCMR1_RESET_VALUE;//
	TIM2->CCMR2 = (u8)TIM2_CCMR2_RESET_VALUE;
	TIM2->CCMR3 = (u8)TIM2_CCMR3_RESET_VALUE;
	TIM2->CNTRH = (u8)TIM2_CNTRH_RESET_VALUE;
	TIM2->CNTRL = (u8)TIM2_CNTRL_RESET_VALUE;
	TIM2->PSCR = (u8)(TIM2_PRESCALER_16);
	TIM2->ARRH  = (u8)TIM2_ARRH_RESET_VALUE;
	TIM2->ARRL  = (u8)TIM2_ARRL_RESET_VALUE;
	TIM2->CCR1H = (u8)TIM2_CCR1H_RESET_VALUE;
	TIM2->CCR1L = (u8)TIM2_CCR1L_RESET_VALUE;
	TIM2->CCR2H = (u8)TIM2_CCR2H_RESET_VALUE;
	TIM2->CCR2L = (u8)TIM2_CCR2L_RESET_VALUE;
	TIM2->CCR3H = (u8)TIM2_CCR3H_RESET_VALUE;
	TIM2->CCR3L = (u8)TIM2_CCR3L_RESET_VALUE;	
        TIM2->CR1 |= TIM2_CR1_CEN;
}


void TIM2_SetCompare1(u16 Compare1)
{
  /* Set the Capture Compare1 Register value */
  TIM2->CCR1H = (u8)(Compare1 >> 8);
  TIM2->CCR1L = (u8)(Compare1);
}

void zcp_time_init(uint16 time)
{
//    TIM2->CNTRH = (u8)TIM2_CNTRH_RESET_VALUE;
//    TIM2->CNTRL = (u8)TIM2_CNTRL_RESET_VALUE;

    TIM2->SR1 &= (u8)(~TIM2_IT_CC1);
    time += (u16)TIM2->CNTRL | (u16)(TIM2->CNTRH << 8);
    TIM2_SetCompare1(time);
    TIM2->IER |= (u8)(TIM2_IT_CC1);


//    TIM2->CR1 |= TIM2_CR1_CEN;
}

extern uint8 relay_delay_flag;

void relay_delay_time(void)
{
    if (_OFF == relay_delay_flag)
        zcp_time_init((uint16)eep_param.relay_off_delay * (uint16)ZEP_TIME_CNT);
    else
        zcp_time_init((uint16)eep_param.relay_on_delay * (uint16)ZEP_TIME_CNT);
}


void EEP_Read(uint8 addr, uint8 buf[], uint8 len)
{
	uint8 i;

	FLASH_Unlock(FLASH_MEMTYPE_DATA);
	for(i = 0; i < len; i++)
	{
		buf[i] = FLASH_ReadByte(OF_EEPROM_ADDRESS+addr+i);
	}
	FLASH_Lock(FLASH_MEMTYPE_DATA);
}

#if 0
uint8 EEP_Read_byte(uint8 addr)
{
    uint8 result; 

    EEP_Read(addr, &result, 1);
    return(result);
}
#endif

void EEP_Write(uint8 addr, uint8 buf[], uint8 len)
{
	uint8 i;

	FLASH_Unlock(FLASH_MEMTYPE_DATA);
	for (i = 0; i < len; i++) 
	{
		FLASH_ProgramByte(OF_EEPROM_ADDRESS+addr+i,buf[i]);
	}
	FLASH_Lock(FLASH_MEMTYPE_DATA);
}

void modules_init(void)
{
    /* hsi 16MHz. Hsi is the default clock source when system power on */    
    CLK->CKDIVR = (u8)((u8)CLK_PRESCALER_CPUDIV1 & (u8)CLK_CKDIVR_CPUDIV) 
                  | ((u8)((u8)CLK_PRESCALER_HSIDIV1 & (u8)CLK_CKDIVR_HSIDIV));//not prescaler

    CLK->SWR = (u8)CLK_SOURCE_HSI;
    while (!(CLK->ICKR & 0x02));//wait for hsi ready
    CLK->ICKR |= CLK_ICKR_HSIEN;

	IWDG->KR = IWDG_KEY_ENABLE;
    IWDG->KR = (u8)IWDG_WriteAccess_Enable;	/* Write Access */ 
	IWDG->PR = (u8)IWDG_Prescaler_256; //will reset after 1020ms
    IWDG->RLR = 0xFF;
    IWDG->KR = IWDG_KEY_REFRESH;/* Refresh IWDG */
	    /* use timer4 cpu clk = 16M 16000000/128/*/
	TIM4->PSCR = TIM4_PRESCALER_128; 
	TIM4->ARR = (u8)((TIM_CLK_FREQUENCY/128)/TICK_TIMER_FREQUENCY - 1);
	TIM4->CR1 |= TIM4_CR1_CEN;

	device_encode_opt();
	
    /*gpio init*/ 
    GPIO_Init(PLC_RESET_PORT,PLC_RESET_PIN,     GPIO_MODE_OUT_PP_HIGH_SLOW);
	UART_DeInit(9600);    

#if PWRCTRL_3
    GPIO_Init(RELAY_CTRL2_PORT,RELAY_CTRL2_PIN, GPIO_MODE_OUT_PP_HIGH_SLOW);
    GPIO_Init(RELAY_CTRL3_PORT,RELAY_CTRL3_PIN, GPIO_MODE_OUT_PP_HIGH_SLOW);
#endif

#if CURTAIN_MOTOR
    GPIO_Init(MOTOR_R_PORT,MOTOR_R_PIN ,GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(MOTOR_F_PORT,MOTOR_F_PIN ,GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(MOTOR_OFF_PORT,MOTOR_OFF_PIN ,GPIO_MODE_OUT_PP_LOW_SLOW);

    GPIO_Init(CHECK_F_PORT, CHECK_F_PIN ,GPIO_MODE_IN_FL_NO_IT);
    GPIO_Init(CHECK_R_PORT, CHECK_R_PIN ,GPIO_MODE_IN_FL_NO_IT);

#endif

    SET_PIN_H(PLC_RESET_PORT, PLC_RESET_PIN);

#if PWRCTRL_3
    RELAY_OFF(RELAY_CTRL2_PIN);
    RELAY_OFF(RELAY_CTRL3_PIN);
#endif

    TIM4->IER |= (u8)TIM4_IT_UPDATE;
    TIM4->CR1 |= TIM4_CR1_CEN;

    zcp_gpio_init();
    TIM2_DeInit();

    init_chn_pool_mgr();

    UART_CHN_Soft_Init();

}





