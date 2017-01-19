/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : Terminal.cpp
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/04/13
* Description  : 终端类程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#include <time.h>

#include "Terminal.h"
#include "log.h"
#include "defines.h"
#include "PlainParse.h"

#define TIMER_SIVAL_CHECK_TASK			1000
#define TIMER_SIVAL_STATUS_NULL			1001
#define TIMER_SIVAL_STATUS_CALL			1002

#define TIMER_TIME_CHECK_TASK			1000
#define TIMER_TIME_STATUS_CALL			3000

CSocketEx *CTerminal::m_psocket_spon = NULL;

CTerminal::CTerminal()
{
	m_id = 3;
	m_login = FALSE;
	memset(&m_socket_addr, 0, sizeof (sockaddr_in));
	m_socket_addr.sin_family = AF_INET;
	m_socket_addr.sin_port = htons(SPON_SERVER_PORT);
	m_socket_addr.sin_addr.s_addr = INADDR_ANY;
	m_tdata_check_task.p_param = this;
	m_tdata_check_task.timerid = NULL;
	m_tdata_check_task.sival = TIMER_SIVAL_CHECK_TASK;
	m_tdata_task_status.p_param = this;
	m_tdata_task_status.timerid = NULL;
	m_tdata_task_status.sival = TIMER_SIVAL_STATUS_NULL;
	m_sip_ua.set_sip_event_callback(this->sip_event_callback, this);
	m_sip_ua.set_audio_recvdata_callback(this->audio_recvdata_callback, this);
	//m_sip_ua.m_audio_stream.enable_sound_card(TRUE);
	//create_timer(&m_tdata_check_task, on_timer_queue, TIMER_TIME_CHECK_TASK, TIMER_SIVAL_CHECK_TASK);
	m_talk_id = m_id;
	m_audio_format = AUDIO_FORMAT_L16;
}

CTerminal::~CTerminal()
{
    BYTE			data[22];

	sip_event_closed(NULL);
	if (m_login)
	{
        memset(data, 0, sizeof (data));
        data[14 + 0] = 0x34;
        data[14 + 1] = 0x12;
        data[14 + 6] = 0x21;
        data[14 + 7] = 0x43;
        data[14 + 4] = 0x01;	// BC --- 0 : 8000; 1 : 22050;
        data[14 + 5] = 0x00;	// CODEC --- 0 : PCM; 1 : ADPCM;
		send_net_data(NET_MSG_LOGON_STATE, 0x00, 0, 0, 0, 0, data, 22);
	}
    if (m_tdata_check_task.timerid != NULL)
        ua_delete_timer_queue(&m_tdata_check_task, TRUE);
}

void CTerminal::running(void)
{
}

void CTerminal::sip_event_callback(eXosip_event_t *p_event, void *p_param)
{
	CTerminal			*p_term = (CTerminal *)p_param;

 	if (p_term != NULL && p_event != NULL)
	{
		switch (p_event->type)
		{
		case EXOSIP_REGISTRATION_FAILURE:
		case EXOSIP_REGISTRATION_SUCCESS:			p_term->sip_event_reg_success(p_event); break;
		case EXOSIP_CALL_RINGING:					p_term->sip_event_call_ringing(p_event); break;
		case EXOSIP_CALL_ANSWERED:					p_term->sip_event_call_answerd(p_event); break;
		case EXOSIP_CALL_INVITE:					p_term->sip_event_invite(p_event); break;
		case EXOSIP_CALL_GLOBALFAILURE:
		case EXOSIP_CALL_REQUESTFAILURE:
		case EXOSIP_CALL_CLOSED:					p_term->sip_event_closed(p_event); break;
		case EXOSIP_MESSAGE_NEW:					p_term->sip_event_message_new(p_event); break;
		default:
			break;
		}
	}
}

