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
#include "mailbox.h"

#define ENTER_CRITICAL_AREA(mutex)  do{pthread_mutex_lock(mutex);}while(0)
#define QUIT_CRITICAL_AREA(mutex)  do{pthread_mutex_unlock(mutex);}while(0)

/* #define UART_DEV_PATH   "/dev/ttymxc2"  /**\brief< 定义打开的串口设备路径 *1/ */
/* #define UART_DEV_PATH1   "/dev/ttymxc1"  /**\brief< 定义打开的串口设备路径 *1/ */

unsigned pthread_index;


LIST_LINK *end_list = NULL;

//typedef struct thread_node
//{
//    pthread_t tid;         //线程id号
//    char name[256];        //线程名字 ,必须唯一
//    Que *amil_head, *mail_tail;
//    th_fun th;              //线程函数

//}TH_NODE;

/* typedef struct mail_box_system */
/* { */
/*     pthread_mutex_t mutex;  //保护邮件系统 */
/*     LIST_LINK *thread_list; */
/* }MBS; */
/* MBS* mbs; */

int send_msg(MBS*msb,char*recvname,struct datatype *data);
int recv_msg(MBS*msb,char*sendname,struct datatype *data);

struct datatype *malloc_data(void)
{
    struct datatype *data = (struct datatype *)malloc(sizeof(struct datatype));
    if(data == NULL)
    {
        printf("datatype malloc fail\n");
        exit(EXIT_FAILURE);
    }
    return data;
}

MBS* create_mail_box_system()
{
    MBS *temp =(MBS*)malloc(sizeof(MBS));

    if(NULL ==  temp)
    {
        perror("create_mail_box_system mutex malloc failure\n");
        return NULL;
    }
    int ret = pthread_mutex_init(&temp->mutex,NULL);
    if(0 != ret)
    {
        perror("create_mail_box_system mutex init failure\n");
        return NULL;
    }
    temp->thread_list = (LIST_LINK *)malloc(sizeof(LIST_LINK));
   // memset(temp->thread_list, 0, sizeof(LIST_LINK));
    temp->thread_list->next = NULL;
    printf("mail box create ok!! \n");
    return temp;
}

int destroy_mail_box_system(MBS*mbs)
{
    pthread_mutex_destroy(&mbs->mutex);
    LIST_LINK *temp = NULL;
    LIST_LINK *find = mbs->thread_list;
    while(find !=  NULL)
    {
        temp = find;
        find = find->next;
        free(temp);
    }
    free(find);
    return 0;
}

void* data_collect_th(void* arg)
{
    int fd = uart_open(UART_DEV_PATH);
    int fd1 = uart_open(UART_DEV_PATH1);
    if (fd1 < 0 || fd < 0) 
    {
        printf("uart_open failed\n");
        return 0;
    }

	int ret = uart_set(fd, 4800, 8, 'n', 1, 'n');
	int ret1 = uart_set(fd1, 4800, 8, 'n', 1, 'n');
    if(ret == -1 || ret1 == -1)
    {
        printf("uart_set fail\n");
        return 0;
    }
    
    while(1)
    {
        struct datatype *data = malloc_data();
        data->sun = recv_sunshine(fd);
        printf("sun = %d\n",data->sun);
        sleep(2);
        data->rain = recv_rain(fd);
        printf("rain = %.1f\n",data->rain);
        sleep(2);

        recv_solid(fd1, data);
        printf("w=%.1f t=%.1f d=%.1f p=%.1f\n", data->humi, data->temp, data->elec, data->ph);

        sleep(10);
        send_msg(mbs,"download", data);
        send_msg(mbs,"sock", data);
        
        free(data);
    }
	close(fd);
	close(fd1);
    return NULL;
}

void* download_th(void* arg)
{
    FILE *fp = fopen("data.csv", "r+");
    if(fp == NULL)
    {
        perror("Error opening csv file");
        exit(EXIT_FAILURE);
    }
    
    int rows = get_rows(fp);
    if(rows == 0)
    {
        fprintf(fp,"id,sun,ph,air_temp,air_humi,sol_humi,sol_temp,sol_elec\n");
    }

    while(1)
    {
        char sendname[256];
        struct datatype *data = malloc_data();
        
        recv_msg(mbs,sendname,data);
        
        write_csv(fp, data);

        free(data);
        sleep(1);
    }
    fclose(fp);
    return NULL;
}

