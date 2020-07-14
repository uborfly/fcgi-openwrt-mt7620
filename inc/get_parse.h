#ifndef __GET_PARSE_H__
#define __GET_PARSE_H__

typedef enum CMD_PARAM
{
    INVALID,
    LOGIN,
    FILE_LIST,
    FILE_DOWNLOAD,
    FILE_UPLOAD,
    FILE_CREATE_DIR,
    FILE_DELETE,
    DISK_TEST
};

int get_para(char *str);

#endif
