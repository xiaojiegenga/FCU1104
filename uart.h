#ifndef _UART_H_
#define _UART_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* struct solid{ */
/*     float humi; */
/*     float temp; */
/*     float elec; */
/*     float ph; */
/* }; */

struct datatype{
    float humi;
    float temp;
    float elec;
    float ph;
    int sun;
    float rain;
};

extern unsigned char get_gz[8];
extern unsigned char get_yl[8];
extern unsigned char get_tr[8];

extern int uart_open(const char *p_path);
static void __print_termios(struct termios *p_termios);
extern int uart_set(int fd, int baudrate, int bits, char parity, int stop, char flow);
extern unsigned char sum(unsigned char * data, int len);
extern int get_sunshine(unsigned char *data);
extern float get_rain(unsigned char *data);
extern float *get_solid(unsigned char *data);
extern unsigned short crc16_modbus(unsigned char *data, unsigned int length); 
extern int check_crc(unsigned char *data, unsigned int len);
extern int recv_sunshine(int fd);
extern float recv_rain(int fd);
extern int recv_solid(int fd, struct datatype *tr);

#endif

