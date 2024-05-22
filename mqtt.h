#ifndef _MQTT_H_
#define _MQTT_H_


#include <MQTTAsync.h>
#include <MQTTClient.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <string.h>
#include "uart.h"
//#define OLD_ADDRESS     "tcp://218.201.45.7:1883"
//#define NEW_ADDRESS     "tcp://183.230.40.96:1883"
#define NEW_ADDRESS     "tcp://62.234.42.187:1883"

//#define DEV_NAME    "SHIXUN"
#define DEV_NAME    "dev2"
#define CLIENTID DEV_NAME
//#define PRODUCT_ID "3Q70dG718l"
#define PRODUCT_ID "admin"
//#define PASSWD "version=2018-10-31&res=products%2F3Q70dG718l%2Fdevices%2FSHIXUN&et=2837255523&method=md5&sign=ZCWQ%2BPFn9Sl6%2FDTEyubXzg%3D%3D";
#define PASSWD "123456";
#define QOS         0
#define TIMEOUT     10000L

/* extern struct solid; */
/* extern struct datatype; */


extern void pack_topic(char * dev_name, char * pro_id);
extern void delivered(void *context, MQTTClient_deliveryToken dt);
extern int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
extern void connlost(void *context, char *cause);
extern int mqtt_init();
extern int mqtt_send(struct datatype *data);
extern void mqtt_deinit();
//#define __cplusplus
#endif // _MQTT_H_
