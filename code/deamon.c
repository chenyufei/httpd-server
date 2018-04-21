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

/***********************************************************
 * Function: init_daemon
 * Input: 
 	1. char *p_name: 进程名称
 	2. int facility: 系统log进程的level
 * Output: 
 * Return: 
 * Autor		Date			Modify
 * Jimmy		2018-04-16		Create
 * 
 ***********************************************************/
void init_daemon(char *p_name, int facility)
{
	int pid = 0;
	int cnt = 0;

	//1. 忽略所有可能的终端信号(守候进程不能受到终端的影响)
	signal(SIGINT, SIG_IGN);	//终端中断符
	signal(SIGTTOU, SIG_IGN);	//后台向控制端tty写作业
	signal(SIGTSTP, SIG_IGN);	//终端挂起
	signal(SIGHUP, SIG_IGN);	//链接断开
	signal(SIGQUIT, SIG_IGN);	//终端退出

	//2. 创建子进程，父进程推出(因为父进程由终端创建)
	if((pid = fork()) > 0)		//在父进程中
		exit(EXIT_SUCCESS);
	else if(pid < 0)
	{
		printf("fork error: [%d:%s]", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	//3. 设置新的会话组长，新进程组长，脱离终端
	setsid();

	//4. 再次创建一个子进程，并让现在的父进程退出
	if((pid = fork()) > 0)		//在父进程中
		exit(EXIT_SUCCESS);
	else if(pid < 0)
	{
		printf("fork error: [%d:%s]", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	//5. 修改子进程的主目录为/tmp
	chdir("/tmp");

	//6. 重置文件掩码
	umask(0);

	//7. 关闭所有父进程打开的文件描述符(包括stdin/stdout/stderr这三个; NOFILE为256)
	for(cnt=0; cnt<NOFILE; cnt++)
		close(cnt);

	//8. 忽略子进程的退出信号
	signal(SIGCHLD, SIG_IGN);

	//9. 与syslogd守候进程关联
	openlog(p_name, LOG_PID, facility);	//每个message都会增加pid进去
	
	return ;
}



/***********************************************************
 * Function: read_conf
 * Description: 读取配置文件，找出命令对应的值
 * Input:
 	1. char *cmd: 欲查找的命令
 * Output: 
 	1. char *data: 查找到的命令的值
 * Return: 
 	1. 命令所在字符串中的位置
 *
 * Autor		Date			Modify
 * Jimmy		2018-04-16		Create
 * 
 ***********************************************************/
int read_conf(char *cmd, char *data)
{
	int fd = -2;
	char buf[1024];
	size_t rd_size;
	char *match = NULL;
	
	memset(buf, 0, sizeof(buf));

	//0. 检查传入的参数
	if((NULL == cmd) || (NULL == data))
	{
		write_info("Input arguments error !");
		return -1;
	}
	
	//1. 打开配置文件
	fd = open("/etc/httpd_test.conf", O_RDONLY);
	if(fd < 0)	//注意，此处fd=0,因为关闭了stdin/stdout/stderr,所以fd应该从0开始 (TMD,排查了很久,以为出错了)
	{
		write_info("open the file /etc/httpd_test.conf failed! [fd=%d] [%d:%s]", fd, errno, strerror(errno));
		return -1;
	}

	//2. 读取文件的所有数据
	rd_size = read(fd, buf, sizeof(buf));
	if((rd_size <= 0) || (rd_size == sizeof(buf)))
	{
		write_info("read the file /etc/httpd_test.conf failed! [%d:%s]", errno, strerror(errno));
		return -1;
	}
	buf[rd_size] = '\0';
	
	//3. 关闭文件
	close(fd);

	//4. 参数匹配
	if(strncmp(cmd, "home_dir", strlen("home_dir")) == 0)
	{
		match = strstr(buf, "home_dir=");
		if(NULL == match)		//若未找到匹配项
		{
			write_info("Does not found the cmd[%s] in the file /etc/httpd_test.conf!", cmd);
			return 0;
		}

		rd_size = sscanf(match, "home_dir=%s", data);
		write_info("Found the command [%s], position=%ld\n", cmd, rd_size);
		return rd_size;
	}

	if(strncmp(cmd, "port_no", strlen("port_no")) == 0)
	{
		match = strstr(buf, "port_no=");
		if(NULL == match)		//若未找到匹配项
		{
			write_info("Does not found the cmd[%s] in the file /etc/httpd_test.conf!", cmd);
			return 0;
		}

		rd_size = sscanf(match, "port_no=%s", data);
		write_info("Found the command [%s], position=%ld\n", cmd, rd_size);
		return rd_size;
	}

	if(strncmp(cmd, "ip_addr", strlen("ip_addr")) == 0)
	{
		match = strstr(buf, "ip_addr=");
		if(NULL == match)		//若未找到匹配项
		{
			write_info("Does not found the cmd[%s] in the file /etc/httpd_test.conf!", cmd);
			return 0;
		}

		rd_size = sscanf(match, "ip_addr=%s", data);
		write_info("Found the command [%s], position=%ld\n", cmd, rd_size);
		return rd_size;
	}

	if(strncmp(cmd, "backlog", strlen("backlog")) == 0)
	{
		match = strstr(buf, "backlog=");
		if(NULL == match)		//若未找到匹配项
		{
			write_info("Does not found the cmd[%s] in the file /etc/httpd_test.conf!", cmd);
			return 0;
		}

		rd_size = sscanf(match, "backlog=%s", data);
		write_info("Found the command [%s], position=%ld\n", cmd, rd_size);
		return rd_size;
	}
	
	return 0;
}

/***********************************************************
 * Function: init_socket
 * Description: 初始化socket
 * Input:
 	1. char *p_sockfd: socket 文件描述符
 * Output: 
 * Return: 
 	0： on success
 	1:	on failure
 *
 * Autor		Date			Modify
 * Jimmy		2018-04-16		Create
 * 
 ***********************************************************/
 int init_socket(int *p_sockfd)
{
	int rtn =0;
	struct sockaddr_in  serv_addr;
	int sock_fd;
	
	//1. 建立socket
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(0 > sock_fd)
	{
		write_info("Create socket failed !! [%d:%s] ", errno, strerror(errno));
		return -1;
	}
	write_info("socket()");
	
	//2. 设置socket允许复用本地IP和端口
	int tmp = 1;
	setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp));

	//3. 绑定本地IP和端口
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(port_no));
	serv_addr.sin_addr.s_addr = inet_addr(ip_addr);

	rtn = bind(sock_fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
	if(rtn < 0)
	{
		write_info("Bind the IP addr [%s] failed !! [%d:%s] ", errno, ip_addr, strerror(errno));
		return -1;
	}
	write_info("bind()");
	
	//4. 监听网络
	rtn = listen(sock_fd, atoi(backlog));
	if(rtn < 0)
	{
		write_info("Listen the network failed !! [%d:%s] ", errno, strerror(errno));
		return -1;
	}
	write_info("listen()");

	*p_sockfd = sock_fd;
	
	return rtn;
}

 /***********************************************************
 * Function: get_ip_addr
 * Description: 初始化socket
 * Input:
 	1. char *p_addr: 本地IP地址
 	2. int port: 本地端口
 	3. int backlog: 最多可接受多少个链接
 * Output: 
 * Return: 
 	0： on success
 	1:	on failure
 *
 * Autor		Date			Modify
 * Jimmy		2018-04-16		Create
 * 
 ***********************************************************/
int get_ip_addr(char *ip_addr)
{
	int sock_fd; 
	struct ifreq ifr;

	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

	strcpy(ifr.ifr_name, "ens33");		//注意此处的修改,不同的系统名称不一样，一般为eth0

	//获取ens33的接口信息
	if(ioctl(sock_fd, SIOCGIFADDR, &ifr) < 0)
	{
		write_info("bind");
		return -1;
	}

	sprintf(ip_addr, "%s", inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr));

	return 0;
}

/***********************************************************
* Function: file_type
* Description: 获取文件的类型
* Input:
   1. mode_t st_mode: 文件类型
* Output: 
* Return: 
	文件类型的标识符
*
* Autor 	   Date 		   Modify
* Jimmy 	   2018-04-17	   Create
* 
***********************************************************/
char file_type(mode_t st_mode)
{
	/*	only support c89
	static char type;
	switch(st_mode & S_IFMT)
	{
		case S_IFSOCK:
			type = 's';
			break;
		case S_IFLNK:
			type = 'l';
			break;
		case S_IFREG:
			type = '-';
			break;
		case S_IFBLK:
			type = 'b';
			break;
		case S_IFCHR:
			type = 'c';
			break;
		case S_IFIFO:
			type = 'p';
			break;
		case S_IFDIR:
			type = 'd';
			break;
		default:
			type = 'E';
			break;
	}

	return type;
	*/

	if (S_ISREG(st_mode))
		return '-';
	else if (S_ISDIR(st_mode))
		return 'd';
	else if (S_ISCHR(st_mode))
		return 'c';
	else if (S_ISBLK(st_mode))
		return 'b';
	else if (S_ISLNK(st_mode))
		return 'l';
	else if (S_ISFIFO(st_mode))
		return 'p';
	//else if (S_ISSOCK(st_mode))
		//return 's';
	else
		return '?';
	
}

/***********************************************************
* Function: dir_up
* Description: 获取上级目录
* Input:
   1. char *dir_path: 目录
* Output: 
* Return: 
	文件类型的标识符
*
* Autor 	   Date 		   Modify
* Jimmy 	   2018-04-17	   Create
* 
***********************************************************/
char *dir_up(char *dir_path)
{
	static char path[128];
	int len;

	strcpy(path, dir_path);
	len = strlen(path);

	if((len > 1) && (path[len-1] == '/'))
		len--;
	while((len > 1) && (path[len-1] != '/'))
		len--;
	
	path[len] = '\0';
	return path;
}

/***********************************************************
* Function: response_client
* Description: 响应客户端的请求:若为dir,则列出所有文件;若为file,则下载
* Input:
   1. char *p_addr: 本地IP地址
   2. char *path:	文件的请求位置
* Output: 
* Return: 
   0： on success
   1:  on failure
*
* Autor 	   Date 		   Modify
* Jimmy 	   2018-04-17	   Create
* 
***********************************************************/
void response_client(int client_sock, char *path)
{
	int rtn = 0;
	int len = 0;
	char real_path[64];
	char file_name[128];
	char tmp_port[8];
	struct stat stat_info;
	char send_msg[512];
	DIR *dir = NULL;
	int fd = -1;
	struct dirent *dirt;
	ssize_t send_size = 0;
	char *p_time ;
	struct passwd *p_user;
	struct group *p_group;
	
	//0. 初始化变量
	memset(real_path, 0, sizeof(real_path));
	memset(tmp_port, 0, sizeof(tmp_port));
	memset(send_msg, 0, sizeof(send_msg));
	memset(file_name, 0, sizeof(file_name));
	//memset(p_time, 0, sizeof(p_time));
	
	//1. 获取绝对路径
	sprintf(real_path, "%s%s", home_dir, path);
	
	write_info("The real_path: %s\n", real_path);
	
	//2. 获取端口信息(用于后续的输出)
	//sprintf(tmp_port, " %s", port_no);

	//3. 获取绝对路径的文件属性(文件还是目录，或其它)
	rtn = stat(real_path, &stat_info);
	if(0 != rtn)
	{
		write_info("Get the path[%s] state failed!! [%d:%s] ", real_path, errno, strerror(errno));
		//将错误信息发送给client端
		sprintf(send_msg, "HTTP/1.1 200 OK\r\nServer: Http test server\r\nConnection: close\r\n\r\n"	\
							"<html><head><title>%d - %s</title></head>"									\
							"<body><font size=+4>Linux http server</font><br><hr width=\"100%%\"><br><center>"	\
							"<table border cols=3 width=\"100%%\"></table><font_color=\"C0000\" size=+2>"		\
							"connect to administrator, error code is \n%s %s </font></body></html>",
							errno, strerror(errno), path, strerror(errno));

		send(client_sock, send_msg, strlen(send_msg)+1, 0);
		if(send_size <= 0)
		{
			write_info("Send http header failed !!! \n");
		}
		else
		{
			write_info("Send http header succeed ! \n");
		}
		
		return -1;
	}

	//如果请求的路径是一个文件，则将改文件直接发送给客户端
	else if(S_ISREG(stat_info.st_mode))
	{
		write_info("The %s is a file! \n", real_path);
		
		fd = open(real_path, O_RDONLY);		//只读的方式打开
		len = lseek(fd, 0, SEEK_END);		//查看打开文件的大小
		lseek(fd, 0, SEEK_SET);				//将文件读写定位到开始位置

		//开辟一个内存空间，用于存放读取到的文件信息
		char *tmp_mem = (char *)malloc(len+1);
		bzero(tmp_mem, len+1);

		for(int i=0; i<len; i+=1024)	//读取文件，每次读 1k bytes
		{
			rtn = read(fd, tmp_mem+i, 1024);
			if(rtn <= 0)
				write_info("Read the file [%s] failed!! [%d:%s] ", real_path, errno, strerror(errno));
		}

		close(fd);		//读完之后，就可以关闭文件了

		//将文件内容发送给客户端
		memset(send_msg, 0, sizeof(send_msg));
		sprintf(send_msg, "HTTP/1.1 200 OK\r\nServer: Http test server\r\nConnection keep alive\r\n"\
							"Content-type: application/*\r\nContent-length:%d\r\n\r\n", len);
		send_size = send(client_sock, send_msg, strlen(send_msg)+1, 0);
		if(send_size <= 0)
		{
			write_info("Send http header failed !!! \n");
		}
		else
		{
			write_info("Send http header succeed ! \n");
		}

		send(client_sock, tmp_mem, len, 0);
		if(send_size <= 0)
		{
			write_info("Send http message failed !!! \n");
		}
		else
		{
			write_info("Send http message succeed, len=%d ! \n", len);
		}

		sleep(20);
		free(tmp_mem);
	}
	
	//如果请求的路径是一个目录，将目录下的列表信息全部输出
	else if(S_ISDIR(stat_info.st_mode))
	{
		write_info("The %s is a directory! \n", real_path);
		
		//将http协议信息发送给客户端，为了便于显示，先发送表格头
		memset(send_msg, 0, sizeof(send_msg));
		sprintf(send_msg, "HTTP/1.1 200 OK\r\nServer: Http test server\r\nConnection: close\r\n\r\n"	\
							"<html><head><title>Jimmy_Nie: %s</title></head>"									\
							"<body><font size=+4>Linux http server file - Jimmy</font><br><hr width=\"100%%\"><br><center>"	\
							"<table border cols=3 width=\"100%%\">"							\
							"<caption><font size=+3> Directory: %s</font></caption>\n"		\
							"<tr><td>Name</td><td>Type</td><td>Owner</td><td>Group</td>"	\
							"<td>Size</td><td>Modify time</td></tr>\n",						\
							real_path, real_path);

		send_size = send(client_sock, send_msg, strlen(send_msg)+1, 0);
		if(send_size <= 0)
		{
			write_info("Send http header failed !!! \n");
		}
		else
		{
			write_info("Send http header succeed ! \n");
		}

		dir = opendir(real_path);	//打开绝对路径的目录
		if(NULL == dir)		//若打开目录失败
		{
			write_info("Open the %s failed! \n", real_path);
			memset(send_msg, 0, sizeof(send_msg));
			sprintf(send_msg, "</table><font color=\"CC0000\" size=+2>%s</font></body></html>", strerror(errno));

			send_size = send(client_sock, send_msg, strlen(send_msg)+1, 0);
			if(send_size <= 0)
			{
				write_info("Send http header failed !!! \n");
			}
			else
			{
				write_info("Send http header succeed ! \n");
			}
			
			return -1;
		}

		while((dirt = readdir(dir)) != NULL)
		{
			if(dirt->d_name[0] == '.')	//即为隐藏文件
				continue;
			
			send(client_sock, "<tr>", strlen("<tr>"), 0);
			bzero(file_name, sizeof(file_name));
			sprintf(file_name, "%s/%s", real_path, dirt->d_name);

			write_info("opened the file %s !\n", file_name);
			
			if(stat(file_name, &stat_info) == 0)	//读取文件信息
			{
				memset(send_msg, 0, sizeof(send_msg));
				if(0 == strcmp(dirt->d_name, ".."))		//若为..，则要求到达上级目录
					sprintf(send_msg, "<td><a href=\"http://%s:%s %s\">(parent)</a></td>",
										ip_addr, port_no, dir_up(path));
				else
					sprintf(send_msg, "<td><a href=\"http://%s:%s %s/%s\">%s</a></td>",
										ip_addr, port_no, path, dirt->d_name, dirt->d_name);

				send_size = send(client_sock, send_msg, strlen(send_msg), 0);
				if(send_size <= 0)
				{
					write_info("Send http header failed !!! \n");
				}
				else
				{
					write_info("Send http header succeed ! \n");
				}

				p_time = ctime(&stat_info.st_mtime);		//获取修改时间
				p_user = getpwuid(stat_info.st_uid);		//获取文件拥有着
				p_group = getgrgid(stat_info.st_gid);		//获取文件组信息

				write_info("Time: %s, user: %s, group: %s\n", p_time, p_user->pw_name, p_group->gr_name);
				
				//向客户端输出文件类型
				memset(send_msg, 0, sizeof(send_msg));
				sprintf(send_msg, "<td>%c</td>", file_type(stat_info.st_mode));
				send_size = send(client_sock, send_msg, strlen(send_msg), 0);
				if(send_size <= 0)
				{
					write_info("Send file type failed !!! \n");
				}
				else
				{
					write_info("Send file type succeed ! \n");
				}

				//向客户端输出文件所有者信息
				memset(send_msg, 0, sizeof(send_msg));
				sprintf(send_msg, "<td>%s</td>", p_user->pw_name);
				send_size = send(client_sock, send_msg, strlen(send_msg), 0);
				if(send_size <= 0)
				{
					write_info("Send file user failed !!! \n");
				}
				else
				{
					write_info("Send file user succeed ! \n");
				}
				
				//向客户端输出文件组信息
				memset(send_msg, 0, sizeof(send_msg));
				sprintf(send_msg, "<td>%s</td>", p_group->gr_name);
				send_size = send(client_sock, send_msg, strlen(send_msg), 0);
				if(send_size <= 0)
				{
					write_info("Send file group failed !!! \n");
				}
				else
				{
					write_info("Send file group succeed ! \n");
				}
				
				//向客户端输出文件大小
				memset(send_msg, 0, sizeof(send_msg));
				sprintf(send_msg, "<td>%d</td>", stat_info.st_size);
				send_size = send(client_sock, send_msg, strlen(send_msg), 0);
				if(send_size <= 0)
				{
					write_info("Send file size failed !!! \n");
				}
				else
				{
					write_info("Send file size succeed ! \n");
				}
				
				//向客户端输出修改时间
				char tmp_send[512];
				memset(send_msg, 0, sizeof(send_msg));		//此处很奇怪，用send_message就发不出去
				memset(tmp_send, 0, sizeof(tmp_send));
				sprintf(tmp_send, "<td>%s</td>", p_time);
								
				send_size = send(client_sock, tmp_send, strlen(tmp_send), 0);
				if(send_size <= 0)
				{
					write_info("Send file modify time failed !!! \n");
				}
				else
				{
					write_info("Send file modify time succeed ! \n");
				}
			}
			
			send(client_sock, "</tr>\n", strlen("</tr>\n"), 0);
		}
		
		send(client_sock, "</table></center></body></html>", strlen("</table></center></body></html>"), 0);
	}

	else	//如果木有权限访问
	{
		memset(send_msg, 0, sizeof(send_msg));
		sprintf(send_msg, "HTTP/1.1 200 OK\r\nServer: Http test server\r\nConnection: close\r\n\r\n"	\
							"<html><head><title>%s</title></head>"									\
							"<body><font size=+4>Linux http server file</font><br><hr width=\"100%%\"><br><center>"	\
							"<table border cols=3 width=\"100%%\"></table>"							\
							"<font color=\"CC0000\" size=+2>[%s]Permission denied, please contact the admin"\
							"</font></body></html>",	path);

		send_size = send(client_sock, send_msg, strlen(send_msg)+1, 0);
		if(send_size <= 0)
		{
			write_info("Send http header failed !!! \n");
		}
		else
		{
			write_info("Send http header succeed ! \n");
		}
	}
	
	return ;
}

