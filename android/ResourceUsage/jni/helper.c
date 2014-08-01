#include <jni.h>
#include "include/resource_usage.h"
#include "include/user_event_log.h"
#include "include/helper.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
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
//#include <sys/types.h>


char* get_name_from_pid(const char* prefix,int pid){
	char file_path[100];
	char name[100];
	char *pres=NULL;
	int fd,len;
	int n = sprintf(file_path,"%s%d//comm",prefix,pid);
	file_path[n]='\0';
	//printf("\nfile path : %s\n",file_path);
	fd = open(file_path,O_RDONLY);
	if(fd<=0){
		//printf("Oh dear, something went wrong with read() on //proc//%d with error! %s\n", pid,strerror(errno));
	}else{
		len = read(fd,name,100);
		if(len){
			//printf("\nlength : %d\n",len);
			name[len-1]='\0';
			pres = malloc(len*sizeof(char));

			strcpy(pres,name);

		}

	}
	return pres;
}

pid_t proc_find(const char* name)
{
    DIR* dir;
    struct dirent* ent;
    char* endptr;
    char buf[512];

    if (!(dir = opendir("/proc"))) {
        perror("can't open /proc");
        return -1;
    }

    while((ent = readdir(dir)) != NULL) {
        /* if endptr is not a null character, the directory is not
         * entirely numeric, so ignore it */
        long lpid = strtol(ent->d_name, &endptr, 10);
        if (*endptr != '\0') {
            continue;
        }

        /* try to open the cmdline file */
        snprintf(buf, sizeof(buf), "/proc/%ld/cmdline", lpid);
        FILE* fp = fopen(buf, "r");

        if (fp) {
            if (fgets(buf, sizeof(buf), fp) != NULL) {
                /* check the first token in the file, the program name */
                char* first = strtok(buf, " ");
                if (!strcmp(first, name)) {
                    fclose(fp);
                    closedir(dir);
                    return (pid_t)lpid;
                }
            }
            fclose(fp);
        }
    }
    closedir(dir);
    return -1;
}
