#include <pthread.h>

#define PORT 6666
#define NO_OF_MESSAGE_PARA 3
#define MAX_PROC 10
#define BUF_LIMIT (1<<8)


/**** Declarations *****/
static void print_devs(int fd,unsigned char minor);
char* get_name_from_pid(const char* prefix,int pid);
//void *server_thread_func (void *ptr);
//int process_message(char *message);//, const char *msg_arr[]);
/*
void record_pid(int sensor_id,int process_id);
void delete_pid(int sensor_id,int process_id);
int* get_pids(int deviceID);
*/
pid_t  proc_find(const char* name);

