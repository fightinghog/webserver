//webserver2.0
#include "webserver.h"
#include "config.h"
int time_sum=0;
struct {
char *ext; 
char *filetype;
}extensions [] = {
{"gif", "image/gif" }, 
{"jpg", "image/jpg" }, 
{"jpeg","image/jpeg"}, 
{"png", "image/png" }, 
{"ico", "image/ico" }, 
{"zip", "image/zip" }, 
{"gz", "image/gz" }, 
{"tar", "image/tar" }, 
{"htm", "text/html" }, 
{"html","text/html" }, 
{0,0} };

/* 日志函数，将运行过程中的提示信息记录到 webserver.log 文件中*/ 
void logger(int type, char *s1,char *s2, int socket_fd)
{
int fd ;
int temp[100],tem;
char logbuffer[BUFSIZE*2];
/*根据消息类型，将消息放入 logbuffer 缓存，或直接将消息通过 socket 通道返回给客户端*/ 
time_t ttt;  
time(&ttt);    
printf("%s\n",ctime(&ttt));
printf("%s %s ",s1,s2);
switch (type) {
  case ERROR: (void)sprintf(logbuffer," ERROR:%s:%s Errno=%d exiting pid=%d",s1, s2, errno,getpid());
  break;
case FORBIDDEN:
  (void)write(socket_fd, "HTTP/1.1 403 Forbidden\nContent-Length: 185\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\n The requested URL, file type or operation is not allowed on this simple static file webserver.\n</body></html>\n",271);
  (void)sprintf(logbuffer," FORBIDDEN: %s:%s",s1, s2); 
  break;
case NOTFOUND:
 (void)write(socket_fd, "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n",224);
 (void)sprintf(logbuffer," NOT FOUND: %s:%s",s1, s2);
 break;
case LOG: (void)sprintf(logbuffer,"TIME:%s INFO: %s:%s:%d",ctime(&ttt),s1, s2,socket_fd);
 break; 
 }
/* 将 logbuffer 缓存中的消息存入 webserver.log 文件*/
if((fd = open("webserver.log", O_CREAT| O_WRONLY | O_APPEND,0644)) >= 0) {
(void)write(fd,logbuffer,strlen(logbuffer));
(void)write(fd,"\n",1); 
(void)close(fd);
 }

  
}
 /* 此函数完成了 WebServer 主要功能，它首先解析客户端发送的消息，然后从中获取客户端请求的文件名，然后根据文件名从本地将此文件读入缓存，并生成相应的 HTTP 响应消息；最后通过服务器与客户 端的 socket 通道向客户端返回 HTTP 响应消息*/
 
void web(int fd, int hit) {
int j, file_fd, buflen; long i, ret, len; char * fstr;
static char buffer[BUFSIZE+1]; /* 设置静态缓冲区 */
ret = read(fd,buffer,BUFSIZE); /* 从连接通道中读取客户端的请求消息 */ 
if(ret == 0 || ret == -1) { //如果读取客户端消息失败，则向客户端发送 HTTP 失败响应信息   0已经读到文件尾
 logger(FORBIDDEN,"failed to read browser request","",fd);
 exit(-1);
}


if(ret > 0 && ret < BUFSIZE) /* 设置有效字符串，即将字符串尾部表示为 0 */
buffer[ret]=0; 
else buffer[0]=0;
for(i=0;i<ret;i++)         /* 移除消息字符串中的“CF”和“LF”字符*/
if(buffer[i] == '\r' || buffer[i] == '\n')
buffer[i]='*';
//logger(LOG,"request",buffer,hit);
/*判断客户端 HTTP 请求消息是否为 GET 类型，如果不是则给出相应的响应消息*/
if( strncmp(buffer,"GET ",4) && strncmp(buffer,"get ",4) ) { 
logger(FORBIDDEN,"Only simple GET operation supported",buffer,fd); 
}
for(i=4;i<BUFSIZE;i++) { /* null terminate after the second space to ignore extra stuff */
if(buffer[i] == ' ') { /* string is "GET URL " +lots of other stuff */
buffer[i] = 0; break;
 }
}

for(j=0;j<i-1;j++) /* 在消息中检测路径，不允许路径中出现“.” */
if(buffer[j] == '.' && buffer[j+1] == '.') {
 logger(FORBIDDEN,"Parent directory (..) path names not supported",buffer,fd); }
if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ) 
 (void)strcpy(buffer,"GET /index.html");