void CTerminal::audio_recvdata_callback(void *p_param)
{
	int					frame_len, read_len;
	BYTE				func;
	WORD				check_sum;
	CTerminal			*p_term = (CTerminal *)p_param;
	BYTE				data[NET_WAVE_FRAME_PCM_SIZE];

	if (p_term->m_sip_ua.m_task_type == SIP_TASK_TYPE_TALK_OUTGOING \
		|| p_term->m_sip_ua.m_task_type == SIP_TASK_TYPE_TALK_INCOMING \
		|| p_term->m_sip_ua.m_task_type == SIP_TASK_TYPE_BC_INCOMING \
		|| p_term->m_sip_ua.m_task_type == SIP_TASK_TYPE_MONITOR_OUTGOING)
	{
		frame_len = NET_WAVE_FRAME_PCM_SIZE;
		if (p_term->m_sip_ua.m_status == SIP_STATUS_BCING)
		{
			if (p_term->m_audio_format == AUDIO_FORMAT_MP3)
			{
				frame_len = NET_MP3_FRAME_MP3_SIZE;
				func = NET_MSG_DATA_MP3_LIVE;
			}
			else
				func = NET_MSG_DATA_WAVE_LIVE;
		}
		else
			func = NET_MSG_DATA_TALK_DATA;

		do
		{
			read_len = p_term-> m_sip_ua.m_audio_stream.read_rtp_recvdata((short *)data, frame_len / sizeof (short));
			if (read_len == frame_len / sizeof (short))
			{
				check_sum = calc_check_sum(0, data, read_len * sizeof (short), FALSE);
				p_term->send_net_data(func, 0, (check_sum & 0xFF), (check_sum >> 8), 0, 0, data, read_len * sizeof (short));
			}
		} while (read_len == frame_len / sizeof (short));
	}
}

void CTerminal::sip_event_call_answerd(eXosip_event_t *p_event)
{
	switch (m_sip_ua.m_task_type)
	{
	case SIP_TASK_TYPE_TALK_OUTGOING:
		send_net_data(NET_MSG_TALK_STATUS, 0x03, BYTE1(m_id), BYTE2(m_id), BYTE1(m_talk_id), BYTE2(m_talk_id), NULL, 0);
		break;
	case SIP_TASK_TYPE_MONITOR_OUTGOING:
		send_net_data(NET_MSG_TALK_STATUS, 0x03, BYTE1(m_id), BYTE2(m_id), BYTE1(m_talk_id), BYTE2(m_talk_id), NULL, 0);
		break;
	case SIP_TASK_TYPE_BC_OUTGOING:
		send_net_data(NET_MSG_BROADCAST_STATE, 0x03, 0x02, 0x01, 0, 0, NULL, 0);
		break;
	}
}

void CTerminal::sip_event_invite(eXosip_event_t *p_event)
{
	BYTE		data[6 + 8 + 4];

	if (p_event != NULL \
		&& p_event->request != NULL \
		&& p_event->request->from != NULL \
		&& p_event->request->from->url != NULL \
		&& p_event->request->from->url->username != NULL)
	{
		m_talk_id = atoi(p_event->request->from->url->username);
		m_talk_id = SPON_SERVER_ID(m_talk_id);
	}
	else
		m_talk_id = m_id;

	switch (m_sip_ua.m_task_type)
	{
	case SIP_TASK_TYPE_TALK_INCOMING:
		send_net_data(NET_MSG_TALK_REQUEST, 0x01, BYTE1(m_id), BYTE2(m_id), BYTE1(m_talk_id), BYTE2(m_talk_id), NULL, 0);
		ua_create_timer_queue(&m_tdata_task_status, on_timer_queue, TIMER_TIME_STATUS_CALL, TIMER_TIME_STATUS_CALL, TIMER_SIVAL_STATUS_CALL);
		break;
	case SIP_TASK_TYPE_MONITOR_INCOMING:
		send_net_data(NET_MSG_TALK_REQUEST, 0x08, BYTE1(m_id), BYTE2(m_id), BYTE1(m_talk_id), BYTE2(m_talk_id), NULL, 0);
		ua_create_timer_queue(&m_tdata_task_status, on_timer_queue, TIMER_TIME_STATUS_CALL, TIMER_TIME_STATUS_CALL, TIMER_SIVAL_STATUS_CALL);
		break;
	case SIP_TASK_TYPE_BC_INCOMING:
		memset(data, 0, sizeof (data));
		data[0] = 0x01;
		data[1] = 0x00;
		data[2] = 0x5E;
		data[3] = 0x00;
		data[4] = 0x00;
		data[5] = 0xFE;
		data[6 + 8 + 0] = 0xEA;;
		data[6 + 8 + 1] = data[3];
		data[6 + 8 + 2] = data[4];
		data[6 + 8 + 3] = 0xFE;
		if (m_sip_ua.m_pmine_type_audio != NULL \
			&& strcasecmp(m_sip_ua.m_pmine_type_audio, g_mime_type_mp3) == 0 \
			&& !m_sip_ua.m_audio_stream.get_enable_mp3decoder_when_memory())
		{
			m_audio_format = AUDIO_FORMAT_MP3;
			send_net_data(NET_MSG_SWITCH_TASK, 0x01, 0, \
					data[6 + 8 + 0], data[6 + 8 + 1], data[6 + 8 + 2], \
					data, sizeof(data));
		}
		else
		{
			m_audio_format = AUDIO_FORMAT_L16;
			send_net_data(NET_MSG_SWITCH_TASK, 0x11, 0, \
					data[6 + 8 + 0], data[6 + 8 + 1], data[6 + 8 + 2], \
					data, sizeof(data));
		}
		break;
	default:
		break;
	}
}

