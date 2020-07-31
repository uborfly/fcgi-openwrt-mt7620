#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>
#include "fcgi_stdio.h"

#define __DEBUG
#ifdef __DEBUG
#define LOG(format, ...) FCGI_printf(format, ##__VA_ARGS__)
#else
#define LOG(format, ...)
#endif

void ret_json(char *code, char *message);

void log_buffer_hex(const void *buffer, uint16_t buff_len);

//data     要转换的字符串
//data_len 要转换的字符串长度
uint32_t tonumber(uint8_t *data, uint8_t data_len);
// uint8_t *tostring(uint32_t num, uint8_t len);

//str 目标字符串
//num 要转换的数字
//len 要转换为字符串的长度
char *tostring(long num);
unsigned char byteToHex(char *out, uint8_t *src, uint8_t len);

// unsigned char byteToHex(char *src, uint8_t len);
unsigned char hexToByte(unsigned char *s);
uint8_t checkIsHex(unsigned char x);

void urlencode(char url[]);
void urldecode(char url[]);
int unicode_to_utf8(unsigned long unic, unsigned char *pOutput, int outSize);
void strdel(char *str, char del);

#endif
