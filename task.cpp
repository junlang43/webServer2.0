#include "task.h"

void removefd( int epollfd, int fd ) {
    epoll_ctl( epollfd, EPOLL_CTL_DEL, fd, 0 );
    close( fd );
}

void reset_oneshot( int epoll_fd, int fd ) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl( epoll_fd, EPOLL_CTL_MOD, fd, &event );
}

void encode_str(char* to, int tosize, const char* from)
{
	int tolen;

	for (tolen = 0; *from != '\0' && tolen + 4 < tosize; ++from) {
		if (isalnum(*from) || strchr("/_.-~", *from) != (char*)0) {
			*to = *from;
			++to;
			++tolen;
		}
		else {
			sprintf(to, "%%%02x", (int)*from & 0xff);
			to += 3;
			tolen += 3;
		}
	}
	*to = '\0';
}

const char *get_file_type(const char *name)
{
    char* dot;

    // 自右向左查找‘.’字符, 如不存在返回NULL
    dot = (char*)strrchr(name, '.');   
    if (dot == NULL)
        return "text/plain; charset=utf-8";
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp( dot, ".wav" ) == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}

void Task::doit() {
    char buf[ buffer_size ] = {0};

    while( int r = recv( accp_fd, buf, 1024, 0 ) ) {
        if( !r ) {
            cout << " browser exit.\n";
            break;
        } else if( r < 0 ) {  // 如果接收出错则继续接收数据
            continue;
        }
		//http request
        int start = 0;
        char method[5], uri[100], version[10];
        sscanf( buf, "%s %s %s", method, uri, version );

        if( char *tmp = strstr( buf, "Range:" ) ) {
            tmp += 13;
            sscanf( tmp, "%d", &start );
        }

        if( !strcmp( method, "GET" ) ) {  // 为GET
            deal_get( uri, start );
        } else if( !strcmp( method, "POST" ) ) {  // 为POST
            deal_post( uri, buf );
        } else {
            const char *header = "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/plain;charset=utf-8\r\n\r\n";
            send( accp_fd, header, strlen(header), 0 );
        }
        break;  // 只要处理完就退出循环，避免浏览器一直处于pending状态
    }
}

void Task::deal_get( const string & uri, int start ) {
    string filename = uri.substr(1);
	if (uri == "/")
		filename = "./";

    struct stat st;
    int ret = stat(filename.c_str(), &st);
    if(ret == -1) { 
        send_file("html/404.html", "text/html", 0, 404, "Not Found");     
        return;
    }

	if (uri == "/"||uri == "/index_algo.html") {
	    send_file("index_algo.html", "text/html", start);
    }
	else { 
        if(S_ISDIR(st.st_mode))
            send_dir(filename, get_file_type(".html"));
        else if(S_ISREG(st.st_mode))
		    send_file(filename, get_file_type(filename.c_str()), start);
	}
}

void Task::deal_post( const string & uri, char *buf ) {
    string filename = uri.substr(1);
    if( uri.find( "adder" ) != string::npos ) {  //使用CGI服务器，进行加法运算
        char *tmp = buf;
        int len, a, b;
        char *l = strstr( tmp, "Content-Length:" );  // 获取请求报文主体大小
        sscanf( l, "Content-Length: %d", &len );
        len = strlen( tmp ) - len;
        tmp += len;
        sscanf( tmp, "a=%d&b=%d", &a, &b );
        sprintf(tmp, "%d+%d,%d", a, b, accp_fd);  // tmp存储发送到CGI服务器的参数

        // fork产生子进程，执行CGI服务器进行计算（webServer一眼只进行解析、发送数据，不进行相关计算）
        if( fork() == 0 ) {
            // dup2( accp_fd, STDOUT_FILENO );
            execl( filename.c_str(), tmp, NULL );
        }
        wait( NULL );  // 等待子进程结束
    } else {
        send_file( "html/404.html", "text/html", 0, 404, "Not Found" );
    }
}

