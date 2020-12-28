/*
 * @Author       : Kexiang Zhang
 * @Date         : 2020-09-23 14:57:46
 * @LastEditors  : Kexiang Zhang
 * @LastEditTime : 2020-12-28 10:28:17
 * @FilePath     : /fcgi-openwrt-mt7620/src/post_parse.c
 * @Description  : post参数解析
 */
#include "post_parse.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "get_parse.h"
#include "fcgi_stdio.h"
#include "utils.h"
#include "json-c/json.h"
#include "openssl/evp.h"
#include "file_page.h"
#include "gpio.h"
#include "blkid/blkid.h"
#include "multipart_parser.h"

/**
 * @description:随机数生成
 * @param {buf 目标buf}
 * @param {len 目标长度}
 * @return {none}
 */
void fill_random(uint8_t *buf, size_t len)
{
    int allCount = len / 4;
    int lessCount = len % 4;
    uint8_t *p = buf;
    uint32_t r;
    uint8_t *rr = (uint8_t *)&r;
    srand((unsigned int)time(0));
    while (allCount-- > 0)
    {
        r = rand() % UINT32_MAX;
        *(p++) = rr[0];
        *(p++) = rr[1];
        *(p++) = rr[2];
        *(p++) = rr[3];
    }
    while (lessCount-- > 0)
    {
        r = rand() % UINT32_MAX;
        *(p++) = rr[0];
    }
}

#define bufLen (1024 * 10) //分包长度
#define ROOT "/mnt"        //挂载根目录

typedef struct data_cmd
{
    char token[32];        //认证token
    char path[256];        //路径
    char filename[64];     //文件名
    char cnt[8];           //当前包数
    char remain[8];        //剩余大小
    char data[bufLen * 2]; //数据
    char end[8];           //结束标志
    char check[8];         //校验
    char length[8];        //长度
    char username[32];     //用户名
    char passwd[32];       //密码
    char devname[16];      //格式化设备名
    char devtype[8];       //格式化目标类型
} data_cmd;

typedef enum
{
    TOKEN,
    PATH,
    FILENAME,
    CNT,
    REMAIN,
    DATA,
    END,
    CHECK,
    LENGTH,
    USERNAME,
    PASSWD,
    DEVNAME,
    DEVTYPE
} data_type;

char globalToken[0x20]; //全局token存储
data_cmd g_data_cmd;
data_type g_data_type;

/**
 * @description:解析post参数类型
 * @param {type}
 * @return {type}
 */
int read_header_value(multipart_parser *p, const char *at, size_t length)
{
    char buf[length];
    strncpy(buf, at, length);
    buf[length] = '\0';

    if (!strcmp(buf, "form-data; name=\"token\""))
    {
        g_data_type = TOKEN;
    }
    else if (!strcmp(buf, "form-data; name=\"path\""))
    {
        g_data_type = PATH;
    }
    else if (!strcmp(buf, "form-data; name=\"filename\""))
    {
        g_data_type = FILENAME;
    }
    else if (!strcmp(buf, "form-data; name=\"cnt\""))
    {
        g_data_type = CNT;
    }
    else if (!strcmp(buf, "form-data; name=\"remain\""))
    {
        g_data_type = REMAIN;
    }
    else if (!strcmp(buf, "form-data; name=\"data\""))
    {
        g_data_type = DATA;
    }
    else if (!strcmp(buf, "form-data; name=\"end\""))
    {
        g_data_type = END;
    }
    else if (!strcmp(buf, "form-data; name=\"check\""))
    {
        g_data_type = CHECK;
    }
    else if (!strcmp(buf, "form-data; name=\"length\""))
    {
        g_data_type = LENGTH;
    }
    else if (!strcmp(buf, "form-data; name=\"username\""))
    {
        g_data_type = USERNAME;
    }
    else if (!strcmp(buf, "form-data; name=\"passwd\""))
    {
        g_data_type = PASSWD;
    }
    else if (!strcmp(buf, "form-data; name=\"dev\""))
    {
        g_data_type = DEVNAME;
    }
    else if (!strcmp(buf, "form-data; name=\"type\""))
    {
        g_data_type = DEVTYPE;
    }
    else
    {
        ret_json("500", "unknown cmd");
    }
    // LOG("%s\n", buf);
    return 0;
}