/* 根据预定义在 extensions 中的文件类型，检查请求的文件类型是否本服务器支持 */ 
buflen=strlen(buffer);
fstr = (char *)0;
for(i=0;extensions[i].ext != 0;i++) {
len = strlen(extensions[i].ext);
if( !strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
fstr =extensions[i].filetype; 
break;
 }
}

  
if(fstr == 0) logger(FORBIDDEN,"file extension type not supported",buffer,clock());
if(( file_fd = open(&buffer[5],O_RDONLY)) == -1) { /* 打开指定的文件名*/
logger(NOTFOUND, "failed to open file",&buffer[5],fd);
exit(-1);
}
//logger(LOG,"SEND",&buffer[5],hit);



len = (long)lseek(file_fd, (off_t)0, SEEK_END); /* 通过 lseek 获取文件长度*/  
(void)lseek(file_fd, (off_t)0, SEEK_SET); /* 将文件指针移到文件首位置*/  
(void)sprintf(buffer,"HTTP/1.1 200 OK\nServer: nweb/%d.0\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", VERSION, len, fstr); /* Header + a blank line */  
//logger(LOG,"Header",buffer,hit); 


(void)write(fd,buffer,strlen(buffer));
/* 不停地从文件里读取文件内容，并通过 socket 通道向客户端返回文件内容*/
 while ( (ret = read(file_fd, buffer, BUFSIZE)) > 0 ) {
(void)write(fd,buffer,ret); 
}
}


