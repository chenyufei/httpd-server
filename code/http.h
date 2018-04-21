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
#ifndef HTTP_H
#define HTTP_H

#include <arpa/inet.h>
#include <dirent.h>

#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <netinet/in.h>

#include <pwd.h>
#include <grp.h>

#include <stdio.h>
#include <sys/socket.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdarg.h>

#include <time.h>
#include <unistd.h>

/*********************************全局变量***************************************/
extern char home_dir[32];
extern char ip_addr[16];
extern char port_no[8];
extern char backlog[8];

#define _BSD_SOURCE

/***********************************************************
 * Function: write_info
 * Input:
 * Output: 
 * Return: 
 * Autor		Date			Modify
 * Jimmy		2018-04-16		Create
 * 
 ***********************************************************/
#define write_info(fmt, arg...)	\
{														\
	char buff[512]; char buff1[256];					\
	bzero(buff, sizeof(buff));							\
	bzero(buff1, sizeof(buff));							\
	sprintf(buff, "<%s:%d> ", __FUNCTION__, __LINE__);	\
	sprintf(buff1, fmt, ##arg);							\
	strcat(buff, buff1);								\
	syslog(LOG_INFO, "%s", buff);						\
}


/*********************************函数声明***************************************/
extern void init_daemon(char *p_name, int facility);
extern  int init_socket(int *p_sockfd);
extern int get_ip_addr(char *ip_addr);
extern void response_client(int client_sock, char *path);
extern int read_conf(char *cmd, char *data);


#endif
