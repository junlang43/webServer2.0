#include "task.h"
#define LF                  (u_char) '\n'
#define CR                  (u_char) '\r'
#define CRLF                "\r\n"

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
    if (strcmp(dot, ".zip") ==0 || strcmp(dot, ".rar") == 0)
        return "application/octet-stream";
    return "text/plain; charset=utf-8";
}
static void parse_request_header(char *data, request_header_t *header)
{
#define move_next_line(x)   while (*x && *(x + 1) && *x != CR && *(x + 1) != LF) x++;
#define next_header_line(x) while (*x && *(x + 1) && *x != CR && *(x + 1) != LF) x++; *x=0;
#define next_header_word(x) while (*x && *x != ' ' && *x != ':' && *(x + 1) && *x != CR && *(x + 1) != LF) x++; *x=0;

    char *p = data;
    char *q;
    int idx = 0;

    memset(header, 0, sizeof(request_header_t));
    // method
    next_header_word(p);
    header->method = data;
    // uri
    data = ++p;
    next_header_word(p);
    header->uri = data;
    // version
    data = ++p;
    next_header_word(p);
    header->version = data;
    // goto fields data
    next_header_line(p);
    data = ++p + 1;
    p++;
    // fields_count
    q = p;
    while (*p)
    {
        move_next_line(p);
        data = ++p + 1;
        p++;
        header->fields_count++;
        if (*data && *(data + 1) && *data == CR && *(data + 1) == LF)
            break;
    }
    // malloc fields
    header->fields = (request_fields_t*)malloc(sizeof(request_fields_t)*header->fields_count);
    // set fields
    data = p = q;
    while (*p)
    {
        next_header_word(p);
        header->fields[idx].key = data;
        data = ++p;
        if (*data == ' ')
        {
            data++;
            p++;
        }
        next_header_line(p);
        header->fields[idx++].value = data;
        data = ++p + 1;
        p++;
        if (*data && *(data + 1) && *data == CR && *(data + 1) == LF)
            break;
    }
    assert(idx == header->fields_count);
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
            request_header_t header;
            parse_request_header(buf, &header);
            //printf("%s\n",header.uri);
            //printf("%d\n",header.fields_count);
            //for(int i=0;i<header.fields_count;i++){
                //printf("%s:%s\n",header.fields[i].key,header.fields[i].value);
            //}
            deal_postfile(header, buf);
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
        else if(S_ISREG(st.st_mode)){
		    send_file(filename, get_file_type(filename.c_str()), start);
        }
	}
}

void Task::deal_postfile( const request_header_t header, char* buf) {
    string sfilename="/home/langjun/exercise/IOtest/mycp.c";
    char filepath[1024];
    memcpy(filepath, header.uri+strlen("/upload?path="),strlen(header.uri)-strlen("/upload?path="));
    char boundary[64];
    char *temp = NULL;
    int content_length=0;
    for (int i=0; i<header.fields_count; i++)
    {
        if (0 == strcmp(header.fields[i].key, "Content-Length"))
        {
            content_length = atoi(header.fields[i].value);
            break;
        }
    }
    if (content_length == 0){
        const char *header = "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/plain;charset=utf-8\r\n\r\n";
        send( accp_fd, header, strlen(header), 0 );
        return;
    }
    
    //get boundary
    for (int i=0; i<header.fields_count; i++)
    {
        if (0 == strcmp(header.fields[i].key, "Content-Type"))
        {
            temp = strstr(header.fields[i].value, "boundary=");
            if (temp)
            {
                temp += strlen("boundary=");
                memcpy(boundary, temp, strlen(temp));
            }
            break;
        }
    }

    FILE* pFile=fopen(sfilename.c_str(),"r");
    long  nFileLen=0;
    fseek(pFile, 0, SEEK_END);
    nFileLen=ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
    char* sFileText=new char[nFileLen+1];
    memset(sFileText, 0, nFileLen+1);
    fread(sFileText, 1, nFileLen, pFile);
    fclose(pFile);

    char body[96*1024] = {0};
    sprintf(body,"%s\r\n",boundary);
    sprintf(body+strlen(body),"Content-Disposition: form-data;filename=\"%s\"\r\n",sfilename.c_str());
    sprintf(body+strlen(body),"Content-Type: application/octet-stream\r\n\r\n");
    sprintf(body+strlen(body),"%s\r\n",sFileText);
    sprintf(body+strlen(body),"%s--\r\n",boundary);

    char Buf[128*1024]={0};
    sprintf(Buf,"%s%s%s\r\n",header.method, header.uri, header.version);
    for(int i=0;i<header.fields_count;i++){
        sprintf(Buf+strlen(Buf),"%s:%s\r\n",header.fields[i].key, header.fields[i].value);
    }
    sprintf(Buf+strlen(Buf), "%s", body);
    printf("%s\n", Buf);

    send(accp_fd,Buf,strlen(Buf),0);

    char res[1024];
    int ret=recv(accp_fd, res, 1024, 0);
    if(ret>0){
        printf("%s\n",res);
    }else{
        printf("failed\n");
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
    sprintf(buf + strlen(buf), "<body><form action=\"/upload?path=%s\" method=\"post\" enctype=\"multipart/form-data\">\r\n", filename.c_str());
    sprintf(buf + strlen(buf), "<input type=\"file\" name=\"file\" multiple=\"multiple\" />\r\n");
    sprintf(buf + strlen(buf), "<input type=\"submit\" value=\"Upload\" /></form><hr><pre>\r\n");
    sprintf(buf + strlen(buf), "<h1>当前目录: %s</h1><table>", filename.c_str());

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