int main() {
double time_t[2];
int i, port, listenfd, socketfd, hit; socklen_t length;
static struct sockaddr_in cli_addr;  /* static = initialised to zeros */ 
static struct sockaddr_in serv_addr; /* static = initialised to zeros */



/*解析命令参数*/
/*if( argc < 3 || argc > 3 || !strcmp(argv[1], "-?") ) {*/
/* (void)printf("hint: nweb Port-Number Top-Directory\t\tversion %d\n\n" "\tnweb is a small and very safe mini web server\n" "\tnweb only servers out file/web pages with extensions named below\n" "\t and only from the named directory or its sub-directories.\n" "\tThere is no fancy features = safe and secure.\n\n" "\tExample:webserver 8181 /home/nwebdir &\n\n" "\tOnly Supports:", VERSION); */
/*for(i=0;extensions[i].ext != 0;i++)  */
/* (void)printf(" %s",extensions[i].ext);*/
/* (void)printf("\n\tNot Supported: URLs including \"..\", Java, Javascript, CGI\n" "\tNot Supported: directories / /etc /bin /lib /tmp /usr /dev /sbin \n" "\tNo warranty given or implied\n\tNigel Griffiths nag@uk.ibm.com\n" );*/
/* printf("test1\n");*/
/* exit(0); */
/* printf("test2\n");*/
/*}*/

/*if( !strncmp(argv[2],"/" ,2 ) || !strncmp(argv[2],"/etc", 5 ) || !strncmp(argv[2],"/bin",5 ) || !strncmp(argv[2],"/lib", 5 ) || !strncmp(argv[2],"/tmp",5 ) || !strncmp(argv[2],"/usr", 5 ) || !strncmp(argv[2],"/dev",5 ) || !strncmp(argv[2],"/sbin",6) ){*/
/*(void)printf("ERROR: Bad top directory %s, see nweb -?\n",argv[2]); */
/*exit(3);*/
/*}*/
/*if(chdir(argv[2]) == -1){　//change the workspace*/
/*(void)printf("ERROR: Can't Change to directory %s\n",argv[2]);*/
/* exit(4);*/
/* } */

//change the myconf
char lis_workspace[30];
char time_log_file[30];
int lis_port;
int log_size;


if(getconfigint("webserver","logsize",&log_size,myini)!=0){
perror("config has error,not found the log_size!\n");
printf("%d",log_size);
exit(-1);
};

if(getconfigint("webserver","port",&lis_port,myini)!=0){
perror("config has error,not found the port!\n");
exit(-1);
};
if(getconfigstr("webserver","addr",lis_workspace,30,myini)!=0){
perror("config has error,not found the wrok_addr!\n");
exit(-1);
};
if(getconfigstr("webserver","logs",time_log_file,30,myini)!=0){
perror("config has error,not found the time_log_addr!\n");
exit(-1);
};
lis_workspace[strlen(lis_workspace)]=0;
if( !strncmp(lis_workspace,"/" ,2 ) || !strncmp(lis_workspace,"/etc", 5 ) || !strncmp(lis_workspace,"/bin",5 ) || !strncmp(lis_workspace,"/lib", 5 ) || !strncmp(lis_workspace,"/tmp",5 ) || !strncmp(lis_workspace,"/usr", 5 ) || !strncmp(lis_workspace,"/dev",5 ) || !strncmp(lis_workspace,"/sbin",6) ){
(void)printf("ERROR: Bad top directory %s, see nweb -?\n",lis_workspace); 
exit(3);
}


if(chdir(lis_workspace) == -1){//change the workspace
(void)printf("ERROR: Can't Change to directory %s\n",lis_workspace);
 exit(4);
} 

//shm and psem 
sem_t* psem;
if((psem=sem_open("signal",O_CREAT,0666,1))==SEM_FAILED){
perror("create signal error");
exit(-1);
}
int shm_fd;
int shr_fp_time;
if((shm_fd=shm_open("memory",O_RDWR|O_CREAT,0666))<0){
perror("create shared memory object error!");
exit(-1);
}


if((shr_fp_time=open(time_log_file,O_RDWR|O_CREAT ,0666))<0){
perror("Open the file shm_time failed");
exit(-1);
}


ftruncate(shr_fp_time,log_size);
ftruncate(shm_fd,sizeof(struct child_time));
void* vshmptr=mmap(NULL,sizeof(struct child_time),PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
if(vshmptr==MAP_FAILED){
perror("create mmap error");
exit(-1);
}
char* vshm_fp=mmap(NULL,log_size,PROT_READ|PROT_WRITE,MAP_SHARED,shr_fp_time,0);
if(vshm_fp==MAP_FAILED){
perror("create vshm_fp error");
exit(-1);
}
struct child_time ti;
ti.tnow=0;
ti.tsum=0;
ti.pid_sum=0;
memcpy(vshmptr,&ti,sizeof(struct child_time));
 
 
 
 pid_t pid;
 if((pid=fork())!=0){
 waitpid(pid,NULL,0);
 
    //
    //unloads the res
    //
    
    sleep(1);
    printf("the number of pid is %d\n",(*(struct child_time*)vshmptr).pid_sum);
    printf("and the total response time is %.4fms\n",(*(struct child_time*)vshmptr).tsum);
    printf("Start To Release The Resources...\n"); 
    
    printf("1.child_thread exit succeed\n");
    if(munmap(vshmptr,sizeof(struct child_time))==-1){
    perror("unmap failed");
    exit(-1);
    }
    printf("2.unmap vshm_fp succeed\n");
    if(munmap(vshm_fp,log_size)==-1){
    perror("unmap failed");
    exit(-1);
    }
    printf("3.unmap vshm succeed\n");
    if(close(shr_fp_time)==-1){
    perror("close shr_fd failed");
    exit(-1);
    }
    printf("4.close shm_fd succeed\n");
    if(close(shm_fd)==-1){
    perror("close shm_fd failed");
    exit(-1);
    }
    printf("5.close shm_fd succeed\n");
    if(sem_close(psem)==-1){
    perror("close psem error");
    exit(-1);
    }
    printf("6.close psem succeed\n");
    if(sem_unlink("signal")==-1){
    perror("sem_unlink error");
    exit(-1);
    }
    printf("7.sem_unlink succeed\n");
    if(shm_unlink("memory")==-1){ 
    perror("shm_unlink error");
    exit(-1);
    }
    printf("8.shm_unlink succeed\n");
 printf("okok\n");
 return 0;
 }
(void)signal(SIGCLD,SIG_IGN);
(void)signal(SIGHUP,SIG_IGN);
port = lis_port;
fprintf(stdout,"let's go and gogoing! chd-pid is %d\n",getpid()); 
time_t[0]=clock();
// 建立服务端侦听 socket*/
if((listenfd = socket(AF_INET, SOCK_STREAM,0)) <0) { // ipv4    tcp-ip    建立一个描述符{
 logger(ERROR, "system call","socket",0); 
 return -1;
 } 

if(port < 0 || port >60000){
 logger(ERROR,"Invalid port number (try 1->60000)"," ",0); 
 return -1;
 }
serv_addr.sin_family = AF_INET;  //地址家族
serv_addr.sin_port = htons(port);   //8位存储  端口
serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //inaddr_any 所有网卡监听
if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0)  //绑定的地址类型要和套接字的设定方式一致
{
 logger(ERROR,"system call","bind",0);
 return -1;
}
if( listen(listenfd,64) <0){
logger(ERROR,"system call","listen",0); 
return -1;
}
length = sizeof(cli_addr); //客户端地址的长度
time_t[1]=clock();
timelog(0,"pre-do is done",time_t[0],time_t[1]);

for(hit=1; ;hit++) {
 if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0) {//阻塞
  logger(ERROR,"system call","accept",0);
  sleep(5);
 }
//char ip_port[30];
//int len_ip;
//sprintf(ip_port,"client->ip:%s:%d",inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port));
//len_ip=strlen(ip_port);
//ip_port[len_ip]=0;
//timelog(hit,ip_port,1,1);
 pid_t pid=fork();
 
 
 if(pid==0){
    close(listenfd);
    clock_t chd_t1=clock();
    web(socketfd,hit); /* never returns  连次数hit*/
    clock_t chd_t2=clock();
    double chd_t3= ((double)(chd_t2-chd_t1))/CLOCKS_PER_SEC*1000;
    sem_wait(psem);
    (*(struct child_time *)vshmptr).tsum+=chd_t3;
    (*(struct child_time *)vshmptr).pid_sum++;
    sem_post(psem);
    sprintf(vshm_fp+(hit-1)*23,"pid:%5d time:%.3fms\n",getpid(),chd_t3);
    sleep(1);
    close(socketfd);
    exit(0);
  }
  else{
    close(socketfd);
  }
 }
}
//2971//webserver2.0
#include "webserver.h"
#include "config.h"
int time_sum=0;
struct {
char *ext; 
char *filetype;
}extensions [] = {
{"gif", "image/gif" }, 
{"jpg", "image/jpg" }, 
{"jpeg","image/jpeg"}, 
{"png", "image/png" }, 
{"ico", "image/ico" }, 
{"zip", "image/zip" }, 
{"gz", "image/gz" }, 
{"tar", "image/tar" }, 
{"htm", "text/html" }, 
{"html","text/html" }, 
{0,0} };