void CTerminal::sip_event_closed(eXosip_event_t *p_event)
{
	BYTE		data[6 + 8 + 4];
	switch (m_sip_ua.m_task_type)
	{
	case SIP_TASK_TYPE_TALK_OUTGOING:
	case SIP_TASK_TYPE_MONITOR_OUTGOING:
		send_net_data(NET_MSG_TALK_STATUS, 0x04, BYTE1(m_id), BYTE2(m_id), BYTE1(m_talk_id), BYTE2(m_talk_id), NULL, 0);
		break;
	case SIP_TASK_TYPE_TALK_INCOMING:
	case SIP_TASK_TYPE_MONITOR_INCOMING:
		send_net_data(NET_MSG_TALK_REQUEST, 0x00, BYTE1(m_id), BYTE2(m_id), BYTE1(m_talk_id), BYTE2(m_talk_id), NULL, 0);
		send_net_data(NET_MSG_TALK_STATUS, 0x04, BYTE1(m_id), BYTE2(m_id), BYTE1(m_talk_id), BYTE2(m_talk_id), NULL, 0);
		break;
	case SIP_TASK_TYPE_BC_INCOMING:
		memset(data, 0, sizeof (data));
		send_net_data(NET_MSG_SWITCH_TASK, 0x00, 0, 0, 0, 0, data, sizeof(data));
		break;
	default:
		break;
	}
	m_audio_format = AUDIO_FORMAT_L16;
}

