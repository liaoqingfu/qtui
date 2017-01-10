int udp_socket_bind(int port)
{
	int sockfd;
	struct sockaddr_in si;
	u_int yes = 1;
	int ret;

	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0)
		return sockfd;

	memset(&si, 0, sizeof(struct sockaddr_in));
	si.sin_family = AF_INET;
	si.sin_addr.s_addr = htonl(INADDR_ANY);
	si.sin_port = htons(port);

	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	if (ret < 0)
		return ret;

	ret = bind(sockfd, (struct sockaddr *)(&si), sizeof(struct sockaddr));
	if (ret < 0)
		return ret;

	return sockfd;
}


static void *udp_recv_thread(void *arg)
{
	
	fd_set rfds;
	int len = -1;
	
	if (c->recv_buf) {
		ncs_free(c->recv_buf);
		c->recv_buf = NULL;
	}
	if (c->recv_buf_len > 0) {
		c->recv_buf = ncs_malloc(c->recv_buf_len);
		ncs_memset(c->recv_buf,0,c->recv_buf_len);
	}


	while(TRUE) {
		
		//此处加入30秒重新连接的机制
		if (c->fd < 0 && !ncs_connect_open(c)) { 
			c->restart_try_count++;
			ncs_connect_recv_thread_status_set(c,FALSE);
			if (c->status_change_func) c->status_change_func(c,FALSE);
		} 
		else {
			c->restart_try_count = 0;
			ncs_connect_recv_thread_status_set(c,TRUE);
			if (c->status_change_func) c->status_change_func(c,TRUE);
		}
		
		pthread_mutex_lock(&c->recv_mutex);
		while (ncs_connect_recv_thread_status_get(c)) {
			ncs_thread_running_update(this_thread);

			fd_set rfds;
			FD_ZERO(&rfds);
			FD_SET(c->fd, &rfds);
			
			//一直等待有数据到来
			//if (select(c->fd + 1, &rfds, NULL, NULL, NULL) <= 0) {

			//等待3秒钟，数据不来再次等待
			struct timeval timeout ={3,0};
			if (select(c->fd + 1, &rfds, NULL, NULL, &timeout) <= 0) {
				continue;
			}
				
			if (FD_ISSET(c->fd, &rfds)) {
				if (c->opt.recv) {
					len = c->opt.recv(c);
				}
				if (len == -1) {
					SPON_LOG_INFO("---[THREAD EXIT:%s ]--------------------------->\n",c->connect_name);
					ncs_connect_recv_thread_status_set(c,FALSE);
				}
			}
		}
		pthread_mutex_unlock(&c->recv_mutex);	

		//此处加入30秒重新连接的机制
		ncs_connect_close(c);
		
		SPON_LOG_INFO("---[CONNECTT :%s  WAIT:%d] \n",c->connect_name,c->restart_wait_sec);
		if (c->restart_wait_sec > 0) sleep(c->restart_wait_sec);
 		else sleep(NCS_CONNECT_RESTART_WAIT_DEFAULT_SEC);
			
		SPON_LOG_INFO("---[THREAD END:%s ]--------------------------->\n",c->connect_name);
	}
	
	if (c->recv_buf_len > 0) {
		if (c->recv_buf) {
			ncs_free(c->recv_buf);
			c->recv_buf = NULL;
		}
	}

	ncs_thread_state_set(this_thread, NCS_THREAD_STOP);
	pthread_exit(NULL);
	return NULL;
}

