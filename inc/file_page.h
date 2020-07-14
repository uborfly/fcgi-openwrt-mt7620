#ifndef __FILE_PAGE_H__
#define __FILE_PAGE_H__

#include <stdint.h>

int file_list_display(char *path);
void file_download(char *path);
void file_upload(char *path, char *fileName);
void file_create_dir(char *path);
void file_delete(char *path);

int dev_check();

#endif
