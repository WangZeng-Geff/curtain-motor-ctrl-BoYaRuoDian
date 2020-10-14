/**********************************************************************************
**--------------File Info----------------------------------------------------------
** File name:           motor_func.c
** Last modified Date:  2013-07-05
** Last Version:        V1.0
** Descriptions:        
**
**---------------------------------------------------------------------------------
** Created by:          胡洪园
** Created date:        2013-07-5
** Version:             V1.0
** Descriptions:        窗帘电机功能函数
**
**---------------------------------------------------------------------------------
**********************************************************************************/
#include "headfiles.h"
#include "smart_plc.h"

uint8 curtain_init;
uint8 curtaindegree; //窗帘开度
uint8 action_cnt;    //窗帘动作次数，用于控制复位
uint8 reset_cnt;     //复位前动作次数
uint8 reset_flag;    //复位标志
uint8 motor_ctrl_mode;  //马达控制方式，时间或开度
uint8 last_degree;   //上次开度控制目标开度

uint8 next_action_cmd;     //复位后下一次动作指令
uint16 next_action_time;   //复位后下一次动作时间

uint16  motor_time0025;
uint16  motor_time2550;
uint16  motor_time5075;
uint16  motor_time7500;

uint16  motordn_time0075;
uint16  motordn_time7550;
uint16  motordn_time5025;
uint16  motordn_time2500;

uint8 relay_delay;
uint8 relay_delay_flag;
uint8 check_flag;
uint8 relay_error;

uint8 lock_init;
uint8 relay_timer;


struct MOTOR Motor1 = 
{
    MOTOR_F_PORT,MOTOR_F_PIN,MOTOR_R_PORT,MOTOR_R_PIN,MOTOR_OFF_PORT,MOTOR_OFF_PIN,MOTOR_ROFF,MOTOR_ROFF, 0, 0, 0, 0, 0
};


#define OFF            0x00
#define RCTRL          0x01
#define FCTRL          0x02


const static uint8 motor_run_state[]=
{//电机正转流程                                       //电机反转流程
    MOTOR_PWR_OFF, MOTOR_ROFF, MOTOR_FOFF, MOTOR_FON, MOTOR_F, MOTOR_PWR_OFF, MOTOR_FOFF, MOTOR_ROFF, MOTOR_RON, MOTOR_R,
};
#define MAX_MOTOR_STATE_SZ          sizeof(motor_run_state)

void relay_check(void)
{
    static uint8 check_cnt = 0;

    if ((Motor1.motorflag) || (relay_error))
        return;

    if ((GPIO_ReadInputPin(CHECK_F_PORT, CHECK_F_PIN))
        || (GPIO_ReadInputPin(CHECK_R_PORT, CHECK_R_PIN)))
        {
            check_cnt++;
        }
    else
        check_cnt = 0;

    if (check_cnt > 5)
        relay_error = 1;

    return;	
}

