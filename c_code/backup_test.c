#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include <sys/poll.h>
#include <linux/user_event_log.h>
#include <linux/input.h>
#include <errno.h>

const char* dev_log = "/dev/event_log_dev";
unsigned char minors[MAX_INPUT_DEVICE];

static void print_devs(int fd,unsigned char minor){
  struct user_event_log log;
  struct user_args args;
  unsigned int i=0;
  args.minor = minor;
  args.p = &log;
  if(ioctl(fd,LGETDEVINFO,&args)<0){
    printf("Oh dear, something went wrong with LGETDEVINFO! %s\n", strerror(errno));
    exit(0);
  }
  for(i=0;i<log.ncount;i++){
    char *pname;
    if(i==0){
      pname = log.name;
    }else{
      pname="";
    }
    char time1[20],time2[20];
    snprintf(time1,sizeof(time1),"%ld.%06ld",log.dev_opened_time.tv_sec,log.dev_opened_time.tv_usec);
    snprintf(time2,sizeof(time2),"%ld.%06ld",log.avg.tv_sec,log.avg.tv_usec);
    printf("%-20s%-20s%-20ld%-20ld%-20hd%-20ld%-20s\n",pname,time1,log.event_generated.count,log.event_dropped.count,log.event_consumed[i].pid,log.event_consumed[i].counts.count,time2);
  }
   
}






int main(){
  int fd,i,ndevices;
  fd = open(dev_log,O_RDWR);
  if(fd<=0){
    printf("file descriptor %d\n", fd);
    printf("Oh dear, something went wrong with read()! %s\n", strerror(errno));
    return 0;
  }else{
    printf("device opened succesfully.");
    printf("fd = %d \n",fd);
  }
  ndevices=0;

  while(ndevices==0){
    if(ioctl(fd,LGETDEVS,minors)<0){
      printf("Oh dear, something went wrong with LGETDEVS! %s and %#hx\n", strerror(errno),LGETDEVS);
      return 0;
    }else{
      printf("successful for LGETDEVS (%#hx).\n",LGETDEVS);
      sleep(1);
    }
    while(minors[ndevices]!=END_MARK){
      printf("%d ",minors[ndevices++]);
    }
  }
  printf("\n");
  while(1){
    i = 0;
    printf("%-20s%-20s%-20s%-20s%-20s%-20s%-20s\n","DEV_NAME","OPENED_TIME","EV_GEN","EV_DROPPED","PIDS","EV_CON/PID","LT");
    while(minors[i]!=END_MARK){
       print_devs(fd,minors[i++]);
    }
    printf("--------------------------------------------------------------------\n");
    usleep(10000);
  }
  return 0;
}
