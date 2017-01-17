// XC9000UATest1.cpp : Defines the entry point for the console application.
//

#include "ua_port.h"
#include "socket.h"
#include "ua_global.h"
#include "SipUA.h"
#include "SipSponServer.h"
#include "log.h"

#ifdef WIN32
#pragma comment(lib, "XC9000UA.lib")
#include <conio.h>
#else
//#include <curses.h>
#endif


#define TERM_COUNT_SET			1000
#define MSGID_SOCKET_SPON		'a'
#define SIP_LOCAL_PORT			6000
#define SIP_SERVER_PORT			5060
#define SIP_SERVER_IP			"192.168.186.13"
#define SIP_SERVER_PASSWORD		"1234"
#define SIP_SERVER_HANDSHAKE_TIME	10000			// ÖÕ¶ËÎÕÊÖ¼ì²â(ms)

#define TEST_ID			"1012"

const char g_user_manual[] = "\n \
	w : wav broadcast\n  \
	m : mp3 broadcast\n  \
	s : sound card broadcast\n \
	t : talk\n \
	n : monitor\n \
	a : answer\n \
	h : hanHandupg up\n \
	0~9 : modify volume\n \
	q : exit\n";

#ifndef WIN32
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
return 0;
}
#endif

#define COMMAND_BUF_LEN		64
//#define TEST_SERVER
int xc9000ua( )
{
#ifdef TEST_SERVER
	BOOL				b_exit = FALSE;
	char				command[COMMAND_BUF_LEN] = {0}, char_input;
	int					cmd_count = 0;
	CSipSponServer		server;
	char				server_ip[64] = {0};

	printf("begin...\n");
    if (argc > 1)
	{
		strcpy(server_ip, argv[1]);
		printf("server ip : %s", server_ip);
	}
	else
	{
		printf("please input server ip(y(%s)/other ip) : ", SIP_SERVER_IP);
		scanf("%s",server_ip);
		if (!strcasecmp(server_ip, "y"))
			strcpy(server_ip, SIP_SERVER_IP);
	}
	server.set_server_addr(server_ip, SIP_SERVER_PORT);
	server.open();
	//set_log_file(LOG_IS_INFO);

	printf("%s", "\n \
	c : clear screen\n  \
	q : exit\n\n"); 

#ifndef WIN32
	fcntl(0, F_SETFL, O_NONBLOCK);
#endif
    do
    {
#ifdef WIN32
		if (_kbhit() != 0)
		{
			char_input = _getch();
			putchar(char_input);
			if (cmd_count > COMMAND_BUF_LEN - 1)
				cmd_count = COMMAND_BUF_LEN - 1;
			if (char_input != 0x0d)
				command[cmd_count++] = char_input;
			else
			{
				putchar('\n');
				command[cmd_count] = '\0';
				cmd_count = 0;
				if (strcasecmp(command, "q") == 0)
					b_exit = TRUE;
				else if (strcasecmp(command, "c") == 0)
					system("cls");
			}
		}
		ua_usleep(10000);
#else
		if (kbhit())
		{
			char_input = getch();
			putchar(char_input);
			if (cmd_count > COMMAND_BUF_LEN - 1)
				cmd_count = COMMAND_BUF_LEN - 1;
			if (char_input != 0x0d)
				command[cmd_count++] = char_input;
			else
			{
				putchar('\n');
				command[cmd_count] = '\0';
				cmd_count = 0;
				if (strcasecmp(command, "q") == 0)
					b_exit = TRUE;
				else if (strcasecmp(command, "c") == 0)
					system("reset");
			}
		}
		ua_usleep(10000);
#endif
    } while (!b_exit);
#else
	struct timeval		ts;
	CSocketEx			socket_spon;
	CSipUA				sip_ua_1, sip_ua_2;
	char				p_username[16];
	char				p_dst_id[64] = "1052";
	char				p_file[PATH_MAX];
	string				str_file;
	BOOL				b_exit = FALSE;
	char				command;

	char		server_ip[64];


	strcpy(server_ip, "192.168.186.13");
	strcpy(p_username, TEST_ID);

	sip_ua_1.set_local_addr(socket_spon.get_first_hostaddr(), SIP_LOCAL_PORT + atoi(p_username));
	sip_ua_1.set_register_addr(server_ip, SIP_SERVER_PORT);
	sip_ua_1.set_username_password(p_username, SIP_SERVER_PASSWORD);
	sip_ua_1.m_audio_stream.set_enable_sound_card(TRUE);
	sip_ua_1.init();
	sip_ua_1.m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);

	sprintf(p_username, "%d", atoi(p_username) + 1);
	sip_ua_2.set_local_addr(socket_spon.get_first_hostaddr(), SIP_LOCAL_PORT + atoi(p_username));
	sip_ua_2.set_register_addr(server_ip, SIP_SERVER_PORT);
	sip_ua_2.set_username_password(p_username, SIP_SERVER_PASSWORD);
	sip_ua_2.init();
	sip_ua_2.m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);

	ua_usleep(500000);
	get_exe_path(p_file, PATH_MAX);
	str_file = p_file;
	str_file = str_file.substr(0, str_file.find_last_of('\\') + 1);
	fflush(stdin);

	printf("%s", g_user_manual); 