void relay_delay_ms(void)
{
    if (relay_delay)
    {
        relay_delay--;		
        if(relay_delay == 0)
        {
            if (_OFF == relay_delay_flag)
                SET_PIN_L(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
            else
            {
                if (!relay_error)
                    SET_PIN_H(Motor1.GPIOx_off,Motor1.GPIO_Pin_off); 
            }
        }
    }
}

uint8 find_state(uint8 cur_s, uint8 next_s)
{
    uint8 i, j;

    for(i = 0; i < MAX_MOTOR_STATE_SZ; i++)
    {
        if(motor_run_state[i] == next_s) break;
    }
    if(i >= MAX_MOTOR_STATE_SZ)  return(0);
 
    for(j = i; j>0; j--)
    {
        if(motor_run_state[j] == cur_s) break;
    }
    return(j);
}


void ctrl_relay(struct MOTOR *motor)
{
    uint8 ctrl = motor_run_state[motor->cur_state];

    motor->delay = 5;//切换方向，延时50ms单位
    if(MOTOR_PWR_OFF == ctrl)
    {
        relay_delay = 4;	//延时超过40mS
        relay_delay_flag = _OFF;
        GPIO_Init(ZCP_PORT, ZCP_PIN, GPIO_MODE_IN_FL_IT);//开中断
        motor->delay = 14;  //电机关闭时间间隔140ms
    }
    else if(MOTOR_FOFF == ctrl)
    {
        SET_PIN_L(motor->GPIOx_f,motor->GPIO_Pin_f);
    }
    else if(MOTOR_ROFF == ctrl)
    {
        SET_PIN_L(motor->GPIOx_r,motor->GPIO_Pin_r);
    }
    else if(MOTOR_FON == ctrl)
    {
        if (!relay_error)
            SET_PIN_H(motor->GPIOx_f,motor->GPIO_Pin_f);
    }
    else if(MOTOR_RON == ctrl)
    {
        if (!relay_error)
            SET_PIN_H(motor->GPIOx_r,motor->GPIO_Pin_r);
    }
    else if((MOTOR_F == ctrl) || (MOTOR_R == ctrl))
    {
        relay_delay = 4;	//延时超过40mS
        relay_delay_flag = _ON;
        GPIO_Init(ZCP_PORT, ZCP_PIN, GPIO_MODE_IN_FL_IT);//开中断
    }
}

uint16 cal_time(uint8 dstdegree)
{
   uint8 difdegree, degree;
   uint16 time;

   if (curtaindegree == INVALID_DEGREE)
   {
       return 0;
   }

   if (curtaindegree > dstdegree)
   {
      degree = curtaindegree - dstdegree;

      if (curtaindegree >= 75)
      {
         difdegree = curtaindegree - 75;
         if (degree > difdegree)
         {
            time = (uint32)difdegree * (uint32)motordn_time0075 / (uint32)25;
            degree -= difdegree;

            if (degree > 25)
            {
               degree -= 25;
               time += motordn_time7550;

               if (degree > 25)
               {
                  degree -= 25;
                  time += motordn_time5025;
                  time += (uint32)degree * (uint32)motordn_time2500 / (uint32)25;
               }
               else
               {
                  time += (uint32)degree * (uint32)motordn_time5025 / (uint32)25;
               }
            }
            else
            {
               time += (uint32)degree * (uint32)motordn_time7550 / (uint32)25;
            }
         }
         else
         {
            time = (uint32)degree * (uint32)motordn_time0075 / (uint32)25;
         }
      }
      else if (curtaindegree >= 50)
      {
         difdegree = curtaindegree - 50;
         if (degree > difdegree)
         {
            time = (uint32)difdegree * (uint32)motordn_time7550 / (uint32)25;
            degree -= difdegree;

            if (degree > 25)
            {
               degree -= 25;
               time += motordn_time5025;

               time += (uint32)degree * (uint32)motordn_time2500 / (uint32)25;
            }
            else
            {
               time += (uint32)degree * (uint32)motordn_time5025 / (uint32)25;
            }
         }
         else
         {
            time = (uint32)degree * (uint32)motordn_time7550 / (uint32)25;
         }
      }
      else if (curtaindegree >= 25)
      {
         difdegree = curtaindegree - 25;
         if (degree > difdegree)
         {
            time = (uint32)difdegree * (uint32)motordn_time5025 / (uint32)25;
            degree -= difdegree;
            time += (uint32)degree * (uint32)motordn_time2500 / (uint32)25;
         }
         else
         {
            time = (uint32)degree * (uint32)motordn_time5025 / (uint32)25;
         }
      }
      else // >= 0
      {
         time = (uint32)degree * (uint32)motordn_time2500 / (uint32)25;
      }
   }

   if (curtaindegree < dstdegree)
   {
      degree = dstdegree - curtaindegree;

      if (curtaindegree <= 25)
      {
         difdegree = 25 - curtaindegree;
         if (degree > difdegree)
         {
            time = (uint32)difdegree * (uint32)motor_time0025 / (uint32)25;
            degree -= difdegree;

            if (degree > 25)
            {
               degree -= 25;
               time += motor_time2550;

               if (degree > 25)
               {
                  degree -= 25;
                  time += motor_time5075;
                  time += (uint32)degree * (uint32)motor_time7500 / (uint32)25;
               }
               else
               {
                  time += (uint32)degree * (uint32)motor_time5075 / (uint32)25;
               }
            }
            else
            {
               time += (uint32)degree * (uint32)motor_time2550 / (uint32)25;
            }
         }
         else
         {
            time = (uint32)degree * (uint32)motor_time0025 / (uint32)25;
         }
      }
      else if (curtaindegree <= 50)
      {
         difdegree = 50 - curtaindegree;
         if (degree > difdegree)
         {
            time = (uint32)difdegree * (uint32)motor_time2550 / (uint32)25;
            degree -= difdegree;

            if (degree > 25)
            {
               degree -= 25;
               time += motor_time5075;

               time += (uint32)degree * (uint32)motor_time7500 / (uint32)25;
            }
            else
            {
               time += (uint32)degree * (uint32)motor_time5075 / (uint32)25;
            }
         }
         else
         {
            time = (uint32)degree * (uint32)motor_time2550 / (uint32)25;
         }
      }
      else if (curtaindegree <= 75)
      {
         difdegree = 75 - curtaindegree;
         if (degree > difdegree)
         {
            time = (uint32)difdegree * (uint32)motor_time5075 / (uint32)25;
            degree -= difdegree;
            time += (uint32)degree * (uint32)motor_time7500 / (uint32)25;
         }
         else
         {
            time = (uint32)degree * (uint32)motor_time5075 / (uint32)25;
         }
      }
      else // <= 100
      {
         time = (uint32)degree * (uint32)motor_time7500 / (uint32)25;
      }
   }

   return time;

}

uint16 time2degree(uint32 time, uint32 total_degree, uint32 degree_time)
{
   if (time > degree_time)
   {
      return (uint16)total_degree;
   }
   return (uint16)(((time + 2) * total_degree) / degree_time); 
}

void cal_degree(struct MOTOR *motor)
{
   uint16 degree, difdegree;
   uint16 diftime;


   if ((eep_param.motor_up_time == 0) || (eep_param.motor_dn_time == 0))
      return;

    if (motor->rotateflag == MOTOR_FORWARD)
    {
        if (curtaindegree > 100)    //无效开度
        {
            if ((eep_param.motor_up_time) && (eep_param.motor_dn_time)) //转动时间超过1.25倍行程时间
            {
                if (motor->run_t >= (eep_param.motor_up_time + (eep_param.motor_up_time >> 2) - 6)) // 减去一个很小的数，提高程序可靠性
                {
                    curtaindegree = 100; 
                }
                return;
            }

           if ((eep_param.motor_up_time == 0) || (eep_param.motor_dn_time == 0))  //未设置行程时间
           {
               if ((Motor1.run_t >= MAX_RUN_TIME)) //转动时间超过最大正转时间
               {
                   curtaindegree = 100;
               }
               return;
           }
        }
        else if (curtaindegree >= 75) // 75 < curtaindegree < 100
       {
          degree = time2degree(motor->run_t, 25, motor_time7500);
       }
       else if (curtaindegree >= 50) // 50 < curtaindegree < 75
       {
          difdegree = 75 - curtaindegree;
          diftime = (uint32)difdegree * (uint32)motor_time5075 / (uint32)25;

          if (motor->run_t > diftime)
          {
             degree = difdegree;
             motor->run_t -= diftime;
             degree += time2degree(motor->run_t, 25, motor_time7500);
          }
          else
          {
             degree = time2degree(motor->run_t, 25, motor_time5075);
          }
       }
       else if (curtaindegree >= 25) // 25 < curtaindegree < 50
       {
          difdegree = 50 - curtaindegree;
          diftime = (uint32)difdegree * (uint32)motor_time2550 / (uint32)25;

          if (motor->run_t >= diftime)
          {
             degree = difdegree;
             motor->run_t -= diftime;
             if (motor->run_t >= motor_time5075)
             {
                degree += 25;
                motor->run_t -= motor_time5075;
                degree += time2degree(motor->run_t, 25, motor_time7500);
             }
             else
             {
                degree += time2degree(motor->run_t, 25, motor_time5075);
             }
          }
          else
          {
             degree = time2degree(motor->run_t, 25, motor_time2550);
          }
       }
       else // 0 < curtaindegree < 25
       {
          difdegree = 25 - curtaindegree;
          diftime = (uint32)difdegree * (uint32)motor_time0025 / (uint32)25;

          if (motor->run_t >= diftime)
          {
             degree = difdegree;
             motor->run_t -= diftime;
             if (motor->run_t >= motor_time2550)
             {
                degree += 25;
                motor->run_t -= motor_time2550;

                if (motor->run_t >= motor_time5075) 
                {
                   degree += 25;
                   motor->run_t -= motor_time5075;
                   degree += time2degree(motor->run_t, 25, motor_time7500);
                }
                else
                {
                   degree += time2degree(motor->run_t, 25, motor_time5075);
                }
             }
             else
             {
                degree += time2degree(motor->run_t, 25, motor_time2550);
             }
          }
          else
          {
             degree = time2degree(motor->run_t, 25, motor_time0025);
          }
       }

       motor->run_t = 0;

       if (curtaindegree == 100)
       {
          return;
       }

       if (((uint8)(curtaindegree + degree)) > 100)
       {
          curtaindegree = 100;
       }
       else
       {
          curtaindegree += (uint8)degree; 
       }
    }

    if (motor->rotateflag == MOTOR_REVERSE)
    {
       if (curtaindegree <= 25)
       {
          degree = time2degree(motor->run_t, 25, motordn_time2500);
       }
       else if (curtaindegree <= 50)
       {
          difdegree = curtaindegree - 25;
          diftime = (uint32)difdegree * (uint32)motordn_time5025 / (uint32)25;

          if (motor->run_t > diftime)
          {
             degree = difdegree;
             motor->run_t -= diftime;
             degree += time2degree(motor->run_t, 25, motordn_time2500);
          }
          else
          {
             degree = time2degree(motor->run_t, 25, motordn_time5025);
          }
       }
       else if (curtaindegree <= 75)
       {
          difdegree = curtaindegree - 50;
          diftime = (uint32)difdegree * (uint32)motordn_time7550 / (uint32)25;

          if (motor->run_t >= diftime)
          {
             degree = difdegree;
             motor->run_t -= diftime;
             if (motor->run_t >= motordn_time5025)
             {
                degree += 25;
                motor->run_t -= motordn_time5025;
                degree += time2degree(motor->run_t, 25, motordn_time2500);
             }
             else
             {
                degree += time2degree(motor->run_t, 25, motordn_time5025);
             }
          }
          else
          {
             degree = time2degree(motor->run_t, 25, motordn_time7550);
          }
       }
       else if (curtaindegree <= 100) // < 100
       {
          difdegree = curtaindegree - 75;
          diftime = (uint32)difdegree * (uint32)motordn_time0075 / (uint32)25;

          if (motor->run_t >= diftime)
          {
             degree = difdegree;
             motor->run_t -= diftime;
             if (motor->run_t >= motordn_time7550)
             {
                degree += 25;
                motor->run_t -= motordn_time7550;

                if (motor->run_t >= motordn_time5025) 
                {
                   degree += 25;
                   motor->run_t -= motordn_time5025;
                   degree += time2degree(motor->run_t, 25, motordn_time2500);
                }
                else
                {
                   degree += time2degree(motor->run_t, 25, motordn_time5025);
                }
             }
             else
             {
                degree += time2degree(motor->run_t, 25, motordn_time7550);
             }
          }
          else
          {
             degree = time2degree(motor->run_t, 25, motordn_time0075);
          }
       }
       else   //无效开度
       {
            if ((eep_param.motor_up_time) && (eep_param.motor_dn_time)) //转动时间超过1.25倍行程时间
            {
                if (motor->run_t >= (eep_param.motor_dn_time + (eep_param.motor_dn_time >> 2) - 6)) // 减去一个很小的数，提高程序可靠性
                {
                    curtaindegree = 0; 
                }
                return;
            }

           if ((eep_param.motor_up_time == 0) || (eep_param.motor_dn_time == 0))  //未设置行程时间
           {
               if (Motor1.run_t >= (MAX_RUN_TIME - 6)) //转动时间超过最大正转时间
               {
                   curtaindegree = 0;
               }
               return;
           }
       }


       motor->run_t = 0;

       if (curtaindegree == 0)
       {
          return;
       }

       if (curtaindegree > (uint8)degree)
       {
          curtaindegree -= (uint8)degree;
       }
       else
       {
          curtaindegree = 0;
       }
    }

    return;
}

void curtain_ctrl(struct MOTOR *motor)
{
   uint8 degree;

    if(((MOTOR_FOFF == motor_run_state[motor->cur_state]) && (MOTOR_FOFF == motor_run_state[motor->next_state]))
       || ((MOTOR_ROFF == motor_run_state[motor->cur_state]) && (MOTOR_ROFF == motor_run_state[motor->next_state])))
    {
      if (motor->motorflag)  //电机正在转动中
      {
         cal_degree((struct MOTOR *)&Motor1);

         if ((curtaindegree == 100) || (curtaindegree == 0))
         {
            action_cnt = 0;
            reset_flag = 0;
         }
         else
         {
//            if (motor_ctrl_mode == MOTOR_DEGREE_CTRL)
            {
               action_cnt++; 
               if (action_cnt >= reset_cnt)
               {
                  reset_flag = 1;
               }
            }
         }

         motor_ctrl_mode = 0;
         motor->time = 0;
         motor->run_t = 0;
         motor->motorflag = 0;
         motor->rotateflag = 0;
         last_degree = curtaindegree;

         eep_param.degree = curtaindegree;
         eep_param.acting = 0;
         EEP_Write(OF_PLC_PARAM + offset_of(struct EEP_PARAM, degree),
             (uint8 *)&(eep_param.degree), sizeof(eep_param.degree) + sizeof(eep_param.acting));

         if ((next_action_cmd == 0) && (eep_param.motor_up_time) && (eep_param.motor_dn_time) && (curtain_init != POWERON_INIT))
         {
            stage_data.state_change_mode = P2P;
            stage_restart_data_init(REPORT_START); 
            stage_data.wait_cnt = 2 * stage_data.equipment_gid;  //间隔2S
         }

         curtain_init = 0;
         
      }

       return; 
    }

    if(motor->delay > 0)
    {
        motor->delay--;
        if(motor->delay) return;
        if(motor->cur_state < motor->next_state)
        {
            motor->cur_state++;
            ctrl_relay(motor);
        }                 
        return;
    }

    if(motor->run_t > MAX_RUN_TIME)
    {
        motor->time = 0;
    }

    if(motor->time > 0)
    {
        motor->time--;
        motor->run_t++;
        return;
    }

    if (motor->rotateflag == MOTOR_FORWARD)
    {
        motor->next_state = find_state(MOTOR_FOFF, MOTOR_FOFF); 
        motor->cur_state = find_state(MOTOR_PWR_OFF, MOTOR_FOFF);
    }
    else
    {
        motor->next_state = find_state(MOTOR_ROFF, MOTOR_ROFF); 
        motor->cur_state = find_state(MOTOR_PWR_OFF, MOTOR_ROFF);
    }
    
    ctrl_relay(motor);
    
    return;
}

void curtain_state_fun(void)
{
    curtain_ctrl((struct MOTOR *)&Motor1);    
}


uint8 motor_ctrl(uint8 cmd, uint16 time, struct MOTOR *motor, uint8 flag)
{
   uint8 degree;

   if (motor_ctrl_mode == MOTOR_INIT_CTRL)
   {
      return 1;
   }

   motor_ctrl_mode = flag;

    switch(cmd)
    {
    case MOTOR_STOP:  //电机暂停
        motor->cur_state = motor->next_state;  
        motor->delay = 0;
        motor->time = 0;
        return(0);

    case MOTOR_FORWARD:    //电机正转
        motor->next_state = find_state(MOTOR_F, MOTOR_F);
        motor->cur_state = find_state(MOTOR_PWR_OFF, MOTOR_F); 
        ctrl_relay(motor);
        motor->time = time;
        motor->run_t = 0;
        motor->motorflag = 1;
        motor->rotateflag = MOTOR_FORWARD;
        break;

    case MOTOR_REVERSE:     //电机反转 
        motor->next_state = find_state(MOTOR_R, MOTOR_R);
        motor->cur_state = find_state(MOTOR_PWR_OFF, MOTOR_R); 
        ctrl_relay(motor);
        motor->time = time;
        motor->run_t = 0;
        motor->motorflag = 1;
        motor->rotateflag = MOTOR_REVERSE;
        break;

    default:
        return(1);
    }

    if (motor->motorflag)
    {
        eep_param.acting = 1;
        EEP_Write(OF_PLC_PARAM + offset_of(struct EEP_PARAM, acting),
             (uint8 *)&(eep_param.acting), sizeof(eep_param.acting));


    }
    return(0);
}


uint8 set_motor_ctrl(uint8 *buff, uint8 w_len)
{
    uint8 ret, cabri = 0, cmd = buff[0];
    uint16 time;

    if(w_len != 0x01) return(LEN_ERR);

    if ((cmd > 3) || (cmd < 1))
       return(DATA_ERR);

    if ((curtain_init != 0) || (motor_ctrl_mode == MOTOR_INIT_CTRL))
       return DEV_BUSY;

    #ifdef CURTAIN_LOCK
    if (eep_param.lock)
        return OTHER_ERR;
    #endif

   if (cmd == MOTOR_FORWARD)
    {
        SET_PIN_H(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
        SET_PIN_L(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
        SET_PIN_L(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
        relay_timer = 13;
    }
    else if (cmd == MOTOR_REVERSE)
    {
        SET_PIN_H(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
        SET_PIN_L(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
        SET_PIN_L(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
        relay_timer = 13;
    }
    else if (cmd == MOTOR_STOP) 
    {
        SET_PIN_H(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
        SET_PIN_L(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
        SET_PIN_L(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
        relay_timer = 13;
    }
    else
    {
        return(DATA_ERR);
    }

    return(NO_ERR);
}

uint8 set_motor_ctrl_inf(uint8 *buff, uint8 w_len)
{
    uint16 time;
    uint8 cmd, style, ret=0;

    if(w_len != 0x04) return(LEN_ERR);

    if ((curtain_init != 0) || (motor_ctrl_mode == MOTOR_INIT_CTRL))
       return DEV_BUSY;

    #ifdef CURTAIN_LOCK
    if (eep_param.lock)
        return OTHER_ERR;
    #endif

    style = buff[0];
    cmd = buff[1] & 0x0F;
    time = buff[3]*256 + buff[2];

    if(style)    //不是交流电机，返回错误
    {
        return(DATA_ERR);
    }

   if (cmd == MOTOR_FORWARD)
    {
        SET_PIN_H(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
        SET_PIN_L(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
        SET_PIN_L(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
        relay_timer = 13;
    }
    else if (cmd == MOTOR_REVERSE)
    {
        SET_PIN_H(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
        SET_PIN_L(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
        SET_PIN_L(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
        relay_timer = 13;
    }
    else if (cmd == MOTOR_STOP) 
    {
        SET_PIN_H(Motor1.GPIOx_off,Motor1.GPIO_Pin_off);
        SET_PIN_L(Motor1.GPIOx_f, Motor1.GPIO_Pin_f);
        SET_PIN_L(Motor1.GPIOx_r, Motor1.GPIO_Pin_r);
        relay_timer = 13;
    }
    else
    {
        return(DATA_ERR);
    }

    return(NO_ERR);
}

uint8 curtain_time_init(void)
{
    uint16 motor_time, cali_up, cali_dn;

    if (eep_param.motor_up_time >= 5000)
    {
       cali_up = 200;
       cali_dn = 120;
       reset_cnt = 13;
    }
    else if (eep_param.motor_up_time >= 4200)
    {
       cali_up = 100;
       cali_dn = 60;
       reset_cnt = 13;
    }
    else if (eep_param.motor_up_time >= 3400)
    {
       cali_up = 50;
       cali_dn = 30;
       reset_cnt = 10;
    }
    else if (eep_param.motor_up_time >= 2600)
    {
       cali_up = 20;
       cali_dn = 15;
       reset_cnt = 7;
    }
    else
    {
       cali_up = 10;
       cali_dn = 5;
       reset_cnt = 7;
    }

    motor_time = eep_param.motor_up_time - (6 * cali_up);
    motor_time7500 = motor_time / 4;
    motor_time5075 = motor_time7500 + cali_up;
    motor_time2550 = motor_time7500 + 2 * cali_up;
    motor_time0025 = motor_time7500 + 3 * cali_up;

    motor_time = eep_param.motor_dn_time - (6 * cali_dn);

    motordn_time0075 = motor_time / 4;
    motordn_time7550 = motordn_time0075 + cali_dn;
    motordn_time5025 = motordn_time0075 + 2 * cali_dn;
    motordn_time2500 = motordn_time0075 + 3 * cali_dn;

    return 0;
}


#if 0
void relay_forward(struct MOTOR *motor)
{
   SET_PIN_H(motor->GPIOx_f_r,motor->GPIO_Pin_f_r);
   SET_PIN_H(motor->GPIOx_off,motor->GPIO_Pin_off);
   return;
}

void relay_reverse(struct MOTOR *motor)
{
   SET_PIN_L(motor->GPIOx_f_r,motor->GPIO_Pin_f_r);
   SET_PIN_H(motor->GPIOx_off,motor->GPIO_Pin_off);
   return;
}


void relay_stop(struct MOTOR *motor)
{
   SET_PIN_L(motor->GPIOx_f_r,motor->GPIO_Pin_f_r);
   SET_PIN_L(motor->GPIOx_off,motor->GPIO_Pin_off);
   return;
}

#endif



