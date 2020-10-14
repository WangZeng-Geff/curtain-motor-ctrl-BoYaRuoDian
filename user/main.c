
#include "headfiles.h"

/********************************************* 
                主程序入口
*********************************************/
void main(void)
{
   disableInterrupts();
   modules_init();
   enableInterrupts();
   system_init();

   while (1)
   {
      task_handle();
   }
}


