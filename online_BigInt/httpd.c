#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<ctype.h>
#include<string.h>
#include<strings.h>
#include<sys/stat.h>
#include<pthread.h>
#include<sys/wait.h>
#include<stdlib.h>



/************函数声明**************/
int startup(uint16_t* port);
void* accept_request(void* arg);
int get_line(int sockfd, char* buf, int size);
void serve_file(int sockfd, const char* path);
void cat(int sockfd, FILE* filename);
void execute_cgi(int sockfd, const char* path, const char* method, const char* query_string);
void unimplememted(int sockfd);
void not_found(int sockfd);
void headers(int sockfd);
void bad_request(int sockfd);
void cannot_execute(int sockfd);
/*********************************/





//服务端启动函数,创建套接字,绑定端口,获取端口号,监听
// 参数为端口号的地址,出参，带回端口号
//返回值为创建的套接字操作句柄
int startup(u_short* port)
{
	int httpd = 0;
	struct sockaddr_in name;

	//创建套接字
	httpd = socket(PF_INET, SOCK_STREAM, 0);
	if(httpd == -1)
	{
		perror("socket");
		exit(1);
	}
	//填充sockaddr_in结构体
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(*port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY为本机任意ip地址

	//绑定地址信息
	if(bind(httpd, (struct sockaddr*)&name, sizeof(name)) < 0)
	{
		perror("bind");
		exit(1);
	}

	//获取套接字信息
	if(*port == 0)
	{	
		int namelen = sizeof(name);
		if(getsockname(httpd, (struct sockaddr*) &name, &namelen) == -1)
			perror("getsockname");
		*port = ntohs(name.sin_port);
	}
	if(listen(httpd, 5) < 0)
	{
		perror("listen");
		exit(1);
	}
	return(httpd);
}

//子线程入口函数,连接成功为其服务
//参数是当前连接的套接字操作句柄
void* accept_request(void* arg)
{
	int sockfd = *(int*)arg;
	char buf[1024]; //用于接收一行数据
	int numchars;  //接收get_line的返回值,表示接收多少字节
	//调用get_line接收请求首行
	numchars = get_line(sockfd, buf, sizeof(buf));

	size_t i = 0, index_buf = 0;
	char method[255]; //存放请求方法
	//以空格符为标志来提取请求方法
	while(!isspace(buf[index_buf]) && (i < sizeof(method) - 1))
	{
		method[i] = buf[index_buf];
		i++;
		index_buf++;
	}
	method[i] = '\0';

	//如果方法既不是'GET'也不是'POST',响应方法没找到页面
	if(strcasecmp(method, "GET") && strcasecmp(method, "POST"))
	{
		unimplememted(sockfd);
		return;
	}

	int cgi = 0; //标记是否调用CGI模块
	//如果是POST方法,CGI=1
	if(strcasecmp(method, "POST") == 0)
		cgi = 1;

	//从buf中提取统一资源定位字符串
	i = 0;
	char url[255];
	while(isspace(buf[index_buf]) && (index_buf < sizeof(buf)))
		index_buf++;
	while(!isspace(buf[index_buf]) && (i < sizeof(url) - 1) && index_buf < sizeof(buf))
	{
		url[i] = buf[index_buf];
		i++;
		index_buf++;
	}
	url[i] = '\0';

	char* query_string = url;
	if(strcasecmp(method, "GET") == 0)
	{
		while((*query_string != '?') && (*query_string != '\0'))
			query_string++;
		if(*query_string == '?')
		{
			cgi = 1;
			*query_string = '\0';
			query_string++;
		}
	}
	
	//组织index.html的路径
	char path[512];
	sprintf(path, "htdocs%s", url);
	if(path[strlen(path) - 1] == '/')
		strcat(path, "myindex.html");

	//如果路径不对,先把缓冲区数据接收完然后响应not_found页面
	struct stat st;
	if(stat(path, &st) == -1)
	{
		while((numchars > 0) && strcmp("\n", buf))
			numchars = get_line(sockfd, buf, sizeof(buf));
		not_found(sockfd);
	}
	else
	{
		if((st.st_mode & S_IFMT) == S_IFDIR)
			strcat(path, "/myindex.html");
		//如果文件有任意可执行权限,CGI=1
		if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
			cgi = 1;
		//直接响应页面或走CGI通道
		if(!cgi)
			serve_file(sockfd, path);
		else
			execute_cgi(sockfd, path, method, query_string);
	}
	close(sockfd);
}

int get_line(int sockfd, char* buf, int size)
{
	int len = 0;
	char ch = '\0';
	int flag = 0;
	while((len < size - 1) && (ch != '\n'))
	{
		//接收一个字节到ch
		flag = recv(sockfd, &ch, 1, 0);
		if(flag > 0)
		{
			if(ch == '\r')
			{
				//如果接收到'\r',为分隔符,探测接收'\n'
				flag = recv(sockfd, &ch, 1, MSG_PEEK);
				//如果成功,则真正把'\n'接收,最终ch都为'\n'
				if((flag > 0) && (ch == '\n'))
					recv(sockfd, &ch, 1, 0);
				else
					ch = '\n';
			}
			//将接收到的数据放到buf中
			buf[len] = ch;
			len++;
		}
		else
			ch = '\n';
	}
	buf[len] = '\0';
	return len;
}

void serve_file(int sockfd, const char* path)
{
	//清空接收缓冲区
	char buf[1024];
	int numchars = 1;
	buf[0] = 'A';
	buf[1] = '\0';
	while((numchars > 0) && strcmp("\n", buf))
		numchars = get_line(sockfd, buf, sizeof(buf));
	
	//打开文件,将文件内容发送出去
	FILE* resource = NULL;
	resource = fopen(path, "r");
	if(resource == NULL)
		not_found(sockfd);
	else
	{
		//发送头部
		headers(sockfd);
		//发送正文
		cat(sockfd, resource);
	}
	fclose(resource);
}

//从文件filename中读取数据发送到sockfd
void cat(int sockfd, FILE* filename)
{
	char buf[1024];

	fgets(buf, sizeof(buf), filename);
	while(!feof(filename))
	{
		send(sockfd, buf, strlen(buf), 0);
		fgets(buf, sizeof(buf), filename);
	}
}

//CGI通道
void execute_cgi(int sockfd, const char* path, const char* method, const char* query_string)
{
	char buf[1024];
	buf[0] = 'A';
	buf[1] = '\0';
	int numchars = 1;
	int content_length = -1; //存放解析出的正文长度
	//如果是"GET"方法,清空接收缓冲区
	if(strcasecmp(method, "GET") == 0)
	{
		while((numchars > 0) && strcmp("\n", buf))
			numchars = get_line(sockfd, buf, sizeof(buf));
	}
	//"POST"方法,解析头部,获取正文长度
	else
	{
		//解析正文长度并且读取弃置请求首部
		numchars = get_line(sockfd, buf, sizeof(buf));
		while((numchars > 0) && strcmp("\n", buf))
		{
			buf[15] = '\0';
			if(strcasecmp(buf, "Content-Length:") == 0)
				content_length = atoi(&buf[16]);
			numchars = get_line(sockfd, buf, sizeof(buf));
		}
		//如果没解析出正文长度,响应bad_request页面
		if(content_length == -1)
		{
			bad_request(sockfd);
			return;
		}
	}
	//发送响应首行
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	send(sockfd, buf, strlen(buf), 0);
	
	//创建读写管道,并且将其重定向到标准输入和标准输出
	//在后台处理程序中直接使用标准输入和标准输出向管道写数据，进而发送到浏览器
	int cgi_output[2];
	int cgi_input[2];
	if(pipe(cgi_output) < 0)
	{
		cannot_execute(sockfd);
		return;
	}
	if(pipe(cgi_input) < 0)
	{
		cannot_execute(sockfd);
		return;
	}

	int pid;
	if((pid = fork()) < 0)
	{
		cannot_execute(sockfd);
		return;
	}
	if(pid == 0)
	{
		
		//将读写管道分别重定位到标准输入和标准输出
		dup2(cgi_output[1], 1);
		dup2(cgi_input[0], 0);	
		close(cgi_output[0]);
		close(cgi_input[1]);

		char meth_env[255];
		char query_env[255];
		char length_env[255];
		sprintf(meth_env, "REQUEST_METHOD=%s", method);
		putenv(meth_env);
		if(strcasecmp(method, "GET") == 0)
		{
			sprintf(query_env, "QUERY_STRING=%s", query_string);
			putenv(query_env);
		}
		else
		{
			sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
			putenv(length_env);
		}
		execl(path, path, NULL);
		exit(0);
	}
	else
	{
		char ch;
		close(cgi_output[1]);
		close(cgi_input[0]);
		if(strcasecmp(method, "POST") == 0);
		{
			//将正文数据接收并发送给后台处理程序
			int i = 0;
			for(i = 0; i < content_length; i++)
			{
				recv(sockfd, &ch, 1, 0);
				write(cgi_input[1], &ch, 1);
			}
		}
		//将后台发送过来的数据发送到前端
		while(read(cgi_output[0], &ch, 1) > 0)
			send(sockfd, &ch, 1, 0);
		int status;
		close(cgi_output[0]);
		close(cgi_input[1]);
		waitpid(pid, &status, 0);
	}
}

/***********************************************************/
/*               响应头部或页面的组织                      */
/***********************************************************/

//方法不对时的响应页面
void unimplememted(int sockfd)
{
	char buf[1024];
	sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
	send(sockfd, buf, strlen(buf), 0);

	sprintf(buf, "Server: httpd/0.0.1\r\n");
	send(sockfd, buf, strlen(buf), 0);

	sprintf(buf, "Content-Type: text/html\r\n");
	send(sockfd, buf, strlen(buf), 0);

	sprintf(buf, "\r\n");
	send(sockfd, buf, strlen(buf), 0);
	//头部结束,正文开始
	
	sprintf(buf, "<html><head><title>Method Not Implemented\r\n");
	send(sockfd, buf, strlen(buf), 0);
	
	sprintf(buf, "</title></head>\r\n");
	send(sockfd, buf, strlen(buf), 0);
	
	sprintf(buf, "<body><p>HTTP request method not supported.\r\n");
	send(sockfd, buf, strlen(buf), 0);
	
	sprintf(buf, "</p></body></html>\r\n");
	send(sockfd, buf, strlen(buf), 0);
}

//文件没找到时的响应页面
void not_found(int sockfd)
{
	char buf[1024];
	
	sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
	send(sockfd, buf, strlen(buf), 0);
	
	sprintf(buf, "Server : httpd/0.0.1\r\n");
	send(sockfd, buf, strlen(buf), 0);
	
	sprintf(buf, "Content-Type: text/html\r\n");
	send(sockfd, buf, strlen(buf), 0);
	
	sprintf(buf, "\r\n");
	send(sockfd, buf, strlen(buf), 0);
	//头部结束,正文开始
	sprintf(buf, "<html><title>Not Found</title>\r\n");
	send(sockfd, buf, strlen(buf), 0);
	
	sprintf(buf, "<body><p>The server could not fulfill,\r\n");
	send(sockfd, buf, strlen(buf), 0);
	
	sprintf(buf, "your request because the resource specified\r\n");
	send(sockfd, buf, strlen(buf), 0);
	
	sprintf(buf, "is unavailable or nonexistent.\r\n");
	send(sockfd, buf, strlen(buf), 0);
	
	sprintf(buf, "</p></body></html>\r\n");
	send(sockfd, buf, strlen(buf), 0);
}

//通用正常处理响应头部
void headers(int sockfd)
{
	char buf[1024];
	
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	send(sockfd, buf, strlen(buf), 0);
	
	sprintf(buf, "Serve: httpd/0.0.1\r\n");
	send(sockfd, buf, strlen(buf), 0);
	
	sprintf(buf, "Content-Type: text/html\r\n");
	send(sockfd, buf, strlen(buf), 0);
	
	sprintf(buf, "\r\n");
	send(sockfd, buf, strlen(buf), 0);
}


//不正确的请求响应页面
void bad_request(int sockfd)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
	send(sockfd, buf, sizeof(buf), 0);

	sprintf(buf, "Content-Type: text/html\r\n");
	send(sockfd, buf, sizeof(buf), 0);

	sprintf(buf, "\r\n");
	send(sockfd, buf, sizeof(buf), 0);

	sprintf(buf, "<p>Your browser sent a bad request,\r\n");
	send(sockfd, buf, sizeof(buf), 0);

	sprintf(buf, "such as a POST without a Content-Length.</p>\r\n");
	send(sockfd, buf, sizeof(buf), 0);
}

