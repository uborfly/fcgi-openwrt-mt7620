/*
 * @Author       : Kexiang Zhang
 * @Date         : 2020-09-23 14:43:17
 * @LastEditors  : Kexiang Zhang
 * @LastEditTime : 2020-09-23 14:58:21
 * @FilePath     : /fcgi-openwrt-mt7620/src/gpio.c
 * @Description  : linux gpio初始化、读写操作
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "gpio.h"

#define BUFFER_MAX 100
#define DIRECTION_MAX 100
#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define POUT 21

/*
int main(int argc, char const *argv[])
{
        int i = 0;

    gpio_export(POUT);
    gpio_direction(POUT, OUT);

    for (i = 0; i < 20; i++) {
        gpio_write(POUT, i % 2);
        usleep(500 * 1000);
    }

    gpio_unexport(POUT);
    return 0;
}
*/

/**
 * @description:导入gpio
 * @param {pin io端口号}
 * @return {执行状态}
 */
int gpio_export(int pin)
{
    char buffer[BUFFER_MAX];
    int len;
    int fd;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0)
    {
        fprintf(stderr, "Failed to open export for writing!\n");
        return (-1);
    }

    len = snprintf(buffer, BUFFER_MAX, "%d", pin);
    if (write(fd, buffer, len) < 0)
    {
        fprintf(stderr, "Fail to export gpio!");
        return -1;
    }

    close(fd);
    return 0;
}

/**
 * @description:导出io
 * @param {pin io端口号}
 * @return {执行状态} 执行状态
 */
int gpio_unexport(int pin)
{
    char buffer[BUFFER_MAX];
    int len;
    int fd;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0)
    {
        fprintf(stderr, "Failed to open unexport for writing!\n");
        return -1;
    }

    len = snprintf(buffer, BUFFER_MAX, "%d", pin);
    if (write(fd, buffer, len) < 0)
    {
        fprintf(stderr, "Fail to unexport gpio!");
        return -1;
    }

    close(fd);
    return 0;
}

/**
 * @description:设置io方向
 * @param {pin io端口号}
 * @param {dir io方向}
 * @return {执行状态}
 */
int gpio_direction(int pin, int dir)
{
    static const char dir_str[] = "in\0out";
    char path[DIRECTION_MAX];
    int fd;

    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        fprintf(stderr, "failed to open gpio direction for writing!\n");
        return -1;
    }

    if (write(fd, &dir_str[dir == IN ? 0 : 3], dir == IN ? 2 : 3) < 0)
    {
        fprintf(stderr, "failed to set direction!\n");
        return -1;
    }

    close(fd);
    return 0;
}

/**
 * @description:写入io电平状态
 * @param {pin io端口号}
 * @param {value io电平状态}
 * @return {执行状态}
 */
int gpio_write(int pin, int value)
{
    static const char values_str[] = "01";
    char path[DIRECTION_MAX];
    int fd;

    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        fprintf(stderr, "failed to open gpio value for writing!\n");
        return -1;
    }

    if (write(fd, &values_str[value == LOW ? 0 : 1], 1) < 0)
    {
        fprintf(stderr, "failed to write value!\n");
        return -1;
    }

    close(fd);
    return 0;
}

/**
 * @description:读取io状态
 * @param {pin io端口号}
 * @return {读取的io状态}
 */
int gpio_read(int pin)
{
    char path[DIRECTION_MAX];
    char value_str[3];
    int fd;

    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "failed to open gpio value for reading!\n");
        return -1;
    }

    if (read(fd, value_str, 3) < 0)
    {
        fprintf(stderr, "failed to read value!\n");
        return -1;
    }

    close(fd);
    return (atoi(value_str));
}