#ifndef WIN32
	fcntl(0, F_SETFL, O_NONBLOCK);
#endif
    ua_get_time(&ts);

	// Ìí¼ÓÎÄ¼þÁÐ±í
	sprintf(p_file, "%s%s", str_file.c_str(), "1.mp3");
	//sip_ua_1.m_audio_stream.m_audio_file.add_file(p_file);
	sprintf(p_file, "%s%s", str_file.c_str(), "1.wav");
	sip_ua_1.m_audio_stream.m_audio_file.add_file(p_file);
	//sip_ua_1.m_audio_file.add_file("C:\\D\\SD¿¨\\wave\\ÂÞÎÄ - ÌúÑªµ¤ÐÄ_8K.wav");
	//sip_ua_1.m_audio_file.add_file("C:\\D\\SD¿¨\\spon\\mÄÁÑòÇú (°é×àÇú).wav");
	//sip_ua_1.m_audio_file.add_file("C:\\D\\SD¿¨\\spon\\mÄÁÑòÇú-Ö£Ð÷á°.mp3");
	sip_ua_1.m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);
	sip_ua_1.talk(p_dst_id, NULL, NULL);
	do
    {
		//ÊäÈëÃüÁî 
		// printf("Please input the command:\n"); 
		scanf("%c",&command); 
		getchar();
		fflush(stdin);

		switch(command) 
		{
		case 's':
			sip_ua_1.m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);
			sip_ua_1.broadcast(p_dst_id, "WAV", TRUE);
			break;
		case 'w':
			sip_ua_1.m_audio_stream.set_audio_src(AUDIO_SRC_FILE);
			sip_ua_1.broadcast(p_dst_id, "WAV", TRUE);
			break;
		case 'm':
			sip_ua_1.m_audio_stream.set_audio_src(AUDIO_SRC_FILE);
			sip_ua_1.broadcast(p_dst_id, "MP3", TRUE);
			break;
		case 't':
			sip_ua_1.m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);
			sip_ua_1.talk(p_dst_id, NULL, NULL);
			break;
		case 'n':
			sip_ua_1.m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);
			sip_ua_1.monitor(p_dst_id, NULL, NULL);
			break;
		case 'h':
			sip_ua_1.task_end();
			break;
		case 'a':
			sip_ua_1.answer();
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			sprintf(p_file, "%c", command);
			sip_ua_1.send_msg_volume(p_dst_id, p_file, p_file, p_file, p_file);
			break;
		case 'q':
			b_exit = TRUE;
			break;
		default:
			// printf("input error! please input again\n");
			break;
		}
		//sip_ua_1.process_sip_event(NULL, NULL);
		//sip_ua_1.process_rtp_data();
		ua_usleep(100000);
    } while (!b_exit);//ua_get_timeout(&ts) < 1200000 && 
#endif

	return 0;
}

