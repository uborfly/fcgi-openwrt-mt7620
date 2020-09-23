/*
 * @Author       : Kexiang Zhang
 * @Date         : 2020-09-23 14:58:48
 * @LastEditors  : Kexiang Zhang
 * @LastEditTime : 2020-09-23 14:58:52
 * @FilePath     : /fcgi-openwrt-mt7620/inc/gpio.h
 * @Description  : gpio头文件
 */
#ifndef __GPIO_H__
#define __GPIO_H__

int gpio_export(int pin);
int gpio_unexport(int pin);
int gpio_direction(int pin, int dir);
int gpio_write(int pin, int value);
int gpio_read(int pin);

#endif