/* 日志函数，将运行过程中的提示信息记录到 webserver.log 文件中*/ 
void logger(int type, char *s1,char *s2, int socket_fd)
{
int fd ;
int temp[100],tem;
char logbuffer[BUFSIZE*2];
/*根据消息类型，将消息放入 logbuffer 缓存，或直接将消息通过 socket 通道返回给客户端*/ 
time_t ttt;  
time(&ttt);    
printf("%s\n",ctime(&ttt));
printf("%s %s ",s1,s2);
switch (type) {
  case ERROR: (void)sprintf(logbuffer," ERROR:%s:%s Errno=%d exiting pid=%d",s1, s2, errno,getpid());
  break;
case FORBIDDEN:
  (void)write(socket_fd, "HTTP/1.1 403 Forbidden\nContent-Length: 185\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\n The requested URL, file type or operation is not allowed on this simple static file webserver.\n</body></html>\n",271);
  (void)sprintf(logbuffer," FORBIDDEN: %s:%s",s1, s2); 
  break;
case NOTFOUND:
 (void)write(socket_fd, "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n",224);
 (void)sprintf(logbuffer," NOT FOUND: %s:%s",s1, s2);
 break;
case LOG: (void)sprintf(logbuffer,"TIME:%s INFO: %s:%s:%d",ctime(&ttt),s1, s2,socket_fd);
 break; 
 }
/* 将 logbuffer 缓存中的消息存入 webserver.log 文件*/
if((fd = open("webserver.log", O_CREAT| O_WRONLY | O_APPEND,0644)) >= 0) {
(void)write(fd,logbuffer,strlen(logbuffer));
(void)write(fd,"\n",1); 
(void)close(fd);
 }

  
}
 /* 此函数完成了 WebServer 主要功能，它首先解析客户端发送的消息，然后从中获取客户端请求的文件名，然后根据文件名从本地将此文件读入缓存，并生成相应的 HTTP 响应消息；最后通过服务器与客户 端的 socket 通道向客户端返回 HTTP 响应消息*/
 