void CTerminal::sip_event_message_new(eXosip_event_t *p_event)
{
	osip_body_t				*p_body = NULL;
	char					*p_cmd_type = NULL;
	char					*p_param1 = NULL, *p_param2 = NULL, *p_param3 = NULL;
	int						data1, data2;

	if (MSG_IS_MESSAGE(p_event->request) || MSG_IS_NOTIFY(p_event->request))
	{
		osip_message_get_body(p_event->request, 0, &p_body);
		if (p_body != NULL && p_body->body != NULL)
		{
			do
			{
				if (strncmp(p_body->body, "[COMMAND]", strlen("[COMMAND]")) != 0)
					break;
				p_cmd_type = strstr(p_body->body, "cmd_type");
				if (p_cmd_type == NULL)
					break;
				p_cmd_type += strlen("cmd_type") + 1;
				if (strncmp(p_cmd_type, "alarm_out", strlen("alarm_out")) == 0)
				{
					p_param1 = strstr(p_body->body, "operation");
					p_param2 = strstr(p_body->body, "alarm_no");
					if (p_param1 == NULL || p_param2 == NULL)
						break;
					p_param1 += strlen("operation") + 1;
					p_param2 += strlen("alarm_no") + 1;
					if (strncmp(p_param1, "control", strlen("control")) == 0)
					{
						p_param3 = strstr(p_body->body, "alarm_status");
						if (p_param3 == NULL)
							break;
						p_param3 += strlen("alarm_status") + 1;
						data1 = atoi(p_param2);
						data2 = atoi(p_param3);
						if ((data1 / 100) == 5)
						{
							data1 %= 100;
							if (data2 == 0)
								send_net_data(NET_MSG_SHORTOUTPUT_CTRL, (BYTE)(data1), 0, 0, 0, 0, NULL, 0);
							else
								send_net_data(NET_MSG_SHORTOUTPUT_CTRL, (BYTE)(data1), 1, 0, 0, 0, NULL, 0);
						}
					}
				}
				else if (strncmp(p_cmd_type, "volume", strlen("volume")) == 0)
				{
					p_param1 = strstr(p_body->body, "broadcast_output_volume");
					if (p_param1 != NULL)
					{
						p_param1 += strlen("broadcast_output_volume") + 1;
						data1 = atoi(p_param1);
						send_net_data(NET_MSG_MODIFY_VOLUME, (BYTE)(data1), 0, 0, 0, 0, NULL, 0);
					}
					p_param1 = strstr(p_body->body, "talk_input_volume");
					p_param2 = strstr(p_body->body, "talk_output_volume");
					if (p_param1 != NULL && p_param2 != NULL)
					{
						p_param1 += strlen("talk_input_volume") + 1;
						p_param2 += strlen("talk_output_volume") + 1;
						data1 = atoi(p_param1);
						data2 = atoi(p_param2);
						send_net_data(NET_MSG_MIXER_IN, MIXERIN_OP_MICROPHONE, (BYTE)(data1), 0, 0, 0, NULL, 0);
						send_net_data(NET_MSG_MIXER_OUT, MIXEROUT_OP_SPEAK, (BYTE)(data2), 0, 0, 0, NULL, 0);
					}
				}
			} while (0);
		}
	}
}

void CTerminal::sip_event_call_ringing(eXosip_event_t *p_event)
{
	BYTE			data[10];

	if (m_psocket_spon != NULL)
	{
        *((IP4ADDR *)(&data[0])) = m_psocket_spon->m_hostaddr.sin_addr.s_addr;
        *((IP4ADDR *)(&data[4])) = m_psocket_spon->m_hostaddr.sin_addr.s_addr;
        *((WORD *)(&data[4 + 4])) = htons(m_psocket_spon->m_hostaddr.sin_port);
		send_net_data(NET_MSG_IP_REPLY, 0x11, data[0], data[1], data[2], data[3], &data[4], 6);
	}
}

void CTerminal::sip_event_reg_success(eXosip_event_t *p_event)
{
    BYTE			data[22];
	time_t          now;
	struct tm       *p_tm_local;

	if (m_login != m_sip_ua.m_registered)
	{
		time(&now);
		p_tm_local = localtime(&now);
		data[0] = (BYTE)(p_tm_local->tm_year & 0xFF);
		data[1] = (BYTE)((p_tm_local->tm_year >> 8) & 0xFF);
		data[2] = (BYTE)(p_tm_local->tm_sec & 0xFF);
		data[3] = (BYTE)((p_tm_local->tm_sec >> 8) & 0xFF);
		data[4] = 0;
		data[5] = 0;
		send_net_data(NET_MSG_DATETIME, (BYTE)p_tm_local->tm_wday, (BYTE)p_tm_local->tm_mon, (BYTE)p_tm_local->tm_mday, (BYTE)p_tm_local->tm_hour, \
			(BYTE)p_tm_local->tm_min, data, 6);

		memset(data, 0, sizeof (data));
		data[14 + 0] = 0x34;
		data[14 + 1] = 0x12;
		data[14 + 6] = 0x21;
		data[14 + 7] = 0x43;
		data[14 + 4] = 0x01;	// BC --- 0 : 8000; 1 : 22050;
		data[14 + 5] = 0x00;	// CODEC --- 0 : PCM; 1 : ADPCM;
		if (m_sip_ua.m_registered)
		{
			m_login = TRUE;
			send_net_data(NET_MSG_LOGON_STATE, 0x01, 0, 0, 0, 0, data, 22);
			send_net_data(NET_MSG_MODIFY_VOLUME, (BYTE)(m_sip_ua.m_bc_output_volume), 0, 0, 0, 0, NULL, 0);
			send_net_data(NET_MSG_MIXER_IN, MIXERIN_OP_MICROPHONE, (BYTE)(m_sip_ua.m_talk_input_volume), 0, 0, 0, NULL, 0);
			send_net_data(NET_MSG_MIXER_OUT, MIXEROUT_OP_SPEAK, (BYTE)(m_sip_ua.m_talk_output_volume), 0, 0, 0, NULL, 0);
		}
		else
		{
			m_login = FALSE;
			send_net_data(NET_MSG_LOGON_STATE, 0x00, 0, 0, 0, 0, data, 22);
		}
	}
}

