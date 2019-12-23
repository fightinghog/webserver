#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <errno.h> 
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 

#define VERSION 23 
#define BUFSIZE 8096 
#define ERROR 42 
#define LOG 44 
#define FORBIDDEN 403 
#define NOTFOUND 404
#ifndef SIGCLD 
#  define SIGCLD SIGCHLD 
#endif
const char myini[30]="/home/jsfeng/os/myini/my.ini";
struct child_time{
double tnow;
double tsum;
int pid_sum;
};
typedef struct req_page
{
	int num;//page_num
	int time;//wait_time	
}req_page;

void timelog(int time_sum,char type[],double t,double tt){
  FILE   *fp_time;
  time_t now_time;  
  time(&now_time);    
  fp_time=fopen("time.log","a+");
  double t0=((double)(tt-t))/CLOCKS_PER_SEC*1000;
  fprintf(fp_time,":%d\n%s\t%.4lfms\n",time_sum,type,t0);
  fclose(fp_time);
}