void web(int fd, int hit) {
int j, file_fd, buflen; long i, ret, len; char * fstr;
static char buffer[BUFSIZE+1]; /* 设置静态缓冲区 */
ret = read(fd,buffer,BUFSIZE); /* 从连接通道中读取客户端的请求消息 */ 
if(ret == 0 || ret == -1) { //如果读取客户端消息失败，则向客户端发送 HTTP 失败响应信息   0已经读到文件尾
 logger(FORBIDDEN,"failed to read browser request","",fd);
 exit(-1);
}


if(ret > 0 && ret < BUFSIZE) /* 设置有效字符串，即将字符串尾部表示为 0 */
buffer[ret]=0; 
else buffer[0]=0;
for(i=0;i<ret;i++)         /* 移除消息字符串中的“CF”和“LF”字符*/
if(buffer[i] == '\r' || buffer[i] == '\n')
buffer[i]='*';
//logger(LOG,"request",buffer,hit);
/*判断客户端 HTTP 请求消息是否为 GET 类型，如果不是则给出相应的响应消息*/
if( strncmp(buffer,"GET ",4) && strncmp(buffer,"get ",4) ) { 
logger(FORBIDDEN,"Only simple GET operation supported",buffer,fd); 
}
for(i=4;i<BUFSIZE;i++) { /* null terminate after the second space to ignore extra stuff */
if(buffer[i] == ' ') { /* string is "GET URL " +lots of other stuff */
buffer[i] = 0; break;
 }
}

for(j=0;j<i-1;j++) /* 在消息中检测路径，不允许路径中出现“.” */
if(buffer[j] == '.' && buffer[j+1] == '.') {
 logger(FORBIDDEN,"Parent directory (..) path names not supported",buffer,fd); }
if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ) 
 (void)strcpy(buffer,"GET /index.html");
/* 根据预定义在 extensions 中的文件类型，检查请求的文件类型是否本服务器支持 */ 
buflen=strlen(buffer);
fstr = (char *)0;
for(i=0;extensions[i].ext != 0;i++) {
len = strlen(extensions[i].ext);
if( !strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
fstr =extensions[i].filetype; 
break;
 }
}

  
if(fstr == 0) logger(FORBIDDEN,"file extension type not supported",buffer,clock());
if(( file_fd = open(&buffer[5],O_RDONLY)) == -1) { /* 打开指定的文件名*/
logger(NOTFOUND, "failed to open file",&buffer[5],fd);
exit(-1);
}
//logger(LOG,"SEND",&buffer[5],hit);



len = (long)lseek(file_fd, (off_t)0, SEEK_END); /* 通过 lseek 获取文件长度*/  
(void)lseek(file_fd, (off_t)0, SEEK_SET); /* 将文件指针移到文件首位置*/  
(void)sprintf(buffer,"HTTP/1.1 200 OK\nServer: nweb/%d.0\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", VERSION, len, fstr); /* Header + a blank line */  
//logger(LOG,"Header",buffer,hit); 


(void)write(fd,buffer,strlen(buffer));
/* 不停地从文件里读取文件内容，并通过 socket 通道向客户端返回文件内容*/
 while ( (ret = read(file_fd, buffer, BUFSIZE)) > 0 ) {
(void)write(fd,buffer,ret); 
}
}


