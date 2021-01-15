/*
 * @Author       : Kexiang Zhang
 * @Date         : 2020-09-23 14:57:46
 * @LastEditors  : Kexiang Zhang
 * @LastEditTime : 2021-01-14 17:49:23
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
#include "sys/statfs.h"

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
 * @description:解析post传来的json数据
 * @param {type}
 * @return {type}
 */
int post_para(int cmd, int length)
{

    LOG("cmd:%d\tlength:%d\n", cmd, length);

    char postBuf[length];

    FCGI_fread(postBuf, sizeof(char), length, stdin);

    postBuf[length] = '\0';

    LOG("*******\n%s\n*******\n", postBuf);

    //字符串转json
    struct json_object *obj;
    obj = json_tokener_parse(postBuf);
    LOG("new_obj.to_string()=%s\n", json_object_to_json_string(obj));
    //解析json
    // json_parse(obj, )
    // {
    //     obj = json_object_object_get(obj, "username");
    //     LOG("new_obj.to_string()=%s\n", json_object_to_json_string(new_obj));
    // }

    // ret_json("200", "ok");
    // return 0;

    switch (cmd)
    {
    case INVALID:
        ret_json("500", "无效的cmd");
        LOG("CMD INVALID\n");
        break;
    case LOGIN:
    {
        struct json_object *login_obj;
        login_obj = json_object_object_get(obj, "username");
        LOG("username:%s\n", json_object_get_string(login_obj));
        if (strcmp(json_object_get_string(login_obj), "admin"))
        {
            ret_json("500", "错误的用户名或密码");
            break;
        }

        login_obj = json_object_object_get(obj, "passwd");
        LOG("passwd:%s\n", json_object_get_string(login_obj));
        if (strcmp(json_object_get_string(login_obj), "123456"))
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
        struct json_object *file_list_obj;

        //verify token
        file_list_obj = json_object_object_get(obj, "token");
        LOG("token:%s\n", json_object_get_string(file_list_obj));
        if (strncmp(json_object_get_string(file_list_obj), globalToken, 0x20))
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
        file_list_obj = json_object_object_get(obj, "path");
        LOG("path:%s\n", json_object_get_string(file_list_obj));
        char rootPath[] = ROOT;
        strcat(rootPath, json_object_get_string(file_list_obj));
        file_list_display(rootPath);
        break;
    }
    case FILE_CREATE_DIR:
    {
        //verify token
        struct json_object *create_dir_obj;
        create_dir_obj = json_object_object_get(obj, "token");
        LOG("token:%s\n", json_object_get_string(create_dir_obj));
        if (strcmp(json_object_get_string(create_dir_obj), globalToken))
        {
            LOG("globalToken:%s\n", globalToken);
            ret_json("500", "token无效");
            break;
        }

        //check mount
        if (dev_check())
        {
            ret_json("500", "设备未挂载");
            break;
        }

        //get path
        create_dir_obj = json_object_object_get(obj, "path");
        LOG("path:%s\n", json_object_get_string(create_dir_obj));

        char rootPath[] = ROOT;
        strcat(rootPath, json_object_get_string(create_dir_obj));
        file_create_dir(rootPath);
        break;
    }
    case FILE_DELETE:
    {
        //verify token
        struct json_object *delete_obj;
        delete_obj = json_object_object_get(obj, "token");
        LOG("token:%s\n", json_object_get_string(delete_obj));
        if (strcmp(json_object_get_string(delete_obj), globalToken))
        {
            LOG("globalToken:%s\n", globalToken);
            ret_json("500", "token无效");
            break;
        }

        //check mount
        if (dev_check())
        {
            ret_json("500", "设备未挂载");
            break;
        }

        //get path
        delete_obj = json_object_object_get(obj, "path");
        LOG("path:%s\n", json_object_get_string(delete_obj));

        char rootPath[] = ROOT;
        strcat(rootPath, json_object_get_string(delete_obj));
        file_delete(rootPath);
        break;
    }
    case VSFTP_CONFIG:
    {
        //verify token
        struct json_object *vsftp_cfg_obj;
        vsftp_cfg_obj = json_object_object_get(obj, "token");
        LOG("token:%s\n", json_object_get_string(vsftp_cfg_obj));
        if (strcmp(json_object_get_string(vsftp_cfg_obj), globalToken))
        {
            LOG("globalToken:%s\n", globalToken);
            ret_json("500", "token无效");
            break;
        }

        //parse username
        vsftp_cfg_obj = json_object_object_get(obj, "username");
        LOG("username:%s\n", json_object_get_string(vsftp_cfg_obj));

        //parse passwd
        vsftp_cfg_obj = json_object_object_get(obj, "passwd");
        LOG("passwd:%s\n", json_object_get_string(vsftp_cfg_obj));

        //set vsftp username and passwd

        ret_json("200", "ok");
        break;
    }
    case DISK_POWER:
    {
        //verify token
        struct json_object *vsftp_cfg_obj;
        vsftp_cfg_obj = json_object_object_get(obj, "token");
        LOG("token:%s\n", json_object_get_string(vsftp_cfg_obj));
        if (strcmp(json_object_get_string(vsftp_cfg_obj), globalToken))
        {
            LOG("globalToken:%s\n", globalToken);
            ret_json("500", "token无效");
            break;
        }

        //get path
        vsftp_cfg_obj = json_object_object_get(obj, "status");
        LOG("status:%d\n", json_object_get_int(vsftp_cfg_obj));

        gpio_export(1);
        gpio_direction(1, 1);
        gpio_write(1, json_object_get_int(vsftp_cfg_obj));
        LOG("out:%d\n", gpio_read(1));
        ret_json("200", "ok");
        break;
    }
    case DEV_LIST:
    {
        //verify token
        struct json_object *vsftp_cfg_obj;
        vsftp_cfg_obj = json_object_object_get(obj, "token");
        LOG("token:%s\n", json_object_get_string(vsftp_cfg_obj));
        if (strcmp(json_object_get_string(vsftp_cfg_obj), globalToken))
        {
            LOG("globalToken:%s\n", globalToken);
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
            char mnt_path[32] = "/mnt";

            strcpy(pathBuf, path);
            strcat(pathBuf, tostring(i));

            if (access(pathBuf, F_OK))
            {
                // ret_json("500", "磁盘分区不存在");
                continue;
            }
            LOG("mnt_path:%s\n", strrchr(pathBuf, '/'));
            strcat(mnt_path, strrchr(pathBuf, '/'));
            LOG("mnt_path:%s\n", mnt_path);
            struct statfs diskInfo;
            statfs(mnt_path, &diskInfo);
            unsigned long long totalBlocks = diskInfo.f_bsize;
            unsigned long long totalSize = totalBlocks * diskInfo.f_blocks;
            size_t mbTotalsize = totalSize >> 20;
            unsigned long long freeDisk = diskInfo.f_bfree * totalBlocks;
            size_t mbFreedisk = freeDisk >> 20;
            LOG("/  total=%dMB, free=%dMB\n", mbTotalsize, mbFreedisk);

            json_object *dev = json_object_new_string(pathBuf);
            json_object *total = json_object_new_int64(mbTotalsize);
            json_object *free = json_object_new_int64(mbFreedisk);

            json_object *j_devinfo = json_object_new_object();
            json_object_object_add(j_devinfo, "dev", dev);
            json_object_object_add(j_devinfo, "total", total);
            json_object_object_add(j_devinfo, "free", free);

            // LOG("%s\n", pathBuf);
            json_object_array_add(j_array, j_devinfo);
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
        struct json_object *vsftp_cfg_obj;
        vsftp_cfg_obj = json_object_object_get(obj, "token");
        LOG("token:%s\n", json_object_get_string(vsftp_cfg_obj));
        if (strcmp(json_object_get_string(vsftp_cfg_obj), globalToken))
        {
            LOG("globalToken:%s\n", globalToken);
            ret_json("500", "token无效");
            break;
        }
        //check mount
        ret_json("200", "ok");
        disk_format(g_data_cmd.devname, g_data_cmd.devtype);
        break;
    }
    case FTP_UPLOAD_PATH:
    { //verify token
        struct json_object *vsftp_cfg_obj;
        vsftp_cfg_obj = json_object_object_get(obj, "token");
        LOG("token:%s\n", json_object_get_string(vsftp_cfg_obj));
        if (strcmp(json_object_get_string(vsftp_cfg_obj), globalToken))
        {
            LOG("globalToken:%s\n", globalToken);
            ret_json("500", "token无效");
            break;
        }

        vsftp_cfg_obj = json_object_object_get(obj, "filename");
        LOG("filename:%d,%s\n", json_object_get_string_len(vsftp_cfg_obj), json_object_get_string(vsftp_cfg_obj));
        if (json_object_get_string_len(vsftp_cfg_obj) > 255)
        {
            ret_json("500", "文件名过长");
            break;
        }
        char name[255] = "/mnt/";
        strncat(name, json_object_get_string(vsftp_cfg_obj), json_object_get_string_len(vsftp_cfg_obj));

        // strncpy(name, json_object_get_string(vsftp_cfg_obj), json_object_get_string_len(vsftp_cfg_obj));

        vsftp_cfg_obj = json_object_object_get(obj, "path");
        LOG("path:%s\n", json_object_get_string(vsftp_cfg_obj));

        if (file_copy(name, json_object_get_string(vsftp_cfg_obj)))
            ret_json("500", "文件不存在");
        else
            ret_json("200", "ok");
        break;
    }
    default:
        break;
    }
    return 0;
}
