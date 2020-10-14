/******************************************************************
*	青岛东软电脑技术有限公司
*	文件：		interrupt.c
*	版本：		v2.00
*	摘要：		系统中断向量表及中断处理函数
*	创建人：	刘毅
*	创建时间：	2009-06-05
******************************************************************/
#include "stm8s_conf.h"
#include "stm8s_type.h"
#include "stm8s.h"
#include "config.h"
#include "main.h"
#include "dev_ctrl.h"

DATA_SEG void _stext();	 /* startup routine */

typedef void @far  (*interrupt_handler_t)(void);

struct interrupt_vector
{
	unsigned char interrupt_instruction;
	interrupt_handler_t interrupt_handler;
};
@far  @interrupt void NonHandledInterrupt(void)
{
	/*
	   in order to detect unexpected events during development,
	   it is recommended to set a breakpoint on the following instruction
	*/
}

//chn CHN_INFRARED TX
@far @interrupt void UART2TX_Interrupt(void)
{
	uint8 c;

	if(((UART2->SR) & (1 << (UART2_IT_TC & 0x0F))))
	{
		if(uart_chn_tx_byte(&c))
		{
			/* Write one byte to the transmit data register */
			UART2->DR = c;
		}
		else
		{
            /* Disable the USART1 Transmit interrupt */
            UART2_ITConfig(UART2_IT_TC, DISABLE);
#if DEBUG
            if(__sys_tick >= uart_tick)
            {
                uart_tick = __sys_tick-uart_tick;
            }
            else
            {
                uart_tick = 0xFFFF-uart_tick+__sys_tick;
            }
#endif
		}    
	}
}

@far @interrupt void UART2RX_Interrupt(void)
{
	uint8 c; 
	//if(UART1_GetITStatus(UART1_IT_RXNE) != RESET)
	if(((UART2->SR) & (1 << (UART2_IT_RXNE & 0x0F))))
	{
		/* Read one byte from the receive data register */        
		c = UART2->DR;     
        if (uart_chn_rx_byte(c))
        {
            uart_rx_hook();
        }	        
	}
	if((UART2->SR) & (1 << (UART2_FLAG_OR_LHE & 0x0F)))
	{
		//over run
		c = UART2->DR;
	}
}
#ifdef CALIBRATION
extern vuint8 calibtate_tick;
DATA_SEG extern vuint8 measure_state;
@far @interrupt void TIM1_CAP_COM_IRQHandler(void)
{
    /* PLC chn rx  chn 3 */
    if (TIM1_GetITStatus(TIM1_IT_CC3))
    {
        TIM1_ClearITPendingBit(TIM1_IT_CC3);
        if ((0 == calibtate_tick) && (MEASURE_START == measure_state))
        {
            calibtate_tick = 250;
            START_CALIBRATION();
        }
    }
}
 
u16 TIM2_GetCapture1(void)
{
    /* Get the Capture 1 Register value */
    u16 tmpccr1 = 0;
    u8 tmpccr1l=0, tmpccr1h=0;

    tmpccr1h = TIM2->CCR1H;
    tmpccr1l = TIM2->CCR1L;

    tmpccr1 = (u16)(tmpccr1l);
    tmpccr1 |= (u16)((u16)tmpccr1h << 8);
    /* Get the Capture 1 Register value */
    return(u16)tmpccr1;
}


u16 TIM2_GetCapture2(void)
{
	/* Get the Capture 2 Register value */
	u16 tmpccr2 = 0;
	u8 tmpccr2l=0, tmpccr2h=0;

	tmpccr2h = TIM2->CCR2H;
	tmpccr2l = TIM2->CCR2L;

	tmpccr2 = (u16)(tmpccr2l);
	tmpccr2 |= (u16)((u16)tmpccr2h << 8);
	/* Get the Capture 2 Register value */
	return(u16)tmpccr2;
}

