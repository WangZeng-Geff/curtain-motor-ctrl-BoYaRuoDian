

#include "headfiles.h" 

uint8 rpt_seq = 0;
struct power_on_report_t power_on_report;
struct stage_data_t stage_data;
//extern struct RAM  ram;
void stage_data_init()
{
    memset_my((uint8 *)&stage_data, 0x00, sizeof(stage_data));
    stage_data.counter_for_gw = 10;
}
void stage_restart_data_init(uint8 state)//5秒内有新任务，重新计数开始
{
   if ((eep_param.motor_up_time == 0) || (eep_param.motor_dn_time == 0))
      return;

    if (state==REPORT_START)
    {
        stage_data.curtain_degree_change = TRUE;
    }
    else
    {
        stage_data.curtain_degree_change = FALSE;
        stage_data.state_change_mode = FREE;
    }
    stage_data.gw_send_flag = FALSE; 
    stage_data.try_cnt = 0;
    stage_data.aid_position = 0;
    stage_data.counter_for_gw = 10;
}
static void insert_said_to_queue(void *s)
{
    uint8 i;
    if(0x00 == memcmp_my(eep_param.gateway_id, s, ID_LEN))
    {
        return;
    }
    for (i = 0;i < MAX_SB_NUM;i++)
    {
        if (0x00 == memcmp_my(&stage_data.said_queue[4*i], s, ID_LEN))
        {
            break;
        }
    }
    if (i >= MAX_SB_NUM)
    {
        mymemcpy(&stage_data.said_queue[4*stage_data.queue_pos], s, ID_LEN); 
	    stage_data.queue_pos++;
        if (stage_data.queue_pos >= MAX_SB_NUM)
        {
            stage_data.queue_pos = 0;
        }
    }
}
uint8 get_message_source(void *source_addr,uint8 destination_flag)
{
    if(destination_flag == GATEWAY_NUM)
    {
        memset_my(stage_data.last_said, 0x00, ID_LEN);
        return (BROADCAST);
    }
    insert_said_to_queue(source_addr);
    stage_data.equipment_gid = 0;
    mymemcpy(stage_data.last_said,source_addr,ID_LEN);
    return (P2P);
}
uint8 get_1byte_bit1_number(uint8 data,uint8 pos)
{
    uint8 i;
    uint8 k = 0;
    for (i=0;i<pos;i++)
    {
        if ((0x01&data)==0x01)
        {
            k++;
	      }
        data>>=0x01;
    }
    return k;
}

uint8 org_fbd_frame(uint8 frame_buf[], uint8 did_l, uint8 did_h,
                    uint8 data[], uint8 data_len)
{
    struct FBD_Frame *pfbd_frame;

    pfbd_frame = (struct FBD_Frame *)frame_buf;
    pfbd_frame->did[0] = did_l;
    pfbd_frame->did[1] = did_h;

    mymemcpy(&pfbd_frame->data[0], &data[0], data_len);
		
    pfbd_frame->ctrl = data_len;
    return(data_len + FBD_FRAME_HEAD);
}


void send_state_to_subscriber(uint8 aid_position)
{
    struct SHS_frame *pshs_frame = (struct SHS_frame *)g_frame_buffer;
    struct FBD_Frame *pfbd_frame;
    uint8 len = 0, databuf[1];
		
    pshs_frame->stc = STC;
    mymemcpy(pshs_frame->said, eep_param.id, ID_LEN);
    if(aid_position != GATEWAY_NUM)
    {
        if(!(eep_param.report_enable & REPORT_SB))
        {
            return;
        }
        mymemcpy(pshs_frame->taid,&stage_data.said_queue[4 * aid_position], ID_LEN);
        #if 0 //liuyi modify last_said need to report
        if(0x00 == memcmp_my(stage_data.last_said, pshs_frame->taid, ID_LEN)) // need to send said again
        {
            return;
        }
        #endif
    }
    else
    {
        if(!(eep_param.report_enable & REPORT_GW))
        {
            return;
        }
        #if 0 //liuyi modify gateway need to report
        if((0x00 == memcmp_my(stage_data.last_said, eep_param.gateway_id, ID_LEN))&&(stage_data.state_change_mode == P2P))
        {
            return;
        }
        #endif
        mymemcpy(pshs_frame->taid,eep_param.gateway_id,ID_LEN);
    }
    if(is_all_xx(pshs_frame->taid, 0x00, ID_LEN))
    {
        return;
    }

    rpt_seq++;
    pshs_frame->seq = (rpt_seq & 0x7F); 
    pshs_frame->infor[0] = RELIABLE_REPORT;
    pfbd_frame = (struct FBD_Frame *)&pshs_frame->infor[1];

    databuf[0] = last_degree;
    len += org_fbd_frame((uint8 *)pfbd_frame, did_item(0x0A03), databuf, 1);
    databuf[0] = curtaindegree;
    len += org_fbd_frame((uint8 *)pfbd_frame + len, did_item(0x0A05), databuf, 1);

    if(GATEWAY_NUM == aid_position)
    {
        len += org_fbd_frame((uint8 *)pfbd_frame + len, did_item(0xC01A), stage_data.taker_id, 4);
    }
    len += 1;//cmd
    pshs_frame->length = len;
    pshs_frame->infor[len] = checksum((uint8 *)pshs_frame, len + SHS_FRAME_HEAD);
    len += SHS_FRAME_HEAD + 1;
    uart_write(g_frame_buffer, len);
}

