#include <stdio.h>
#include "uart.h"

#define UART_DEV_PATH   "/dev/ttymxc1"  /**\brief< 定义打开的串口设备路径 */

unsigned char get_gzz[8] = {0x01,0x03,0x00,0x00,0x00,0x01,0x84,0x0A};


int main()
{
    int i = 0;
    int fd = uart_open(UART_DEV_PATH);
    if (fd < 0) {
        printf("open %s failed\n", UART_DEV_PATH);
        return 0;
    }
	int ret = uart_set(fd, 9600, 8, 'n', 1, 'n');
        
    char data[100] = {0};
    while(1)
    {
        puts("1");
        ret = write(fd, get_gzz, sizeof(get_gzz));
        printf("ret = %d\n",ret);
        ret = read(fd, data, sizeof(data));
        printf("ret2 = %d\n",ret);

        for(i= 0; i < ret; ++i)
            printf("%#x ",data[i]);

        sleep(2);
    }

    return 0;
}

