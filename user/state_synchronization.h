#ifndef __STATE_SYNCHRONIZATION_H__
#define __STATE_SYNCHRONIZATION_H__

#define HZ (10)
#define GATEWAY_NUM (0xFF)
#define GATEWAY_MAX_TRY_CNT (0x03)
#define MAX_SB_NUM   (0x03) //设备最多3个
#define REPORT_OVER 0
#define REPORT_START 1

#define     RELIABLE_REPORT   0x01
#define     REPORT_NO         0X00
#define     REPORT_GW         0X01
#define     REPORT_SB         0X02

enum {
        FREE,
        KEY,
        BROADCAST,
        P2P,
        GATEWAY_ACK
     };

struct stage_data_t
{
    uint8  aid_position;
    uint8  equipment_gid;
    uint8  try_cnt;
    uint8  state_change_mode;
    uint8  sid_equal_flag;
    uint8  last_said[4];
    uint8  queue_pos;
    uint8  said_queue[12];
    uint32 counter_for_gw;
    uint8  gw_send_flag;
    uint8  curtain_degree_change;
    uint8  wait_cnt;
    uint8  find_myself_flag;
    uint8  taker_id[4];
};


struct power_on_report_t
{
	uint8	report_try_cnt;		//上报重试次数
	uint16	report_delay_time;	//上报延时时间
	uint16	report_counter;		//上报计时
	uint8	report_flag;		//上报网关标志
	uint8	report_ack;		//网关回复标志
	uint16	device_sid;
	uint8	report_seq;
	uint8	report_end;

};
#define	MAX_REPORT_TRY_CNT	3
#define	REPORT_END			0xFF

extern struct power_on_report_t power_on_report;
extern struct stage_data_t stage_data;


uint8 get_1byte_bit1_number(uint8 data,uint8 pos);

uint8 get_message_source(void *source_addr,uint8 destination_flag);

void stage_hook(void);

void check_gw_ack(void);
void stage_data_init(void);
void stage_restart_data_init(uint8 state);
void check_state_change(void);


void poweron_report_init(void);
void check_power_on(void);
void power_on_report_task(void);
void check_power_on_report_ack(void);
void power_on_report_over(void);
uint16 cal_report_delay_time(uint8 try_count);
uint16 take_small_value_with_100(uint16 report_freq);
uint8 org_fbd_frame(uint8 frame_buf[], uint8 did_l, uint8 did_h,uint8 data[], uint8 data_len);


#endif