void* sock_th(void* arg)
{
    mqtt_init();

    while(1)
    {
        struct datatype *data = malloc_data();
        char sendname[256];
        recv_msg(mbs,sendname,data);
        
        mqtt_send(data);
        
        free(data);
        sleep(1);
    }
    mqtt_deinit();
    return NULL;
}

char *get_th_name(MBS*msb)
{
    pthread_t tid = pthread_self();
    LIST_LINK *find = msb->thread_list;
    LIST_LINK *end = end_list;
    while(find != end)
    {
        if(find->elem.tid == tid)
            break;
        find = find->next;
    }
    if(find->elem.tid == tid)
    {
        //printf("cant find the recv th\n");
        return find->elem.name;
    }
    else
        return NULL;
}


int register_to_mail_system(MBS *mbs,char name[],th_fun th)
{
    LIST_LINK* temp =  (LIST_LINK *)malloc(sizeof(LIST_LINK));
    if(NULL == temp)
    {
        perror("register to mail malloc  \n");
        return -1;
    }
    strcpy(temp->elem.name ,name);
    temp->elem.th = th;
    temp->next = NULL;
    init_que(temp);

    pthread_t ret = pthread_create(&(temp->elem.tid),NULL,th,NULL);
    if(0!=ret)
    {
        perror("register to mail thread create\n");
        return -1;
    }

    list_add(mbs->thread_list, temp);
    printf("register mail system  |%s|  ok \n", temp->elem.name);

    return 0;
}

int unregister_from_mailbox(MBS*msb,char*name)
{
    LIST_LINK* find=msb->thread_list->next;
    LIST_LINK *temp = NULL;

    while(find !=  NULL)
    {
        temp = find;
        find = find->next;
        if(0 == strcmp(temp->elem.name ,name))
        {
            destroy(find);
            free(temp);
            return 0;
        }
    }

    if(0 == strcmp(find->elem.name ,name))
    {
        destroy(find);
        free(find);
        return 0;
    }
    return -1;
}

int wait_all_end(MBS*msb)
{
    LIST_LINK *find=msb->thread_list->next;
    LIST_LINK *end=end_list;
    while(find != end)
    {
        // pthread_join(find,NULL);

        pthread_join(find->elem.tid,NULL);
        find = find->next;
    }
    pthread_join(find->elem.tid,NULL);
    return 0;
}



int send_msg(MBS*msb, char*recvname, struct datatype *data)
{
    MAIL_DATA* temp =  (MAIL_DATA *)malloc(sizeof(MAIL_DATA));
   
    temp->data = malloc_data();
    memcpy(temp->data, data, sizeof(struct datatype));
    
    temp->id_of_sender = pthread_self();

    LIST_LINK *find = list_for_each(msb->thread_list, recvname);
    if (find == NULL)
    {
        printf("can,t find msg \n");
    }

    char* name = get_th_name(msb);
    strcpy(temp->name_of_sender,name);
    strcpy(temp->name_of_recver,recvname);
    ENTER_CRITICAL_AREA(&msb->mutex);
    in_queue(find, temp);
    QUIT_CRITICAL_AREA(&msb->mutex);
//    printf("send msg is ok |%s| msg is %s\n", temp->name_of_recver, temp->data);
    return 0;

}

int recv_msg(MBS*msb,char*sendname,struct datatype *data)
{
    MAIL_DATA* temp =  (MAIL_DATA *)malloc(sizeof(MAIL_DATA));
    pthread_t tid =  pthread_self();

    LIST_LINK *find = msb->thread_list;

    while(find != NULL)
    {
        if( find->elem.tid == tid)
            break;
        find = find->next;
    }

    if( find->elem.tid == tid)
    {
        while (1)
        {
            if(find->elem.mail_head != find->elem.mail_tail)
            {
                ENTER_CRITICAL_AREA(&msb->mutex);
                out_queue(find, temp);
                QUIT_CRITICAL_AREA(&msb->mutex);
                break;
            }
        }
    }

    strcpy(sendname, temp->name_of_sender);
    
    memcpy(data, temp->data, sizeof(struct datatype));

    free(temp);

    return 0;

}

/* int main() */
/* { */

/*     mbs = create_mail_box_system(); */

/*     register_to_mail_system(mbs,"download",download_th); */
/*     register_to_mail_system(mbs,"sock",sock_th); */
/*     register_to_mail_system(mbs,"data",data_collect_th); */


/*     wait_all_end(mbs); */
/*     destroy_mail_box_system(mbs); */


/* //    printf("Hello World!"); */
/*     return 0; */
/* } */