void stage_hook(void)
{
    uint8 i,j = 0;
    if(stage_data.gw_send_flag == TRUE) 
    {
        return;
    }
    if(stage_data.state_change_mode == FREE)
    {
        return;
    }
    if(!(eep_param.report_enable & REPORT_SB))
    {
        stage_data.gw_send_flag = TRUE;
        return;
    }
    for (i = 0;i < MAX_SB_NUM;i++)
    {
        if (is_all_xx(&stage_data.said_queue[4*i], 0x00, ID_LEN))
        {
            j++;
        }
    }
    if(MAX_SB_NUM == j)
    {
        stage_data.gw_send_flag = TRUE;
        return;
    }
    send_state_to_subscriber(stage_data.aid_position);
    stage_data.aid_position ++;
    if(stage_data.aid_position >= MAX_SB_NUM)
    {
        stage_data.aid_position = 0;
        stage_data.gw_send_flag = TRUE;
    }
}

void check_gw_ack(void)//100ms
{
    if(stage_data.gw_send_flag != TRUE) 
    {
        return;
    }
    if(stage_data.state_change_mode == FREE)
    {
        return;
    }

    if(!(eep_param.report_enable & REPORT_GW))
    {
        stage_restart_data_init(REPORT_OVER);
        return;
    }
    if(stage_data.state_change_mode == GATEWAY_ACK)
    {
	stage_restart_data_init(REPORT_OVER);
        return;
    }

    if(stage_data.counter_for_gw > 0) 
    {
        stage_data.counter_for_gw--;
        return;
    }
    
    stage_data.try_cnt++;
    if(stage_data.try_cnt == 1)
    {
        stage_data.counter_for_gw = 100;
    }
    if(stage_data.try_cnt == 2)
    {
        stage_data.counter_for_gw = 1000;
    }
    if(stage_data.try_cnt > GATEWAY_MAX_TRY_CNT) 
    {
        stage_restart_data_init(REPORT_OVER);
        return;
    }
    send_state_to_subscriber(GATEWAY_NUM);
}

void check_state_change(void)
{
    if(TRUE == stage_data.curtain_degree_change)
    {
        if(stage_data.wait_cnt > 0)
        {
            stage_data.wait_cnt--;
            return;
        }
        stage_hook();
    }
}


void org_report_frame(uint8 report_id[], uint8 report)
{
    struct SHS_frame *pshs_frame = (struct SHS_frame *)g_frame_buffer;
    struct FBD_Frame *pfbd_frame;
    uint8 len=0;
    uint8 data_buff[1];

    pshs_frame->stc = STC;
    power_on_report.report_seq++;
    pshs_frame->seq = (power_on_report.report_seq & 0x7F);

    mymemcpy(pshs_frame->said, eep_param.id, ID_LEN);
    mymemcpy(pshs_frame->taid, report_id, ID_LEN);

    pshs_frame->infor[0] = report;
    pfbd_frame = (struct FBD_Frame *)&pshs_frame->infor[1];

    data_buff[0] = last_degree;
    len += org_fbd_frame((uint8 *)pfbd_frame, did_item(0x0A03), data_buff, 1);
    data_buff[0] = curtaindegree;
    len += org_fbd_frame((uint8 *)pfbd_frame + len, did_item(0x0A05), data_buff, 1);
    len += org_fbd_frame((uint8 *)pfbd_frame + len, did_item(0xC01A), eep_param.id, 4);

    len += 1;  //cmd
    pshs_frame->length = len;
    pshs_frame->infor[len] = checksum((uint8 *)pshs_frame, len+SHS_FRAME_HEAD);
    len += SHS_FRAME_HEAD + 1;//cs
    uart_write(g_frame_buffer, len);
}

uint16 take_small_value_with_100(uint16 report_freq)
{
   if(0 == report_freq)
   {
      return (100);
   }
   else
   {
      return ((report_freq > 100 ? 100 : report_freq));
   }
}

uint16 cal_report_delay_time(uint8 try_count)
{
   power_on_report.device_sid = (eep_param.sid[1] << 8) + eep_param.sid[0];
   return ((uint16)(1 + try_count) * (uint16)60 + (power_on_report.device_sid % take_small_value_with_100(0)));
}

void power_on_report_over(void)
{
    power_on_report.report_try_cnt = MAX_REPORT_TRY_CNT;
    power_on_report.report_flag = FALSE;
    power_on_report.report_end = REPORT_END;
}
void check_power_on_report_ack(void)
{
    if ((GATEWAY_ACK == stage_data.state_change_mode) || (GATEWAY_ACK == power_on_report.report_ack))
    {
        power_on_report_over();
	return;
    }

    if (TRUE != power_on_report.report_flag) 
    {
        return;
    }

   power_on_report.report_try_cnt++;
   power_on_report.report_flag = FALSE;
   if(power_on_report.report_try_cnt < MAX_REPORT_TRY_CNT)
   {
      power_on_report.report_delay_time = cal_report_delay_time(power_on_report.report_try_cnt);
   }
}

void power_on_report_task(void)
{
   if(is_all_xx(eep_param.gateway_id, 0x00, ID_LEN))
      return;

   if ((eep_param.motor_up_time == 0) || (eep_param.motor_dn_time == 0))
      return;

   org_report_frame(eep_param.gateway_id, RELIABLE_REPORT);

   power_on_report.report_flag = TRUE;
}

void check_power_on(void)
{
   power_on_report.report_counter++;

   if ((power_on_report.report_counter >= power_on_report.report_delay_time)
          && (power_on_report.report_try_cnt < MAX_REPORT_TRY_CNT)
          && (power_on_report.report_end != REPORT_END))
   {
      power_on_report.report_counter = 0;
      power_on_report_task();
   }
}


void poweron_report_init(void)
{
    memset_my(&power_on_report, 0x00, sizeof(power_on_report));
    power_on_report.report_delay_time = cal_report_delay_time(power_on_report.report_try_cnt);
}


