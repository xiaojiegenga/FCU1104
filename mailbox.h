#ifndef _MAILBOX_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "list.h"
#include "uart.h"
#include "queue.h"
#include "mqtt.h"
#include "csv.h"
#include <errno.h>
#include <string.h>

#define UART_DEV_PATH   "/dev/ttymxc2"  /**\brief< 定义打开的串口设备路径 */
#define UART_DEV_PATH1   "/dev/ttymxc1"  /**\brief< 定义打开的串口设备路径 */

typedef struct mail_box_system
{
    pthread_mutex_t mutex;  //保护邮件系统
    LIST_LINK *thread_list;
}MBS;
MBS* mbs;

int send_msg(MBS*msb,char*recvname,struct datatype *data);
int recv_msg(MBS*msb,char*sendname,struct datatype *data);
struct datatype *malloc_data(void);
MBS* create_mail_box_system();
int destroy_mail_box_system(MBS*mbs);
void* data_collect_th(void* arg);
void* download_th(void* arg);
void* sock_th(void* arg);
char *get_th_name(MBS*msb);
int register_to_mail_system(MBS *mbs,char name[],th_fun th);
int unregister_from_mailbox(MBS*msb,char*name);
int wait_all_end(MBS*msb);
int send_msg(MBS*msb, char*recvname, struct datatype *data);
int recv_msg(MBS*msb,char*sendname,struct datatype *data);


#endif
#define _MAILBOX_H_
