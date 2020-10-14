//#include <string.h>
#include "comfunc.h"
#include "headfiles.h"
#include "main.h"
#include "config.h"

uint8 checksum (uint8 *data,uint8 len)
{
	uint8 cs = 0;

	while(len-- > 0)
		cs += *data++;
	return(cs);
}

void mymemcpy(void *dst,void *src,uint8 len)
{
	while(len--)
	{
		*(char *)dst = *(char *)src;
		dst = (char *)dst + 1;
		src = (char *)src + 1;
	}
}
uint8 memcmp_my(const void *s1, const void *s2, uint8 n)
{
    while (n && *(char*)s1 == *(char*)s2) 
    {
        s1 = (char*)s1 + 1;
        s2 = (char*)s2 + 1;
        n--;
    }

    if(n > 0)
    {
        return(1);
    }
    return(0);
}

uint8 is_all_xx(const void *s1, uint8 value, uint8 n)
{
    while(n && *(char*)s1 == value)
    {
        s1 = (char*)s1 + 1;
        n--;
    }
    return(!n);
}


void memset_my(void *s1, uint8 value, uint8 n)
{
    do
    {
        *(char*)s1 = value;
        s1 = (char *)s1 + 1;
    }
    while(--n);
}
void* memmove_my(void* dest, void* source, uint16 n) 
{
    int8* ret = (int8*)dest; 

    if (ret <= (int8*)source || ret >= (int8*)source + n)
    { 
        while (n --)
        {    
           *ret++ = *(int8*)source;
           source = (int8 *)source + 1;
        }
    }
    else
    {
        ret += n - 1; 
        source = (int8*)source + n - 1; 
        while (n--)
        {
           *ret-- = *(int8*)source;
           source = (int8*)source -1;
        }
    } 
    return dest;
}

uint8 find_max(uint8 buf[], uint8 n)
{
    uint8 i;
    uint8 max_d;

    max_d = buf[0];
    for(i=0; i<n; i++)
    {
        if(max_d < buf[i]) max_d = buf[i];
    }
    return(max_d);
}

uint16 sp_crc16_with_init(uint16 crc, const uint8 *buf, uint8 size)
{
  unsigned char i;

  while(size--!=0)
  {
    for(i = 0x80; i != 0; i>>=1)
    {
      if((crc&0x8000) != 0) 
      {
          crc <<= 1; 
          crc ^= 0x1021;
      } /* 車角那?CRC3?辰?2?迄?車CRC */
      else 
      {
          crc <<= 1;
      }
      if(((*buf)&i)!=0) 
      {
          crc ^= 0x1021; /* ?迄?車谷?㊣???米?CRC */
      }
    }
    buf++;
  }
  return(crc);
}

uint32 get_le_val(const uint8 * p, uint8 bytes)
{
    uint32 ret = 0;

    while (bytes-- > 0)
    {
        ret <<= 8;
        ret |= *(p + bytes);
    }
    return ret;
}

#if 0
struct RXTX_QUEUE {
    uint8 cnt;
    uint8 queue;
};
static struct RXTX_QUEUE rxtx_queue;

void push_rxtx_status(enum RXTX_STATUS status)
{
    if (rxtx_queue.cnt < 0x04)
    {
        rxtx_queue.queue <<= 2;
        rxtx_queue.queue |= status;
        rxtx_queue.cnt++;
    }
}

uint8 get_rxtx_status(void)
{
    /* queue empty */
    if (0x00 == rxtx_queue.cnt) return(NO_STATUS);

    rxtx_queue.cnt--;
    return((rxtx_queue.queue >> (rxtx_queue.cnt<<1)) & 0x03);
}
#endif



