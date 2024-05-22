/**
 * filename: uart.c
 * author: Suzkfly
 * date: 2021-01-16
 * platform: Ubuntu
 * 将USB转串口连接至Ubuntu中，运行程序，能打印串口接收到的数据，也能从终端中获取
 * 数据发送出去。
 * 如果不能成功打开设备，使用则ls /dev/ttyUSB0命令看设备是否存在，若存在，则使用
 * sudo chmod 666 /dev/ttyUSB0 修改文件权限后重新运行程序。
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "uart.h"

unsigned char get_gz[8] = {0x01,0x03,0x00,0x02,0x00,0x02,0x65,0xCB};
unsigned char get_yl[8] = {0x01,0x03,0x00,0x00,0x00,0x01,0x84,0x0A};
unsigned char get_tr[8] = {0x02,0x03,0x00,0x00,0x00,0x04,0x44,0x3A};

/**
 * \brief define
 * @{
 */
/* #define UART_DEV_PATH   "/dev/ttymxc2"  /**\brief< 定义打开的串口设备路径 *1/ */
/* #define UART_DEV_PATH1   "/dev/ttymxc1"  /**\brief< 定义打开的串口设备路径 *1/ */

/**
 * @}
 */

/**
 * \brief 打开串口设备
 *
 * \param[in] p_path：设备路径
 *
 * \retval 成功返回文件描述符，失败返回-1
 */
int uart_open(const char *p_path)
{
    /* O_NOCTTY：阻止操作系统将打开的文件指定为进程的控制终端,如果没有指定这个标
                 志，那么任何一个输入都将会影响用户的进程。 */
    /* O_NONBLOCK：使I/O变成非阻塞模式，调用read如果没有接收到数据则立马返回-1，
            并且设置errno为EAGAIN。*/
    /* O_NDELAY： 与O_NONBLOCK完全相同 */
    return open(p_path, O_RDWR | O_NOCTTY);
}

/**
 * \brief 测试函数，打印struct termios各成员值
 */
static void __print_termios(struct termios *p_termios)
{
    printf("c_iflag = %#08x\n", p_termios->c_iflag);
    printf("c_oflag = %#08x\n", p_termios->c_oflag);
    printf("c_cflag = %#08x\n", p_termios->c_cflag);
    printf("c_lflag = %#08x\n\n", p_termios->c_lflag);
}

/**
 * \brief 设置串口属性
 *
 * \param[in] fd：打开的串口设备的文件描述符
 * \param[in] baudrate：波特率
 *            #{0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800,
 *              2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}
 * \param[in] bits：数据位
 *            #{5, 6, 7, 8}
 * \param[in] parity：校验
 *            #'n'/'N'：无校验
 *            #'o'/'O'：奇校验
 *            #'e','E'：偶校验
 * \param[in] stop：停止位
 *            #1：1个停止位
 *            #2：2个停止位
 * \param[in] flow：流控
 *            #'n'/'N'：不使用流控
 *            #'h'/'H'：使用硬件流控
 *            #'s'/'S'：使用软件流控
 *
 * \retval 成功返回0，失败返回-1，并打印失败原因
 *
 * \note 虽然波特率设置支持这么多值，但并不代表输入表中支持的值波特率就
 *       一定能设置成功。
 */
