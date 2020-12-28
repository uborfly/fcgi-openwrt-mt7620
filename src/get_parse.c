/*
 * @Author       : Kexiang Zhang
 * @Date         : 2020-09-22 11:54:12
 * @LastEditors  : Kexiang Zhang
 * @LastEditTime : 2020-12-28 14:20:12
 * @FilePath     : /fcgi-openwrt-mt7620/src/get_parse.c
 * @Description  : HTTP GET接口解析
 */
#include "fcgi_stdio.h"
#include <stdlib.h>
#include <string.h>
#include "get_parse.h"
#include "utils.h"

char tokenArray[100][50];
char getName[100][50];
char getParam[100][50];

/**
 * @description:解析各接口，返回接口类型
 * @param {type}
 * @return {type}
 */
int parse_para(int num)
{
    int ret = 0;
    for (int i = 0; i < num; i++)
    {
        LOG("tokenArray[%d] %s<br />", i, tokenArray[i]);
        const char s[2] = "=";
        char *token;

        token = strtok(tokenArray[i], s);
        strcpy(getName[i], token);
        LOG("getName:%s<br />", getName[i]);

        if (strcasecmp("cmd", getName[i]))
        {
            LOG("Err getName<br />");
            return 0;
        }

        token = strtok(NULL, s);
        strcpy(getParam[i], token);
        LOG("getParam:%s<br />", getParam[i]);

        if (!strcasecmp("login", getParam[i]))
        {
            ret = LOGIN;
        }
        else if (!strcasecmp("file_list", getParam[i]))
        {
            ret = FILE_LIST;
        }
        else if (!strcasecmp("file_download", getParam[i]))
        {
            ret = FILE_DOWNLOAD;
        }
        else if (!strcasecmp("file_upload", getParam[i]))
        {
            ret = FILE_UPLOAD;
        }
        else if (!strcasecmp("file_transport", getParam[i]))
        {
            ret = FILE_TRANSPORT;
        }
        else if (!strcasecmp("file_create_dir", getParam[i]))
        {
            ret = FILE_CREATE_DIR;
        }
        else if (!strcasecmp("file_delete", getParam[i]))
        {
            ret = FILE_DELETE;
        }
        else if (!strcasecmp("vsftp_config", getParam[i]))
        {
            ret = VSFTP_CONFIG;
        }
        else if (!strcasecmp("disk_power", getParam[i]))
        {
            ret = DISK_POWER;
        }
        else if (!strcasecmp("dev_list", getParam[i]))
        {
            ret = DEV_LIST;
        }
        else if (!strcasecmp("disk_format", getParam[i]))
        {
            ret = DISK_FORMAT;
        }
        else
        {
            ret = 0;
        }
    }
    return ret;
}

int get_para(char *str)
{
    const char s[2] = "&";
    char *token;

    // 获取第一个子字符串
    token = strtok(str, s);
    int i = 0;
    // 继续获取其他的子字符串
    while (token != NULL)
    {
        LOG("%s<br />", token);
        strcpy(tokenArray[i], token);
        token = strtok(NULL, s);
        i++;
    }
    LOG("i=%d<br />", i);
    if (1 == i)
        return parse_para(i);
    else
        return 0;
}