void CTerminal::net_msg_login_request(SOCKET_DATA *p_socket_data)
{
    BYTE			data[22];
	time_t          now;
	struct tm       *p_tm_local;

	m_handshake_timeout = 0;
	if (m_login)
	{
		m_socket_addr = p_socket_data->addr_from;
		time(&now);
		p_tm_local = localtime(&now);
		data[0] = (BYTE)(p_tm_local->tm_year & 0xFF);
		data[1] = (BYTE)((p_tm_local->tm_year >> 8) & 0xFF);
		data[2] = (BYTE)(p_tm_local->tm_sec & 0xFF);
		data[3] = (BYTE)((p_tm_local->tm_sec >> 8) & 0xFF);
		data[4] = 0;
		data[5] = 0;
		send_net_data(NET_MSG_DATETIME, (BYTE)p_tm_local->tm_wday, (BYTE)p_tm_local->tm_mon, (BYTE)p_tm_local->tm_mday, (BYTE)p_tm_local->tm_hour, \
			(BYTE)p_tm_local->tm_min, data, 6);

        memset(data, 0, sizeof (data));
        data[14 + 0] = 0x34;
        data[14 + 1] = 0x12;
        data[14 + 6] = 0x21;
        data[14 + 7] = 0x43;
        data[14 + 4] = 0x01;	// BC --- 0 : 8000; 1 : 22050;
        data[14 + 5] = 0x00;	// CODEC --- 0 : PCM; 1 : ADPCM;
		send_net_data(NET_MSG_LOGON_STATE, 0x01, 0, 0, 0, 0, data, 22);
		send_net_data(NET_MSG_MODIFY_VOLUME, (BYTE)(m_sip_ua.m_bc_output_volume), 0, 0, 0, 0, NULL, 0);
		send_net_data(NET_MSG_MIXER_IN, MIXERIN_OP_MICROPHONE, (BYTE)(m_sip_ua.m_talk_input_volume), 0, 0, 0, NULL, 0);
		send_net_data(NET_MSG_MIXER_OUT, MIXEROUT_OP_SPEAK, (BYTE)(m_sip_ua.m_talk_output_volume), 0, 0, 0, NULL, 0);

		m_sip_ua.m_registered = FALSE;
		m_sip_ua.register_server();
	}
}

void CTerminal::net_msg_handshake(SOCKET_DATA *p_socket_data)
{
	int		tmp_id = m_talk_id;

	m_handshake_timeout = 0;
	m_talk_id = m_id;
	send_net_data(NET_MSG_HAND_SHAKE, 0, 0, 0, 0, 0, NULL, 0);
	m_talk_id = tmp_id;
}

void CTerminal::net_msg_control_door(SOCKET_DATA *p_socket_data)
{
	NET_MSG_HDR			*p_msg = (NET_MSG_HDR *)p_socket_data->p_buf;
	char				p_alarm_no[20];
	char				p_status[10];
	char				p_id[SIP_UA_USERNAME_LEN];

	sprintf(p_alarm_no, "%d", p_msg->param1 + 1 + 500);
	if (p_msg->param2 == 0)
		sprintf(p_status, "%d", 0);
	else
		sprintf(p_status, "%d", 1);
	sprintf(p_id, "%s", m_sip_ua.m_callee_username);
	m_sip_ua.send_msg_alarm_out(p_id, "control", p_alarm_no, p_status);
}

