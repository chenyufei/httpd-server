/**********************************************************************************************
*Copyright (C), 2018 ,Jimmy_Nie.  https://www.cnblogs.com/Jimmy1988/
*
*
*File name: httpd.c
*Description:实现httpd功能
* Author		Date		Modify 
* Jimmy			2018-04-16	Create
*
**********************************************************************************************/
#include "http.h"

char home_dir[32];
char ip_addr[16];
char port_no[8];
char backlog[8];

/***********************************************************
 * Function: main函数
 * Input:
 * Output: 
 * Return: 
 * Autor		Date			Modify
 * Jimmy		2018-04-16		Create
 * 
 ***********************************************************/
int main(int argc, char *argv[])
{
	int rtn = 0;
	int sock_fd = -1;
	struct sockaddr_in client_addr;
	char buf[256];
	pid_t pid = -1;
	
	//1. 以守候进程的方式运行此http进程
	init_daemon(argv[0], LOG_INFO);

	//2. 获取参数值
	//2.1 获取home_dir, 如果配置文件没有，则默认为/tmp
	rtn = read_conf("home_dir", home_dir);
	if(rtn < 0)
		exit(EXIT_FAILURE);
	else if(0 == rtn)	//即未从conf文件中读取到数据
	{
		bzero(home_dir, sizeof(home_dir));
		strcpy(home_dir, "/tmp");
	}

	//2.2 获取ip_addr, 如果配置文件没有，则设置为本机的wlan0的ip地址
	rtn = read_conf("ip_addr", ip_addr);
	if(rtn < 0)
		exit(EXIT_FAILURE);
	else if(0 == rtn)	//即未从conf文件中读取到数据
	{
		bzero(ip_addr, sizeof(ip_addr));
		get_ip_addr(ip_addr);
	}

	//2.3 获取port_no, 如果配置文件没有，则默认为80
	rtn = read_conf("port_no", port_no);
	if(rtn < 0)
		exit(EXIT_FAILURE);
	else if(0 == rtn)	//即未从conf文件中读取到数据
	{
		bzero(port_no, sizeof(port_no));
		strcpy(port_no, "80");
	}

	//2.4 获取port_no, 如果配置文件没有，则默认为10(最多链接10个client)
	rtn = read_conf("backlog", backlog);
	if(rtn < 0)
		exit(EXIT_FAILURE);
	else if(0 == rtn)	//即未从conf文件中读取到数据
	{
		bzero(backlog, sizeof(backlog));
		strcpy(backlog, "10");
	}

	write_info("home_dir=%s, ip_addr=%s, port_no=%d, backlog=%d\n", home_dir, ip_addr, atoi(port_no), atoi(backlog));
	
	//3. 初始化socket
	rtn = init_socket(&sock_fd);
	if(rtn < 0)
		exit(EXIT_FAILURE);

	//4. 接收新的client链接
	while(1)
	{
		int len;
		int new_fd = -1;

		len = sizeof(struct sockaddr_in);;
		memset(&client_addr, 0, len);
		new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &len);

		if(new_fd < 0)
		{
			write_info("accept new client connection failed !! [%d:%s] \n", errno, strerror(errno));	
			exit(EXIT_FAILURE);
		}

		bzero(buf, sizeof(buf));
		sprintf(buf, "Connect from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		write_info("%s\n", buf);

		if((pid = fork()) == -1)
		{
			write_info("fork() failed");
			exit(EXIT_FAILURE);
		}
		
		else if(0 == pid)	//在子进程中
		{
			bzero(buf, sizeof(buf));
			len = recv(new_fd, buf, sizeof(buf), 0);
			if(len > 0)		//接到了信息
			{
				char req[256];
				bzero(req, sizeof(req));

				sscanf(buf, "GET %s HTTP", req);
				bzero(buf, sizeof(buf));
				sprintf(buf, "Request get the file: %s", req);
				write_info("%s\n", buf);
				response_client(new_fd, req);
			}
			sleep(30);	//建议使用chrome，firefox和IE貌似一直在请求，一旦tcp链接断开，就GG了
			exit(EXIT_SUCCESS);
		}
		
		else 	//在父进程中
		{
			close(new_fd);
			continue;
		}
	}

	close(sock_fd);
	return 0;
}
