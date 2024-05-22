#include <stdio.h>
#include "mqtt.h"

static char topic[2][200] = {0};
static MQTTClient client;
static int id =10000;
volatile static MQTTClient_deliveryToken deliveredtoken;

void pack_topic(char * dev_name, char * pro_id)
{
	sprintf(topic[0], "$swun_dev2");	//订阅
	sprintf(topic[1], "$swun_dev2");			//发布
	/* sprintf(topic[0], "$swun_dev2/%s/%s/thing/property/post/reply", pro_id, dev_name);	//订阅 */
	/* sprintf(topic[1], "$swun_dev2/%s/%s/thing/property/post", pro_id, dev_name);			//发布 */
}

// 发送成功后callback
void delivered(void *context, MQTTClient_deliveryToken dt)
{
	printf("Message with token value %d delivery confirmed\n", dt);
	deliveredtoken = dt;
}
// 接收到消息的callback
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	int i;
	char* payloadptr;

	printf("Message arrived\n");
	printf("     topic: %s\n", topicName);
	printf("   message: ");

	payloadptr = (char*)message->payload;
	for(i=0; i<message->payloadlen; i++)
	{
		putchar(*payloadptr++);
	}
	putchar('\n');
	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
	return 1;
}
// 掉线后的callback
void connlost(void *context, char *cause)
{
	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);
}

int mqtt_init()
{
	pack_topic(DEV_NAME, PRODUCT_ID);
    int rc = MQTTClient_create(&client, NEW_ADDRESS, CLIENTID,
                               MQTTCLIENT_PERSISTENCE_NONE, NULL);
    int ch;
    if(MQTTCLIENT_SUCCESS != rc)
    {
        printf("create mqtt client failre...\n");
        exit(1);
    }
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username=PRODUCT_ID;
    conn_opts.password=PASSWD;
    rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    if(MQTTCLIENT_SUCCESS !=rc )
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

#if 0
    //订阅单个主题  如果需要订阅的话    
    MQTTClient_subscribe(client, topic[0], QOS);
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           , topic[0], CLIENTID, QOS);
#endif 

	return rc;
}

int mqtt_send(struct datatype *data)
{
	MQTTClient_deliveryToken deliveryToken;
    MQTTClient_message test2_pubmsg = MQTTClient_message_initializer;
    // 需要发送的正文
    char message[1024]={0};
    test2_pubmsg.qos = QOS;
    test2_pubmsg.retained = 0;
    test2_pubmsg.payload =message;
    int value = 0;

	/* sprintf(message,"{\"id\":\"%d\",\"version\":\"1.0\",\"params\":{\"%s\":{\"value\":%d}}}",id++, key, value); */
	sprintf(message,"{\"co2\":%d,\"light_intensity\":%d,\"ph\":%.1f,\"rainfall\":%.1f,\"temp\":%.1f,\"humi\":%.1f,\"soil_mois\":%.1f,\"soil_temp\":%.1f,\"elc\":%d}", 
                      value, data->sun, data->ph, data->rain, data->temp - 3, data->humi + 20, data->humi, data->temp, (int)data->elec);
    test2_pubmsg.payloadlen = strlen(message);

	printf("%s\n",message);
	int rc = MQTTClient_publishMessage(client,topic[1],&test2_pubmsg,&deliveryToken);
	if(MQTTCLIENT_SUCCESS != rc)
	{
		printf("client to publish failure.. %lu\n",pthread_self());
		exit(1);
	}
	printf("Waiting for up to %d seconds for publication on topic %s for client with ClientID: %s\n"
			,(int)(TIMEOUT/1000), topic[0], CLIENTID);
	MQTTClient_waitForCompletion(client,deliveryToken,TIMEOUT);
	sleep(1);

	return rc;
}

void mqtt_deinit()
{
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
}