void CTerminal::net_msg_control_shortoutput(SOCKET_DATA *p_socket_data)
{
	NET_MSG_HDR			*p_msg = (NET_MSG_HDR *)p_socket_data->p_buf;
	char				p_alarm_no[20];
	char				p_status[10];
	char				p_id[SIP_UA_USERNAME_LEN];

	sprintf(p_alarm_no, "%d", p_msg->param1 + 500);
	if (p_msg->param2 == 0)
		sprintf(p_status, "%d", 0);
	else
		sprintf(p_status, "%d", 1);
	sprintf(p_id, "%s", m_sip_ua.m_callee_username);
	m_sip_ua.send_msg_alarm_out(p_id, "control", p_alarm_no, p_status);
}

void CTerminal::net_msg_state_report(SOCKET_DATA *p_socket_data)
{
	NET_MSG_HDR			*p_msg = (NET_MSG_HDR *)p_socket_data->p_buf;
	char				p_alarm_no[20];
	char				p_status[10];
	char				p_id[SIP_UA_USERNAME_LEN];
	int					i;
	char				p_op_report[] = "report", p_op_reply[] = "reply";
	char				*p_op_sel = p_op_report;

	if (p_msg->param1 == 0)									// 防拆
		sprintf(p_alarm_no, "%d", 101);
	else if (p_msg->param1 >= 1 && p_msg->param1 <= 0x7D)	// 报警输入端口
		sprintf(p_alarm_no, "%d", p_msg->param1);
	else if (p_msg->param1 == 0x7E)							// 喧哗
		sprintf(p_alarm_no, "%d", 141);
	else if (p_msg->param1 == 0x7F)							// 巡更
		sprintf(p_alarm_no, "%d", 161);
	else if (p_msg->param1 >= 0x81 && p_msg->param1 <= 0xFF)// 报警输出端口
	{
		sprintf(p_alarm_no, "%d", p_msg->param1 - 0x81 + 501);
		p_op_sel = p_op_reply;
	}

	if (p_msg->param2 == 0)
		sprintf(p_status, "%d", 0);
	else
		sprintf(p_status, "%d", 1);

	for (i = 0; i < SIP_UA_MESSAGE_TARGET_MAX && m_sip_ua.m_message_target[i] != 0; i++)
	{
		sprintf(p_id, "%d", m_sip_ua.m_message_target[i]);
		m_sip_ua.send_msg_alarm_in(p_id, p_op_sel, p_alarm_no, p_status);
	}
}

void CTerminal::net_msg_talk_status(SOCKET_DATA *p_socket_data)
{
	NET_MSG_HDR			*p_msg = (NET_MSG_HDR *)p_socket_data->p_buf;

	switch (p_msg->param1)
	{
	case 0:
	case 4:
		m_sip_ua.task_end();
		break;
	case 2:
		m_sip_ua.ring();
		break;
	case 3:
		m_sip_ua.answer();
		break;
	default:
		break;
	}
}

void CTerminal::net_msg_talk_request(SOCKET_DATA *p_socket_data)
{
	NET_MSG_HDR			*p_msg = (NET_MSG_HDR *)p_socket_data->p_buf;

	switch (p_msg->param1)
	{
	case 0:
	case 4:
		m_sip_ua.task_end();
		break;
	case 1:
		if (m_sip_ua.m_status == SIP_STATUS_CALLING || m_sip_ua.m_status == SIP_STATUS_RINGING)
			send_net_data(NET_MSG_TALK_STATUS, 0x02, BYTE1(m_id), BYTE2(m_id), BYTE1(m_talk_id), BYTE2(m_talk_id), NULL, 0);
		break;
	default:
		break;
	}
}