int main() {
double time_t[2];
int i, port, listenfd, socketfd, hit; socklen_t length;
static struct sockaddr_in cli_addr;  /* static = initialised to zeros */ 
static struct sockaddr_in serv_addr; /* static = initialised to zeros */



/*解析命令参数*/
/*if( argc < 3 || argc > 3 || !strcmp(argv[1], "-?") ) {*/
/* (void)printf("hint: nweb Port-Number Top-Directory\t\tversion %d\n\n" "\tnweb is a small and very safe mini web server\n" "\tnweb only servers out file/web pages with extensions named below\n" "\t and only from the named directory or its sub-directories.\n" "\tThere is no fancy features = safe and secure.\n\n" "\tExample:webserver 8181 /home/nwebdir &\n\n" "\tOnly Supports:", VERSION); */
/*for(i=0;extensions[i].ext != 0;i++)  */
/* (void)printf(" %s",extensions[i].ext);*/
/* (void)printf("\n\tNot Supported: URLs including \"..\", Java, Javascript, CGI\n" "\tNot Supported: directories / /etc /bin /lib /tmp /usr /dev /sbin \n" "\tNo warranty given or implied\n\tNigel Griffiths nag@uk.ibm.com\n" );*/
/* printf("test1\n");*/
/* exit(0); */
/* printf("test2\n");*/
/*}*/

/*if( !strncmp(argv[2],"/" ,2 ) || !strncmp(argv[2],"/etc", 5 ) || !strncmp(argv[2],"/bin",5 ) || !strncmp(argv[2],"/lib", 5 ) || !strncmp(argv[2],"/tmp",5 ) || !strncmp(argv[2],"/usr", 5 ) || !strncmp(argv[2],"/dev",5 ) || !strncmp(argv[2],"/sbin",6) ){*/
/*(void)printf("ERROR: Bad top directory %s, see nweb -?\n",argv[2]); */
/*exit(3);*/
/*}*/
/*if(chdir(argv[2]) == -1){　//change the workspace*/
/*(void)printf("ERROR: Can't Change to directory %s\n",argv[2]);*/
/* exit(4);*/
/* } */

//change the myconf
char lis_workspace[30];
char time_log_file[30];
int lis_port;
int log_size;


if(getconfigint("webserver","logsize",&log_size,myini)!=0){
perror("config has error,not found the log_size!\n");
printf("%d",log_size);
exit(-1);
};

if(getconfigint("webserver","port",&lis_port,myini)!=0){
perror("config has error,not found the port!\n");
exit(-1);
};
if(getconfigstr("webserver","addr",lis_workspace,30,myini)!=0){
perror("config has error,not found the wrok_addr!\n");
exit(-1);
};
if(getconfigstr("webserver","logs",time_log_file,30,myini)!=0){
perror("config has error,not found the time_log_addr!\n");
exit(-1);
};
lis_workspace[strlen(lis_workspace)]=0;
if( !strncmp(lis_workspace,"/" ,2 ) || !strncmp(lis_workspace,"/etc", 5 ) || !strncmp(lis_workspace,"/bin",5 ) || !strncmp(lis_workspace,"/lib", 5 ) || !strncmp(lis_workspace,"/tmp",5 ) || !strncmp(lis_workspace,"/usr", 5 ) || !strncmp(lis_workspace,"/dev",5 ) || !strncmp(lis_workspace,"/sbin",6) ){
(void)printf("ERROR: Bad top directory %s, see nweb -?\n",lis_workspace); 
exit(3);
}


if(chdir(lis_workspace) == -1){//change the workspace
(void)printf("ERROR: Can't Change to directory %s\n",lis_workspace);
 exit(4);
} 

//shm and psem 
sem_t* psem;
if((psem=sem_open("signal",O_CREAT,0666,1))==SEM_FAILED){
perror("create signal error");
exit(-1);
}
int shm_fd;
int shr_fp_time;
if((shm_fd=shm_open("memory",O_RDWR|O_CREAT,0666))<0){
perror("create shared memory object error!");
exit(-1);
}


if((shr_fp_time=open(time_log_file,O_RDWR|O_CREAT ,0666))<0){
perror("Open the file shm_time failed");
exit(-1);
}


ftruncate(shr_fp_time,log_size);
ftruncate(shm_fd,sizeof(struct child_time));
void* vshmptr=mmap(NULL,sizeof(struct child_time),PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
if(vshmptr==MAP_FAILED){
perror("create mmap error");
exit(-1);
}
char* vshm_fp=mmap(NULL,log_size,PROT_READ|PROT_WRITE,MAP_SHARED,shr_fp_time,0);
if(vshm_fp==MAP_FAILED){
perror("create vshm_fp error");
exit(-1);
}
struct child_time ti;
ti.tnow=0;
ti.tsum=0;
ti.pid_sum=0;
memcpy(vshmptr,&ti,sizeof(struct child_time));
 
 
 
 pid_t pid;
 if((pid=fork())!=0){
 waitpid(pid,NULL,0);
 
    //
    //unloads the res
    //
    
    sleep(1);
    printf("the number of pid is %d\n",(*(struct child_time*)vshmptr).pid_sum);
    printf("and the total response time is %.4fms\n",(*(struct child_time*)vshmptr).tsum);
    printf("Start To Release The Resources...\n"); 
    
    printf("1.child_thread exit succeed\n");
    if(munmap(vshmptr,sizeof(struct child_time))==-1){
    perror("unmap failed");
    exit(-1);
    }
    printf("2.unmap vshm_fp succeed\n");
    if(munmap(vshm_fp,log_size)==-1){
    perror("unmap failed");
    exit(-1);
    }
    printf("3.unmap vshm succeed\n");
    if(close(shr_fp_time)==-1){
    perror("close shr_fd failed");
    exit(-1);
    }
    printf("4.close shm_fd succeed\n");
    if(close(shm_fd)==-1){
    perror("close shm_fd failed");
    exit(-1);
    }
    printf("5.close shm_fd succeed\n");
    if(sem_close(psem)==-1){
    perror("close psem error");
    exit(-1);
    }
    printf("6.close psem succeed\n");
    if(sem_unlink("signal")==-1){
    perror("sem_unlink error");
    exit(-1);
    }
    printf("7.sem_unlink succeed\n");
    if(shm_unlink("memory")==-1){ 
    perror("shm_unlink error");
    exit(-1);
    }
    printf("8.shm_unlink succeed\n");
 printf("okok\n");
 return 0;
 }
(void)signal(SIGCLD,SIG_IGN);
(void)signal(SIGHUP,SIG_IGN);
port = lis_port;
fprintf(stdout,"let's go and gogoing! chd-pid is %d\n",getpid()); 
time_t[0]=clock();
// 建立服务端侦听 socket*/
if((listenfd = socket(AF_INET, SOCK_STREAM,0)) <0) { // ipv4    tcp-ip    建立一个描述符{
 logger(ERROR, "system call","socket",0); 
 return -1;
 } 

if(port < 0 || port >60000){
 logger(ERROR,"Invalid port number (try 1->60000)"," ",0); 
 return -1;
 }
serv_addr.sin_family = AF_INET;  //地址家族
serv_addr.sin_port = htons(port);   //8位存储  端口
serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //inaddr_any 所有网卡监听
if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0)  //绑定的地址类型要和套接字的设定方式一致
{
 logger(ERROR,"system call","bind",0);
 return -1;
}
if( listen(listenfd,64) <0){
logger(ERROR,"system call","listen",0); 
return -1;
}
length = sizeof(cli_addr); //客户端地址的长度
time_t[1]=clock();
timelog(0,"pre-do is done",time_t[0],time_t[1]);

for(hit=1; ;hit++) {
 if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0) {//阻塞
  logger(ERROR,"system call","accept",0);
  sleep(5);
 }
//char ip_port[30];
//int len_ip;
//sprintf(ip_port,"client->ip:%s:%d",inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port));
//len_ip=strlen(ip_port);
//ip_port[len_ip]=0;
//timelog(hit,ip_port,1,1);
 pid_t pid=fork();
 
 
 if(pid==0){
    close(listenfd);
    clock_t chd_t1=clock();
    web(socketfd,hit); /* never returns  连次数hit*/
    clock_t chd_t2=clock();
    double chd_t3= ((double)(chd_t2-chd_t1))/CLOCKS_PER_SEC*1000;
    sem_wait(psem);
    (*(struct child_time *)vshmptr).tsum+=chd_t3;
    (*(struct child_time *)vshmptr).pid_sum++;
    sem_post(psem);
    sprintf(vshm_fp+(hit-1)*23,"pid:%5d time:%.3fms\n",getpid(),chd_t3);
    sleep(1);
    close(socketfd);
    exit(0);
  }
  else{
    close(socketfd);
  }
 }
}
//2971
