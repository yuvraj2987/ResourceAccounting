#include <jni.h>
#include "include/resource_usage.h"
#include "include/user_event_log.h"
#include "include/helper.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include <sys/limits.h>
#include <sys/poll.h>
#include <linux/input.h>
#include <errno.h>
#include <unistd.h>
#include <android/log.h>
int pid_system_server = -1;
const char* dev_log = "/dev/event_log_dev";
unsigned char minors[MAX_INPUT_DEVICE];
int fd = -1;

JNIEXPORT jintArray JNICALL Java_com_cse_buffalo_resourceusage_BackgroundThread_getMinors(JNIEnv *env, jclass thiz) {

	unsigned char lminors[MAX_INPUT_DEVICE];
	jcharArray result;
	int i;
	int len = 0;

	fd = open(dev_log,O_RDONLY);
	if(fd<=0){
		__android_log_print(ANDROID_LOG_DEBUG  , "RESOURCE_JNI","ERR fd=%d %s",fd,strerror(errno));
		return NULL;
	}
	if(ioctl(fd,LGETDEVS,lminors)<0){
		__android_log_print(ANDROID_LOG_DEBUG  , "RESOURCE_JNI","Oh dear, something went wrong with LGETDEVS!=%s ",strerror(errno));
		return NULL;
	}
	__android_log_print(ANDROID_LOG_DEBUG  , "RESOURCE_JNI","hello");
	while(lminors[len]!=END_MARK){
		len++;
	}
	if(len==0)
		return NULL;
	__android_log_print(ANDROID_LOG_DEBUG  , "RESOURCE_JNI","length=%d",len);

	result = (*env)->NewIntArray(env,len);

	if (result == NULL)
		return NULL;  //out of memory error thrown

	jint temp[MAX_INPUT_DEVICE];
	// fill a temp structure to use to populate the java int array
	for (i = 0; i < len; i++) {
		temp[i] = (int)lminors[i];
	}
	// move from the temp structure to the java structure
	(*env)->SetIntArrayRegion(env, result, 0, len, temp);
	return result;

}

JNIEXPORT jobject JNICALL Java_com_cse_buffalo_resourceusage_BackgroundThread_getReading(JNIEnv *env, jclass jc, jint minor_num){
	__android_log_print(ANDROID_LOG_DEBUG  , "RESOURCE_JNI","getReading");

	pid_system_server=proc_find("system_server");
	//__android_log_print(ANDROID_LOG_DEBUG  , "RESOURCE_JNI","pid_system_server=%d",pid_system_server);
	struct user_event_log log;
	struct user_args args;
	unsigned int i=0,j=0;
	char time1[20];
	char time2[20];
	char* pname;//
	char* temp;
	char prefix[100];
	int n=0;
	int len_pname = 0;
	args.minor = minor_num;
	args.p = &log;
	if(ioctl(fd,LGETDEVINFO,&args)<0)
	{
		__android_log_print(ANDROID_LOG_DEBUG  , "RESOURCE_JNI","Oh dear, something went wrong with LGETDEVS!=%s ",strerror(errno));
		exit(0);
	}
	//printf("\n");
	__android_log_print(ANDROID_LOG_DEBUG  , "RESOURCE_JNI","log name = %s", log.name);
	__android_log_print(ANDROID_LOG_DEBUG  , "RESOURCE_JNI","log count = %d", log.ncount);
	//for(i=0;i<log.ncount;i++)
	//{
		//if(i==0)
		//{
			n=sprintf(prefix,"//proc//%d//task//",pid_system_server);
			prefix[n]='\0';
			pname = get_name_from_pid(prefix,log.event_consumed[i].pid);
			len_pname = strlen(pname);
			__android_log_print(ANDROID_LOG_DEBUG  , "RESOURCE_JNI","pname =%s",pname);
			if(pname!=NULL){
				snprintf(time1,sizeof(time1),"%ld.%06ld",log.dev_opened_time.tv_sec,log.dev_opened_time.tv_usec);
				snprintf(time2,sizeof(time2),"%ld.%06ld",log.avg.tv_sec,log.avg.tv_usec);
				jclass complexClass = (*env)->FindClass(env,"com/cse/buffalo/resourceusage/OutputTableRow");
				jmethodID constructor = (*env)->GetMethodID(env,complexClass, "<init>", "(JJJLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V"); //The name of constructor method is "<init>"

				jstring name 	= (*env)->NewStringUTF( env,log.name );
				jstring jtime1 	= (*env)->NewStringUTF(env, time1);
				jlong ev_gen 	= log.event_generated.count;
				jlong ev_drp	= log.event_dropped.count;

				temp = malloc(len_pname*sizeof(char));
				bzero(temp,sizeof(temp));

				for(j=0;j<len_pname;j++){
						temp[j]=*pname;
						pname++;
					}
				temp[j] = '\0';
				jstring jpname 	= (*env)->NewStringUTF( env,temp );
				//__android_log_print(ANDROID_LOG_DEBUG  , "RESOURCE_JNI","temp=%s",temp);
				free(temp);
				jlong ev_cns 	= log.event_consumed[i].counts.count;
				jstring jtime2 	= (*env)->NewStringUTF( env,time2 );
				__android_log_print(ANDROID_LOG_DEBUG  , "RESOURCE_JNI", "%-20s%-20s%-20ld%-20ld%-20s%-20ld%-20s\n",log.name,time1,log.event_generated.count
					,log.event_dropped.count,pname,log.event_consumed[i].counts.count,time2);
				return (*env)->NewObject(env, complexClass, constructor,ev_gen,ev_drp,ev_cns,name,jtime1,jtime2,jpname);
			}
	//	}
	//}

	__android_log_print(ANDROID_LOG_DEBUG  , "RESOURCE_JNI","getReading done");
}
JNIEXPORT jstring JNICALL Java_com_cse_buffalo_resourceusage_BackgroundThread_getProcessName(JNIEnv *env, jclass jc, jint pid)
{

		char* pname = get_name_from_pid("//proc//",(int)pid);
		if(pname == NULL){
			return NULL;
		}
		//__android_log_print(ANDROID_LOG_DEBUG  , "RESOURCE_JNI","last func pname =%s",pname);

		int len_pname = strlen(pname);
		int j = 0;
		char* temp = malloc(len_pname*sizeof(char));
		bzero(temp,sizeof(temp));

		for(j=0;j<len_pname;j++){
				temp[j]=*pname;
				pname++;
			}
		temp[j] = '\0';
		jstring jpname 	= (*env)->NewStringUTF( env,temp );
		free(temp);
		return jpname;

}