/**
 * @description:解析post参数数据
 * @param {type}
 * @return {type}
 */
int read_header_data(multipart_parser *p, const char *at, size_t length)
{
    switch (g_data_type)
    {
    case TOKEN:
    {
        strncpy(g_data_cmd.token, at, length);
        g_data_cmd.token[length] = '\0';
        // LOG("case token %s\n", g_data_cmd.token);
        break;
    }
    case PATH:
    {
        strncpy(g_data_cmd.path, at, length);
        g_data_cmd.path[length] = '\0';
        // LOG("%s\n", g_data_cmd.path);
        break;
    }
    case FILENAME:
    {
        strncpy(g_data_cmd.filename, at, length);
        g_data_cmd.filename[length] = '\0';
        // LOG("%s\n", g_data_cmd.filename);
        break;
    }
    case CNT:
    {
        strncpy(g_data_cmd.cnt, at, length);
        g_data_cmd.cnt[length] = '\0';
        // LOG("%s\n", g_data_cmd.cnt);
        break;
    }
    case REMAIN:
    {
        strncpy(g_data_cmd.remain, at, length);
        g_data_cmd.remain[length] = '\0';
        // LOG("%s\n", g_data_cmd.remain);
        break;
    }
    case DATA:
    {
        strncpy(g_data_cmd.data, at, length);
        g_data_cmd.data[length] = '\0';
        // LOG("%s\n", g_data_cmd.data);
        break;
    }
    case END:
    {
        strncpy(g_data_cmd.end, at, length);
        g_data_cmd.end[length] = '\0';
        // LOG("%s\n", g_data_cmd.end);
        break;
    }
    case CHECK:
    {
        strncpy(g_data_cmd.check, at, length);
        g_data_cmd.check[length] = '\0';
        // LOG("%s\n", g_data_cmd.check);
        break;
    }
    case LENGTH:
    {
        strncpy(g_data_cmd.length, at, length);
        g_data_cmd.length[length] = '\0';
        // LOG("%s\n", g_data_cmd.length);
        break;
    }
    case USERNAME:
    {
        strncpy(g_data_cmd.username, at, length);
        g_data_cmd.username[length] = '\0';
        break;
    }
    case PASSWD:
    {
        strncpy(g_data_cmd.passwd, at, length);
        g_data_cmd.passwd[length] = '\0';
        break;
    }
    case DEVNAME:
    {
        strncpy(g_data_cmd.devname, at, length);
        g_data_cmd.devname[length] = '\0';
        break;
    }
    case DEVTYPE:
    {
        strncpy(g_data_cmd.devtype, at, length);
        g_data_cmd.devtype[length] = '\0';
        break;
    }
    default:
    {
        LOG("unknown type\n");
        ret_json("500", "unknown type");
        break;
    }
    }
    return 0;
}

/**
 * @description:解析post传来的form数据
 * @param {type}
 * @return {type}
 */
