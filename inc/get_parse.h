#ifndef __GET_PARSE_H__
#define __GET_PARSE_H__

typedef enum CMD_PARAM
{
    INVALID,
    LOGIN,
    FILE_LIST,
    FILE_DOWNLOAD,
    FILE_UPLOAD,
    FILE_TRANSPORT,
    FILE_CREATE_DIR,
    FILE_DELETE,
    SAMBA_CONFIG,
    DISK_TEST,
    DEV_LIST,
    DISK_FORMAT
};

int get_para(char *str);

#endif