int uart_set(int fd, int baudrate, int bits, char parity, int stop, char flow)
{
    struct termios termios_uart;
    int ret = 0;
    speed_t uart_speed = 0;

    /* 获取串口属性 */
    memset(&termios_uart, 0, sizeof(termios_uart));
    ret = tcgetattr(fd, &termios_uart);
    if (ret == -1) {
        printf("tcgetattr failed\n");
        return -1;
    }

    //__print_termios(&termios_uart);

    /* 设置波特率 */
    switch (baudrate) {
        case 0:      uart_speed = B0;      break;
        case 50:     uart_speed = B50;     break;
        case 75:     uart_speed = B75;     break;
        case 110:    uart_speed = B110;    break;
        case 134:    uart_speed = B134;    break;
        case 150:    uart_speed = B150;    break;
        case 200:    uart_speed = B200;    break;
        case 300:    uart_speed = B300;    break;
        case 600:    uart_speed = B600;    break;
        case 1200:   uart_speed = B1200;   break;
        case 1800:   uart_speed = B1800;   break;
        case 2400:   uart_speed = B2400;   break;
        case 4800:   uart_speed = B4800;   break;
        case 9600:   uart_speed = B9600;   break;
        case 19200:  uart_speed = B19200;  break;
        case 38400:  uart_speed = B38400;  break;
        case 57600:  uart_speed = B57600;  break;
        case 115200: uart_speed = B115200; break;
        case 230400: uart_speed = B230400; break;
        default: printf("Baud rate not supported\n"); return -1;
    }
    cfsetspeed(&termios_uart, uart_speed);

    /* 设置数据位 */
    switch (bits) {
        case 5:     /* 数据位5 */
            termios_uart.c_cflag &= ~CSIZE;
            termios_uart.c_cflag |= CS5;
        break;

        case 6:     /* 数据位6 */
            termios_uart.c_cflag &= ~CSIZE;
            termios_uart.c_cflag |= CS6;
        break;

        case 7:     /* 数据位7 */
            termios_uart.c_cflag &= ~CSIZE;
            termios_uart.c_cflag |= CS7;
        break;

        case 8:     /* 数据位8 */
            termios_uart.c_cflag &= ~CSIZE;
            termios_uart.c_cflag |= CS8;
        break;

        default:
            printf("Data bits not supported\n");
            return -1;
    }

    /* 设置校验位 */
    switch (parity) {
        case 'n':   /* 无校验 */
        case 'N':
            termios_uart.c_cflag &= ~PARENB;
            termios_uart.c_iflag &= ~INPCK;        /* 禁能输入奇偶校验 */
        break;

        case 'o':   /* 奇校验 */
        case 'O':
            termios_uart.c_cflag |= PARENB;
            termios_uart.c_cflag |= PARODD;
            termios_uart.c_iflag |= INPCK;        /* 使能输入奇偶校验 */
            termios_uart.c_iflag |= ISTRIP;        /* 除去第八个位（奇偶校验位） */
        break;

        case 'e':   /* 偶校验 */
        case 'E':
            termios_uart.c_cflag |= PARENB;
            termios_uart.c_cflag &= ~PARODD;
            termios_uart.c_iflag |= INPCK;        /* 使能输入奇偶校验 */
            termios_uart.c_iflag |= ISTRIP;        /* 除去第八个位（奇偶校验位） */
        break;

        default:
            printf("Parity not supported\n");
            return -1;
    }

    /* 设置停止位 */
    switch (stop) {
        case 1: termios_uart.c_cflag &= ~CSTOPB; break; /* 1个停止位 */
        case 2: termios_uart.c_cflag |= CSTOPB;  break; /* 2个停止位 */
        default: printf("Stop bits not supported\n");
    }

    /* 设置流控 */
    switch (flow) {
        case 'n':
        case 'N':   /* 无流控 */
            termios_uart.c_cflag &= ~CRTSCTS;
            termios_uart.c_iflag &= ~(IXON | IXOFF | IXANY);
        break;

        case 'h':
        case 'H':   /* 硬件流控 */
            termios_uart.c_cflag |= CRTSCTS;
            termios_uart.c_iflag &= ~(IXON | IXOFF | IXANY);
        break;

        case 's':
        case 'S':   /* 软件流控 */
            termios_uart.c_cflag &= ~CRTSCTS;
            termios_uart.c_iflag |= (IXON | IXOFF | IXANY);
        break;

        default:
            printf("Flow control parameter error\n");
            return -1;
    }

    /* 其他设置 */
    termios_uart.c_cflag |= CLOCAL;    /* 忽略modem（调制解调器）控制线 */
    termios_uart.c_cflag |= CREAD;    /* 使能接收 */

    /* 禁能执行定义（implementation-defined）输出处理，意思就是输出的某些特殊数
       据会作特殊处理，如果禁能的话那么就按原始数据输出 */
    termios_uart.c_oflag &= ~OPOST;

    /**
     *  设置本地模式位原始模式
     *  ICANON：规范输入模式，如果设置了那么退格等特殊字符会产生实际动作
     *  ECHO：则将输入字符回送到终端设备
     *  ECHOE：如果ICANON也设置了，那么收到ERASE字符后会从显示字符中擦除一个字符
     *         通俗点理解就是收到退格键后显示内容会往回删一个字符
     *  ISIG：使终端产生的信号起作用。（比如按ctrl+c可以使程序退出）
     */
    termios_uart.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    /**
     * 设置等待时间和最小接收字符
     * 这两个值只有在阻塞模式下有意义，也就是说open的时候不能传入O_NONBLOCK，
     * 如果经过了c_cc[VTIME]这么长时间，缓冲区内有数据，但是还没达到c_cc[VMIN]个
     * 数据，read也会返回。而如果当缓冲区内有了c_cc[VMIN]个数据时，无论等待时间
     * 是否到了c_cc[VTIME]，read都会返回，但返回值可能比c_cc[VMIN]还大。如果将
     * c_cc[VMIN]的值设置为0，那么当经过c_cc[VTIME]时间后read也会返回，返回值
     * 为0。如果将c_cc[VTIME]和c_cc[VMIN]都设置为0，那么程序运行的效果与设置
     * O_NONBLOCK类似，不同的是如果设置了O_NONBLOCK，那么在没有数据时read返回-1，
     * 而如果没有设置O_NONBLOCK，那么在没有数据时read返回的是0。
     */
    termios_uart.c_cc[VTIME] = 1;   /* 设置等待时间，单位1/10秒 */
    termios_uart.c_cc[VMIN]  = 1;    /* 最少读取一个字符 */

    tcflush(fd, TCIFLUSH);          /* 清空读缓冲区 */

    /* 写入配置 */
    ret = tcsetattr(fd, TCSANOW, &termios_uart);
    if (ret == -1) {
        printf("tcsetattr failed\n");
    }

    return ret;
}