int post_para(int cmd, int length)
{
    multipart_parser_settings callbacks;

    memset(&callbacks, 0, sizeof(multipart_parser_settings));
    callbacks.on_header_value = read_header_value;
    callbacks.on_part_data = read_header_data;

    LOG("cmd:%d\tlength:%d\n", cmd, length);

    // if (length < bufLen)
    //     nowReadLen = length;
    // else
    //     nowReadLen = bufLen;
    char postBuf[length];

    FCGI_fread(postBuf, sizeof(char), length, stdin);

    postBuf[length] = '\0';

    char *bufReadP;
    int boundaryLen = 0;
    bufReadP = strstr(postBuf, "\r\n");
    boundaryLen = strlen(postBuf) - strlen(bufReadP);
    char boundary[boundaryLen];
    strncpy(boundary, postBuf, boundaryLen);
    boundary[boundaryLen] = '\0';
    LOG("boundary:%d\n%s\n", boundaryLen, boundary);

    multipart_parser *parser = multipart_parser_init(boundary, &callbacks);
    multipart_parser_execute(parser, postBuf, length);
    LOG("multipart_parser_execute\n");
    multipart_parser_free(parser);

    switch (cmd)
    {
    case INVALID:
        ret_json("500", "无效的cmd");
        LOG("CMD INVALID\n");
        break;
    case LOGIN:
    {
        LOG("username:%s\n", g_data_cmd.username);
        LOG("passwd:%s\n", g_data_cmd.passwd);
        if (strcmp(g_data_cmd.username, "admin") || strcmp(g_data_cmd.passwd, "123456"))
        {
            ret_json("500", "错误的用户名或密码");
            break;
        }

        uint8_t randBuf[0x10];
        fill_random(randBuf, 0x10);
        byteToHex(globalToken, randBuf, 0x10);
        LOG("randDataBuf:%s\n", globalToken);

        //return
        FCGI_printf("Status:200 OK\r\n");
        FCGI_printf("Content-type: text/html;charset=utf-8\r\n"
                    "\r\n");

        json_object *j_cfg = json_object_new_object();
        json_object *j_code = json_object_new_string("200");
        json_object_object_add(j_cfg, "code", j_code);
        json_object *j_msg = json_object_new_string("ok");
        json_object_object_add(j_cfg, "message", j_msg);
        json_object *j_data = json_object_new_string(globalToken);
        json_object_object_add(j_cfg, "data", j_data);

        FCGI_printf("%s", json_object_to_json_string(j_cfg));
        break;
    }
    case FILE_LIST:
    {
        //verify token
        LOG("token:%s\ng_token:%s\n",
            g_data_cmd.token, globalToken);
        if (strncmp(g_data_cmd.token, globalToken, 0x20))
        {
            ret_json("500", "token无效");
            break;
        }
        //check mount
        if (dev_check())
        {
            ret_json("500", "设备未挂载");
            break;
        }
        LOG("path:%s\n", g_data_cmd.path);
        char rootPath[] = ROOT;
        strcat(rootPath, g_data_cmd.path);
        file_list_display(rootPath);
        break;
    }
    case FILE_DOWNLOAD:
    {
        //verify token
        LOG("token:%s\ng_token:%s\n",
            g_data_cmd.token, globalToken);
        if (strncmp(g_data_cmd.token, globalToken, 0x20))
        {
            ret_json("500", "token无效");
            break;
        }
        char rootPath[] = ROOT;
        strcat(rootPath, g_data_cmd.path);
        file_download(rootPath);
        break;
    }
    case FILE_UPLOAD:
    {
        //verify token
        LOG("token:%s\ng_token:%s\n",
            g_data_cmd.token, globalToken);
        if (strncmp(g_data_cmd.token, globalToken, 0x20))
        {
            ret_json("500", "token无效");
            break;
        }
        char path[256 + 64] = "/mnt";

        LOG("path:%s\nfilename:%s\ncnt:%s\nremain:%s\n",
            g_data_cmd.path, g_data_cmd.filename, g_data_cmd.cnt, g_data_cmd.remain);
        //path
        strcat(path, g_data_cmd.path);
        strcat(path, g_data_cmd.filename);
        LOG("path:%s\n", path);
        if (!file_upload_init(path,
                              tonumber(g_data_cmd.cnt, strlen(g_data_cmd.cnt)),
                              tonumber(g_data_cmd.remain, strlen(g_data_cmd.remain))))
            ret_json("200", "ok");
        break;
    }
    case FILE_TRANSPORT:
    {
        LOG("token:%s\ng_token:%s\n",
            g_data_cmd.token, globalToken);
        //verify token
        if (strncmp(g_data_cmd.token, globalToken, 0x20))
        {
            ret_json("500", "token无效");
            break;
        }
        int dataLength = tonumber(g_data_cmd.length, strlen(g_data_cmd.length));
        char dataBuf[dataLength];
        // strcpy(dataBuf, g_data_cmd.data);
        // dataBuf[bufLen] = '\0';

        LOG("cnt:%s\nlength:%s\ndata:%s\nend:%s\ncheck:%s\n",
            g_data_cmd.cnt, g_data_cmd.length, g_data_cmd.data, g_data_cmd.end, g_data_cmd.check);

        char writeBuf[bufLen * 2];
        strcpy(writeBuf, g_data_cmd.data);
        writeBuf[bufLen * 2] = '\0';
        for (size_t i = 0; i < dataLength; i++)
        {
            if (!checkIsHex(writeBuf[i * 2]) || !checkIsHex(writeBuf[i * 2 + 1]))
            {
                // ret_json("501", tostring(dataLength));
                break;
            }
            dataBuf[i] = hexToByte(&writeBuf[i * 2]);
        }

        if (!file_upload_data(tonumber(g_data_cmd.cnt, strlen(g_data_cmd.cnt)),
                              dataLength,
                              g_data_cmd.end,
                              dataBuf,
                              g_data_cmd.check))
            ret_json("200", "ok");
        break;
    }
    case FILE_CREATE_DIR:
    {
        //get path
        char *dataBuf;
        char *bufReadP;
        char path[32];
        int boundaryLen = 0;
        bufReadP = postBuf;

        dataBuf = strstr(postBuf, "name=");
        boundaryLen = strlen(postBuf) - strlen(dataBuf);
        LOG("boundaryLen:%d\n", boundaryLen);
        bufReadP += boundaryLen;
        sscanf(dataBuf, "name=\"%s%*s", path);
        if (strcmp(path, "path\""))
        {
            ret_json("500", "不符合规定的path");
            break;
        }
        bufReadP += strlen(path) + 7 + strlen("\r\n");

        dataBuf = bufReadP;
        sscanf(dataBuf, "%s\r\n%*s", path);
        LOG("path:%s\n", path);
        //get token
        char token[32];
        bufReadP += boundaryLen + strlen(path) + strlen("\r\n");

        sscanf(bufReadP, " name=\"%s\"", token);

        bufReadP += strlen(token) + 7 + strlen("\n");

        sscanf(bufReadP, "%s\r\n%*s", token);
        LOG("token:%s\n", token);
        LOG("globalToken:%s\n", globalToken);
        //verify token
        if (strcmp(token, globalToken))
        {
            ret_json("500", "token无效");
            break;
        }
        //check mount
        if (dev_check())
        {
            ret_json("500", "设备未挂载");
            break;
        }
        char rootPath[] = ROOT;
        strcat(rootPath, path);
        file_create_dir(rootPath);
        break;
    }
    case FILE_DELETE:
    {
        //get path
        char *dataBuf;
        char *bufReadP;
        char path[32];
        int boundaryLen = 0;
        bufReadP = postBuf;

        dataBuf = strstr(postBuf, "name=");
        boundaryLen = strlen(postBuf) - strlen(dataBuf);
        LOG("boundaryLen:%d\n", boundaryLen);
        bufReadP += boundaryLen;
        sscanf(dataBuf, "name=\"%s%*s", path);
        if (strcmp(path, "path\""))
        {
            ret_json("500", "不符合规定的path");
            break;
        }
        bufReadP += strlen(path) + 7 + strlen("\r\n");

        dataBuf = bufReadP;
        sscanf(dataBuf, "%s\r\n%*s", path);
        LOG("path:%s\n", path);
        //get token
        char token[32];
        bufReadP += boundaryLen + strlen(path) + strlen("\r\n");

        sscanf(bufReadP, " name=\"%s\"", token);

        bufReadP += strlen(token) + 7 + strlen("\n");

        sscanf(bufReadP, "%s\r\n%*s", token);
        LOG("token:%s\n", token);
        LOG("globalToken:%s\n", globalToken);
        //verify token
        if (strcmp(token, globalToken))
        {
            ret_json("500", "token无效");
            break;
        }
        //check mount
        if (dev_check())
        {
            ret_json("500", "设备未挂载");
            break;
        }
        char rootPath[] = ROOT;
        strcat(rootPath, path);
        file_delete(rootPath);
        break;
    }
    case SAMBA_CONFIG:
    {
        char *dataBuf;
        char *bufReadP;
        char userName[64];
        char passwd[64];
        int boundaryLen = 0;
        bufReadP = postBuf;
        //parse username
        dataBuf = strstr(postBuf, "name=");
        boundaryLen = strlen(postBuf) - strlen(dataBuf);
        LOG("boundaryLen:%d\n", boundaryLen);
        bufReadP += boundaryLen;
        sscanf(dataBuf, "name=\"%s%*s", userName);
        if (strcmp(userName, "username\""))
        {
            ret_json("500", "不符合规定的username");
            break;
        }
        bufReadP += strlen(userName) + 7 + strlen("\r\n");

        dataBuf = bufReadP;
        sscanf(dataBuf, "%s\r\n%*s", userName);
        LOG("username:%s\n", userName);

        //parse passwd
        bufReadP += boundaryLen + strlen(userName) + strlen("\r\n");

        sscanf(bufReadP, " name=\"%s\"", passwd);
        if (strcmp(passwd, "passwd\""))
        {
            ret_json("500", "不符合规定的passwd");
            break;
        }
        bufReadP += strlen(passwd) + 7 + strlen("\r\n");

        sscanf(bufReadP, "%s\r\n%*s", passwd);
        LOG("passwd:%s\n", passwd);

        //get token
        char token[32];
        bufReadP += boundaryLen + strlen(passwd) + 5 + strlen("\r\n");

        sscanf(bufReadP, " name=\"%s\"", token);

        bufReadP += strlen(token) + 7 + strlen("\n");

        sscanf(bufReadP, "%s\r\n%*s", token);
        LOG("token:%s\n", token);
        LOG("globalToken:%s\n", globalToken);
        // //verify token
        // if (strcmp(token, globalToken))
        // {
        //     ret_json("500", "token无效");
        //     break;
        // }
        //set samba username and passwd

        ret_json("200", "ok");

        break;
    }
    case DISK_TEST:
    {
        //get path
        char *dataBuf;
        char *bufReadP;
        char path[32];
        int boundaryLen = 0;
        bufReadP = postBuf;

        dataBuf = strstr(postBuf, "name=");
        boundaryLen = strlen(postBuf) - strlen(dataBuf);
        LOG("boundaryLen:%d\n", boundaryLen);
        bufReadP += boundaryLen;
        sscanf(dataBuf, "name=\"%s%*s", path);
        if (strcmp(path, "path\""))
        {
            ret_json("500", "不符合规定的path");
            break;
        }
        bufReadP += strlen(path) + 7 + strlen("\r\n");

        dataBuf = bufReadP;
        sscanf(dataBuf, "%s\r\n%*s", path);
        LOG("states:%d\n", atoi(path));

        gpio_export(1);
        gpio_direction(1, 1);
        gpio_write(1, atoi(path));
        LOG("out:%d\n", gpio_read(1));
        ret_json("200", "ok");
        break;
    }
    case DEV_LIST:
    {
        //verify token
        LOG("token:%s\ng_token:%s\n",
            g_data_cmd.token, globalToken);
        if (strncmp(g_data_cmd.token, globalToken, 0x20))
        {
            ret_json("500", "token无效");
            break;
        }

        //check mount
        char path[] = "/dev/sda";
        if (dev_check())
        {
            ret_json("500", "存储设备未连接");
            break;
        }
        json_object *j_code = json_object_new_string("200");
        json_object *j_msg = json_object_new_string("ok");

        json_object *j_array = json_object_new_array();

        for (int i = 1; i < 100; i++)
        {
            char pathBuf[11];

            strcpy(pathBuf, path);
            strcat(pathBuf, tostring(i));

            if (access(pathBuf, F_OK))
            {
                // ret_json("500", "磁盘分区不存在");
                continue;
            }
            json_object *j_data = json_object_new_string(pathBuf);
            // LOG("%s\n", pathBuf);
            json_object_array_add(j_array, j_data);
        }
        json_object *j_cfg = json_object_new_object();
        json_object_object_add(j_cfg, "code", j_code);
        json_object_object_add(j_cfg, "message", j_msg);
        json_object_object_add(j_cfg, "data", j_array);

        FCGI_printf("Status:200 OK\r\n");
        FCGI_printf("Content-type: application/json;charset=utf-8\r\n"
                    "\r\n"
                    "%s",
                    json_object_to_json_string(j_cfg));
        break;
    }
    case DISK_FORMAT:
    {
        //verify token
        LOG("token:%s\ng_token:%s\n",
            g_data_cmd.token, globalToken);
        if (strncmp(g_data_cmd.token, globalToken, 0x20))
        {
            ret_json("500", "token无效");
            break;
        }
        //check mount
        ret_json("200", "ok");
        disk_format(g_data_cmd.devname, g_data_cmd.devtype);
        break;
    }
    default:
        break;
    }
    return 0;
}