extern vuint16 diff;
@far @interrupt void TIM2_CAP_COM_IRQHandler(void)
{
    static uint16 first_capture,last_capture;

    if (TIM2->SR1 & TIM2_IT_CC1)
    {
        TIM2->SR1 = (u8)(~TIM2_IT_CC1);
        if ((MEASURE_START != measure_state))
        {
            return;
        }
        first_capture = TIM2_GetCapture1();
        measure_state = MEASURE_INPROGRESS;
    }
    if (TIM2->SR1 & TIM2_IT_CC2)
    {
        TIM2->SR1 = (u8)(~TIM2_IT_CC2); 
        if (MEASURE_INPROGRESS != measure_state)
        {
            return;
        }
        last_capture = TIM2_GetCapture2();
        diff = (last_capture - first_capture);

        TIM2->IER = 0x00;//disable the interrupt
        TIM2->CR1 = 0x00;//disable the tim2
        measure_state = MEASURE_COMPLETED;
    }
} 
#endif

extern uint8 relay_delay_flag;
@far @interrupt void TIM2_CAP_COM_IRQHandler(void)//过零延迟中断
{
	if ((TIM2->SR1 & TIM2_IT_CC1)&&(TIM2->IER & TIM2_IT_CC1))
	{
            TIM2->SR1 &= (u8)(~TIM2_IT_CC1);
            TIM2->IER &= (u8)(~TIM2_IT_CC1);
            
            if (_OFF == relay_delay_flag)
                SET_PIN_L(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
            else
                SET_PIN_H(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
	}
} 

@far  @interrupt void PORTB_Interrupt_IRQHandler(void)//过零中断
{			 
    GPIO_Init(ZCP_PORT, ZCP_PIN,GPIO_MODE_IN_FL_NO_IT);	//关中断	
    relay_delay_time();
    #if 0
    if (_OFF == relay_delay_flag)
                SET_PIN_L(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
    else
        SET_PIN_H(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
    #endif
}


@far @interrupt void TIM4_UPD_OVF_IRQHandler(void)
{ 
    TIM4->SR1 = (u8)(~TIM4_IT_UPDATE);
    __sys_tick += 2;
    //GPIO_WriteReverse(RELAY_CTRL1_PORT,RELAY_CTRL1_PIN);
}


extern const uint8 _device_type[];

struct interrupt_vector const _vectab[] = {
	{0x82, (interrupt_handler_t)_stext},			//Reset
	{0x82, (interrupt_handler_t)_device_type}, /* trap  */
	{0x82, NonHandledInterrupt}, /* irq0  */
	{0x82, NonHandledInterrupt}, /* irq1  */
	{0x82, NonHandledInterrupt}, /* irq2  */
	{0x82, NonHandledInterrupt}, /* irq3  */
	{0x82, PORTB_Interrupt_IRQHandler}, /* irq4  */		//PortB
	{0x82, NonHandledInterrupt}, /* irq5  */
	{0x82, NonHandledInterrupt}, /* irq6  */                //PortD
	{0x82, NonHandledInterrupt}, /* irq7  */		//PortE
	{0x82, NonHandledInterrupt}, /* irq8  */
	{0x82, NonHandledInterrupt}, /* irq9  */
	{0x82, NonHandledInterrupt}, /* irq10 */
	{0x82, NonHandledInterrupt}, /* irq11 */
#ifdef CALIBRATION
	{0x82, TIM1_CAP_COM_IRQHandler},				//TIM1 Capture/Compare
#else
    {0x82, NonHandledInterrupt}, /* irq12 */
#endif
	{0x82, (interrupt_handler_t)_stext}, /* irq13 */
#ifdef CALIBRATION
    {0x82, TIM2_CAP_COM_IRQHandler}, /* irq14 */    //TIM2 Capture/Compare
#else
        {0x82, TIM2_CAP_COM_IRQHandler}, /* irq14 */    //TIM2 Capture/Compare
#endif	    
	{0x82, NonHandledInterrupt},					//TIM3 Update/Overflow
	{0x82, (interrupt_handler_t)_stext},		/* irq16 */			//TIM3 Update/Overflow
	{0x82, NonHandledInterrupt},		/* irq17 */		//UART1 Tx complete	
	{0x82, NonHandledInterrupt},						//UART1 Receive Register DATA FULL
	#if 1	
    {0x82, NonHandledInterrupt},						
	{0x82, UART2TX_Interrupt}, /* irq20 */
	{0x82, UART2RX_Interrupt}, /* irq21 */		
	{0x82, NonHandledInterrupt}, /* irq22 */		//UART3 Receive Register DATA FULL
	{0x82, TIM4_UPD_OVF_IRQHandler}, /* irq23 */    //TIM4 Update/Overflow
    #endif
};