unsigned char sum(unsigned char * data, int len)
{
	int i = 0;
	unsigned char sum = 0;
	for(i = 0; i < len; i++)
	{
		sum += data[i];	
	}

	return sum;
}
/*
 * 获得光照强度  传参为校验后的data字符数组
 * 单位为lux
 */
int get_sunshine(unsigned char *data)
{
    unsigned char high_byte = data[3];
    unsigned char low_byte = data[4];
    unsigned char high_byte1 = data[5];
    unsigned char low_byte1 = data[6];

    unsigned short high_value = (high_byte << 8) | low_byte;
    unsigned short low_value = (high_byte1 << 8) | low_byte1;

    unsigned int sunshine = (high_value << 16) | low_value;
    
    return sunshine;
}

/*
 * 获得土壤数据 传参为校验后的data字符数组
 * 返回值为float数组 0：土壤湿度 1：土壤温度　２：土壤电导率　３：土壤ＰＨ
 * 单位为               ％              ℃           uS/cm           ph
 */
float *get_solid(unsigned char *data)
{
    unsigned char high_humi = data[3];
    unsigned char low_humi = data[4];
    
    unsigned char high_temp = data[5];
    unsigned char low_temp = data[6];

    unsigned char high_elec = data[7];
    unsigned char low_elec = data[8];

    unsigned char high_ph = data[9];
    unsigned char low_ph = data[10];

    unsigned short humi = (high_humi << 8) | low_humi;
    unsigned short temp = (high_temp << 8) | low_temp;
    unsigned short elec = (high_elec << 8) | low_elec;
    unsigned short ph = (high_ph << 8) | low_ph;

    float *value = (float *)malloc(sizeof(float) * 4);
    value[0] = humi / 10.0;
    value[1] = temp / 10.0;
    value[2] = elec;
    value[3] = ph / 10.0;
    
    return value;
}

/*
 * 获得当前总雨量
 * 返回值为flaot 型　单位为mm
 */
float get_rain(unsigned char *data)
{
    unsigned char high_rain = data[3];
    unsigned char low_rain = data[4];

    unsigned short rain = (high_rain << 8) | low_rain;
    
    float value = rain / 10.0;

    return value;
}

/*
 * 计算ＣＲＣ校验和　冗余校验
 * 返回两位校验码
 */
unsigned short crc16_modbus(unsigned char *data, unsigned int length) 
{
    int i = 0, j = 0;
    unsigned short crc = 0xFFFF;  // 初始值
    for (i = 0; i < length; i++) 
    {
        crc ^= data[i];
        for (j = 0; j < 8; j++) 
        {
            if (crc & 0x0001) 
            {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}
/*
 * 把计算的校验码和原数据的校验码进行比较
 * 如果相同返回１
 */
int check_crc(unsigned char *data, unsigned int len)
{
    unsigned char high_crc = data[len - 1];
    unsigned char low_crc = data[len - 2];
    
    unsigned short crc = (high_crc << 8) | low_crc;
    
    return crc == crc16_modbus(data,len-2);
}

/*
 * 查询光照强度
 * 发送问询帧　处理回答帧　进行数据分析
 * 返回正常数据　如果校验不正确返回　-1
 */

int recv_sunshine(int fd)
{
    int read_num = 0;
    int ret = 0;
    unsigned char data[9] = {0};
    ret = write(fd, get_gz, sizeof(get_gz));
    read_num = read(fd, data, sizeof(data));
    
    if(check_crc(data,read_num))
    {
        int sunshine = get_sunshine(data);
        return sunshine;
    }else
    {
        return -1;
    }
}

/*
 * 查询降雨量
 * 发送问询帧　处理回答帧　进行数据分析
 * 返回正常数据　如果校验不正确返回　-1
 */

float recv_rain(int fd)
{
    int read_num = 0;
    int ret = 0;
    unsigned char data[9] = {0};
    ret = write(fd, get_yl, sizeof(get_yl));
    read_num = read(fd, data, sizeof(data));
    
    if(check_crc(data,read_num))
    {
        float rain = get_rain(data);
        return rain;
    }else
    {
        return -1;
    }
}

/*
 * 查询土壤参数
 * 发送问询帧　处理回答帧　进行数据分析
 * 返回正常数据（参数为一个结构体　具体成员查询.h文件）　
 * 如果校验不正确返回　-1
 */

int recv_solid(int fd, struct datatype *tr)
{
    int read_num = 0;
    int ret = 0;
    unsigned char data[13] = {0};
    ret = write(fd, get_tr, sizeof(get_tr));
    read_num = read(fd, data, sizeof(data));
    
    if(check_crc(data,read_num))
    {
        float *value = get_solid(data);

        tr->humi = value[0];
        tr->temp = value[1];
        tr->elec = value[2];
        tr->ph = value[3];

        return 0;
    }else
    {
        return -1;
    }
}