//服务端处理错误响应页面
void cannot_execute(int sockfd)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
	send(sockfd, buf, strlen(buf), 0);

	sprintf(buf, "Content-Type: text/html\r\n");
	send(sockfd, buf, strlen(buf), 0);

	sprintf(buf, "\r\n");
	send(sockfd, buf, strlen(buf), 0);

	sprintf(buf, "<p>Error prohibited CGI execution.</p>\r\n");
	send(sockfd, buf, strlen(buf), 0);
}

int main(int argc, char* argv[])
{
	int server_sock = -1;
	u_short port = 0;
	int client_sock = -1;
	struct sockaddr_in client_name;
	int client_name_len = sizeof(client_name);
	pthread_t newthread;

	//调用startup函数启动服务端tcp进行侦听
	server_sock = startup(&port);
	
	printf("httpd running on port %d\n", port);

	while(1)
	{
		//接收连接
		client_sock = accept(server_sock, (struct sockaddr*)&client_name, &client_name_len);
		if(client_sock == -1)
		{
			perror("accept");
			exit(1);
		}
		//若连接成功,创建子线程为它服务
		pthread_t newthread;
		if(pthread_create(&newthread, NULL, accept_request, &client_sock) != 0)
			perror("pthread_create");
	}
	close(server_sock);
	return 0;
}