void CTerminal::net_msg_talk_data(SOCKET_DATA *p_socket_data)
{
	NET_MSG_HDR			*p_msg = (NET_MSG_HDR *)p_socket_data->p_buf;
	WORD				check_sum = 0;

	if (m_sip_ua.m_audio_stream.m_audio_src == AUDIO_SRC_MEMORY)
		m_sip_ua.m_audio_stream.write_rtp_senddata((short *)p_msg->data, (p_socket_data->data_len - NET_MSG_HDR_SIZE) / sizeof (short));
}

void CTerminal::net_msg_wave_live(SOCKET_DATA *p_socket_data)
{
	NET_MSG_HDR			*p_msg = (NET_MSG_HDR *)p_socket_data->p_buf;
	WORD				check_sum = 0;

	if (m_sip_ua.m_audio_stream.m_audio_src == AUDIO_SRC_MEMORY)
		m_sip_ua.m_audio_stream.write_rtp_senddata((short *)p_msg->data, (p_socket_data->data_len - NET_MSG_HDR_SIZE) / sizeof (short));
}

void CTerminal::net_msg_ip_request(SOCKET_DATA *p_socket_data)
{
	NET_MSG_HDR			*p_msg = (NET_MSG_HDR *)p_socket_data->p_buf;
	WORD				to_id = p_msg->param2 | (p_msg->param3 << 8);
	char				str_to_id[SIP_UA_USERNAME_LEN];

	if (m_sip_ua.m_status == SIP_STATUS_NULL)
	{
		if (to_id == 0)
			sprintf(str_to_id, "%s", m_sip_ua.m_call_target[0]);
		else if (to_id == 0xffff)
			sprintf(str_to_id, "%s", m_sip_ua.m_call_target[1]);
		else
			sprintf(str_to_id, "%d", SIP_SERVER_USERNAME(to_id));
		m_talk_id = to_id;
		printf_log(LOG_IS_INFO, "[%d][CTerminal::net_msg_ip_request()] : Call - %s\n", m_id, str_to_id);
		m_sip_ua.task_end();
		if (p_msg->param1 == 1)
			m_sip_ua.monitor(str_to_id, NULL, NULL);
		else
			m_sip_ua.talk(str_to_id, NULL, NULL);
	}
}

#define BC_MAX		20
void CTerminal::net_msg_broadcast_request(SOCKET_DATA *p_socket_data)
{
	NET_MSG_HDR			*p_msg = (NET_MSG_HDR *)p_socket_data->p_buf;
	char				str_to_id[64] = {0};
	int					i, j, str_len, to_id;
	int					count = 0;
	int					term_len = p_socket_data->data_len - 4;
	BYTE				*p_term = &p_msg->param2;
	BYTE				data[6 + 8 + 4];

	switch (p_msg->param1)
	{
	case 0x00:
	case 0x06:
	case 0x08:
		if (m_sip_ua.m_status != SIP_STATUS_NULL)
		{
			memset(data, 0, sizeof (data));
			send_net_data(NET_MSG_SWITCH_TASK, 0x00, 0, 0, 0, 0, data, sizeof(data));
		}
		m_sip_ua.task_end();
		break;
	case 0x01:
		for (i = 0; i < term_len && count < BC_MAX; i++)
		{
			for (j = 0; j < 8 && count < BC_MAX; j++)
			{
				if ((p_term[i] & (1 << j)) != 0)
				{
					to_id = i * 8 + j + 1;
					to_id = (to_id % 32) + (to_id / 32 * 30);	// 分区广播的分区号
					str_len = strlen(str_to_id);
					if (count == 0)
						sprintf(&str_to_id[str_len], "%d", SIP_SERVER_USERNAME(to_id));
					else
						sprintf(&str_to_id[str_len], "-%d", SIP_SERVER_USERNAME(to_id));
					count++;
				}
			}
		}
		printf_log(LOG_IS_INFO, "[%d][CTerminal::net_msg_broadcast_request()] : Broadcast - %s\n", m_id, str_to_id);
		m_sip_ua.task_end();
		m_sip_ua.broadcast(str_to_id, "WAV", FALSE);
		break;
	case 0x07:
	case 0x09:
		for (i = 0; i < term_len && count < BC_MAX; i++)
		{
			for (j = 0; j < 8 && count < BC_MAX; j++)
			{
				if ((p_term[i] & (1 << j)) != 0)
				{
					to_id = i * 8 + j + 1;
					str_len = strlen(str_to_id);
					if (count == 0)
						sprintf(&str_to_id[str_len], "%d", SIP_SERVER_USERNAME(to_id));
					else
						sprintf(&str_to_id[str_len], "-%d", SIP_SERVER_USERNAME(to_id));
					count++;
				}
			}
		}
		printf_log(LOG_IS_INFO, "[%d][CTerminal::net_msg_broadcast_request()] : Broadcast - %s\n", m_id, str_to_id);
		m_sip_ua.task_end();
		m_sip_ua.broadcast(str_to_id, "WAV", TRUE);
		break;
	default:
		break;
	}
}

