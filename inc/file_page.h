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