void Task::send_dir(const string & filename, const char *type, const int num, const char *info) {
	char header[200];
	sprintf(header, "HTTP/1.1 %d %s\r\nServer: niliushall\r\nContent-Length: %d\r\nContent-Type: %s;charset:utf-8\r\n\r\n", num, info, -1, type);

	// send第二个参数只能是c类型字符串，不能使用string
	send(accp_fd, header, strlen(header), 0);

    int i, ret;

	// 拼一个html页面<table></table>
	char buf[4094] = { 0 };

	sprintf(buf, "<html><head><title>目录名: %s</title></head>", filename.c_str());
	sprintf(buf + strlen(buf), "<body><h1>当前目录: %s</h1><table>", filename.c_str());

	char enstr[1024] = { 0 };
	char path[1024] = { 0 };

	// 目录项二级指针
	struct dirent** ptr;
	int dir_num = scandir(filename.c_str(), &ptr, NULL, alphasort);

	// 遍历
	for (i = 0; i < dir_num; ++i) {

		char* name = ptr[i]->d_name;

		// 拼接文件的完整路径
		sprintf(path, "%s/%s", filename.c_str(), name);
		printf("path = %s ===================\n", path);
		struct stat st;
		stat(path, &st);

		// 编码生成 %E5 %A7 之类的东西
		if(filename == "algorithmSource")
            encode_str(enstr, sizeof(enstr), path);
        else
            encode_str(enstr, sizeof(enstr), name);

		// 如果是文件
		if (S_ISREG(st.st_mode)) {
			sprintf(buf + strlen(buf),
				"<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
				enstr, name, (long)st.st_size);
		}
		else if (S_ISDIR(st.st_mode)) {		// 如果是目录       
			sprintf(buf + strlen(buf),
				"<tr><td><a href=\"%s/\">%s/</a></td><td>%ld</td></tr>",
				enstr, name, (long)st.st_size);
		}
		ret = send(accp_fd, buf, strlen(buf), 0);
		if (ret == -1) {
			if (errno == EAGAIN) {
				perror("send error:");
				continue;
			}
			else if (errno == EINTR) {
				perror("send error:");
				continue;
			}
			else {
				perror("send error:");
				exit(1);
			}
		}
		memset(buf, 0, sizeof(buf));
		// 字符串拼接
	}

	sprintf(buf + strlen(buf), "</table></body></html>");
	send(accp_fd, buf, strlen(buf), 0);

	printf("dir message send OK!!!!\n");
}

// type对应response的Content-Type，num为状态码，info为状态描述
int Task::send_file(const string & filename, const char *type, 
                            int start, const int num, const char *info) {
    struct stat filestat;
    int ret = stat( filename.c_str(), &filestat );
    if( ret < 0 || !S_ISREG( filestat.st_mode ) ) {  // 打开文件出错或没有该文件
        cout << "file not found : " << filename << endl;
        send_file( "html/404.html", "text/html", 0, 404, "Not Found" );
        return -1;
    }

    char header[200];
    sprintf( header, "HTTP/1.1 %d %s\r\nServer: niliushall\r\nContent-Length: %d\r\nContent-Type: %s;charset:utf-8\r\n\r\n", num, info, int(filestat.st_size - start), type );

    // send第二个参数只能是c类型字符串，不能使用string
    send( accp_fd, header, strlen(header), 0 );

    int fd = open( filename.c_str(), O_RDONLY );
    int sum = start;

    while( sum < filestat.st_size ) {
        off_t t = sum;

        int r = sendfile( accp_fd, fd, &t, filestat.st_size );

        if( r < 0 ) {
            printf("errno = %d, r = %d\n", errno, r);
            // perror("sendfile : ");
            if( errno == EAGAIN ) {
                printf("errno is EAGAIN\n");
                // reset_oneshot( epoll_fd, accp_fd );
                continue;
            } else {
                perror( "sendfile " );
                close( fd );
                break;
            }
        } else {
            sum += r;
        }
    }
    close( fd );
// printf( "sendfile finish, %d\n", accp_fd );
    return 0;
}

int Task::get_size( const string & filename ) {
    struct stat filestat;
    int ret = stat( filename.c_str(), &filestat );
    if( ret < 0 ) {
        cout << "file not found : " << filename << endl;
        return 0;
    }
    return filestat.st_size;
}