BOOL CTerminal::send_net_data(BYTE msg, BYTE param1, BYTE param2, BYTE param3, BYTE param4, BYTE param5, BYTE *p_buf, WORD size)
{
	BOOL				b_send = FALSE;
	BYTE				data[SEND_BUFFER_SIZE];
	NET_MSG_HDR			*p_msg = (NET_MSG_HDR *)data;

	if (m_psocket_spon != NULL)
	{
		memset(p_msg, 0, NET_MSG_HDR_SIZE);
		p_msg->id = (WORD)m_talk_id - 1;
		p_msg->msg = msg;
		p_msg->param1 = param1;
		p_msg->param2 = param2;
		p_msg->param3 = param3;
		p_msg->param4 = param4;
		p_msg->param5 = param5;
		if (size > SEND_BUFFER_SIZE - NET_MSG_HDR_SIZE)
			size = SEND_BUFFER_SIZE - NET_MSG_HDR_SIZE;
		if (p_buf != NULL)
			memcpy(p_msg->data, p_buf, size);
		else
			size = 0;
		b_send = m_psocket_spon->send_data(&m_socket_addr, data, size + NET_MSG_HDR_SIZE);

	}

	return b_send;
}

#ifdef WIN32
// 定时器处理
void CALLBACK CTerminal::on_timer_queue(void *p_param, BOOL timeorwait)
{
    timer_data_t	*p_tdata = (timer_data_t *)p_param;
    CTerminal       *p_terminal = (CTerminal *)p_tdata->p_param;

    switch (p_tdata->sival)
    {
    case TIMER_SIVAL_CHECK_TASK:
        break;
    case TIMER_SIVAL_STATUS_CALL:
		if (p_terminal->m_sip_ua.m_status == SIP_STATUS_CALLING || p_terminal->m_sip_ua.m_status == SIP_STATUS_RINGING)
			p_terminal->send_net_data(NET_MSG_TALK_REQUEST, 0x01, BYTE1(p_terminal->m_id), BYTE2(p_terminal->m_id), \
				BYTE1(p_terminal->m_talk_id), BYTE2(p_terminal->m_talk_id), NULL, 0);
		else if (p_tdata->timerid != NULL)
            ua_delete_timer_queue(p_tdata, FALSE);
        break;
    default:
        break;
    }
}
#else
// 定时器处理
void CTerminal::on_timer_queue(union sigval sig_val)
{
    timer_data_t	*p_tdata = (timer_data_t *)sig_val.sival_ptr;
    CTerminal		*p_terminal = (CTerminal *)p_tdata->p_param;

    switch (p_tdata->sival)
    {
    case TIMER_SIVAL_CHECK_TASK:
        break;
    case TIMER_SIVAL_STATUS_CALL:
		if (p_terminal->m_sip_ua.m_status == SIP_STATUS_CALLING || p_terminal->m_sip_ua.m_status == SIP_STATUS_RINGING)
			p_terminal->send_net_data(NET_MSG_TALK_REQUEST, 0x01, BYTE1(p_terminal->m_id), BYTE2(p_terminal->m_id), \
				BYTE1(p_terminal->m_talk_id), BYTE2(p_terminal->m_talk_id), NULL, 0);
		else
            ua_delete_timer_queue(p_tdata, FALSE);
        break;
    default:
        break;
    }
}
#endif
