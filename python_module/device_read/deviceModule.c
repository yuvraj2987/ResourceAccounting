#include <Python.h>
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
/*open device*/
static PyObject * py_open_device(PyObject *self, PyObject *args)
{
  const char *dev_name;
  int fd;
 /* if (!PyArg_ParseTuple(args, "s", &dev_name))
            return NULL;
  */
  fd = open(dev_log,O_RDWR);
  if(fd < 0)
    return NULL;
  
  return Py_BuildValue("i", fd);
}

/*Get device*/
static PyObject * py_get_device(PyObject *self, PyObject *args)
{
  int fd;
  int ndevices = 0;
  if (!PyArg_ParseTuple(args, "i", &fd))
     return Py_BuildValue("i", -1);

  if(ioctl(fd,LGETDEVS,minors)<0)
    return Py_BuildValue("i", -1);
  
  while(minors[ndevices]!=END_MARK)
  {
    ndevices++;
  }

  return Py_BuildValue("i", ndevices);

}

static PyObject * py_set_device_info(PyObject *self, PyObject *args)
{
  int deviceId;
  int fd;
  if (!PyArg_ParseTuple(args, "ii", &fd, &deviceId))
    return Py_BuildValue("i", 0);;
  
  if(minors[deviceId]==END_MARK)
    return NULL;
  
  struct user_event_log log;
  struct user_args cargs;
  unsigned int i=0;
  cargs.minor = deviceId;
  cargs.p = &log;
  if(ioctl(fd,LGETDEVINFO,&cargs)<0)
    return NULL;

  for(i=0;i<log.ncount;i++)
  {
    char *pname;
    for(i=0;i<log.ncount;i++)
    {
      if(i==0)
      {
        pname = log.name;
      }
      else
      {
        //pname="No Process Name";
      }
      char time1[20],time2[20];
      snprintf(time1,sizeof(time1),"%ld.%06ld",log.dev_opened_time.tv_sec,log.dev_opened_time.tv_usec);
      snprintf(time2,sizeof(time2),"%ld.%06ld",log.avg.tv_sec,log.avg.tv_usec);
    }
  //set_device_info();
  //snprintf(time2,sizeof(time2),"%ld.%06ld",log.avg.tv_sec,log.avg.tv_usec);
  //130     printf("%-20s%-20s%-20ld%-20ld%-20hd%-20ld%-20s\n",pname,time1,log.event_generated.count,log.event_dropped.count,log.event_consumed[i].    pid,log.event_consumed[i].counts.count,time2);
  return Py_BuildValue("sii", pname, log.dev_opened_time.tv_sec, log.dev_opened_time.tv_usec, log.event_generated.count, log.event_dropped.count, log.event_consumed[i].pid[i].counts.count, log.avg.tv) ;
  }
}

/*
 *  * Bind Python function names to our C functions
 *   */
static PyMethodDef deviceModule_methods[] = {
    {"open", py_open_device, METH_VARARGS},
    {"get_device", py_get_device, METH_VARARGS},
    {"set_device_info", py_set_device_info, METH_VARARGS},
        {NULL, NULL}
};

/*
 *  * Python calls this to let us initialize our module
 *   */
PyMODINIT_FUNC initdeviceModule(void)
{
    (void) Py_InitModule("deviceModule", deviceModule_methods);
}
/*
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
  }
  ndevices=0;
  while(ndevices==0){
    if(ioctl(fd,LGETDEVS,minors)<0){
      printf("Oh dear, something went wrong with LGETDEVS! %s and %#hx\n", strerror(errno),LGETDEVS);
      return 0;
    }else{
      printf("successful for LGETDEVS.\n");
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
    sleep(15);
  }
  return 0;
}*/
