#ifndef _EX_UARTS_IO_CTRL_H_
#define _EX_UARTS_IO_CTRL_H_


#define	IOCTL_CLEAN				0x6b01				
#define	IOCTL_BPS_SET			0x6b02				
#define	IOCTL_FMT_SET			0x6b03
	
#define	IOCTL_RTS_SET			0x6b10
#define IOCTL_CTS_GET			0x6b11


#define HARD_CTS_RTS_CTL				0x6B20



#define IOCTL_GET_IDLE_TICKS			0x6B30

#define IOCTL_SET_MONITOR_PID			0x6b50
#define IOCTL_SET_OVER_TICK			0x6b51

#define IOCTL_SET_TXING_NO_RX			0x6c51

//#define IOCTL_SET_UART_CFG				0x6D01
//#define IOCTL_GET_UART_CFG				0x6D02
		
#define ENABLE_HARD_RTS				(0x40)
#define ENABLE_HARD_CTS				(0x80)  
 

#define UART_STOPBIT1			 1
#define UART_STOPBIT2		   2
/*
#define UART_NO_PARITY		(0x00<<6)
#define UART_ODD_PARITY		(0x01<<6)
#define UART_EVEN_PARITY		(0x02<<6)
*/
#endif


