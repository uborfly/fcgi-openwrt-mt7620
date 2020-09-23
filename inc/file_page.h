/*
 * @Author       : Kexiang Zhang
 * @Date         : 2020-08-06 10:01:06
 * @LastEditors  : Kexiang Zhang
 * @LastEditTime : 2020-09-23 15:33:23
 * @FilePath     : /fcgi-openwrt-mt7620/inc/file_page.h
 * @Description  : 文件管理模块头文件
 */
#ifndef __FILE_PAGE_H__
#define __FILE_PAGE_H__

#include <stdint.h>

int file_list_display(char *path);
void file_download(char *path);

int file_upload_init(char *filePath, int cnt, int remain);
int file_upload_data(int fileCnt, int dataLength, char *end, char *data, char *check);

void file_create_dir(char *path);
void file_delete(char *path);
int disk_format(char *devName, char *devType);

int dev_check();

#endif
